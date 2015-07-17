//---------------------------------------------------------------------------

// $Workfile:   ts_mesh.h  $
// $Revision:   2.8  $
// $Version$
// $Date:   15 Sep 1995 10:27:12  $
// $Log:   R:\darkstar\develop\ts\vcs\ts_mesh.h_v  $
// 

//---------------------------------------------------------------------------

#ifndef _TS_SHAPE_H_
#define _TS_SHAPE_H_

#include <ml.h>
#include <m_collision.h>

#include <persist.h>
#include <ts_Types.h>
#include <ts_Material.h>
#include <ts_transform.h>
//#include <ts_RenderContext.h>

class ResourceManager;

namespace TS
{
struct unpackedFaceList;
class  CollisionInfo;
class  MaterialList;
class  ShapeInstance;
class  ObjectInfo;
class  RenderContext;
//---------------------------------------------------------------------------
//------------------------------------------------------------------

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class ObjectInstance;

//--------------------------------------
//------------------------------------------------------------------------------
// a structure to keep track of potential collisions w/ objects
class ObjectInfo
{
public:
   TS::ObjectInstance * pObj;
   TS::ObjectInstance * pObj2; // used by collideShape
   TMat3F objTrans;       // from object to shape space
   TMat3F invObjTrans;    // from shape space to object space
   // the following 4 could be boiled down to 2 -- at least at the moment
   Point3F pointA;      // line/tube start, sphere center (in object space)
   Point3F pointB;      // line/tube end
   Point3F aOverlap;   // overlap vector of 1st box in box-box box-shape collisions
   Point3F bOverlap;   // overlap vector of 2nd box in box-box box-shape collisions
   float radius;
   float hitTime;
   int operator < (ObjectInfo const & oi) { return hitTime < oi.hitTime; }

   ObjectInfo() {}
};

typedef Vector<ObjectInfo> objectList;

class DLLAPI ObjectInstance
{
public:
   virtual void animate( ShapeInstance *pShapeInst ) {};
   virtual void render( ShapeInstance *pShapeInst, RenderContext & rc ) {};

   virtual bool collideLineObj( const TMat3F & trans, const Point3F & a, const Point3F & b,
                                ObjectInfo & oi ) { return false; }
   virtual bool collideLine( CollisionSurface&, ObjectInfo& ) { return false; }

   virtual bool collidePlaneObj( const TMat3F & trans, const Point3F & normal, float k,
                                ObjectInfo & oi ) { return false; }
   virtual bool collidePlane( CollisionSurface&, ObjectInfo& ) { return false; }

   virtual bool collideTubeObj( const TMat3F & trans, const Point3F & a, const Point3F & b, float radius,
                             ObjectInfo & oi ) { return false; }
   virtual bool collideTube( CollisionSurface &, ObjectInfo & ) { return false; }

   virtual bool collideSphereObj( const TMat3F & trans, const Point3F & center, float radius,
                                 ObjectInfo & oi ) { return false; }
   virtual bool collideSphere( CollisionSurfaceList &, ObjectInfo & ) { return false; }

   virtual bool collideBoxObj( const TMat3F & trans, const Point3F & radii, const TMat3F & boxToShape,
                           ObjectInfo & oi) { return false; }

   virtual bool collideBoxObj( TMat3F & trans, const Point3F & radii,
                                       Point3F & aOverlap, Point3F & bOverlap ) { return false; }

   virtual bool collideShapeObj( const TMat3F & nodeToShape, const TMat3F & shapeToOtherShape,
                              objectList & otherOL, objectList & thisOL) { return false; }

   virtual void getPolys( ShapeInstance *pShapeInst, const TMat3F * nodeToShape,
                                  unpackedFaceList & fl) {}

   virtual void getBox(ShapeInstance *pShapeInst, Box3F & box) {}

   virtual void getObject( const TMat3F & nodeToShape, objectList & ol) {}
};



//------------------------------------------------------------------------------
class DLLAPI Shape : public Persistent::VersionedBase
{
   //----------------------------------------------------
   // Persistent Methods:
  public:
   DECLARE_PERSISTENT(Shape);
   int                    version();
   Persistent::Base::Error read( StreamIO &, int version, int user = 0 );
   Persistent::Base::Error write( StreamIO &, int version, int user = 0 );
  private:
   //
   //----------------------------------------------------

  public:

   class Keyframe
   {
      RealF       fPosition;   
      // keyValue is:
      //  index into shapes transform array for Node SubSequences
      //  index into meshes frame array for Cel SubSequences
      UInt16      fKeyValue;
      // matIndex only holds 4 flags plus the index into meshes materials array
      // high bit = visibility,
      // 2nd highest bit = cares about visibility,
      // 3rd highest bit = cares about material,
      // 4th highest bit = cares about frame
      // remaing = materialIndex (not relavent for node subsequences)
      UInt16      fMatIndex;
     public:
      Keyframe( RealF position, int index, int mi, 
                Bool visMatters = FALSE, Bool visible = FALSE, 
                Bool matMatters = FALSE, Bool frameMatters = FALSE )
      {
         fPosition = position;
         fKeyValue = index;
         fMatIndex = mi;
         if( visible )
            fMatIndex |= 0x8000;
         if( visMatters )
            fMatIndex |= 0x4000;
         if( matMatters )
            fMatIndex |= 0x2000;
         if( frameMatters )
            fMatIndex |= 0x1000;
      }
      int         getVisibility() const { return fMatIndex & 0x8000; }
      int         usesVisibilityTrack() const { return fMatIndex & 0x4000; }
      int         usesMaterialTrack() const { return fMatIndex & 0x2000; }
      int         usesFrameTrack() const { return fMatIndex & 0x1000; }
      int         getIndex() const { return fKeyValue; }
      int         getMatIndex() const { return fMatIndex & 0x0fff; }
      RealF       getPosition() const { return fPosition; }
   };

   class Sequence
   {
     public:
      Int32       fName;
      Int32       fCyclic;
      RealF       fDuration;
      Int32       fPriority;

      Int32       fFirstFrameTrigger;
      Int32       fNumFrameTriggers;
      
      Int32       fNumIFLSubSequences;
      Int32       fFirstIFLSubSequence;
      
      void   handleORP( RealF *position, int *count ) const;
      void   buildNodeList( Shape const &shape, Vector<int> * pList ) const;
      int    getIndex( Shape const & shape ) const;
   };

   class SubSequence
   {
     public:
      Int16       fSequenceIndex;// index into shapes sequence array
      Int16       fnKeyframes;
      Int16       fFirstKeyframe;// index into shapes keyframe array

      Keyframe const &   findCelKey( Shape const &shape, RealF pos ) const;
      void   findTransformKeys( Shape const &shape, RealF pos, 
                        int *prevKey, int *nextKey, RealF *interpParam,
                        Bool *visible = 0 ) const;
      void   interpolateTransform( Shape const &shape, int prevKey,
                        int nextKey, RealF interpParam, Transform *pDest ) const;
      void   interpolateTransform( Shape const &shape, int prevKey,
                        int nextKey, RealF interpParam, TMat3F *pDest ) const;
   };

   class Transition
   {
     public:
      Int32       fStartSequence;   // index into shapes sequence array
      Int32       fEndSequence;     // index into shapes sequence array
      RealF       fStartPosition;
      RealF       fEndPosition;
      RealF       fDuration;
      Transform   fTransform;
   };

   class Node
   {
     public:
      Int16       fName;   // index into shapes name array
      Int16       fParent; // index into shapes node array
      Int16       fnSubSequences;  
      Int16       fFirstSubSequence;// index into shapes nodesequence array
      Int16       fDefaultTransform;   // index into shapes transform array

      SubSequence const * findSubSequence( Shape const & shape, int seqIndex ) const;
      int    getIndex( Shape const & shape ) const;
   };

   class Object
   {
     public:
      enum ObjectFlags { DefaultInvisible = 1 };
      Int16       fName;
      Int16       fFlags;
      Int32       fMeshIndex; // index in mesh list of the object
      Int16       fNodeIndex; // index of node to attach to
      // TMat3F      fObjectOffset; // offset relative to node
      Point3F     fObjectOffset; // offset relative to node
      Int16       fnSubSequences;
      Int16       fFirstSubSequence;

      TMat3F offsetTransform(void) const 
      {
         TMat3F   trans ( true );
         trans.p = fObjectOffset;
         trans.flags = TMat3F::Matrix_HasTranslation;
         return trans;
      }
      SubSequence const * findSubSequence( Shape const & shape, int seqIndex ) const;
      int    getIndex( Shape const & shape ) const;
   };

   class Detail
   {
     public:
      Int32       fRootNodeIndex;   // root node of this detail
      RealF       fSize;            // min projected size of this detail
   };

   class Mesh : public Persistent::VersionedBase
   {
     public:
      virtual ~Mesh() = 0;
      virtual void render( RenderContext &rc, 
                                   const ShapeInstance *shapeInst,
                                   MaterialList const & mats, 
                                   int frame,
                                   int matFrameIndex,
                                   int *matRemap ) const = 0;
      // test whether line collides with bounding box of this mesh
      virtual bool collideLineBox( int frameIndex, const Point3F & a, const Point3F & b, float & hitTime) const = 0;

      // more detailed collision check of line w/ mesh
      virtual bool collideLine( int frameIndex, const Point3F & a, const Point3F & b, CollisionSurface & cs, float minTime) const = 0;

      // test whether plane collides with bounding box of this mesh
      virtual bool collidePlaneBox( int frameIndex, const Point3F & normal, float k) const = 0;

      // more detailed collision check of plane w/ mesh
      virtual bool collidePlane( int frameIndex, const Point3F & normal, float k, CollisionSurface & cs) const = 0;

      // test whether tube coolides with bounding box of this mesh
      virtual bool collideTubeBox( int frameIndex, const Point3F & a, const Point3F & b, float radius, float & hitTime ) const = 0;

      // more detailed collision check of tube w/ mesh
      virtual bool collideTube( int frameIndex, const Point3F & a, const Point3F & b, float radius, CollisionSurface & cs, float minTime ) const = 0;

      // test whether sphere collides with bounding box of this mesh
      virtual bool collideSphereBox( int frameIndex, const Point3F & center, float radius ) const = 0;

      // more detailed collision check of line w/ mesh
      virtual bool collideSphere( int frameIndex, const Point3F & center, float radius,
                                  CollisionSurfaceList & csl ) const = 0;

      virtual bool collideShapeBox(int frameIndex,
                                 TMat3F & objToOtherShape,
                                 objectList & otherOL,
                                 objectList & thisOL) const = 0;

      virtual bool collideBoxBox(int frameIndex, 
                                         TMat3F &trans,
                                         const Point3F & bRadii,
                                         Point3F & aOverlap,
                                         Point3F & bOverlap) const = 0;

      virtual void getBox( int frameIndex, Box3F & box) const = 0;

      virtual void getPolys(int frameIndex, int matFrameIndex,
                                    const TMat3F * objToShape, unpackedFaceList & fl) const = 0;

      // importFrame imports 1 mesh frame from 'fromMesh' into this mesh (puts the
      // frame at the end of the frame list) and returns index to frame in this mesh
      // this function used when dynamically importing a sequence from one shape to another
      virtual int importFrame(const Mesh & fromMesh,int fromFrameIndex) = 0;

      // setCommonScale sets both meshes scales to be the same
      virtual void setCommonScale( Mesh & othermesh ) = 0;
   };
   
   // 
   class FrameTrigger
   {
      float fPosition;
      Int32 fValue;
     public:  
      FrameTrigger( float pos, int val, Bool forward );
      Bool isForward() const { return fPosition >= 0; }
      float getPosition() const 
            { return (fPosition < 0)? -fPosition : fPosition; }
      int getValue() const { return fValue; }
   };

  public:
   RealF                fRadius; // used for gross clipping and detail selection
   Point3F              fCenter;
   Box3F                fBounds; //version 8 addition

   Vector<Node>         fNodes;
   Vector<Sequence>     fSequences;
   Vector<Transition>   fTransitions;
   Vector<SubSequence>  fSubSequences;
   Vector<Keyframe>     fKeyframes;
   Vector<Transform>    fTransforms;
   Vector<Name>         fNames;
   Vector<Object>       fObjects;
   VectorPtr<Mesh *>    fMeshes;
   MaterialList *       fpMaterials;
   Vector<Detail>       fDetails;
   Vector<FrameTrigger> fFrameTriggers;

   Int32                fnDefaultMaterials;
   // always animate and draw hierarchy under this node, regardless
   //  of detail.  -1 means there isn't an always node.
   Int32                fAlwaysNode;      

  protected:
  public:
   Shape();
   ~Shape();

   // Detail levels prefixed with these names will be considered
   // utility details.  This means they will never be chosen by
   // select detail, but must be chosen explicitly (e.g., for
   // collision checking).
   static Vector<const char*> fUtilityDetails;
   void markUtilityDetails();

   // routines used for importing sequences from another shape (basic shape must be identical)
   void addSubSequence(int at,const SubSequence & newSS,Int16 & SScount);
   void importNodeKeyframe(const Shape *fromShape, int fromNum);
   void importObjectKeyframe(const Shape *fromShape, int fromNum, int meshNum);
   bool importSequence(const Shape *impShape, int impSeqNum); // returns false if can't do it

   int getSequenceIndex( const char *sequence ) const;
   int selectDetail( RealF projectedSize ) const;
   Keyframe const & findKey( RealF pos, int firstKey, 
                                       int numKeys, int seq ) const;

   int getDetailRootNodeIndex( int detailLevel ) const
   {
      // if there are no details, use single implicit detail:
      if( !fDetails.size() )
         return 1;
      return fDetails[detailLevel].fRootNodeIndex;
   }
   Node const & getNode( int index ) const
   {
      AssertFatal( index >= 0 && index < fNodes.size(),
            "TS::Shape::getNode: node index out of range" );
      return fNodes[index];
   }

   int findNode( char *name ) const;

   Sequence const & getSequence( int index ) const
   {
      AssertFatal( index >= 0 && index < fSequences.size(),
            avar("TS::Shape::getSequence: sequence index out of range: (s: %d, i: %d)",
                 fSequences.size(), index));
      return fSequences[index];
   }

   SubSequence const & getSubSequence( int index ) const
   {
      AssertFatal( index >= 0 && index < fSubSequences.size(),
            "TS::Shape::getSubSequence: subSequence index out of range" );
      return fSubSequences[index];
   }

   Keyframe const & getKeyframe( int index ) const
   {
      AssertFatal( index >= 0 && index < fKeyframes.size(),
            "TS::Shape::getKeyframe: keyframe index out of range" );
      return fKeyframes[index];
   }

   Transform const & getTransform( int index ) const
   {
      AssertFatal( index >= 0 && index < fTransforms.size(),
            "TS::Shape::getTransform: transform index out of range" );
      return fTransforms[index];
   }

   Object const & getObject( int index ) const
   {
      AssertFatal( index >= 0 && index < fObjects.size(),
            "TS::Shape::getObject: object index out of range" );
      return fObjects[index];
   }

   Mesh const * getMesh( int index ) const
   {
      AssertFatal( index >= 0 && index < fMeshes.size(),
            "TS::Shape::getMesh: mesh index out of range" );
      return fMeshes[index];
   }
      
   FrameTrigger const & getFrameTrigger( int index ) const
   {
      AssertFatal( index >= 0 && index < fFrameTriggers.size(),
            "TS::Shape::getFrameTrigger: object index out of range" );
      return fFrameTriggers[ index ];
   }        

   int          lookupName( const char *name ) const;
   const char * getName( int index );
   int findDetail( char *name ) const;

   void lockMaterialList( ResourceManager &rm, Bool block ) const
   {
      if( fpMaterials )
         fpMaterials->lock( rm, block );
   }

   void unlockMaterialList() const
   {
      if( fpMaterials )
         fpMaterials->unlock();
   }

   MaterialList const * getMaterialList() const
   {
      return fpMaterials;
   }

   void setMaterialList( MaterialList * pMats )
   {
      fpMaterials = pMats;
   }

   SphereF getBoundingSphere()
   {
      return SphereF(fCenter,fRadius);
   }
};


   //------------------------------------------------------------------
#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif


};  // namespace TS

//
typedef TS::Shape TSShape;

#endif
