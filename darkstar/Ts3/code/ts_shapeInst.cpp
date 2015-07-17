//---------------------------------------------------------------------------

// $Workfile:   ts_mesh.h  $
// $Revision:   2.8  $
// $Version$
// $Date:   15 Sep 1995 10:27:12  $
// $Log:   R:\darkstar\develop\ts\vcs\ts_mesh.h_v  $
//   

//---------------------------------------------------------------------------

#if defined(__BORLANDC__) && (__BORLANDC__ >= 0x500)
#pragma option -Jg
#endif

#include <g_pal.h>
#include <g_surfac.h>

#include "ts_shapeInst.h"
#include "ts_celanimmesh.h"

namespace TS
{
static CelAnimMesh b_o_g_u_s;      //just to make it link itself in.
//   static TransAnimMesh b_o_g_u_s_2;      //just to make it link itself in.

// if projectedRadius > (PrefPerspective percentage of the screen width) 
// then perspective is enabled

MaterialList * ShapeInstance::fAlwaysMat = NULL;

//---------------------------------------------------------------------------
// NodeInstance
//---------------------------------------------------------------------------

ShapeInstance::NodeInstance::NodeInstance( Shape::Node const & node, 
   TMat3F *pTransform, TMat3F const *pParentTransform ) : fNode( node )
{
   fpTransform = pTransform;
   fpParentTransform = pParentTransform;
   fpNext = fpChild = fpNextDetail = 0;
   fpThread = 0;
   fVisible = TRUE;
   fActive = TRUE;
	fpMatOverride[0] = 0;
	fpMatOverride[1] = 0;
}

//---------------------------------------------------------------------------

ShapeInstance::NodeInstance::NodeInstance( Shape::Node const & node, 
   TMat3F *pTransform ) : fNode( node )
{
   fpTransform = pTransform;
   fpParentTransform = 0;
   fpNext = fpChild = fpNextDetail = 0;
   fpThread = 0;
   fVisible = TRUE;
   fActive = TRUE;
	fpMatOverride[0] = 0;
	fpMatOverride[1] = 0;
}

//---------------------------------------------------------------------------

ShapeInstance::NodeInstance::~NodeInstance()
{
   for( int ob = 0; ob < fObjectList.size(); ob++ )
      delete fObjectList[ob];
}

//---------------------------------------------------------------------------
Shape::SubSequence const * 
ShapeInstance::NodeInstance::findSubSequence( Shape::Sequence const & seq )
{
   int seqIndex = seq.getIndex( getShape() );
   return fNode.findSubSequence(getShape(),seqIndex);
}

//---------------------------------------------------------------------------
Shape::SubSequence const * 
ShapeInstance::NodeInstance::findSubSequence( int seqIndex )
{
   return fNode.findSubSequence(getShape(),seqIndex);
}

//---------------------------------------------------------------------------
void 
ShapeInstance::NodeInstance::selectThread( ShapeInstance *pShapeInst )
{
   fpThread = pShapeInst->selectThread( fNode.getIndex( pShapeInst->getShape() ) );   
}

//---------------------------------------------------------------------------
void 
ShapeInstance::NodeInstance::getPathTransform( PathPoint const & pp, 
   TMat3F * pathTransform )
{
   if( !pp.fpTransition )
   {
      // interpolate transform using keyframe tranforms
      // then set matrix
      Shape::SubSequence const * pns = findSubSequence( pp.fSequence );
      if (!pns) // this can happen when entering a transition
      {
         getShape().fTransforms[fNode.fDefaultTransform].setMatrix( pathTransform );
         pathTransform->flags &= ~RMat3F::Matrix_HasScale;
         return;
      }
      int   prevKey, nextKey;
      RealF interpParam;
      pns->findTransformKeys( getShape(), pp.fPosition, &prevKey, &nextKey, 
         &interpParam );
      pns->interpolateTransform( getShape(), prevKey, nextKey, interpParam,
                                 pathTransform );
      pathTransform->flags &= ~RMat3F::Matrix_HasScale;
   }
   else
   {
      // get start,end transforms for transition, if not already done
      // interpolate transform using transition transforms
      // then set matrix
      Transform   prevTrans;
      
      prevTrans.identity();

      Transform::interpolate( prevTrans, pp.fpTransition->fTransform, 
                              pp.fPosition, pathTransform );
      pathTransform->flags &= ~RMat3F::Matrix_HasScale;
   }
} 

//---------------------------------------------------------------------------
void 
ShapeInstance::NodeInstance::addToPathTransform( 
   TMat3F * nextTransform, TMat3F * prevTransform,
   TMat3F * totalTransform )
{
   // delta transform = nextTransform * inverse(prevTransform):
   TMat3F invPrevTransform;
   invPrevTransform = *prevTransform;
   invPrevTransform.inverse();

   TMat3F deltaTransform;
   m_mul( *nextTransform, invPrevTransform, &deltaTransform );

   // new totalTransform = deltaTransform * totalTransform:
   TMat3F temp;
   m_mul( deltaTransform, *totalTransform, &temp );
   *totalTransform = temp;
}

//---------------------------------------------------------------------------
void 
ShapeInstance::NodeInstance::accumulatePathTransform(
   PathPoint const & pp, PathPoint const & prevPP, TMat3F * prevTransform,
   TMat3F * totalTransform ) 
{
   if( pp.fRepeatCount != 0 )
   {
      // wrapping in which direction
      int direction; // direction of animation
      float cycleStart; //do we enter animation cycle at 0 or 1
      float cycleEnd; //do we leave animation cycle at 0 or 1
      if (pp.fRepeatCount>0)
      {
         direction=1;
         cycleStart=0.0f;
         cycleEnd=1.0f;
      }
      else
      {
         direction=-1;
         cycleStart=1.0f;
         cycleEnd=0.0f;
      }

      // work variables
      PathPoint ipp = pp;
      TMat3F iTrans;

      // add from the prev position to the end of the cycle to the total tranform
      ipp.fPosition = cycleEnd;
      getPathTransform( ipp, &iTrans );
      addToPathTransform( &iTrans, prevTransform, totalTransform );

      // get the tranform for the start of the cycle
      ipp.fPosition = cycleStart;
      getPathTransform( ipp, &iTrans );

      // accumulate wraps
      for (int count = pp.fRepeatCount; count-=direction; )
      {
         // the next 2 lines are here rather than above since count rarely > 1
         ipp.fPosition = cycleEnd; 
         getPathTransform(ipp, prevTransform); // prevTransform used as work variable here 
         addToPathTransform( prevTransform, &iTrans, totalTransform );
      }

      // add in from the start of the cycle to the new position:
      getPathTransform(pp,prevTransform); 
         // prevTransform now set for next time this proc. called
      addToPathTransform(prevTransform,&iTrans,totalTransform);
   }
   else
   {
      TMat3F oldPrevTransform = *prevTransform;
      getPathTransform(pp,prevTransform);
      // addToPathTransform unless leaving or entering a transition
      if (pp.fpTransition==prevPP.fpTransition)
         addToPathTransform(prevTransform,&oldPrevTransform,totalTransform);
   }
}

//---------------------------------------------------------------------------
void 
ShapeInstance::NodeInstance::animateRoot( ShapeInstance *pShapeInst ) 
{
   // set up single level undo buffer:
   pShapeInst->pushRootDeltaXform();

   // select highest priority thread:
   selectThread( pShapeInst );

   // animate using thread:
   if( fpThread )
   {
      Vector<PathPoint>::const_iterator pp = fpThread->PathBegin();
      Vector<PathPoint>::const_iterator prevPP = fpThread->PathBegin();
      if( pp == fpThread->PathEnd() )
         return;

      TMat3F prevTransform;
      getPathTransform( *prevPP, &prevTransform );

      while( ++pp != fpThread->PathEnd() )
      {
         accumulatePathTransform( *pp, *prevPP, &prevTransform, &pShapeInst->fRootDeltaTransform );
         prevPP++; // prevPP lags pp by one
         // prevTransform is set for next iteration by accumulatePathTransform
      }
   }
}

//---------------------------------------------------------------------------
int ShapeInstance::findTriggerFrames( int *vals, int max_vals, bool bidirectional )
{
   int trig_count = 0;
   
   const Shape &shp = getShape();
   
   for ( int i = 0; i < fThreadList.size() && trig_count < max_vals; i++ )
      trig_count += fThreadList[ i ]->findTriggerFrames( shp, 
                     &vals[ trig_count ], max_vals - trig_count, bidirectional );
   
   return trig_count;
}

//---------------------------------------------------------------------------
int ShapeInstance::Thread::findTriggerFrames( const Shape &shp,
                                             int *vals, int max_vals, bool bidirectional )
{
   Vector<PathPoint>::const_iterator pp = PathBegin();
   Vector<PathPoint>::const_iterator prev = pp;
   int trig_count = 0;
   
   if ( pp != PathEnd() )
      while ( ++pp != PathEnd() && trig_count < max_vals )
      {
         if ( !pp->fpTransition && !prev->fpTransition && 
              pp->fSequence == prev->fSequence )
         {
            const Shape::Sequence &seq = shp.getSequence( pp->fSequence );
            const Shape::FrameTrigger *trig;

            Bool forward = pp->fRepeatCount > 0  ||
                  (!pp->fRepeatCount && pp->fPosition > prev->fPosition);
            
            for ( int i = 0; i < seq.fNumFrameTriggers; i++ )
            {
               trig = &shp.getFrameTrigger( seq.fFirstFrameTrigger + i );
               
               if ( bidirectional || forward == trig->isForward() )
               {
                  float trig_pos = trig->getPosition();
                  Bool after_prev = trig_pos >= prev->fPosition;
                  Bool before_next = trig_pos < pp->fPosition;
               
                  if ( !forward )
                  {
                     after_prev  = !after_prev;
                     before_next = !before_next;
                  }
               
                  if ( pp->fRepeatCount? (after_prev || before_next) :
                                         (after_prev && before_next) )
                  {
                     vals[ trig_count++ ] = trig->getValue();
                  }
               }
            }
         }
         prev++;
      }
      
   return trig_count;
}


//---------------------------------------------------------------------------

void ShapeInstance::Thread::setPriority(int p)
{
   if( fPriority != p )
   {
      setDirty();
      fPriority = p;
   }
}


//---------------------------------------------------------------------------
void 
ShapeInstance::NodeInstance::animateRootUndo( ShapeInstance *pShapeInst ) 
{
   // restore from single level undo buffer:
   pShapeInst->fRootDeltaTransform = pShapeInst->fRootDeltaTransformUndo;
}

//---------------------------------------------------------------------------
void 
ShapeInstance::NodeInstance::animateThis( ShapeInstance *pShapeInst ) 
{
   //AssertFatal(fpParentTransform,"ShapeInstance::NodeInstance::animateThis: no parent transform");
   if (fpParentTransform == NULL)
      return;

   // animate dependent obects:
   for( VectorPtr<ObjectInstance*>::iterator it = fObjectList.begin();
      it != fObjectList.end(); it++ )
      (*it)->animate( pShapeInst );

   if (!fActive)
      return;

   // select highest priority thread (root has already selected a thread):
   selectThread( pShapeInst );

   // animate using thread:
   if( fpThread )
   {
      fpThread->UpdateSubscriberList();

      switch( fpThread->getState() )
      {
         case Thread::TransitionPending:
         case Thread::NoTransition:
            {
            // interpolate transform using keyframe tranforms
            // then set matrix
            Shape::SubSequence const * pns = findSubSequence( fpThread->getSequence() );
            #ifdef DEBUG
            if (pns == NULL) {
               // get some useful information about the non-existence of the subseqence
               const char *nodeName = (fNode.fName >= 0 && fNode.fName <= getShape().fNames.size())
                  ? (const char *)(getShape().fNames[fNode.fName]) : "(none)";
               const char *seqName = (fpThread->getSequence().fName >= 0 && fpThread->getSequence().fName <= getShape().fNames.size())
                  ? (const char *)(getShape().fNames[fpThread->getSequence().fName]) : "(none)";
               AssertFatal(pns, avar("TSShapeInstance::NodeInstance::animateThis:  SubSequence \"%s\" not found for node \"%s\" on shape \"%s\"",
                  seqName, nodeName, getShapeInst().getShapeResource().getFileName()));
            }
            #endif
            int   prevKey, nextKey;
            RealF interpParam;
            pns->findTransformKeys( getShape(), fpThread->getPosition(), &prevKey, &nextKey, 
               &interpParam, &fVisible );
            TMat3F temp;
            pns->interpolateTransform( getShape(), prevKey, nextKey, interpParam,
                                       &temp );
            temp.flags &= ~RMat3F::Matrix_HasScale;
				if (int override = pShapeInst->getOverride()) {
					if (override == 1 && fpMatOverride[0]) {
						fpMatOverride[0]->p = fpParentTransform->p;
	               m_mul( temp, *fpMatOverride[0], fpTransform );
						break;
					}
					else
						if (override == 2 && fpMatOverride[1]) {
							fpMatOverride[0]->p.z = fpMatOverride[1]->p.z;
							fpMatOverride[0]->p.x = fpMatOverride[0]->p.y = 0;
		               m_mul( temp, *fpMatOverride[0], fpTransform );
							break;
						}
				}
            m_mul( temp, *fpParentTransform, fpTransform );
            break;
            }
         case Thread::InTransition:
            {
            // get start,end transforms for transition, if not already done
            // interpolate transform using transition transforms
            // then set matrix

            int         prevKey, nextKey;
            RealF       interpParam;
            Transform   prevTrans, nextTrans;

            Shape::SubSequence const * pnsp = 
               findSubSequence( fpThread->getTransition().fStartSequence );
            if (pnsp)
            {
               pnsp->findTransformKeys( getShape(), fpThread->getTransition().fStartPosition, 
                  &prevKey, &nextKey, &interpParam, &fVisible );
               pnsp->interpolateTransform( getShape(), prevKey, nextKey, interpParam,
                     &prevTrans );
            }
            else
               prevTrans=getShape().fTransforms[fNode.fDefaultTransform];

            Shape::SubSequence const * pnsn = 
               findSubSequence( fpThread->getTransition().fEndSequence );
            if (pnsn)
            {
               pnsn->findTransformKeys( getShape(), fpThread->getTransition().fEndPosition, 
                  &prevKey, &nextKey, &interpParam );
               pnsn->interpolateTransform( getShape(), prevKey, nextKey, interpParam,
                     &nextTrans );
            }
            else
               nextTrans=getShape().fTransforms[fNode.fDefaultTransform];
            
            TMat3F temp;
            Transform::interpolate( prevTrans, nextTrans, 
                                    fpThread->getPosition(), &temp );
            temp.flags &= ~RMat3F::Matrix_HasScale;
            m_mul( temp, *fpParentTransform, fpTransform );
            break;
            }
      }
      
   }
   else
   {
      //set default transform
      Transform const & transform = 
         pShapeInst->getShape().getTransform( fNode.fDefaultTransform );
      TMat3F temp;
      transform.setMatrix( &temp );
      temp.flags &= ~RMat3F::Matrix_HasScale;
      m_mul( temp, *fpParentTransform, fpTransform );
   }
}

//---------------------------------------------------------------------------
void 
ShapeInstance::NodeInstance::animate( ShapeInstance *pShapeInst, bool animateSiblings )
{
   animateThis( pShapeInst );
   if( fpChild )
      fpChild->animate( pShapeInst, true );
   if( animateSiblings && fpNext )
      fpNext->animate( pShapeInst, true );
}

//---------------------------------------------------------------------------
void 
ShapeInstance::NodeInstance::collideLineParts(const Point3F & a, const Point3F & b,
                   objectList & oList, bool checkSiblings)
{
   if (fVisible)
   {
      VectorPtr<ObjectInstance*>::iterator it;
      for( it = fObjectList.begin(); it != fObjectList.end(); it++ )
      {
         oList.increment();
         ObjectInfo & oi = oList.last();
         if (!(*it)->collideLineObj( *fpTransform,a,b,oi ))
            oList.decrement();
      }
   }

   if( fpChild )
        fpChild->collideLineParts( a,b,oList, true );
   if( checkSiblings && fpNext )
      fpNext->collideLineParts( a,b,oList, true );
}

//---------------------------------------------------------------------------
void 
ShapeInstance::NodeInstance::collidePlaneParts(const Point3F & normal, float k,
                   objectList & oList, bool checkSiblings)
{
   if (fVisible)
   {
      VectorPtr<ObjectInstance*>::iterator it;
      for( it = fObjectList.begin(); it != fObjectList.end(); it++ )
      {
         oList.increment();
         ObjectInfo & oi = oList.last();
         if (!(*it)->collidePlaneObj( *fpTransform,normal,k,oi ))
            oList.decrement();
      }
   }

   if( fpChild )
        fpChild->collidePlaneParts( normal,k,oList, true );
   if( checkSiblings && fpNext )
      fpNext->collidePlaneParts( normal,k,oList, true );
}

//---------------------------------------------------------------------------
void 
ShapeInstance::NodeInstance::collideTubeParts(const Point3F & a, const Point3F & b, float radius,
                             objectList & oList, bool checkSiblings)
{
   if (fVisible)
   {
      VectorPtr<ObjectInstance*>::iterator it;
      for( it = fObjectList.begin(); it != fObjectList.end(); it++ )
      {
         oList.increment();
         ObjectInfo & oi = oList.last();
         if ( !(*it)->collideTubeObj( *fpTransform,a,b,radius,oi ) )
            oList.decrement();
      }
   }

   if( fpChild )
     fpChild->collideTubeParts( a,b,radius,oList, true );
   if( checkSiblings && fpNext )
   fpNext->collideTubeParts( a,b,radius,oList, true );
}

//---------------------------------------------------------------------------
void 
ShapeInstance::NodeInstance::collideSphereParts(const Point3F & center, float radius,
                             objectList & oList, bool checkSiblings)
{
   if (fVisible)
   {
      VectorPtr<ObjectInstance*>::iterator it;
      for( it = fObjectList.begin(); it != fObjectList.end(); it++ )
      {
         oList.increment();
         ObjectInfo & oi = oList.last();
         if ( !(*it)->collideSphereObj( *fpTransform,center,radius,oi ) )
            oList.decrement();
      }
   }

   if( fpChild )
      fpChild->collideSphereParts( center,radius,oList, true );
   if( checkSiblings && fpNext )
      fpNext->collideSphereParts( center,radius,oList, true );
}

//---------------------------------------------------------------------------
void 
ShapeInstance::NodeInstance::collideBoxParts(const Point3F & radii, const TMat3F & toShape,
                             objectList & oList, bool checkSiblings)
{
   if (fVisible)
   {
      VectorPtr<ObjectInstance*>::iterator it;
      for( it = fObjectList.begin(); it != fObjectList.end(); it++ )
      {
         oList.increment();
         ObjectInfo & oi = oList.last();
         if ( !(*it)->collideBoxObj( *fpTransform,radii,toShape,oi ) )
            oList.decrement();
      }
   }

   if( fpChild )
      fpChild->collideBoxParts( radii,toShape,oList, true );
   if( checkSiblings && fpNext )
      fpNext->collideBoxParts( radii,toShape,oList, true );
}

//---------------------------------------------------------------------------
void 
ShapeInstance::NodeInstance::collideShapeParts(const TMat3F & toOtherShape, objectList & otherOL,
                             objectList & thisOL, bool checkSiblings)
{
   if (fVisible)
   {
      VectorPtr<ObjectInstance*>::iterator it;
       for( it = fObjectList.begin(); it != fObjectList.end(); it++ )
         // unlike in other NodeInstance::collide routines, pass the whole ol ("thisOL")
         // since the object can add more than just one entry
         (*it)->collideShapeObj( *fpTransform, toOtherShape, otherOL, thisOL);
   }

   if( fpChild )
     fpChild->collideShapeParts( toOtherShape,otherOL,thisOL,true );
   if( checkSiblings && fpNext )
   fpNext->collideShapeParts( toOtherShape,otherOL,thisOL,true );
}
   
//---------------------------------------------------------------------------
void 
ShapeInstance::NodeInstance::getObjects( objectList & ol, bool getSiblings)
{
   if (fVisible)
   {
      VectorPtr<ObjectInstance*>::iterator it;
       for( it = fObjectList.begin(); it != fObjectList.end(); it++ )
         (*it)->getObject( *fpTransform,ol);
   }

   if( fpChild )
        fpChild->getObjects( ol, true);
   if( getSiblings && fpNext )
      fpNext->getObjects( ol, true);
}

//---------------------------------------------------------------------------
void 
ShapeInstance::NodeInstance::render( ShapeInstance *pShapeInst, 
   RenderContext & rc, Bool renderSiblings )
{
   if( fVisible && !fObjectList.empty())
   {
      rc.getCamera()->pushTransform( *fpTransform );
   
      for( VectorPtr<ObjectInstance*>::iterator it = fObjectList.begin();
         it != fObjectList.end(); it++ )
         (*it)->render( pShapeInst, rc );

      rc.getCamera()->popTransform();
   }

   AssertFatal( this != fpChild,"geez");
   if( fpChild )
      fpChild->render( pShapeInst, rc, TRUE );
   if( renderSiblings && fpNext )
      fpNext->render( pShapeInst, rc, TRUE );
}

//---------------------------------------------------------------------------
void 
ShapeInstance::NodeInstance::AddChild( NodeInstance * pNodeInst )
{
   pNodeInst->fpNext = fpChild;
   fpChild = pNodeInst;
}

//---------------------------------------------------------------------------
void 
ShapeInstance::NodeInstance::AddSibling( NodeInstance * pNodeInst )
{
   pNodeInst->fpNext = fpNext;
   fpNext = pNodeInst;
}

//---------------------------------------------------------------------------
void 
ShapeInstance::NodeInstance::AddDependentObject( ObjectInstance * pObj )
{
   fObjectList.push_back( pObj );
}

//---------------------------------------------------------------------------
void 
ShapeInstance::NodeInstance::AddPluginObject( RenderItem * pItem, 
   TMat3F const & offset )
{
   AddDependentObject( new PluginObjectInstance( pItem, offset ) );
}

//---------------------------------------------------------------------------
void 
ShapeInstance::NodeInstance::RemovePluginObject( RenderItem * pItem )
{
   for( VectorPtr<ObjectInstance*>::iterator oi = fObjectList.begin();
      oi != fObjectList.end(); oi++ )
      if( dynamic_cast<PluginObjectInstance*>(*oi) &&
         ((PluginObjectInstance*)(*oi))->fpItem == pItem )
            {
            delete *oi;
            *oi = fObjectList.last();
            fObjectList.decrement();
            return;
            }
   AssertFatal( 0, 
      "TS::ShapeInstance::NodeInstance::RemovePluginObject: object not in list" );
}

//---------------------------------------------------------------------------
// PulginObjectInstance
//---------------------------------------------------------------------------

void 
ShapeInstance::PluginObjectInstance::render( ShapeInstance *pShapeInst, RenderContext & rc )
{
   pShapeInst;
   rc.getCamera()->pushTransform( fObjectOffset );
   fpItem->render( rc );
   rc.getCamera()->popTransform();
}

//---------------------------------------------------------------------------
// ShapeObjectInstance
//---------------------------------------------------------------------------

ShapeInstance::ShapeObjectInstance::ShapeObjectInstance( Shape::Object const & object, const Shape *pshp )
   : fObject( object )
{
   fFrameIndex = 0;
   fMatIndex = 0;
   fpThread = 0;
   fpShape = pshp;
   fVisible = TRUE;
   fActive = TRUE;
}

//---------------------------------------------------------------------------
Shape::SubSequence const * 
ShapeInstance::ShapeObjectInstance::findSubSequence( Shape::Sequence const & seq )
{
   int seqIndex = seq.getIndex( getShape() );
   return fObject.findSubSequence( getShape(), seqIndex );
}

//---------------------------------------------------------------------------
Shape::SubSequence const * 
ShapeInstance::ShapeObjectInstance::findSubSequence( int seqIndex )
{
   return fObject.findSubSequence( getShape(), seqIndex );
}

//---------------------------------------------------------------------------
void 
ShapeInstance::ShapeObjectInstance::selectThread( ShapeInstance *pShapeInst )
{
   fpThread = pShapeInst->selectThread( 0x10000 + fObject.getIndex( pShapeInst->getShape() ) );   
}

//---------------------------------------------------------------------------
void 
ShapeInstance::ShapeObjectInstance::animate( ShapeInstance *pShapeInst ) 
{
   if (!fActive)
      return;

   // set default values in case no thread uses this object
   fFrameIndex=0;
   fMatIndex=0;
   fVisible = !(fObject.fFlags & Shape::Object::DefaultInvisible);

   int i;
   // loop through threads here, rather than selecting one
   for (i=pShapeInst->threadCount()-1; i>=0; i--)
   if ( pShapeInst->getThread(i)->IsSubscriber(0x10000+fObject.getIndex(pShapeInst->getShape())) )
   {
      // animate using thread:
      fpThread=pShapeInst->getThread(i);
      const Shape::Keyframe * pKey;
      
      switch( fpThread->getState() )
      {
         case Thread::TransitionPending:
         case Thread::NoTransition:
         {
            // find nearest keyframe:
            Shape::SubSequence const * pns = 
               findSubSequence( fpThread->getSequence() );
            AssertFatal(pns,"TS::ShapeInstance::ShapeObjectInstance::animate:  SubSequence not found");
            pKey = &pns->findCelKey( getShape(), fpThread->getPosition() );
         }
            break;

         case Thread::InTransition:
         {
            // find nearest keyframe of nearest sequence:

            if( fpThread->getPosition() < 0.5f )
            {
               Shape::SubSequence const * pns = 
                  findSubSequence( fpThread->getTransition().fStartSequence );
               AssertFatal(pns,"TS::ShapeInstance::ShapeObjectInstance::animate:  SubSequence not found");
               pKey = &pns->findCelKey( getShape(), fpThread->getTransition().fStartPosition );
            }
            else
            {
               Shape::SubSequence const * pns = 
                  findSubSequence( fpThread->getTransition().fEndSequence );
               AssertFatal(pns,"TS::ShapeInstance::ShapeObjectInstance::animate:  SubSequence not found");
               pKey = &pns->findCelKey( getShape(), fpThread->getTransition().fEndPosition );
            }
         }
      }

      // only set vars if this keyframe cares about the values   
      if (pKey->usesFrameTrack())
         fFrameIndex=pKey->getIndex();
      if (pKey->usesVisibilityTrack() != 0)
         fVisible = (pKey->getVisibility() != 0);
      if (pKey->usesMaterialTrack())
         fMatIndex = pKey->getMatIndex();
   }
}

//---------------------------------------------------------------------------
bool 
ShapeInstance::ShapeObjectInstance::collideLineObj( const TMat3F & trans, 
                           const Point3F & a, const Point3F & b, ObjectInfo & oi )
{
   if (fVisible)
   {
      m_mul(fObject.fObjectOffset,trans,&oi.objTrans);
      oi.invObjTrans=oi.objTrans;
      oi.invObjTrans.inverse();
      m_mul(a,oi.invObjTrans,&oi.pointA);
      m_mul(b,oi.invObjTrans,&oi.pointB);
      oi.pObj=this;
      return ( getShape().getMesh( fObject.fMeshIndex )->collideLineBox( fFrameIndex,oi.pointA,oi.pointB,oi.hitTime ) );
   }
   return false;
}

//---------------------------------------------------------------------------
bool 
ShapeInstance::ShapeObjectInstance::collideLine( CollisionSurface & cs, ObjectInfo & oi )
{
   if (getShape().getMesh( fObject.fMeshIndex )->collideLine( fFrameIndex,oi.pointA,oi.pointB,cs,oi.hitTime))
   {
		// The mesh only stores the material index in cs.material
		// it needs to be converted to a material pointer.
		cs.material = int(&((*getShape().getMaterialList())[cs.material]));
      cs.part = fObject.getIndex(getShape());
      Point3F n=cs.normal;
      m_mul(n,RMat3F(oi.objTrans),&cs.normal);
      // position figured via time later
      return true;
   }
   return false;
}

//---------------------------------------------------------------------------
bool 
ShapeInstance::ShapeObjectInstance::collidePlaneObj( const TMat3F & trans, 
                           const Point3F & normal, float k, ObjectInfo & oi )
{
   if (fVisible)
   {
      m_mul(fObject.fObjectOffset,trans,&oi.objTrans);
      oi.invObjTrans=oi.objTrans;
      oi.invObjTrans.inverse();

      m_mul(normal,(RMat3F&)oi.invObjTrans,&oi.pointA);
      oi.hitTime = k*m_dot(oi.pointA,oi.pointA) + m_dot(oi.pointA,oi.invObjTrans.p);

      oi.pObj=this;
      return ( getShape().getMesh( fObject.fMeshIndex )->
         collidePlaneBox( fFrameIndex,oi.pointA,oi.hitTime ) );
   }
   return false;
}

//---------------------------------------------------------------------------
bool 
ShapeInstance::ShapeObjectInstance::collidePlane( CollisionSurface & cs, ObjectInfo & oi )
{
   if ( getShape().getMesh( fObject.fMeshIndex )->
        collidePlane( fFrameIndex,oi.pointA,oi.hitTime,cs ) )
   {
		// The mesh only stores the material index in cs.material
		// it needs to be converted to a material pointer.
		cs.material = int(&getShape().getMaterialList()[cs.material]);
      cs.part = fObject.getIndex(getShape());
      Point3F n=cs.normal;
      m_mul(n,(RMat3F&)oi.objTrans,&cs.normal);
      Point3F p=cs.position;
      m_mul(p,oi.objTrans,&cs.position);
      return true;
   }
   return false;
}

//---------------------------------------------------------------------------
bool 
ShapeInstance::ShapeObjectInstance::collideTubeObj( const TMat3F & trans,
                                       const Point3F & a, const Point3F & b, float radius,
                                       ObjectInfo & oi)
{
   if (fVisible)
   {
      m_mul(fObject.fObjectOffset,trans,&oi.objTrans);
      oi.invObjTrans=oi.objTrans;
      oi.invObjTrans.inverse();
      m_mul(a,oi.invObjTrans,&oi.pointA);
      m_mul(b,oi.invObjTrans,&oi.pointB);
      oi.radius = radius; // don't really need to remember this, but makes interface simpler and more consistent
      oi.pObj=this;
      return ( getShape().getMesh( fObject.fMeshIndex )->collideTubeBox( fFrameIndex,oi.pointA,oi.pointB,radius,oi.hitTime ) );
   }
   return false;
}

//---------------------------------------------------------------------------
bool 
ShapeInstance::ShapeObjectInstance::collideTube( CollisionSurface & cs, ObjectInfo & oi )
{
   if (getShape().getMesh( fObject.fMeshIndex )->collideTube( fFrameIndex,oi.pointA,oi.pointB,oi.radius,cs,oi.hitTime))
   {
		// The mesh only stores the material index in cs.material
		// it needs to be converted to a material pointer.
		cs.material = int(&((*getShape().getMaterialList())[cs.material]));
      cs.part = fObject.getIndex(getShape());
      Point3F hp=cs.position;
      m_mul(hp,oi.objTrans,&cs.position);
      Point3F n=cs.normal;
      m_mul(n,RMat3F(oi.objTrans),&cs.normal);
      return true;
   }
   return false;
}

//---------------------------------------------------------------------------
bool 
ShapeInstance::ShapeObjectInstance::collideSphereObj( const TMat3F & trans, 
                                         const Point3F & center, float radius,
                                         ObjectInfo & oi )
{
   if (fVisible)
   {
      m_mul(fObject.fObjectOffset,trans,&oi.objTrans);
      oi.invObjTrans=oi.objTrans;
      oi.invObjTrans.inverse();
      m_mul(center,oi.invObjTrans,&oi.pointA);
      oi.radius = radius; // don't really need to remember this, but makes interface simpler and more consistent
      oi.pObj=this;
      return ( getShape().getMesh( fObject.fMeshIndex )->
         collideSphereBox(fFrameIndex,oi.pointA,radius) );
   }
   return false;
}

//---------------------------------------------------------------------------
bool 
ShapeInstance::ShapeObjectInstance::collideSphere( CollisionSurfaceList & csl, ObjectInfo & oi)
{
   int startListSize=csl.size();
   if (getShape().getMesh( fObject.fMeshIndex )->collideSphere( 
                              fFrameIndex,oi.pointA,oi.radius,csl))
   {
      for (int i=startListSize;i<csl.size();i++)
      {
			// The mesh only stores the material index in cs.material
			// it needs to be converted to a material pointer.
         CollisionSurface & cs = csl[i];
			cs.material = int(&((*getShape().getMaterialList())[cs.material]));
         cs.part = fObject.getIndex(getShape());

         Point3F hp=cs.position;
         m_mul(hp,oi.objTrans,&cs.position);
         Point3F n=cs.normal;
         m_mul(n,RMat3F(oi.objTrans),&cs.normal);
      }
      return true;
   }
   return false;
}

//---------------------------------------------------------------------------
bool 
ShapeInstance::ShapeObjectInstance::collideBoxObj(const TMat3F & trans, 
                                      const Point3F & radii,
                                      const TMat3F & boxToShape,
                                      ObjectInfo & oi)
{
   if (fVisible)
   {
      m_mul(fObject.fObjectOffset,trans,&oi.objTrans);
      oi.invObjTrans=oi.objTrans;
      oi.invObjTrans.inverse();
      oi.pObj=this;
      // for now, we don't store off box information (we'd need boxToObj --below-- and
      // radii if detailed box to face collisions are ever implemented)
      TMat3F boxToObj;
      m_mul(boxToShape,oi.invObjTrans,&boxToObj);
      return getShape().getMesh( fObject.fMeshIndex )->
         collideBoxBox(fFrameIndex,boxToObj,radii,oi.aOverlap,oi.bOverlap);
   }
   return false;
}

//---------------------------------------------------------------------------
bool 
ShapeInstance::ShapeObjectInstance::collideBoxObj(TMat3F & trans, const Point3F & radii,
                                                  Point3F & aOverlap, Point3F & bOverlap)
{
   if (fVisible)
   {
      return getShape().getMesh( fObject.fMeshIndex )->
         collideBoxBox(fFrameIndex,trans,radii,aOverlap,bOverlap);
   }
   return false;
}

//---------------------------------------------------------------------------
bool 
ShapeInstance::ShapeObjectInstance::collideShapeObj(const TMat3F & nodeToShape,const TMat3F & shapeToOtherShape,
                                     objectList & otherOL, objectList & thisOL)
{
   int startSize=thisOL.size();
   // we don't need to store transforms w/ objectList ("thisOL") since we don't do detailed coll.
   TMat3F tmpTrans;
   m_mul(fObject.fObjectOffset,nodeToShape,&tmpTrans);
   // tmpTrans now holds transform from this object to shape
   TMat3F objToOtherShape;
   m_mul(tmpTrans,shapeToOtherShape,&objToOtherShape);
   // objToOtherShape now holds transform from this object to other shape
   getShape().getMesh( fObject.fMeshIndex )->
      collideShapeBox(fFrameIndex,objToOtherShape,otherOL,thisOL);

   if (thisOL.size()==startSize)
      return false;

   for (int i=startSize;i<thisOL.size();i++)
      thisOL[i].pObj=this;

   return true;
}

//---------------------------------------------------------------------------
void 
ShapeInstance::ShapeObjectInstance::getBox(ShapeInstance *pShapeInst, Box3F & box)
{
   pShapeInst->getShape().getMesh( fObject.fMeshIndex )->
      getBox( fFrameIndex, box );
}

//---------------------------------------------------------------------------
void 
ShapeInstance::ShapeObjectInstance::getPolys(ShapeInstance *pShapeInst, const TMat3F * trans,
                                             unpackedFaceList & fl)
{
   pShapeInst->getShape().getMesh( fObject.fMeshIndex )->
      getPolys(fFrameIndex,fMatIndex,trans,fl);
}

//---------------------------------------------------------------------------
void 
ShapeInstance::ShapeObjectInstance::getObject(const TMat3F & nodeToShape, objectList & ol)
{
   if (fVisible)
   {
      ol.increment();
      ObjectInfo & oi = ol.last();
      oi.pObj = this;
      m_mul(fObject.fObjectOffset,nodeToShape,&oi.objTrans);
   }
}

//---------------------------------------------------------------------------
void 
ShapeInstance::ShapeObjectInstance::render( ShapeInstance *pShapeInst, RenderContext & rc )
{
   if ( fVisible )
   {
      const Shape &shp = pShapeInst->getShape();
      const Shape::Mesh *pMesh = shp.getMesh( fObject.fMeshIndex );
      
      rc.getCamera()->pushTransform( fObject.offsetTransform() );
      
      // ensure that the RGB's in the Material list have been
      // resolved to palette indices
      GFXPalette *pal = rc.getSurface()->getPalette();
      MaterialList &mats = (MaterialList &)pShapeInst->getMaterialList();
      if ( mats.RGB_NeedsRemap(pal) )
         mats.mapRGBtoPal(pal);

      pMesh->render( rc, pShapeInst, mats, fFrameIndex, fMatIndex, 
                     pShapeInst->getMaterialRemap() );
      rc.getCamera()->popTransform();
   }
}

//---------------------------------------------------------------------------
// Thread
//---------------------------------------------------------------------------

ShapeInstance::Thread::Thread( ShapeInstance & shapeInst ) : fShapeInst( shapeInst )
{
   setDirty();
   fTimeScale = 1.0f;
   SetSequence( 0, 0.0f );
}

//---------------------------------------------------------------------------
inline IndexList const & 
ShapeInstance::Thread::getSequenceSubscriberList( Shape::Sequence const *pSeq ) const
{
   return fShapeInst.getSequenceSubscriberList( pSeq );
}

//---------------------------------------------------------------------------
Bool 
ShapeInstance::Thread::IsSubscriber( int subscriberIndex )
{
   // is the node Index in the node list?
   return fSubscriberList.end() != 
          find( fSubscriberList.begin(), 
                fSubscriberList.end(), 
                subscriberIndex );
}

//---------------------------------------------------------------------------
void 
ShapeInstance::Thread::UpdateSubscriberList()
{
   if( fSubscriberListValid == FALSE )
   {
      // set list to contain all nodes controlled by current sequence
      // also, if in transition or pending transition, add any other
      // sequences from target sequence
      switch (fState)
      {
         case NoTransition:
         case TransitionPending:
            {
            // copy subscriber list for current sequence
            fSubscriberList.clear();
            fSubscriberList.add( getSequenceSubscriberList( fpSequence ) );
            }
            break;
         case InTransition:
            {
            // copy node list for both current and transition sequences
            fSubscriberList.clear();
            fSubscriberList.add( getSequenceSubscriberList( &(getShape().fSequences[fpTransition->fStartSequence]) ) );
            fSubscriberList.add( getSequenceSubscriberList( &(getShape().fSequences[fpTransition->fEndSequence]) ) );
            }
            break;
         default:
            AssertFatal( 0, "TS::ShapeInstance::Thread::UpdateSubscriberList: invalid thread state" );
            break;
      }
      fSubscriberListValid = TRUE;
   }
}

//---------------------------------------------------------------------------
// returns how many transitions we set (0 or 1)
int 
ShapeInstance::Thread::SetTransition( Shape::Transition const * pTrans )
{
   if (fState == InTransition)
      return 0;

   if (fState==NoTransition)
   {
      fpForwardTransition=0;
      fpBackwardTransition=0;
   }

   // setOne is so we know how many transitions we set (0 or 1)
   int setOne=0;

   if (fpSequence->getIndex( getShape() ) == pTrans->fStartSequence)
   {
      fpForwardTransition=pTrans;
      setOne=1;
   }
   else if (fpSequence->getIndex( getShape() ) == pTrans->fEndSequence)
   {
      fpBackwardTransition=pTrans;
      setOne=1;
   }

   if ((fpBackwardTransition!=0) ||   (fpForwardTransition!=0))
      fState = TransitionPending;

   return setOne;
}

//---------------------------------------------------------------------------
// returns number of transitions set (0,1, or 2)
// returns w/o doing anything if InTransition
int 
ShapeInstance::Thread::TransitionToSequence(int newSeq)
{
   // if in transition then do nothing
   if (fState==InTransition)
      return 0;

   int curSequence = fpSequence->getIndex(getShape());
   if ( newSeq==curSequence )
   {
      // already in this sequence: clear any transitions and return
      fState=NoTransition;
      return 0;
   }

   // get transition list from shape
   Vector<TSShape::Transition> const & transList = getShape().fTransitions;

   // set these variables for return value
   int setF = 0;
   int setB = 0;

   // look for forward and backward transitions
   float curFMinDist = 2.0f; // larger than any valid transition
   float curBMinDist = 2.0f; // larger than any valid transition
   fpForwardTransition=0;
   fpBackwardTransition=0;
   for (int i=0; i<transList.size(); i++)
   {
      if ( (transList[i].fStartSequence==curSequence) &&
           (transList[i].fEndSequence==newSeq) ) 
      // got a forward transition
      {
         // how far away is it
         float dist=transList[i].fStartPosition-fPosition;
         if ((dist<0.0f) && fpSequence->fCyclic)
            dist += 1.0f;
         else if (dist<0.0f)
            dist = 3.0f; // this one won't get chosen
         if (dist<curFMinDist) // choose this one for now
         {
            curFMinDist = dist;
            fpForwardTransition=&(transList[i]);
            setF=1;
         }
      }
      if ( (transList[i].fStartSequence==newSeq) &&
           (transList[i].fEndSequence==curSequence) )
      // got a backward transition
      {
         // how far away is it
         float dist=fPosition-transList[i].fEndPosition;
         if ((dist<0.0f) && fpSequence->fCyclic)
            dist += 1.0f;
         else if (dist<0.0f)
            dist = 3.0f; // this one won't get chosen
         if (dist<curBMinDist) // choose this one for now
         {
            curBMinDist = dist;
            fpBackwardTransition=&(transList[i]);
            setB=1;
         }
      }
   }

   if ((fpBackwardTransition!=0) ||   (fpForwardTransition!=0))
      fState = TransitionPending;

   return setF+setB;
}

//---------------------------------------------------------------------------
void 
ShapeInstance::Thread::SetSequence( char *sequence, RealF startTime, bool updateSSL )
{
   // note dirty flag is set in call to SetSequence() below.  
   const TSShape &shape = fShapeInst.getShape();
   
   int seqNameIndex = shape.lookupName( sequence );

   for( int si = 0; si < shape.fSequences.size(); si++ )
      if( shape.fSequences[si].fName == seqNameIndex )
      {
         SetSequence( si, startTime, updateSSL );
      	return;
      }
   AssertFatal(0,"ShapeInstance::SetSequence: no such sequence");
}

//---------------------------------------------------------------------------
int 
ShapeInstance::Thread::GetSequenceIndex( char *sequence )
{
   const TSShape &shape = fShapeInst.getShape();
   
   int seqNameIndex = shape.lookupName( sequence );

   for( int si = 0; si < shape.fSequences.size(); si++ )
      if( shape.fSequences[si].fName == seqNameIndex )
      {
         return si;
      }
      
   return -1;
}

//---------------------------------------------------------------------------
void 
ShapeInstance::Thread::SetSequence( int sequence, RealF startTime, bool updateSSL )
{
   setDirty();
   fpSequence = &fShapeInst.getSequence( sequence );
   fPosition = startTime / fpSequence->fDuration;
	fPriority = fpSequence->fPriority;

   fSubscriberListValid = FALSE;
   fState = NoTransition;            // cancel any transition
   if (updateSSL)
      UpdateSubscriberList();
}

//---------------------------------------------------------------------------
void 
ShapeInstance::Thread::SetTime( RealF newTime )
{
   if (fState==InTransition)
      SetPosition( newTime / fpTransition->fDuration );
   else
      SetPosition( newTime / fpSequence->fDuration );
}

//---------------------------------------------------------------------------
void 
ShapeInstance::Thread::SetPosition( RealF newPosition )
{
   AdvancePosition( (newPosition - fPosition)/fTimeScale );
}

//---------------------------------------------------------------------------
void 
ShapeInstance::Thread::AdvanceTime( RealF deltaTime )
{
   if (fState==InTransition)
      AdvancePosition( deltaTime / fpTransition->fDuration );
   else
      AdvancePosition( deltaTime / fpSequence->fDuration );
}

void 
ShapeInstance::Thread::AdvanceTimeUndo()
{
   setDirty();
   fState=fSaveState.fState; // enough to do this since seq. and trans. pointers unchanged
   fPosition=fSaveState.fPosition;
   fpSequence=fSaveState.fpSequence;
   if (fSaveState.fDifferentSubscriberList)
   {
      fSubscriberListValid=false;
      UpdateSubscriberList();
   }
}

//---------------------------------------------------------------------------
void 
ShapeInstance::Thread::AdvancePosition( RealF deltaPosition, Bool startPath )
{
   if( startPath )
   {
      fPath.clear();

      // save state at beginning of path
      fSaveState.fState=fState;
      fSaveState.fPosition=fPosition;
      fSaveState.fpSequence=fpSequence;
      // also check to see if subscriber list is invalid on exit...
   }

   switch (fState)
   {
      case NoTransition:
         AdvancePositionNoTransition( deltaPosition );
         break;
      case TransitionPending:
         AdvancePositionTransitionPending( deltaPosition );
         break;
      case InTransition:
         AdvancePositionInTransition( deltaPosition );
         break;
      default:
         AssertFatal( 0, 
        "TS::ShapeInstance::Thread::AdvancePosition: invalid thread state" );
         break;
   }

   if ( startPath ) // end path now...
   {
      fSaveState.fDifferentSubscriberList=!fSubscriberListValid;
      UpdateSubscriberList();
   }
   
      
   const Shape &shp = fShapeInst.getShape();
   int ss_index = fpSequence->fFirstIFLSubSequence;
   
   for ( int i = 0; i < fpSequence->fNumIFLSubSequences; i++, ss_index++ )
   {
      const Shape::SubSequence &ss = shp.getSubSequence( ss_index );
      const Shape::Keyframe &key = ss.findCelKey( shp, fPosition );
      fShapeInst.setMaterialRemap( key.getMatIndex(), key.getIndex() );
   }                                   
}

//---------------------------------------------------------------------------
Vector<ShapeInstance::PathPoint>::const_iterator 
ShapeInstance::Thread::PathBegin() const
{
   return fPath.begin();
}

//---------------------------------------------------------------------------
Vector<ShapeInstance::PathPoint>::const_iterator 
ShapeInstance::Thread::PathEnd() const
{
   return fPath.end();
}

//---------------------------------------------------------------------------
void 
ShapeInstance::Thread::AdvancePositionNoTransition( RealF deltaPosition )
{
   float savePos = fPosition;
   
   fPath.push_back( PathPoint( fpSequence->getIndex( getShape() ), 0, 
      fPosition, 0 ) );

   deltaPosition *= fTimeScale;
   fPosition += deltaPosition;
   
   int count;
   fpSequence->handleORP( &fPosition, &count );

   fPath.push_back( PathPoint( fpSequence->getIndex( getShape() ), 0, 
      fPosition, count ) );

   // see if our sequence has changed.  For large deltas, it could wrap and only
   //    look the same, so count it as dirty to be safe.  
   if( fPosition != savePos || fabs(deltaPosition) > 0.99 )
      setDirty();
}

//---------------------------------------------------------------------------
void 
ShapeInstance::Thread::AdvancePositionTransitionPending( 
   RealF deltaPosition )
{
   // advance in the current sequence
   // check to see if we cross into the transition
   // if so, set state to InTransition and call AdvancePosition again
   // with remaining time to advance in the transition
   deltaPosition *= fTimeScale;
      fPath.push_back( PathPoint(fpSequence->getIndex(getShape()), 0, fPosition, 0 ) );
   if ( (deltaPosition>0.0f) && fpForwardTransition )
   {
      // we are moving forwards - look for start of transition:
      RealF remaining = fpForwardTransition->fStartPosition - fPosition;
      if( remaining < 0.0f && fpSequence->fCyclic )
         remaining += 1.0f;
      if( deltaPosition > remaining )
      {
         setDirty();
         fPosition += remaining;
         int count;
         fpSequence->handleORP( &fPosition, &count );
         fPath.push_back( PathPoint( fpSequence->getIndex( getShape() ), 0, 
            fPosition, count ) );

         // adjust remaining time delta to transition time scale:
         deltaPosition = 
            ((deltaPosition - remaining) * fpSequence->fDuration / 
                  fpForwardTransition->fDuration);
         // undo time scaling since AdvancePosition (below) will redo it
         deltaPosition /= fTimeScale;
         fPosition = 0.0f;
         fpTransition = fpForwardTransition;
         fState = InTransition;
         fSubscriberListValid = FALSE;
         AdvancePosition( deltaPosition, FALSE );
         return;
      }
   }
   else if ( (deltaPosition<0.0f) && fpBackwardTransition )
   {
      // we are moving backwards - look for end of transition:
      RealF remaining = fpBackwardTransition->fEndPosition - fPosition;
      if( remaining > 0.0f && fpSequence->fCyclic )
         remaining -= 1.0f;
      if( deltaPosition < remaining )
      {
         setDirty();
         fPosition += remaining;
         int count;
         fpSequence->handleORP( &fPosition, &count );
         fPath.push_back( PathPoint( fpSequence->getIndex( getShape() ), 0, 
            fPosition, count ) );

         // adjust remaining time delta to transition time scale:
         deltaPosition = 
            ((deltaPosition - remaining) * fpSequence->fDuration /
            fpBackwardTransition->fDuration);
         // undo time scaling since AdvancePosition (below) will redo it
         deltaPosition /= fTimeScale;
         fPosition = 1.0f;
         fpTransition = fpBackwardTransition;
         fState = InTransition;
         fSubscriberListValid = FALSE;
         AdvancePosition( deltaPosition, FALSE );
         return;
      }
   }

   // update position if we didn't just enter a transition
   float savePos = fPosition;
   fPosition += deltaPosition;
   int count;
   fpSequence->handleORP( &fPosition, &count );
   fPath.push_back( PathPoint( fpSequence->getIndex(getShape()), 0, fPosition, count) );
   
   // see if our sequence has changed.  For large deltas, it could wrap and only
   //    look the same, so count it as dirty to be safe.  
   if( fPosition != savePos || fabs(deltaPosition) > 0.99f )
      setDirty();
}

//---------------------------------------------------------------------------
void 
ShapeInstance::Thread::AdvancePositionInTransition( RealF deltaPosition )
{
   // advance in the transition
   // check to see if we cross out of the transition
   // if so, set state to NoTransition and call AdvancePosition again
   // with remaining time to advance in the target sequence
   fPath.push_back( PathPoint(0,fpTransition,fPosition,0) );

   if( (deltaPosition *= fTimeScale) != 0.0f )
      setDirty();
   fPosition += deltaPosition;
   if( fPosition < 0.0f )
   {
      fPath.push_back( PathPoint(0,fpTransition,0,0) );
      RealF remaining = fPosition;
      SetSequence( fpTransition->fStartSequence, 0.0f, false ); // don't update sslist quite yet
      fPosition = fpTransition->fStartPosition;
      remaining /= fTimeScale;
      AdvancePosition( (remaining * fpTransition->fDuration / 
         fpSequence->fDuration), FALSE );
   }
   else if( fPosition > 1.0f )
   {
      fPath.push_back( PathPoint(0,fpTransition,1,0) );
      RealF remaining = fPosition - 1.0f;
      SetSequence( fpTransition->fEndSequence, 0.0f, false ); // don't update sslist quite yet
      fPosition = fpTransition->fEndPosition;
      remaining /= fTimeScale;
      AdvancePosition( (remaining * fpTransition->fDuration / 
          fpSequence->fDuration), FALSE );
   }
   else
   {
      // transitions aren't cyclic, so handleORP not called
      fPath.push_back( PathPoint(0,fpTransition,fPosition,0) );
   }
}

//---------------------------------------------------------------------------
int 
ShapeInstance::Thread::operator <( Thread const & thread )
{
   return fPriority < thread.fPriority;
}

//---------------------------------------------------------------------------
void ShapeInstance::Thread::setStatus(int state, RealF _fTimeScale, 
   RealF _fPosition, int sequence, int transition, int forwardTransition, int backwardTransition)
{
   setDirty();
   fState = (States)state;
   fTimeScale = _fTimeScale;
   fPosition = _fPosition;
   const TSShape &shape = getShape();
   fpSequence = &(shape.fSequences[sequence]); 
   if (transition >= 0 && transition < shape.fTransitions.size())
      fpTransition = &(shape.fTransitions[transition]); 
   if (forwardTransition >= 0 && forwardTransition < shape.fTransitions.size())
      fpForwardTransition = &(shape.fTransitions[forwardTransition]); 
   if (backwardTransition >= 0 && backwardTransition < shape.fTransitions.size())
      fpBackwardTransition = &(shape.fTransitions[backwardTransition]); 
   fSubscriberListValid = false;
   fSaveState.fState = fState;
   fSaveState.fPosition = fPosition;
   fSaveState.fpSequence = fpSequence;
   fSaveState.fDifferentSubscriberList = true;
   UpdateSubscriberList();
} 

//---------------------------------------------------------------------------
void ShapeInstance::Thread::getStatus(int &state, RealF &_fTimeScale, 
   RealF &_fPosition, int &sequence, int &transition, int &forwardTransition, int &backwardTransition)
{
   state = (int)getState();
   _fTimeScale = getTimeScale();
   _fPosition = getPosition();
   const TSShape &shape = getShape();
   sequence = getSequence().getIndex(shape);
   if (state==TransitionPending || state==InTransition)
   {
      transition = &(getTransition()) - shape.fTransitions.address();
      forwardTransition = &(getForwardTransition()) - shape.fTransitions.address();
      backwardTransition = &(getBackwardTransition()) - shape.fTransitions.address();
   }
   else
      transition = forwardTransition = backwardTransition = 0;
}

//---------------------------------------------------------------------------
// ThreadList
//---------------------------------------------------------------------------
void 
ShapeInstance::ThreadList::insertThread( Thread *pThread )
{
   AssertFatal( end() == ::find( begin(), end(), pThread ),
      "TS::ShapeInstance::ThreadList::insert: thread already in list" );
   push_back( pThread );
   forceDirty();
}

//---------------------------------------------------------------------------
void 
ShapeInstance::ThreadList::removeThread( Thread *pThread )
{
   iterator loc = ::find( begin(), end(), pThread );
   AssertFatal( loc != end(), 
      "TS::ShapeInstance::ThreadList::remove: thread not in list" );
   *loc = last();
   decrement();
   forceDirty();
}

//---------------------------------------------------------------------------
ShapeInstance::Thread * 
ShapeInstance::ThreadList::selectThread( int index )
{
   for( iterator it = begin(); it < end(); it++ )
      if( (*it)->IsSubscriber( index ) )
         return *it;
   return 0;
}
//---------------------------------------------------------------------------
//
// Check to see if any of the threads are dirty.  Since we're going through all the
//    threads at this point we can see if they are in order priority-wise and 
//    avoid the sort if the order is fine.  If no threads have changed, return 
//       false and don't sort either way.  
//
// This also clears the dirty flag on each thread - so it is assuming that this will
//    will then be an accurate state.  ThreadList::forceListDirty is also cleared here - 
//    so this is a method which depends on other things happening right after...
// 
bool ShapeInstance::ThreadList::checkDirtyAndSort( bool doCleaning )
{
   bool     listAlreadySorted = true;
   bool     somethingIsDirty = forceListDirty;       // ThreadList might want to force.
   Thread   *prev = 0, *tPtr;
   
   for( iterator it = begin(); it < end(); it++ )
   {
      if( (tPtr = *it)->isDirty() )
      {
         if( doCleaning )
            tPtr->setClean();
         somethingIsDirty = true;
      }
         
      if( prev && (prev->getPriority() > tPtr->getPriority()) )
         listAlreadySorted = false;
         
      prev = tPtr;
   }
   
   if( somethingIsDirty )
   {
      if( !listAlreadySorted )
         sort();

      if( doCleaning )      
         forceListDirty = false;    // clear the dirty force 'pulse'. 
      
      return true;
   }

   return false;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// ShapeInstance
//---------------------------------------------------------------------------

ShapeInstance::ShapeInstance( Resource<Shape> const & shape, 
   ResourceManager &rm )
{
   frShape = shape;
   fpShape = (Shape const *)shape;

   // use the shape's material list:
   frMaterialList = 0;
   fMaterialRemap = 0;
   fpMaterialList = fpShape->getMaterialList();
	fUseOverride = 0;

   const_cast<MaterialList*>(fpMaterialList)->load( rm, 1 );
   init();
}

static bool checkBaseName(const char *baseName, const char *test, char *buf)
{
   while(*baseName)
   {
      if(tolower(*baseName++) != tolower(*test++))
         return false;
   }
   strcat(buf, test);
   return true;
}

//---------------------------------------------------------------------------

ShapeInstance::ShapeInstance( Resource<Shape> const & shape,
   Resource<MaterialList> const & mats, ResourceManager &rm )
{
   frShape        = shape;
   fpShape        = (Shape const *)shape;
   frMaterialList = mats;
   fpMaterialList = (MaterialList const *)mats;

   const_cast<MaterialList*>(fpMaterialList)->load( rm, 1 );
   init();
}

//---------------------------------------------------------------------------
void ShapeInstance::init()
{
   // default to first detail:
   fDetailLevel = 0;
   clrDetailsAnimateSame();
   fDrawFlatPerspective = false;
   fDrawFlatPerspectiveDetail = false;
   perspectiveScale = 0.0f;   
   detailScale = 1.0f;
   fAlwaysAlpha = false;
   fUseAnimation = true;
   fUseDetails = true;

   fNodeInstanceList.setSize( fpShape->fNodes.size() );
   fTransformList.setSize( fpShape->fNodes.size() );
   Transform const & defaultRootTransform = 
      fpShape->getTransform( fpShape->fNodes[0].fDefaultTransform );
   defaultRootTransform.setMatrix( &fRootDeltaTransform );
   fRootDeltaTransform.flags = TMat3F::Matrix_HasRotation | TMat3F::Matrix_HasTranslation;
   fTransformList[0].identity();
   
   // non-animating shapes seem to have transforms too, and they'll
   // be garbage since animate() will never be called, temporary fix
   // is to just zap all the transforms
   for (int i = 1; i < fTransformList.size(); i++)
      fTransformList[i].identity();
   
   // first node (shape) has no parent
   AssertFatal( fpShape->fNodes[0].fParent == -1,
      "TS::ShapeInstance::ShapeInstance: root node has a parent!" );
   fNodeInstanceList[0] = new NodeInstance( fpShape->fNodes[0], 
      &fTransformList[0] );

   // all other nodes have a parent preceding them in shape's node list:
   for( int n = 1; n < fpShape->fNodes.size(); n++ )
   {
      AssertFatal( fpShape->fNodes[n].fParent == -1 ||
         (fpShape->fNodes[n].fParent >= 0 && 
         fpShape->fNodes[n].fParent < n),
         "TS::ShapeInstance::ShapeInstance: node has invalid parent!" );

      if( fpShape->fNodes[n].fParent == -1 )
      {
         fNodeInstanceList[n] = new NodeInstance( 
            fpShape->fNodes[n], &fTransformList[n] );
   
         fNodeInstanceList[0]->AddSibling( fNodeInstanceList[n] );
      }
      else
      {
         fNodeInstanceList[n] = new NodeInstance( 
            fpShape->fNodes[n], &fTransformList[n], 
            &fTransformList[fpShape->fNodes[n].fParent] );

         fNodeInstanceList[fpShape->fNodes[n].fParent]->AddChild( 
            fNodeInstanceList[n] );
      }
   }

   if ( fpShape->fAlwaysNode >= 0 )
      fAlwaysNode = fNodeInstanceList[ fpShape->fAlwaysNode ];
   else 
      fAlwaysNode = 0;

   for( int ob = 0; ob < fpShape->fObjects.size(); ob++ )
      fNodeInstanceList[fpShape->fObjects[ob].fNodeIndex]->
         AddDependentObject( new ShapeObjectInstance(fpShape->fObjects[ob], fpShape ) );

   fSequenceSubscriberLists.setSize( fpShape->fSequences.size() );
   for( int s = 0; s < fpShape->fSequences.size(); s++ )
      fSequenceSubscriberLists[s] = new IndexList;
   UpdateSequenceSubscriberLists();

   if ( fpShape->fnDefaultMaterials )
   {
      if (fMaterialRemap)
         delete [] fMaterialRemap;

      fMaterialRemap = new int[ fpShape->fnDefaultMaterials ];
      for ( int i = 0; i < fpShape->fnDefaultMaterials; i++ )
         fMaterialRemap[i] = i;
   }
   else
   {
      if (fMaterialRemap)
         delete [] fMaterialRemap;

      fMaterialRemap = 0;
   }

   animateRoot();
   animate();
}

//---------------------------------------------------------------------------

ShapeInstance::~ShapeInstance()
{
   for( int n = 0; n < fNodeInstanceList.size(); n++ )
      delete fNodeInstanceList[n];
   for( int s = 0; s < fSequenceSubscriberLists.size(); s++ )
      delete fSequenceSubscriberLists[s];
   for( int t = 0; t < fThreadList.size(); t++ )
      delete fThreadList[t];

   if (fMaterialRemap)
   {
      delete [] fMaterialRemap;
      fMaterialRemap = 0;
   }
}

//---------------------------------------------------------------------------
void 
ShapeInstance::AddPluginObject( char *node, RenderItem * pItem, TMat3F const & offset )
{
   int nodeIndex = fpShape->findNode( node );
   fNodeInstanceList[nodeIndex]->AddPluginObject( pItem, offset );
   clearThreadOptimizations();
}

//---------------------------------------------------------------------------
void 
ShapeInstance::RemovePluginObject( char *node, RenderItem * pItem )
{
   int nodeIndex = fpShape->findNode( node );
   fNodeInstanceList[nodeIndex]->RemovePluginObject( pItem );
   clearThreadOptimizations();
}

//---------------------------------------------------------------------------
bool ShapeInstance::lastDetailDifferent( int thisDetailLevel )
{
   if( ! fUseDetails )  // not sure what part instances want to do here, maybe this
      return true;      // method can be virtual?  
      
   if( !detailsAnimateSame() )
      if( fLastAnimatedDetailLevel != thisDetailLevel )
         return true;
         
   return false;
}
//---------------------------------------------------------------------------

void 
ShapeInstance::animateRoot()
{
   // sort threads by priority:
   // fThreadList.sort();
   
   // animate nodes recursively:
   if( fThreadList.checkDirtyAndSort( false ) )
      fNodeInstanceList.first()->animateRoot( this );
   else
      pushRootDeltaXform();         // probably still want this to happen
}

//---------------------------------------------------------------------------
void 
ShapeInstance::animateRootUndo()
{
   // animate nodes recursively:
   fNodeInstanceList.first()->animateRootUndo( this );
   clearThreadOptimizations();
}

//---------------------------------------------------------------------------
void 
ShapeInstance::animate()
{
   if (fDetailLevel != -1 && fUseAnimation) 
   {
      // sort threads by priority if needed...   
      if( fThreadList.checkDirtyAndSort()  ||  lastDetailDifferent(fDetailLevel) )
      {
   		// Animate always nodes first
         if ( fAlwaysNode )
            fAlwaysNode->animate( this, false );

         // Animate the current detail level
         getDetail()->animate( this, false );
         
         setLastAnimatedDetail( fDetailLevel );
      }
   }    
}

//---------------------------------------------------------------------------
void 
ShapeInstance::animateDetail(int dl)
{
   if (!fUseAnimation)
      return;

   // sort threads by priority if needed  
   if( fThreadList.checkDirtyAndSort()  ||  lastDetailDifferent(dl) )
   {
      // make sure this detail level exists
      AssertFatal(dl<fpShape->fDetails.size(),
            "TS::ShapeInstance::animateDetail:  detail level out of range.");
      getDetail(dl)->animate( this, false );
      
      setLastAnimatedDetail( dl );
   }
}

//---------------------------------------------------------------------------
// collide line routines start here
void 
ShapeInstance::collideLineParts(int dl, const Point3F & a, const Point3F & b,
                        const TMat3F * toShape, objectList & ol)
{
   AssertFatal(dl<fpShape->fDetails.size(),"TS::ShapeInstance::collideLineParts:  detail level out of range");

   Point3F ta,tb;
   if (toShape)
   {
      m_mul(a,*toShape,&ta);
      m_mul(b,*toShape,&tb);
   }
   else
   {
      ta=a;
      tb=b;
   }

   getDetail(dl)->collideLineParts(ta,tb,ol,false);
}

//---------------------------------------------------------------------------
bool 
ShapeInstance::collideLine(int dl, const Point3F &a, const Point3F & b,
                     const TMat3F * toShape, CollisionSurfaceList * list,
                     bool faceData)
{
   // transform line endpoints here so we have them below when figuring hitpoint
   Point3F ta,tb;
   if (toShape)
   {
      m_mul(a,*toShape,&ta);
      m_mul(b,*toShape,&tb);
   }
   else
   {
      ta = a;
      tb = b;
   }   

//   list->clear();

   potentialObjList.clear();
   collideLineParts(dl,ta,tb,0,potentialObjList);

   // sort by front intersection point
   int i;
   sortedObjListPtrs.clear();
   for (i=0;i<potentialObjList.size();i++)
      sortedObjListPtrs.push_back(&(potentialObjList[i]));
   sortedObjListPtrs.sort();

   if (faceData)
   {
      // do detailed intersection of objs w/ line, quiting when we find an intersection point
      // that is in front of the frontmost point of the next object
      list->increment();
      CollisionSurface & cs = list->last();
      memset(&(list->last()), 0, sizeof(CollisionSurface));

      float & hitTime = cs.time;
      hitTime=1.1f;
      i=0;
      bool gotOne = false;
      while ( i<potentialObjList.size() && hitTime>sortedObjListPtrs[i]->hitTime )
      {
         gotOne |= sortedObjListPtrs[i]->pObj->collideLine(cs,*sortedObjListPtrs[i]);
         i++;
      }

      if (gotOne)
      {
         cs.position  = tb;
         cs.position -= ta;
         cs.position *= hitTime;
         cs.position += ta;
         
         cs.distance = 0.0f;     // lines always have 0 dist...
         cs.normal.normalize();
         
         return true;
      }
      list->clear();
      return false;
   }

   // don't want face data, just copy closest object hit onto collisionInfoList
   if (!potentialObjList.empty())
   {
      // fill out 1 collisionInfoList entry
      list->increment();
      CollisionSurface & cs = list->last();
      memset(&(list->last()), 0, sizeof(CollisionSurface));
     
      ObjectInfo & oi = potentialObjList[0];
      cs.part = static_cast<ShapeObjectInstance *>(oi.pObj)->fObject.getIndex(getShape());
      cs.time = oi.hitTime;
      cs.position  = tb;
      cs.position -= ta;
      cs.position *= cs.time;
      cs.position += ta;
		cs.material = 0;
      // normal?
      return true;
   }
   return false;
}

//---------------------------------------------------------------------------
// collide plane routines start here
void 
ShapeInstance::collidePlaneParts(int dl, const Point3F & normal, float k,
                        const TMat3F * toShape, objectList & ol)
{
   AssertFatal(dl<fpShape->fDetails.size(),"TS::ShapeInstance::collideLineParts:  detail level out of range");

   Point3F tn;
   float tk;
   if (toShape)
   {
      m_mul(normal,(RMat3F&)*toShape,&tn);
      tk = k*m_dot(tn,tn) + m_dot(tn,toShape->p);
   }
   else
   {
      tn=normal;
      tk=k;
   }

   getDetail(dl)->collidePlaneParts(tn,tk,ol,false);
}

//---------------------------------------------------------------------------
bool 
ShapeInstance::collidePlane(int dl, const Point3F &normal, float k,
                     const TMat3F * toShape, CollisionSurfaceList * list,
                     bool faceData)
{
   Point3F tn;
   float tk;
   int i;

   if (toShape)
   {
      m_mul(normal,(RMat3F&)*toShape,&tn);
      tk = k*m_dot(tn,tn) + m_dot(tn,toShape->p);
   }
   else
   {
      tn=normal;
      tk=k;
   }

   list->clear();
   potentialObjList.clear();

   collidePlaneParts(dl,tn,tk,0,potentialObjList);

   if (faceData)
   {
      // do detailed intersection of objs w/ plane
      // report one intersection (deepest most one) per intersecting part
      list->increment();
      memset(&(list->last()), 0, sizeof(CollisionSurface));
      for (i=0;i<potentialObjList.size();i++)
      {
         CollisionSurface & cs = list->last();
         if ( potentialObjList[i].pObj->collidePlane(cs,potentialObjList[i]) )
         {
            list->increment();
            memset(&(list->last()), 0, sizeof(CollisionSurface));
         }
      }
      list->decrement(); // always have one too many
   }
   else
   {
      // don't want face data, just copy each object w/ colliding bbox onto collisionInfoList
      for (i=0;i<potentialObjList.size();i++)
      {
         // fill out 1 collisionInfoList entry
         list->increment();
         memset(&(list->last()), 0, sizeof(CollisionSurface));
         CollisionSurface & cs = list->last();
         ObjectInfo & oi = potentialObjList[i];
         cs.part = static_cast<ShapeObjectInstance *>(oi.pObj)->fObject.getIndex(getShape());
         cs.position = Point3F(0,0,0);
         cs.normal   = Point3F(0,0,1);
         cs.material = 0;
      }
   }
   return !list->empty();
}

//---------------------------------------------------------------------------
// collide Tube routines start here
void 
ShapeInstance::collideTubeParts(int dl, const Point3F & a, const Point3F & b,float radius,
                        const TMat3F * toShape,objectList & ol)
{
   AssertFatal(dl<fpShape->fDetails.size(),"TS::ShapeInstance::collideTubeParts:  detail level out of range");

   Point3F ta,tb;
   if (toShape)
   {
      m_mul(a,*toShape,&ta);
      m_mul(b,*toShape,&tb);
   }
   else
   {
      ta=a;
      tb=b;
   }
   
   getDetail(dl)->collideTubeParts(ta,tb,radius,ol,false);
}

//---------------------------------------------------------------------------
bool 
ShapeInstance::collideTube(int dl, const Point3F &a, const Point3F & b,float radius,
                           const TMat3F * toShape, CollisionSurfaceList * list, bool faceData)
{
   // transform line endpoints here so we have them below when figuring hitpoint
   Point3F ta,tb;
   m_mul(a,*toShape,&ta);
   m_mul(b,*toShape,&tb);

//   list->clear();

   potentialObjList.clear();
   collideTubeParts(dl,ta,tb,radius,0,potentialObjList);

   // sort by front intersection point
   int i;
   sortedObjListPtrs.clear();
   for (i=0;i<potentialObjList.size();i++)
      sortedObjListPtrs.push_back(&(potentialObjList[i]));
   sortedObjListPtrs.sort();

   if (faceData)
   {
      // do detailed intersection of objs w/ tube, quitting when we find an intersection point
      // that is in front of the frontmost point of the next object
      list->increment();
      memset(&(list->last()), 0, sizeof(CollisionSurface));
      CollisionSurface & cs = list->last();
      float & hitTime = cs.time;
      hitTime=1.1f;
      i=0;
      bool gotOne = false;
      while ( i<potentialObjList.size() && hitTime>sortedObjListPtrs[i]->hitTime )
      {
         gotOne |= sortedObjListPtrs[i]->pObj->collideTube(cs,*sortedObjListPtrs[i]);
         i++;
      }

      if (gotOne)
         return true;
      list->clear();
      return false;
   }

   // don't want face data, just copy closest object hit onto collisionInfoList
   if (!potentialObjList.empty())
   {
      // fill out 1 collisionInfoList entry
      list->increment();
      memset(&(list->last()), 0, sizeof(CollisionSurface));
      CollisionSurface & cs = list->last();
      ObjectInfo & oi = potentialObjList[0];
      cs.part = static_cast<ShapeObjectInstance *>(oi.pObj)->fObject.getIndex(getShape());
      cs.time = oi.hitTime;
      cs.position  = tb;
      cs.position -= ta;
      cs.position *= cs.time;
      cs.position += ta;
		cs.material = 0;
      // normal?
      return true;
   }
   return false;
}

//---------------------------------------------------------------------------
// sphere collision routines start here
void 
ShapeInstance::collideSphereParts(int dl, const Point3F & center,float radius,
                          const TMat3F * toShape,objectList & ol)
{
   AssertFatal(dl<fpShape->fDetails.size(),"TS::ShapeInstance::collideSphereParts:  detail level out of range");

   Point3F tcenter;
   if (toShape)
      m_mul(center,*toShape,&tcenter);
   else
      tcenter=center;

   getDetail(dl)->collideSphereParts(tcenter,radius,ol,false);
}

//---------------------------------------------------------------------------
bool 
ShapeInstance::collideSphere(int dl, const Point3F &center,float radius,
                             const TMat3F * toShape, CollisionSurfaceList * list, bool faceData)
{
//   list->clear();

   potentialObjList.clear();
   collideSphereParts(dl,center,radius,toShape,potentialObjList);

   if (faceData)
   {
      //  no sorting because we want all collision faces
      int i;
      for (i=0;i<potentialObjList.size();i++)
      {
         ObjectInfo & oi = potentialObjList[i];
         oi.pObj->collideSphere(*list,oi);
      }

      return !list->empty();
   }

   // don't want face data, transfer items from object list to collisionInfoList
   for (int j=0;j<potentialObjList.size();j++)
   {
      ObjectInfo & oi = potentialObjList[j];
      ShapeObjectInstance * hitPart = static_cast<ShapeObjectInstance *>(oi.pObj);

      // part bbox
      Box3F box;
      hitPart->getBox(this,box);
      Point3F radii = box.fMax - box.fMin; radii *= 0.5f;
      Point3F boxCenter = box.fMax + box.fMin; boxCenter *= 0.5f;

      // sphere center in box space
      Point3F sphereCenter = oi.pointA - boxCenter;

      // loop through and add collision surfaces for x,y, and z faces (1 face/coord)
      for (int k=0; k<3; k++)
      {
         float centerCoord, coordRadius;
         Point3F normal, pos;
         // set up x,y,z coord cases
         switch (k)
         {
            case 0 : // x
               centerCoord = sphereCenter.x;
               normal.set( 1.0f, 0.0f, 0.0f);
               coordRadius = radii.x;
               break;

            case 1 : // y
               centerCoord = sphereCenter.y;
               normal.set( 0.0f, 1.0f, 0.0f);
               coordRadius = radii.y;
               break;

            case 2 : // z
               centerCoord = sphereCenter.z;
               normal.set( 0.0f, 0.0f, 1.0f);
               coordRadius = radii.z;
               break;
         }

         // don't add this dimension if sphere projects
         // inside box when this axis is removed
         if (fabs(centerCoord) + radius < coordRadius)
            continue;

         list->increment();
         CollisionSurface & cs = list->last();
         memset(&(list->last()), 0, sizeof(CollisionSurface));

         if (centerCoord<0)
            normal *= -1.0f;
         
         pos = normal;
         pos *= radius;
         pos += boxCenter;

         m_mul(pos,oi.objTrans,&cs.position);
         cs.distance = m_distf(cs.position, center); 
         m_mul(normal,(RMat3F &)oi.objTrans,&cs.normal);

         cs.part = hitPart->fObject.getIndex(getShape());
		   cs.material = 0;
      }
   }
   return (!list->empty());
}

//---------------------------------------------------------------------------
// box collision routines start here
void 
ShapeInstance::collideBoxParts(int dl, const Point3F & radii, const TMat3F * toShape, objectList & ol)
{
   AssertFatal(dl<fpShape->fDetails.size(),"TS::ShapeInstance::collideBoxParts:  detail level out of range");
   AssertFatal(toShape,"TS::ShapeInstance::collideBoxParts:  transform into shape space must be supplied");

   // box center at origin of domain space of toShape transform
   getDetail(dl)->collideBoxParts(radii,*toShape,ol,false);
}

//---------------------------------------------------------------------------
bool 
ShapeInstance::collideBox(int dl, const Point3F &radii,
                          const TMat3F * toShape, CollisionSurfaceList * list, bool faceData)
{
   AssertFatal(!faceData,"TS::ShapeInstance::collideBox:  detailed box collision not implemented");

   list->clear();

   potentialObjList.clear();
   collideBoxParts(dl,radii,toShape,potentialObjList);

   // transfer items from object list to collisionInfoList
   for (int j=0;j<potentialObjList.size();j++)
   {
      list->increment();
      memset(&(list->last()), 0, sizeof(CollisionSurface));
      CollisionSurface & cs = list->last();
      ObjectInfo & oi = potentialObjList[j];
      cs.part = static_cast<ShapeObjectInstance *>(oi.pObj)->fObject.getIndex(getShape());
		cs.material = 0;
   }
   return (!list->empty());
}

//---------------------------------------------------------------------------
// the shape to shape collision routine
// this one works a bit different...first we find all objects on other shape that might collide w/ us,
// then we check each of these objects against each of our own objects...
void 
ShapeInstance::collideShapeParts(int dl, int otherDL, ShapeInstance & otherShape, 
                         const TMat3F * toThisShape, objectList & ol)
{
   AssertFatal(dl<fpShape->fDetails.size(),"TS::ShapeInstance::collideShapeParts:  detail level out of range");
   AssertFatal(toThisShape,"TS::ShapeInstance::collideShapeParts:  transform into shape space must be supplied");

   // we need to account for root delta transform
   TMat3F rootInv = fRootDeltaTransform;
   rootInv.inverse();
   TMat3F toShape2;
   m_mul(*toThisShape,rootInv,&toShape2);

   // borrow the other shapes object list...
   objectList & otherOL = otherShape.potentialObjList;
   otherOL.clear();

   // we need to go both ways...wink, wink
   TMat3F toThatShape = *toThisShape;
   toThatShape.inverse();

   // for now we'll collide objects w/ a sphere, eventually we'd do better w/ a box
   otherShape.collideSphereParts(otherDL,fpShape->fCenter,fpShape->fRadius,&toThatShape,otherOL);

   if (otherOL.empty())
      return;

   // now pass otherOL on to each object of this shape to see if there's a collision
   getDetail(dl)->
      collideShapeParts(toThatShape,otherOL,ol,false);
}

//---------------------------------------------------------------------------
bool 
ShapeInstance::collideShape(int dl,int otherDL, ShapeInstance & otherShape,
                            const TMat3F *toThisShape, CollisionSurfaceList * list, bool faceData)
{
   AssertFatal(!faceData,"TS::ShapeInstance::collideShape:  shape face to shape face collisions not implemented");

   list->clear();
   potentialObjList.clear();

   collideShapeParts(dl,otherDL,otherShape,toThisShape,potentialObjList);

   // got some hits, move info over to collisionInfoList
   for (int i=0;i<potentialObjList.size();i++)
   {
      list->increment();
      memset(&(list->last()), 0, sizeof(CollisionSurface));
      CollisionSurface & cs = list->last();
      ObjectInfo & oi = potentialObjList[i];
      cs.part = static_cast<ShapeObjectInstance *>(oi.pObj)->fObject.getIndex(getShape());
      cs.surface = static_cast<ShapeObjectInstance *>(oi.pObj2)->fObject.getIndex(getShape());
		cs.material = 0;
   }
   return !potentialObjList.empty();
}

//---------------------------------------------------------------------------
void 
ShapeInstance::getPolys( int dl, unpackedFaceList &fl )
{
   AssertFatal(dl<fpShape->fDetails.size(),"TS::ShapeInstance::getPolys:  detail level out of range");

   potentialObjList.clear();
   fl.clear();
   getDetail(dl)->getObjects(potentialObjList,false);
   for (int i=0;i<potentialObjList.size();i++)
   {
      ObjectInfo & oi = potentialObjList[i];
      oi.pObj->getPolys( this, &oi.objTrans, fl );
   }
}

//---------------------------------------------------------------------------
void 
ShapeInstance::getObjects( int dl, objectList &ol )
{
   AssertFatal(dl<fpShape->fDetails.size(),"TS::ShapeInstance::getObjects:  detail level out of range.");
   getDetail(dl)->getObjects(ol,false);
}

//---------------------------------------------------------------------------
void ShapeInstance::render( RenderContext & rc )
{
   // if we're mapping a single material onto all surfaces,
   // ensure that the RGB's in the material have been
   // resolved to palette indices
   if (fAlwaysMat)
   {
      GFXPalette *pal = rc.getSurface()->getPalette();
      if ( fAlwaysMat->RGB_NeedsRemap(pal) )
         fAlwaysMat->mapRGBtoPal(pal);
   }

   if (fDetailLevel != -1) 
   {
      // should already be animated.
      rc.getCamera()->pushTransform( fRootDeltaTransform );
      getDetail()->render( this, rc, FALSE );
      
      if ( fAlwaysNode )
         fAlwaysNode->render( this, rc, FALSE );
      
      rc.getCamera()->popTransform();
   }         
}

//------------------------------------------------------------------------------
void ShapeInstance::setDetailScale(float scale)
{
   detailScale = max(0.0f, scale);
}   

//------------------------------------------------------------------------------
void ShapeInstance::setPerspectiveScale(float scale)
{
   perspectiveScale = max(0.0f, scale);
   fDrawFlatPerspective = IsZero(perspectiveScale) ? false : true;
}   

//---------------------------------------------------------------------------
int ShapeInstance::getNodeAtCurrentDetail(const char * nodeName)
{
   if( fDetailLevel != -1 ){
      
      const TSShape * pShape = & getShape();
      
      AssertFatal ( fDetailLevel < pShape->fDetails.size(), "Inst: Bad Detail" );
      
      // Look up the projected distance for this detail, make a name out of 
      //    it, and find that node.  
      
      float floatDet = pShape->fDetails[ fDetailLevel ].fSize;
      int   intDet = int(floatDet);
      
      AssertFatal( float(intDet) == floatDet, "Want integer detail size" );
      AssertFatal( strlen(nodeName) < 50, avar("%s too much node",nodeName) );

      char  tempName[60];
      strcpy( tempName, nodeName );
      itoa( intDet, tempName + strlen(tempName), 10 );
      
      return pShape->findNode( tempName );
   }
   
   return -1;
}

//---------------------------------------------------------------------------
void ShapeInstance::setDetailLevel(int detailLevel)
{
   AssertFatal(getShape().fDetails.size() > detailLevel, 
      avar("Bad detail Level: %d", detailLevel));

   fDetailLevel = detailLevel;      
   fDrawFlatPerspectiveDetail = (fDrawFlatPerspective && (fDetailLevel == 0));
}

//---------------------------------------------------------------------------

ShapeInstance::NodeInstance * 
ShapeInstance::getDetail(int dl)
{
   if (dl == -1)
      return NULL;
   else if (fUseDetails)
      return fNodeInstanceList[fpShape->getDetailRootNodeIndex(dl)];
   else
   {
      int i = dl;
      NodeInstance * detailNode = fNodeInstanceList[1];
      while (i-- && detailNode->fpNext)
         detailNode = detailNode->fpNext;
      return detailNode;
   }
}


//---------------------------------------------------------------------------

int ShapeInstance::minDetail = -1;
int ShapeInstance::maxDetail = 1000;

int 
ShapeInstance::selectDetail( RenderContext & rc, int minDetail, int maxDetail )
{
   if (minDetail==-1)
      minDetail = ShapeInstance::minDetail;

   if (maxDetail==-1)
      maxDetail = ShapeInstance::maxDetail;

   // decide which detail to use, based on camera position
   // and any imposed limits:
   // (root should already be animated.)

   // transform and project bounding sphere:
   rc.getCamera()->pushTransform( fRootDeltaTransform );
   RealF projectedSize = detailScale * rc.getCamera()->transformProjectRadius( fpShape->fCenter, fpShape->fRadius );
   projectedSize *= rc.getCamera()->getPixelScale();
   rc.getCamera()->popTransform();

   // select the detail based on the projected size:
   fDetailLevel = fpShape->selectDetail( projectedSize );

   // limit as indicated:
   if ( fDetailLevel != -1 )
   {
      if( fDetailLevel < minDetail )
         fDetailLevel = minDetail;
      if( fDetailLevel > maxDetail )
         fDetailLevel = maxDetail;
   }

   // make sure we're not out of range
   int numD = getShape().fDetails.size();
   if (fDetailLevel >= numD)
      fDetailLevel = numD-1;

   if (fDrawFlatPerspective)
   {  // perspective is enabled if the projected radius is greater than
      // some percentage of the screen width
      float size = (1.0-perspectiveScale)*rc.getSurface()->getWidth();
      fDrawFlatPerspectiveDetail = (projectedSize > size) ? true : false;
   }

   else
      fDrawFlatPerspectiveDetail = false;

   return fDetailLevel;
}

//---------------------------------------------------------------------------
ShapeInstance::Thread * 
ShapeInstance::CreateThread()
{
   AssertFatal(fUseAnimation,"TSShapeInstance::CreateThread:  shape does not support animation.");

   if (fpShape->fSequences.size() == 0) {
      return NULL;
   } else {
      Thread *pThread = new Thread( *this );
      fThreadList.insertThread( pThread );
      return pThread;
   }
}

//---------------------------------------------------------------------------
void 
ShapeInstance::DestroyThread( Thread * pThread )
{
   fThreadList.removeThread( pThread );
   delete pThread;
}

//---------------------------------------------------------------------------
ShapeInstance::Thread * 
ShapeInstance::selectThread( int index )
{
   return fThreadList.selectThread( index );
}

//---------------------------------------------------------------------------
void 
ShapeInstance::UpdateSequenceSubscriberLists()
{
   // this method must be called any time the nodes or sequences change

   // reset all sequences:
   for( int si = 0; si < fSequenceSubscriberLists.size(); si++ )
      fSequenceSubscriberLists[si]->clear();

   // make sure the list is the right size (in case # of seqs changed)
   if (fSequenceSubscriberLists.size() > fpShape->fSequences.size())
   {
      // delete excess entries
      for (int i=fpShape->fSequences.size();i<fSequenceSubscriberLists.size();i++)
         delete fSequenceSubscriberLists[i];
      fSequenceSubscriberLists.setSize( fpShape->fSequences.size() );
   }
   else if (fSequenceSubscriberLists.size() < fpShape->fSequences.size())
   {
      int oldSize=fSequenceSubscriberLists.size();
      fSequenceSubscriberLists.setSize( fpShape->fSequences.size() );
      for (int i=oldSize; i < fpShape->fSequences.size(); i++)
         fSequenceSubscriberLists[i] = new IndexList;
   }

   // 'register' each node with each sequence that the node uses
   for( int ni = 0; ni < fpShape->fNodes.size(); ni++ )
   {
      Shape::Node const & node = fpShape->getNode( ni );
      for( int si = 0; si < node.fnSubSequences; si++ )
      {
         Shape::SubSequence const & nodeseq = 
            fpShape->getSubSequence( node.fFirstSubSequence + si );
         fSequenceSubscriberLists[nodeseq.fSequenceIndex]->add( ni );
      }
   }
   // 'register' each object with each sequence that the object uses
   for( int oi = 0; oi < fpShape->fObjects.size(); oi++ )
   {
      Shape::Object const & object = fpShape->getObject( oi );
      for( int si = 0; si < object.fnSubSequences; si++ )
      {
         Shape::SubSequence const & objseq = 
            fpShape->getSubSequence( object.fFirstSubSequence + si );
         fSequenceSubscriberLists[objseq.fSequenceIndex]->add( oi + 0x10000 );
      }
   }
}

//---------------------------------------------------------------------------
IndexList const & 
ShapeInstance::getSequenceSubscriberList( Shape::Sequence const *pSeq ) const
{
   return *fSequenceSubscriberLists[pSeq->getIndex( *fpShape )];
}

void ShapeInstance::insertOverride(const char* name,int x,TMat3F* mat)
{
	int len = strlen(name);
	for (int i = 0; i < fNodeInstanceList.size(); i++) {
		NodeInstance* ni = fNodeInstanceList[i];
		if (!strncmp(fpShape->fNames[ni->fNode.fName],name,len))
			ni->fpMatOverride[x] = mat;
	}
}	

void ShapeInstance::resetActiveFlag()
{
	for (int i = 0; i < fNodeInstanceList.size(); i++) {
		NodeInstance& ni = *fNodeInstanceList[i];
	   for (int b = 0; b < ni.fObjectList.size(); b++)
	      if (ShapeObjectInstance * object = 
		         dynamic_cast<ShapeObjectInstance*>(ni.fObjectList[b]))
	         object->fActive = true;
	}
}


//---------------------------------------------------------------------------
}; // namespace TS
