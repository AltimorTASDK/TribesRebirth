//---------------------------------------------------------------------------

//	$Workfile:   ts_mesh.h  $
//	$Revision:   2.8  $
// $Version$
//	$Date:   15 Sep 1995 10:27:12  $
//	$Log:   R:\darkstar\develop\ts\vcs\ts_mesh.h_v  $
//	

//---------------------------------------------------------------------------

#ifndef _TS_CELANIMMESH_H_
#define _TS_CELANIMMESH_H_

#include <persist.h>
#include <ts_PointArray.h>
#include <ts_RenderItem.h>
#include <ts_Material.h>
#include <ts_shape.h>

namespace TS
{

class ShapeInstance;

   //---------------------------------------------------------------------------
   //------------------------------------------------------------------

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

   //------------------------------------------------------------------
   class CelAnimMesh : public Shape::Mesh
      {
      //----------------------------------------------------
      // Persistent Methods:
   public:
      DECLARE_PERSISTENT(CelAnimMesh);
      int                    version();
      Persistent::Base::Error read( StreamIO &, int version, int user = 0 );
      Persistent::Base::Error write( StreamIO &, int version, int user = 0 );
   private:
      //
      //----------------------------------------------------

   public:
      class Face
         {
      public:
         VertexIndexPair   fVIP[3];
         Int32             fMaterial;
         };

      class v2Frame
         {
      public:
         Int32             fFirstVert; // min vert, max vert, other verts
         };

      class Frame
         {
      public:
         Int32             fFirstVert; // min vert, max vert, other verts
			Point3F				fScale;
			Point3F				fOrigin;
         };

public:
      Int32                fnVertsPerFrame;
      Int32                fnTextureVertsPerFrame;

      Vector<PackedVertex> fVerts;
      Vector<Point2F>      fTextureVerts;
      Vector<Face>         fFaces;
      Vector<Frame>        fFrames;

      RealF                fRadius;    // for clipping sphere

   protected:
      void      validateFace( //RenderContext &rc, 
      										  MaterialList const & mats,
      										  Face const & face, int matFrameIndex ) const;
      int       faceVisibility( PointArray * pa, Face const & face, 
                           int first ) const;

		enum
		{
			v1Recycled = 1,
			v2Recycled = 2,
			v3Recycled = 4,
			e1Recycled = 8,
			e2Recycled = 16,
			e3Recycled = 32
		};
		Vector<Int8>			faceReuseFlags;

   public:
      void      render( RenderContext & rc, const ShapeInstance *shape,
      								  MaterialList const & mats, int frame, 
										  int matFrameIndex, int *matRemap ) const;
		int  		importFrame( const Shape::Mesh & fromMesh, int fromFrameIndex);
		void 	setCommonScale( Shape::Mesh & otherMesh );
		bool 		collideLineBox( int frameIndex, const Point3F & a, const Point3F & b,
												 float & hitTime) const;
		bool 		collideLine( int frameIndex, const Point3F & a, const Point3F & b,
												 CollisionSurface & cs, float minTime) const;
		bool 		collidePlaneBox( int frameIndex, const Point3F & normal, float k ) const;
		bool 		collidePlane( int frameIndex, const Point3F & normal, float k,
												 CollisionSurface & cs) const;
		bool 		collideTubeBox( int frameIndex, const Point3F & a, const Point3F & b, float rad,
												 float & hitTime) const;
		bool 	collideTube( int frameIndex, const Point3F & a, const Point3F &b, float radius,
												CollisionSurface & cs, float minTime) const;
		bool 		collideSphereBox( int frameIndex, const Point3F & center, float radius ) const;
		bool 		collideSphere( int frameIndex, const Point3F & center, float radius,
												 CollisionSurfaceList & csl ) const;
		bool 	collideShapeBox(int frameIndex, TMat3F & objToOtherShape,
												 objectList &otherOL,
												 objectList & thisOL) const;
		bool 	collideBoxBox(int frameIndex,
		                                  TMat3F &trans,
		                                  const Point3F & bRadii,
		                                  Point3F & aOverlap,
		                                  Point3F & bOverlap) const;
		bool 	collideBox(int frameIndex,
	                                   const TMat3F & trans,       // from box to mesh space (box center origin)
                                       const TMat3F & invTrans,    // from mesh to box space
	                                   const Point3F &radii,
	                                   CollisionSurface & cs) const;

		void 		getPolys(int frameIndex, int matFrameIndex,
		                             const TMat3F * objToShape, unpackedFaceList & fl) const;

		void 	getBox( int frameIndex, Box3F & box) const;

      CelAnimMesh();
      ~CelAnimMesh();
      };

   //------------------------------------------------------------------
#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif


};  // namespace TS

#endif
