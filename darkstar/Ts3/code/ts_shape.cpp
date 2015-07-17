//---------------------------------------------------------------------------
//	$Workfile:   ts_mesh.h  $
//	$Revision:   2.8  $
// $Version: $
//	$Date:   15 Sep 1995 10:27:12  $
//	$Log:   R:\darkstar\develop\ts\vcs\ts_mesh.h_v  $
//	
//---------------------------------------------------------------------------

#include "ts_Res.h"

#pragma option -Jg
#include <tio.h>

namespace TS
{
	static ResourceTypeTSShape _resdts(".dts");

   //---------------------------------------------------------------------------
   //---------------------------------------------------------------------------

	Shape::Mesh::~Mesh()
	{
	}

   //---------------------------------------------------------------------------
   // Sequence
   //---------------------------------------------------------------------------
   
   void 
   TS::Shape::Sequence::handleORP( RealF *position, int *count ) const
      {
      if( *position < 0.0f )
         {
         if( fCyclic )
            {
            *count = floor(*position);
            *position = *position - floor(*position);
            }
         else
            {
            *count = 0;
            *position = 0.0f;
            }
         }
      else if( *position > 1.0f )
         {
         if( fCyclic )
            {
            *count = floor(*position);
            *position = *position - floor(*position);
            }
         else
            {
            *count = 0;
            *position = 1.0f;
            }
         }
      else
         *count = 0;
      }

   //---------------------------------------------------------------------------

   int 
   Shape::Sequence::getIndex( Shape const & shape ) const
      {
      int index = this - shape.fSequences.address();
      AssertFatal( index >= 0 && index <= shape.fSequences.size(),
         "TS::Shape::Sequence::getIndex: invalid sequence" );
      return index;
      }


	//---------------------------------------------------------
	
	Shape::FrameTrigger::FrameTrigger( float pos, int val, Bool forward )
	{ 
		fPosition = pos; fValue = val; 
		if ( !forward ) 
			if ( fPosition )
				fPosition = -fPosition;
			else 
				fPosition = -0.001f;
	}


   //---------------------------------------------------------------------------
   // SubSequence
   //---------------------------------------------------------------------------

	Shape::Keyframe const & Shape::findKey( RealF pos, int firstKey, 
																int numKeys, int seq ) const
	{
      AssertFatal( pos >= 0.0f && pos <= 1.0f,
            "TS::Shape::findKey: position out of range" );
      int prevKey = firstKey - 1;
		
      for( int ki = firstKey; ki < firstKey + numKeys; ki++ )
      {
         Keyframe const &key = getKeyframe( ki );
			
         if ( key.getPosition() <= pos )
            prevKey = ki;
         else
            break;
      }
		
      if ( getSequence( seq ).fCyclic )
		{
         if ( prevKey < firstKey )
            prevKey = firstKey + numKeys - 1;
		}
		else if ( prevKey < firstKey )
         prevKey = firstKey;
		
      return getKeyframe( prevKey );
	}																			

   Shape::Keyframe const & 
   Shape::SubSequence::findCelKey( Shape const &shape, RealF pos ) const
   {
		return shape.findKey( pos, fFirstKeyframe, fnKeyframes, fSequenceIndex );
   }

   //---------------------------------------------------------------------------

   void 
   Shape::SubSequence::findTransformKeys( Shape const &shape, RealF pos, 
      int *prevKey, int *nextKey, RealF *interpParam, Bool *visible ) const
      {
      AssertFatal( pos >= 0.0f && pos <= 1.0f,
            "TS::Shape::SubSequence::findTransformKeys: position out of range" );

      *prevKey = fFirstKeyframe - 1;
      *nextKey = fFirstKeyframe + fnKeyframes;
      // start out with a reasonable guess
      int checkIndex = fFirstKeyframe + pos * ((float)fnKeyframes-0.01f);
      while (*prevKey+1<*nextKey)
      {
         const Keyframe & key = shape.getKeyframe(checkIndex);
         float checkPos = key.getPosition();
         if (checkPos <= pos + 0.001f)
            *prevKey = checkIndex;
         if (checkPos >= pos - 0.001f)
            *nextKey = checkIndex;
         checkIndex = (*prevKey+*nextKey) >> 1;
      }

      AssertFatal(shape.getKeyframe(*prevKey).getPosition()-0.001f <= pos &&
                  shape.getKeyframe(*nextKey).getPosition()+0.001f >= pos,
                  "Shape::findTransformKeys:  keys out of order?");

      // now we have key pair; time to evaluate interpolation parameter:
      if( shape.getSequence( fSequenceIndex ).fCyclic )
         {
         // handle wrapping for cyclic sequences:
         RealF denom;
         if( *prevKey < fFirstKeyframe )
            {
            // before first keyframe, use last keyframe as prev:
            *prevKey = fFirstKeyframe + fnKeyframes - 1;
            denom = (shape.getKeyframe(*nextKey).getPosition() - 
                     shape.getKeyframe(*prevKey).getPosition() + 1.0f);
            }
         else if( *nextKey == fFirstKeyframe + fnKeyframes )
            {
            // after last keyframe, use first keyframe as next:
            *nextKey = fFirstKeyframe;
            denom = (shape.getKeyframe(*nextKey).getPosition() + 1.0f - 
                     shape.getKeyframe(*prevKey).getPosition());
            }
         else
            {
            denom = (shape.getKeyframe(*nextKey).getPosition() - 
                     shape.getKeyframe(*prevKey).getPosition());
            }
         if( *prevKey == *nextKey )
            *interpParam = 0.0f;
         else if( IsZero( denom ) )
            {
            // keys are at the same time or at start & end of sequence.
            // doesn't really matter unless it's a cyclic sequence with
            // different values at start and end (such as a walking animation).
            if( IsEqual( pos, shape.getKeyframe(*prevKey).getPosition() ) )
               *interpParam = 0.0f;
            else
               *interpParam = 1.0f;
            }
         else
            *interpParam = (pos - shape.getKeyframe(*prevKey).getPosition())/denom;
         }
      else
         {
         // non-cyclic so no wrapping:
         if( *prevKey < fFirstKeyframe )
            {
            // before first keyframe, so at start of sequence:
            *prevKey = fFirstKeyframe;
            *interpParam = 0.0f;
            }
         else if( *nextKey == fFirstKeyframe + fnKeyframes )
            {
            // after last keyframe, so at end of sequence:
            *nextKey = fFirstKeyframe + fnKeyframes - 1;
            *interpParam = 1.0f;
            }
         else if( *prevKey == *nextKey )
            *interpParam = 0.0f;
         else
            {
            RealF denom = (shape.getKeyframe(*nextKey).getPosition() - 
                           shape.getKeyframe(*prevKey).getPosition());
            if( !denom )
               *interpParam = 0.0f;
            else
               *interpParam = (pos - shape.getKeyframe(*prevKey).getPosition())/denom;
            }
         }
         // visibility is based on previous key:
         if( visible )
            *visible = (shape.getKeyframe( *prevKey ).getVisibility() != 0);
      }

   //---------------------------------------------------------------------------

   void 
   Shape::SubSequence::interpolateTransform( Shape const &shape, int prevKey, 
      int nextKey, RealF interpParam, Transform *pDest ) const
      {
      Transform const & tPrev = shape.getTransform( 
         shape.getKeyframe(prevKey).getIndex() );
      Transform const & tNext = shape.getTransform( 
         shape.getKeyframe(nextKey).getIndex() );
      pDest->interpolate( tPrev, tNext, interpParam );
      }

	void 
   Shape::SubSequence::interpolateTransform( Shape const &shape, int prevKey, 
      int nextKey, RealF interpParam, TMat3F *pDest ) const
      {
      Transform const & tPrev = shape.getTransform( 
         shape.getKeyframe(prevKey).getIndex() );
      Transform const & tNext = shape.getTransform( 
         shape.getKeyframe(nextKey).getIndex() );
      Transform::interpolate( tPrev, tNext, interpParam, pDest );
      }

   //---------------------------------------------------------------------------
   // Node
   //---------------------------------------------------------------------------
   
   int 
   Shape::Node::getIndex( Shape const & shape ) const
      {
      int index = this - shape.fNodes.address();
      AssertFatal( index >= 0 && index <= shape.fNodes.size(),
         "TS::ShapeInstance::Node::getIndex: invalid node" );
      return index;
      }

   //---------------------------------------------------------------------------

  Shape::SubSequence const * 
  Shape::Node::findSubSequence( Shape const & shape, int seqIndex ) const
      {
      if (!fnSubSequences)
         return NULL;
         
      // get a good guess to start
      int checkIndex = fFirstSubSequence; 
      if (seqIndex<fnSubSequences)
         checkIndex += seqIndex;
      else
         checkIndex += fnSubSequences >> 1;
      int a = fFirstSubSequence;
      int b = a + fnSubSequences;
      
      while (a+1<b)
      {
         const SubSequence & ns = shape.getSubSequence(checkIndex);
         int test = ns.fSequenceIndex-seqIndex;
         if (test>0)
            b=checkIndex;
         else if (test<0)
            a=checkIndex;
         else
            return &ns;
         checkIndex = (a+b) >> 1;
      }
      const SubSequence & ns = shape.getSubSequence(checkIndex);
      if (ns.fSequenceIndex == seqIndex)
         return &ns;

      return 0;
      }

   //---------------------------------------------------------------------------
   // Object
   //---------------------------------------------------------------------------
   
   int 
   Shape::Object::getIndex( Shape const & shape ) const
      {
      int index = this - shape.fObjects.address();
      AssertFatal( index >= 0 && index <= shape.fObjects.size(),
         "TS::ShapeInstance::Object::getIndex: invalid object" );
      return index;
      }

   //---------------------------------------------------------------------------

   Shape::SubSequence const * 
   Shape::Object::findSubSequence( Shape const & shape, int seqIndex ) const
      {
      if (!fnSubSequences)
         return NULL;
         
      // get a good guess to start
      int checkIndex = fFirstSubSequence; 
      if (seqIndex<fnSubSequences)
         checkIndex += seqIndex;
      else
         checkIndex += fnSubSequences >> 1;
      int a = fFirstSubSequence;
      int b = a + fnSubSequences;
      
      while (a+1<b)
      {
         const SubSequence & ns = shape.getSubSequence(checkIndex);
         int test = ns.fSequenceIndex-seqIndex;
         if (test>0)
            b=checkIndex;
         else if (test<0)
            a=checkIndex;
         else
            return &ns;
         checkIndex = (a+b) >> 1;
      }
      const SubSequence & ns = shape.getSubSequence(checkIndex);
      if (ns.fSequenceIndex == seqIndex)
         return &ns;

      return 0;
      }

   //---------------------------------------------------------------------------
   // Shape
   //---------------------------------------------------------------------------

   
   Shape::Shape()
      {
      fpMaterials = 0;
      }

   
   Shape::~Shape()
      {
		for (int i = 0; i < fMeshes.size(); i++)
			delete fMeshes[i];
      delete fpMaterials;
      }

   int          
   Shape::lookupName( const char *name ) const
      {
      for( int index = 0; index < fNames.size(); index++ )
         if( !stricmp( name, fNames[index] ) )
            return index;   
      return -1;
      }

   const char* 
   Shape::getName(int index)
   {
      AssertFatal(index >= 0 && index < fNames.size(),
                  "Error, out of range name index");
      return fNames[index];
   }

	int 
	Shape::findDetail( char *name) const
	{
		int nodeNum = findNode(name);
		if (nodeNum==-1)
			return -1;
		for (int i=0; i <fDetails.size();i++)
			if (fDetails[i].fRootNodeIndex==nodeNum)
				return i;
		return -1;
	}

   int 
   Shape::findNode( char *name ) const
      {
      int nameIndex = lookupName( name );
      for( int ni = 0; ni < fNodes.size(); ni++ )
         if( fNodes[ni].fName == nameIndex )
            return ni;
      return -1;
      }

   int 
   Shape::selectDetail( RealF projectedSize ) const
   {
      // if there are no details, use single implicit detail:
      if( fDetails.size() <= 1 )
         return 0;
      // if object gets too close, it can give negative projected size:
      projectedSize = fabs( projectedSize );
      for( int d = 0; d < fDetails.size(); d++ )
         if( projectedSize >= fDetails[d].fSize )
			{
				if (IsEqual(fDetails[d].fSize,0.0f))
               // went beyond the actual detail levels, which means the
               // object is smaller than the last detail level
               return -1;
				else
					return d;
			}
      return -1; 
   }

	void 
	Shape::addSubSequence(int at,const SubSequence & newSS, Int16 & SScount)
	// SScount is the # of SS's of the node or object that owns the new SS
	// addSubSequence will update the count -- because it's important to do it after
	// the add and not before...
	{
		fSubSequences.insert(at);
		fSubSequences[at]=newSS;

		// now, anyone who points to a subsequence at or beyond 'at' must be ++
		// the && !=0 is important below:  w/o it the node/object that owns the new
		// subsequence would no longer point to this subseq if it was the node/object's
		// first subsequence
      int i;
		for (i=0;i<fNodes.size();i++)
			if ( (fNodes[i].fFirstSubSequence>=at) && (fNodes[i].fnSubSequences!=0) )
				fNodes[i].fFirstSubSequence++;
		for (i=0;i<fObjects.size();i++)
			if ( (fObjects[i].fFirstSubSequence>=at) && (fObjects[i].fnSubSequences!=0) )
				fObjects[i].fFirstSubSequence++;
		for (i=0;i<fSequences.size();i++)
			if( (fSequences[i].fFirstIFLSubSequence>=at) && (fSequences[i].fNumIFLSubSequences!=0) )
				fSequences[i].fFirstIFLSubSequence++;
		SScount++;
	}	

	void 
	Shape::importNodeKeyframe(const Shape *fromShape, int fromNum)
	{
		fKeyframes.push_back(Keyframe( 
										fromShape->fKeyframes[fromNum].getPosition(),
										fTransforms.size(),
										0,
										false,
										true
									));
		fTransforms.push_back(fromShape->fTransforms[fromShape->fKeyframes[fromNum].getIndex()]);
	}

	void 
	Shape::importObjectKeyframe(const Shape *fromShape, int fromNum, int meshNum)
	{
		const Mesh & fromMesh = *(fromShape->fMeshes[meshNum]);
		Mesh & toMesh = const_cast<Mesh &>(*fMeshes[meshNum]);

		// copy mesh frame over, ret value is new mesh frame index in toMesh
		int toMeshFrameIndex = toMesh.importFrame(fromMesh,fromShape->fKeyframes[fromNum].getIndex());

		int toMeshMatIndex = 0; // for now

		// copy keyframe over to shape
		fKeyframes.push_back(Keyframe( 
										fromShape->fKeyframes[fromNum].getPosition(),
										toMeshFrameIndex,
										toMeshMatIndex,
										fromShape->fKeyframes[fromNum].usesVisibilityTrack() != 0,
										fromShape->fKeyframes[fromNum].getVisibility() != 0,
										fromShape->fKeyframes[fromNum].usesMaterialTrack() != 0,
										fromShape->fKeyframes[fromNum].usesFrameTrack() != 0
									));
	}

	bool 
	Shape::importSequence( const Shape *impShape, int impSeqNum)
	{
      int   this_nNodes = fNodes.size();
      int   this_nObjects = fObjects.size();
      int   this_nDetails = fDetails.size();
      int   this_nMeshes = fMeshes.size();

      int   that_nNodes = impShape->fNodes.size();
      int   that_nObjects = impShape->fObjects.size();
      int   that_nDetails = impShape->fDetails.size();
      int   that_nMeshes = impShape->fMeshes.size();
      int   i, j;
      int   *map_nodes, *map_objects;

      // Build list of names for each object.  Just useful in the
      //    debugger.  I'm also using autoclass arrays, instead of newing
      //    them, for debug display purposes..  
#ifdef DEBUG
#define  MAX_FNAMES  400
      const char     *this_names[MAX_FNAMES];        // debug..
      const char     *that_names[MAX_FNAMES];
      int            this_nNames = fNames.size();
      int            that_nNames = impShape->fNames.size();

      if ( this_nNames <= MAX_FNAMES && that_nNames <= MAX_FNAMES )
      {
         for ( i = 0; i < fNames.size(); i++ )
            this_names [ i ] = fNames [ i ];
         for ( i = 0; i < impShape->fNames.size(); i++ )
            that_names [ i ] = impShape->fNames [ i ];
      }
#endif


      // Do rough test to make sure these are identical (close enough) shapes
		// if not successful return false
		if (impSeqNum < 0   || impSeqNum >= fSequences.size() ||
            this_nNodes != that_nNodes  ||  this_nObjects != that_nObjects  ||
            this_nDetails != that_nDetails  || this_nMeshes != that_nMeshes )
      {
			return false;
      }


      // Build mapping tables and make sure all nodes and objects
      //    correspond.  
      {
         map_nodes = new int [ that_nNodes ];
         map_objects = new int [ that_nObjects ];

         memset ( map_nodes, 0xff, that_nNodes * sizeof(int) );
         memset ( map_objects, 0xff, that_nObjects * sizeof(int) );

         //  Build a table which tells us how to find the nodes in this 
         //       shape which correspond to the nodes we are trying to import.  
         //       Then the same for objects. 
         for ( i = 0; i < that_nNodes; i++ )
         {
            const char * impName = impShape->fNames[impShape->fNodes[i].fName];
            for ( j = 0; j < this_nNodes; j++ )
            {
               if (! stricmp ( impName, fNames[ fNodes[j].fName ] ) )
               {
                  map_nodes [ i ] = j;
                  break;
               }
            }

            if ( j == this_nNodes )
            {
               // FIXME:  make this an alert and return false.  Same below.
               char     msg[200];
               sprintf ( msg, "The following Node from input shape does not"
                        " occur in target shape: %s", impName );
               AssertFatal ( 0, msg );
                              
            }
            // AssertFatal ( j != fNodes.size(), "Import shape has nodes"
              //                "which are not present in the target sequence" );
         }

         for ( i = 0; i < that_nObjects; i++ )
         {
            const char * impName = impShape->fNames[impShape->fObjects[i].fName];
            for ( j = 0; j < this_nObjects; j++ )
            {
               if (! stricmp ( impName, fNames[ fObjects[j].fName ] ) )
               {
                  map_objects [ i ] = j;
                  break;
               }
            }

            if ( j == this_nObjects )
            {
               char     msg[200];
               sprintf ( msg, "This Object: %s (from import shape) is not"
                        " present in target shape.", impName );
               AssertFatal ( 0, msg );
            }
         }
      }

		// Copy basic sequence data over
		const Sequence & impSeq = impShape->fSequences[impSeqNum];
		Sequence newSeq;
		newSeq.fCyclic = impSeq.fCyclic;
		newSeq.fDuration = impSeq.fDuration;
		newSeq.fPriority = impSeq.fPriority;
		fNames.push_back( impShape->fNames[impSeq.fName] );
		newSeq.fName = fNames.size()-1;
		newSeq.fFirstFrameTrigger = fFrameTriggers.size();
		newSeq.fNumFrameTriggers = impSeq.fNumFrameTriggers;
		newSeq.fNumIFLSubSequences = 0; //impSeq.fNumIFLSubSequences;
		newSeq.fFirstIFLSubSequence = 0; //fSubSequences.size();
		fSequences.push_back(newSeq);
		int newSeqNum = fSequences.size()-1;

		// use the larger of two shapes radius's
		if (impShape->fRadius > fRadius)
			fRadius=impShape->fRadius;
         
      // Adjust bounds box:  
      fBounds.fMin.setMin( impShape->fBounds.fMin );
      fBounds.fMax.setMax( impShape->fBounds.fMax );

		// put the meshes from each shape on the same scale
		for (i=0;i<fMeshes.size();i++)
		{
			Shape::Mesh * pImpMesh = const_cast<Shape::Mesh *>(impShape->fMeshes[i]);
			fMeshes[i]->setCommonScale(*pImpMesh);
		}

		// import node subsequences and transforms
      VectorPtr<SubSequence const *>   saveFound;
      VectorPtr<Node *>                saveNodes;
		for ( i=0; i < that_nNodes; i++ )
		{
			// import and target nodes
			const Node & impNode = impShape->fNodes[i];
			Node & targetNode = const_cast<Node &>(fNodes[map_nodes[i]]);

			// get subSequence from import shape
			SubSequence const * impSubSeq = impNode.findSubSequence(*impShape,impSeqNum);

			// was there a subsequence on this node for impSeqNum?
			if (impSubSeq)
			{
            // see if it has already been put in.  Except for case where targetNode() doesn't
            //    have any subsequences already.  
            bool  already = false;
            int   foundIndex = -1;
            for( int s = 0; s < saveFound.size(); s++ )
               if( saveFound[s] == impSubSeq )
               {
                  foundIndex = s;
                  already = true;
                  break;
               }

            if( ! already )
            {
               // Keeps track of when a sequence already exists for a corresponding node. 
               saveFound.push_back( impSubSeq );
               saveNodes.push_back( &targetNode );
               
   				// add new subSequence to target shape
   				SubSequence newSS;
   				newSS.fSequenceIndex=newSeqNum;
   				newSS.fnKeyframes=impSubSeq->fnKeyframes;
   				newSS.fFirstKeyframe=fKeyframes.size();
               // new subseq must be put in this nodes part of subsequence 
               // list.  addSubSequence() updates other indices.  Note
               //    that last parameter is reference, which is where 
               //       our target node gets modified
   				addSubSequence(targetNode.fFirstSubSequence+ 
   										targetNode.fnSubSequences, 
   									newSS,								
   									targetNode.fnSubSequences);

   				// copy key-frames over
   				for (int j=0;j<impSubSeq->fnKeyframes;j++)
   					// this will copy the transform too
   					importNodeKeyframe(impShape,j+impSubSeq->fFirstKeyframe);
            }
            else{
               // update our target node's sequence list.  If the target node 
               //    is getting it's first SubSequence, then we need to set the
               //    index.  
               ++targetNode.fnSubSequences;
               Node  * saved = saveNodes[ foundIndex ];
               AssertFatal( targetNode.fnSubSequences == saved->fnSubSequences, 
                        "ts_shape:importSequence():  Sequence fixups" );
               if( targetNode.fnSubSequences == 1 )
                  targetNode.fFirstSubSequence = saved->fFirstSubSequence;
            }
			}
		}

		// import object subsequences and meshes
		for ( i=0; i < that_nObjects; i++ )
		{
			const Object & impObj = impShape->fObjects[i];
			Object & targetObj = const_cast<Object &>(fObjects[map_objects[i]]);

			// get the subsequence for impSeqNum
			const SubSequence * impSubSeq = impObj.findSubSequence(*impShape,impSeqNum);

			// is there really a subseq on this object for this sequence?
			if (impSubSeq)
			{
				// add new subSequence to target shape
				SubSequence newSS;
				newSS.fSequenceIndex = newSeqNum;
				newSS.fnKeyframes = impSubSeq->fnKeyframes;
				newSS.fFirstKeyframe = fKeyframes.size();
				addSubSequence(targetObj.fFirstSubSequence+  // new subseq must be put in this
										targetObj.fnSubSequences,  // nodes part of subsequence list...
									newSS,								// indexes will have to be updated
									targetObj.fnSubSequences);

				// copy key-frames over
				for (int j=0;j<impSubSeq->fnKeyframes;j++)
					// this will copy the mesh frames too
					importObjectKeyframe(impShape,
										 		j+impSubSeq->fFirstKeyframe,
										 		impObj.fMeshIndex);
			}
		}
		
		for ( i = 0; i < impShape->fFrameTriggers.size(); i++ )
		{
			const FrameTrigger &impTrig = impShape->fFrameTriggers[i];
			fFrameTriggers.push_back( impTrig );
		}


      delete map_nodes;
      delete map_objects;
	
		return true;
	}

   //---------------------------------------------------------------------------

	int Shape::getSequenceIndex( const char *sequence ) const
	{
	   int seqNameIndex = lookupName( sequence );
	   for( int si = 0; si < fSequences.size(); si++ )
	      if( fSequences[si].fName == seqNameIndex )
	      {
	         return si;
	      }
	   return -1;
	}

   //---------------------------------------------------------------------------


#if 0	
	bool Shape::getBoundsBox( int sequence, Box3F *box ) const
	{
		Vector<Object>::const_iterator obj;
		const SubSequence *subseq;
		Box3F tempbox;
		int first = TRUE;
		
		for ( obj = fObjects.begin(); obj != fObjects.end(); obj++ )
		{
			subseq = obj->findSubSequence( *this, sequence );

			if ( subseq )
			{
				const Keyframe &key = getKeyframe( subseq->fFirstKeyframe );
				const Mesh *mesh = getMesh( obj->fMeshIndex );
				int index = key.getIndex();
				
				if ( mesh )
				{
					mesh->getBox( index, tempbox );
					
					if ( first )
					{
						*box = tempbox;
						first = FALSE;
					}
					else
					{
						if ( tempbox.fMin.x < box->fMin.x )
							box->fMin.x = tempbox.fMin.x;
						if ( tempbox.fMin.y < box->fMin.y )
							box->fMin.y = tempbox.fMin.y;
						if ( tempbox.fMin.z < box->fMin.z )
							box->fMin.z = tempbox.fMin.z;
					
						if ( tempbox.fMax.x > box->fMax.x )
							box->fMax.x = tempbox.fMax.x;
						if ( tempbox.fMax.y > box->fMax.y )
							box->fMax.y = tempbox.fMax.y;
						if ( tempbox.fMax.z > box->fMax.z )
							box->fMax.z = tempbox.fMax.z;
					}
				}
			}
		}
		
		return !first;
	}
#endif	
	
   //---------------------------------------------------------------------------

   Vector<const char*> Shape::fUtilityDetails;

   void Shape::markUtilityDetails()
   {
      // go through detail levels and make sure utility details have fSize = 0
      // and come after all rendering detail levels.  Utility details are those
      // that have size 0 to start with or have one of the utilit detail prefixes
      // in fUtilityDetails. Note: utility detail levels are those that are used
      // for purposes other than rendering (like the collision detail) and
      // are not meant to be chosen by the selectDetail method.
      
      if (fUtilityDetails.empty())
         return;
         
      int firstUtility = -1;
      for ( int dl = 0; dl < fDetails.size(); dl++)
      {
         // get detail name
         const char * detailName = fNames[fNodes[fDetails[dl].fRootNodeIndex].fName];

         // is this a utility detail?
         bool util = IsEqual(fDetails[dl].fSize,0.0f);
         if (!util)
            // does this detail start with one of the utility prefixes
            for (int u = 0; u < fUtilityDetails.size(); u++)
            {
               const char * s1 = detailName, * s2 = fUtilityDetails[u];
               while (*s2 && *s1++==*s2++);
               util = (!*s2);
               if (util)
                  break;
            }            

         //
         if (util)
         {
            // this is a utility detail
            fDetails[dl].fSize = 0.0f;
            if (firstUtility<0)
               firstUtility=dl;
         }
         else if (firstUtility>=0)
         {
            // eek, we have a non-utility detail after the first utility detail
            // move it to just before the first utility
            fDetails.insert(firstUtility);
            fDetails[firstUtility]=fDetails[dl+1];
            fDetails.erase(dl+1);
            firstUtility++;
         }         
      }
   }
   
   IMPLEMENT_PERSISTENT(Shape);

   int 
   Shape::version()
      {
      return 8;
      }

   #pragma argsused

		// a copy of the keyframe type used in Version 2 of the shapes so that
		// we can convert to current version
      class V2Keyframe
         {
         RealF       fPosition;   
         // keyValue is:
         //  index into shapes transform array for Node SubSequences
         //  index into meshes frame array for Cel SubSequences
         //  high bit indicates visibility for Node SubSequences
         UInt32      fKeyValue;
      public:
         V2Keyframe( RealF position, int index, Bool visible = FALSE, 
         			Bool invalid = FALSE )
            {
            fPosition = position;
            fKeyValue = index;
            if( visible )
               fKeyValue |= 0x80000000;
				if( invalid )
               fKeyValue |= 0x40000000;
            }
         int         getVisibility() const { return fKeyValue & 0x80000000; }
         int         isValid() const { return !(fKeyValue & 0x40000000); }
         int         getIndex() const { return fKeyValue & 0x3fffffff; }
         RealF       getPosition() const { return fPosition; }
         };


		class V3Sequence
		{
      public:
         Int32       fName;
         Int32       fCyclic;
         RealF       fDuration;
         Int32       fPriority;
		};

		class V4Sequence
		{
      public:
         Int32       fName;
         Int32       fCyclic;
         RealF       fDuration;
         Int32       fPriority;
			
			Int32 		fFirstFrameTrigger;
			Int32 		fNumFrameTriggers;
		};

		class V6Transition
		{
      public:
         Int32    	fStartSequence;   // index into shapes sequence array
         Int32    	fEndSequence;     // index into shapes sequence array
         RealF    	fStartPosition;
         RealF    	fEndPosition;
			RealF			fDuration;
			V6Transform	fTransform;
		};

   //-----------------------------v7 temps-------------------------------------
      // Version 7 structures for the upgrade to 8.  Some 32 bit values we reduced
      //    to Int16s, so the CheckBounds() method makes sure it's OK.  Signed values
      //    were retained to avoid hard-to-track-down problems (which often happens)
      //    and since our largest shapes have around 6000 xforms / keyframes.  
      class V7Node
      {
         Int32       fName;
         Int32       fParent;
         Int32       fnSubSequences;  
         Int32       fFirstSubSequence;
         Int32       fDefaultTransform;
      public:
         void CheckBounds(const char * msg) const 
         {
            AssertFatal( fName<32768 && fName>=-32768, msg );
            AssertFatal( fParent<32768 && fParent>=-32768, msg );
            AssertFatal( fnSubSequences<32768 && fnSubSequences>=-32768, msg );
            AssertFatal( fFirstSubSequence<32768 && fFirstSubSequence>=-32768, msg );
            AssertFatal( fDefaultTransform<32768 && fDefaultTransform>=-32768, msg );
         }
         void AssignToNew( Shape::Node & giveTo ) const 
         {
            giveTo.fName = fName;
            giveTo.fParent = fParent;
            giveTo.fnSubSequences = fnSubSequences;
            giveTo.fFirstSubSequence = fFirstSubSequence;
            giveTo.fDefaultTransform = fDefaultTransform;
         }
      };

		class V7SubSequence
		{
         Int32       fSequenceIndex;
         Int32       fnKeyframes;
         Int32       fFirstKeyframe;
      public:
         void CheckBounds(const char * msg) const 
         {
            AssertFatal( fSequenceIndex<32768 && fSequenceIndex>=-32768, msg );
            AssertFatal( fnKeyframes<32768 && fnKeyframes>=-32768, msg );
            AssertFatal( fFirstKeyframe<32768 && fFirstKeyframe>=-32768, msg );
         }
         void AssignToNew( Shape::SubSequence & giveTo ) const 
         {
            giveTo.fSequenceIndex = fSequenceIndex;
            giveTo.fnKeyframes = fnKeyframes;
            giveTo.fFirstKeyframe = fFirstKeyframe;
         }
		};
      
      class V7Keyframe
      {
         RealF       fPosition;
         UInt32      fKeyValue;
         UInt32      fMatIndex;
      public:
         bool  usesVisibilityTrack() const { return (fMatIndex & 0x40000000)!=0; }
         bool  getVisibility() const { return (fMatIndex & 0x80000000)!=0; }
         bool  usesMaterialTrack() const { return (fMatIndex & 0x20000000)!=0; }
         bool  usesFrameTrack() const { return (fMatIndex & 0x10000000)!=0; }
         int   getIndex() const { return fKeyValue; }
         int   getMatIndex() const { return fMatIndex & 0x0fffffff; }
         RealF getPosition() const { return fPosition; }
         void CheckBounds(const char * msg) const 
         {
            AssertFatal( getIndex() < 65535, msg );
            AssertFatal( getMatIndex() < 4096, msg );
         }
      };
      
      class V7Object
      {
         Int16       fName;
         Int16       fFlags;
         Int32       fMeshIndex; // index in mesh list of the object
         Int32       fNodeIndex; // index of node to attach to
         TMat3F      fObjectOffset; // offset relative to node
         Int32       fnSubSequences;
         Int32       fFirstSubSequence;
      public:
         void CheckBounds(const char * msg) const 
         {
            // AssertFatal( fMeshIndex<32768 && fMeshIndex>=-32768, msg );
            AssertFatal( fNodeIndex<32768 && fNodeIndex>=-32768, msg );
            AssertFatal( fnSubSequences<32768 && fnSubSequences>=-32768, msg );
            AssertFatal( fFirstSubSequence<32768 && fFirstSubSequence>=-32768, msg );
         }
         void AssignToNew( Shape::Object & giveTo ) const 
         {
            giveTo.fName = fName;
            giveTo.fFlags = fFlags;
            giveTo.fMeshIndex = fMeshIndex;
            giveTo.fNodeIndex = fNodeIndex;
            giveTo.fObjectOffset = fObjectOffset.p;
            giveTo.fnSubSequences = fnSubSequences;
            giveTo.fFirstSubSequence = fFirstSubSequence;
         }
      };
      
      class V7Transition
      {
         Int32          fStartSequence;   // index into shapes sequence array
         Int32          fEndSequence;     // index into shapes sequence array
         RealF          fStartPosition;
         RealF          fEndPosition;
         RealF          fDuration;
         V7Transform    fTransform;
      public:
         void AssignToNew( Shape::Transition & giveTo ) const 
         {
            giveTo.fStartSequence = fStartSequence;
            giveTo.fEndSequence = fEndSequence;
            giveTo.fStartPosition = fStartPosition;
            giveTo.fEndPosition = fEndPosition;
            giveTo.fDuration = fDuration;
            fTransform.AssignToNew( giveTo.fTransform );
         }
      };
      
      
   //-----------------------------end v7 temps---------------------------------

   Persistent::Base::Error 
   Shape::read( StreamIO &sio, int version, int user )
      {
		user;

      Persistent::Base::Error err = Persistent::Base::Ok;

      Int32 nNodes, nSequences, nSubSequences, nKeyframes, nTransforms, 
         nNames, nObjects, nDetails, nMeshes;
		Int32 nTransitions = 0, nFrameTriggers = 0;
		
		Vector<V2Keyframe> oldStyleKeyframes;
		Vector<V3Sequence> oldSequencesV3;
		Vector<V4Sequence> oldSequencesV4;
		Vector<V6Transform> oldTransformsV6;
		Vector<V6Transition> oldTransitionsV6;
      Vector<V7Node>          oldNodesV7;
      Vector<V7Object>        oldObjectsV7;
      Vector<V7Keyframe>      oldKeyframesV7;
      Vector<V7Transform>     oldTransformsV7;
      Vector<V7SubSequence>   oldSubSequencesV7;
      Vector<V7Transition>    oldTransitionsV7;

      sio.read(&nNodes);
      sio.read(&nSequences);
      sio.read(&nSubSequences);
      sio.read(&nKeyframes);
      sio.read(&nTransforms);
      sio.read(&nNames);
      sio.read(&nObjects);
      sio.read(&nDetails);
      sio.read(&nMeshes);
      if ( version >= 2 )
			sio.read( &nTransitions );
		if ( version >= 4 )
			sio.read( &nFrameTriggers );

      sio.read(&fRadius);
      fCenter.read( sio );
      bool needsBounds = false;
      if( version > 7 )
      {
         fBounds.fMin.read(sio);
         fBounds.fMax.read(sio);
      }
      else
      {
         needsBounds = true;
         Box3F  box( fCenter, fCenter );
         box.fMin -= fRadius;
         box.fMax += fRadius;
         fBounds = box;
         // we'll shrink it later...
      }

//fSequences.reserve(100);
//fNodes.reserve(100);
//fSubSequences.reserve(100);
//fKeyframes.reserve(500);
//fNames.reserve(500);
//fDetails.reserve(10);
//fTransitions.reserve(100);
//fObjects.reserve(100);
      #ifdef TOOL_VERSION
      AssertFatal( 0, "TS_SHAPE: code out of date (v8 upgrade below)" );
      readVector( sio, nNodes,         &fNodes );
		
		if ( version >= 5 )
	      readVector( sio, nSequences,  &fSequences );
		else if ( version >= 4 )
	      readVector( sio, nSequences,  &oldSequencesV4 );
		else
			readVector( sio, nSequences,  &oldSequencesV3 );			
		
      readVector( sio, nSubSequences,  &fSubSequences );
		if (version >= 3)
	      readVector( sio, nKeyframes,     &fKeyframes );
		else
			readVector( sio, nKeyframes,		&oldStyleKeyFrames;
		if (version < 7)
	      readVector( sio, nTransforms,    &oldTransformsV6 );
		else
	      readVector( sio, nTransforms,    &fTransforms );
      readVector( sio, nNames,         &fNames );
      readVector( sio, nObjects,       &fObjects );
      readVector( sio, nDetails,       &fDetails );
      if ( version >= 2 )
		{
			if ( version < 7 )
	         readVector( sio, nTransitions, &oldTransitionsV6 );
			else
	         readVector( sio, nTransitions, &fTransitions );
		}
		if ( version >= 4 )
			readVector( sio, nFrameTriggers, &fFrameTriggers );
      #else
      if( version <= 7 )
         lockVector( sio, nNodes,         &oldNodesV7 );
      else
         lockVector( sio, nNodes,         &fNodes );
		
		if ( version >= 5 )
	      lockVector( sio, nSequences,  &fSequences );
		else if ( version >= 4 )
	      lockVector( sio, nSequences,  &oldSequencesV4 );
		else
			lockVector( sio, nSequences,  &oldSequencesV3 );			

      // version 7-8 upgrade of subsequences (just a space savings).  
      if( version <= 7 )
         lockVector( sio, nSubSequences,  &oldSubSequencesV7 );
      else
         lockVector( sio, nSubSequences,  &fSubSequences );
      
		if (version >= 3)
      {
         if ( version <= 7 )
	         lockVector( sio, nKeyframes,     &oldKeyframesV7 );
         else
	         lockVector( sio, nKeyframes,     &fKeyframes );
      }
		else
			lockVector( sio, nKeyframes,		&oldStyleKeyframes);

      //for version 8 upgrade, we can stick with oldTransformsV6, but now the 
      //    TRANS_USE_SCALE variable is cleared so the convert code below
      //       should still work (though it now compiles differently).  
		if (version < 7 )
	      lockVector( sio, nTransforms,    &oldTransformsV6 );
		else if ( version == 7 )
	      lockVector( sio, nTransforms,    &oldTransformsV7 );
      else
	      lockVector( sio, nTransforms,    &fTransforms );
         
      lockVector( sio, nNames,         &fNames );
      if( version <= 7 )
         lockVector( sio, nObjects, &oldObjectsV7 );
      else
         lockVector( sio, nObjects, &fObjects );
      lockVector( sio, nDetails,       &fDetails );
      if ( version >= 2 )
		{
			if ( version < 7 )
	         lockVector( sio, nTransitions, &oldTransitionsV6 );
         else if ( version == 7 )
	         lockVector( sio, nTransitions, &oldTransitionsV7 );
			else
	         lockVector( sio, nTransitions, &fTransitions );
		}
		if ( version >= 4 )
			lockVector( sio, nFrameTriggers, &fFrameTriggers );
      #endif

       #ifdef DEBUG
         //  Node name list for easy watching in the debugger:
         #define  MAX_FNAMES  400
         const char     *this_names[MAX_FNAMES];
         int            this_nNames = fNames.size();
         if ( this_nNames <= MAX_FNAMES )
            for ( int k = 0; k < fNames.size(); k++ )
               this_names [ k ] = fNames [ k ];
      #endif

		
		if ( version >= 5 )
			sio.read( &fnDefaultMaterials );
		else
			fnDefaultMaterials = 0;

		if ( version >= 6 )
			sio.read( &fAlwaysNode );
		else
			fAlwaysNode = -1;

		// convert keyframes over if oldstyle
		if (version < 3)
		{
			fKeyframes.setSize(nKeyframes);
			for (int i=0; i< nKeyframes; i++)
				fKeyframes[i] = Keyframe( oldStyleKeyframes[i].getPosition(),
												  oldStyleKeyframes[i].getIndex(),
												  0,
												  !oldStyleKeyframes[i].isValid(),
												  oldStyleKeyframes[i].getVisibility() != 0,
												  false,
												  true );
		}
      else if ( version <= 7 )  // version 8 upgrade conversion.
      {
         fKeyframes.setSize(nKeyframes);
         for (int i=0; i< nKeyframes; i++)
         {
            V7Keyframe & k = oldKeyframesV7[i];
            
            k.CheckBounds( "TS_SHAPE: Old Keyframe bounds problem" );
            
            fKeyframes[i] = Keyframe( k.getPosition(), k.getIndex(), k.getMatIndex(),
                                 k.usesVisibilityTrack(),
                                 k.getVisibility(),
                                 k.usesMaterialTrack(),
                                 k.usesFrameTrack()         );
         }
      }

		if ( version < 5 )
		{
			fSequences.setSize( nSequences );
			for ( int i = 0; i < nSequences; i++ )
			{
				Sequence s;
				
				if ( version < 4 )
				{
					s.fName 		= oldSequencesV3[i].fName;
					s.fCyclic 	= oldSequencesV3[i].fCyclic;
					s.fDuration = oldSequencesV3[i].fDuration;
					s.fPriority = oldSequencesV3[i].fPriority;
					s.fFirstFrameTrigger = s.fNumFrameTriggers = 0;
				}
				else
				{
					s.fName 					= oldSequencesV4[i].fName;
					s.fCyclic 				= oldSequencesV4[i].fCyclic;
					s.fDuration 			= oldSequencesV4[i].fDuration;
					s.fPriority 			= oldSequencesV4[i].fPriority;
					s.fFirstFrameTrigger = oldSequencesV4[i].fFirstFrameTrigger;
					s.fNumFrameTriggers  = oldSequencesV4[i].fNumFrameTriggers;
				}
					
				s.fNumIFLSubSequences = s.fFirstIFLSubSequence = 0;	
				fSequences[i] = s;
			}
		}
		
		if ( version <= 7 )
		{
			fTransforms.setSize( nTransforms );
			int i;
         Point3F  unity( 1.0f, 1.0f, 1.0f ), scale;
			for ( i = 0; i < nTransforms; i++ )
         {
            if( version < 7 )
            {
				   fTransforms[ i ].set( oldTransformsV6[ i ] );
               scale = oldTransformsV6[ i ].fScale;
            }
            else //is version 7.  
            {
               oldTransformsV7[ i ].AssignToNew ( fTransforms[ i ] );
               scale = oldTransformsV7[ i ].fScale;
            }
            AssertWarn ( scale == unity, 
                  avar("Non-1 xform scale in v%d shape = trouble",version) );
         }

			fNodes.setSize( nNodes );
         for( i = 0; i < nNodes; i++ )
         {
            oldNodesV7[i].CheckBounds( "TS_SHAPE: Old node bad bounds" );
            oldNodesV7[i].AssignToNew( fNodes[i] );
         }
            
			fSubSequences.setSize( nSubSequences );
         for( i = 0; i < nSubSequences; i++ )
         {
            oldSubSequencesV7[i].CheckBounds( "TS_SHAPE: Subsequence bounds problem" );
            oldSubSequencesV7[i].AssignToNew ( fSubSequences[i] );
         }
         
			fObjects.setSize( nObjects );
         for( i = 0; i < nObjects; i++ )
         {
            oldObjectsV7[i].CheckBounds( "TS_SHAPE: Object Int bounds" );
            oldObjectsV7[i].AssignToNew ( fObjects[i] );
         }
	
         if ( version < 7 ){
   			fTransitions.setSize( nTransitions );
   			for ( i = 0; i < nTransitions; i++ )
   			{
   				Transition &t = fTransitions[i];
   				t.fStartSequence 	= oldTransitionsV6[ i ].fStartSequence;				
   				t.fEndSequence		= oldTransitionsV6[ i ].fEndSequence;				
   				t.fStartPosition 	= oldTransitionsV6[ i ].fStartPosition;				
   				t.fEndPosition		= oldTransitionsV6[ i ].fEndPosition;				
   				t.fDuration			= oldTransitionsV6[ i ].fDuration;				
   				t.fTransform.set( oldTransitionsV6[ i ].fTransform );
   			}
			}
         else{  //v7
            fTransitions.setSize( nTransitions );
            for ( i = 0; i < nTransitions; i++ )
               oldTransitionsV7[i].AssignToNew ( fTransitions[i] );
         }
		}

      fMeshes.setSize(nMeshes);
      for( int m = 0; m < nMeshes && err == Ok; m++ )
         fMeshes[m] = (Mesh *)load( sio, &err );

      // fObjectOffset is really just an offset -- matrix is identity,
      // but for some reason the exported transforms have rotation
      // and _SCALE_ flags set -- bad, bad
      // for ( int o=0; o < nObjects; o++)
      //    fObjects[o].fObjectOffset.flags = RMat3F::Matrix_HasTranslation;

      // make sure all of our utility detail levels come after our genuine
      // ones, and that fSize is 0 on them all
      markUtilityDetails();
      
      // get tight fitting bounds box for old shape versions
      if (needsBounds)
      {
         // find object that attaches to node 0
         int objNum=0;
         while (objNum<fObjects.size())
         {
            if (fObjects[objNum].fNodeIndex==0)
               break;
            objNum++;
         }
         AssertFatal(objNum!=fObjects.size(),"TSShape::read: no bounds object");
         fMeshes[fObjects[objNum].fMeshIndex]->getBox(0,fBounds);
      }
         
      #ifndef TOOL_VERSION
//      fMeshes.setReadOnly();
      #endif

      // optional embedded material list:
      Int32 hasMaterials;
      sio.read( &hasMaterials );
      if( hasMaterials )
         fpMaterials = (TS::MaterialList*)load( sio, &err );
      else
         fpMaterials = 0;

      return err;
      }

   #pragma argsused

   Persistent::Base::Error 
   Shape::write( StreamIO & sio, int version, int user )
      {
		user;

      sio.write( fNodes.size() );
      sio.write( fSequences.size() );
      sio.write( fSubSequences.size() );
      sio.write( fKeyframes.size() );
      sio.write( fTransforms.size() );
      sio.write( fNames.size() );
      sio.write( fObjects.size() );
      sio.write( fDetails.size() );
      sio.write( fMeshes.size() );
      sio.write( fTransitions.size() );
		sio.write( fFrameTriggers.size() );

      sio.write(fRadius);
      fCenter.write( sio );
      fBounds.fMin.write(sio);
      fBounds.fMax.write(sio);

      writeVector( sio, fNodes );
      writeVector( sio, fSequences );
      writeVector( sio, fSubSequences );
      writeVector( sio, fKeyframes );
      writeVector( sio, fTransforms );
      writeVector( sio, fNames );
      writeVector( sio, fObjects );
      writeVector( sio, fDetails );
      writeVector( sio, fTransitions );
		writeVector( sio, fFrameTriggers );

		sio.write( fnDefaultMaterials );
		sio.write( fAlwaysNode );

      for( int m = 0; m < fMeshes.size(); m++ )
         fMeshes[m]->store( sio );

      // optional embedded material list:
      Int32 hasMaterials = fpMaterials ? 1 : 0;
      sio.write( hasMaterials );
      if( hasMaterials )
         fpMaterials->store( sio );

      return Ok;
      }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
};  // namespace TS
