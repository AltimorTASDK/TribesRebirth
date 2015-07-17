//---------------------------------------------------------------------------

//	$Workfile:   ts_mesh.h  $
//	$Revision:   2.8  $
// $Version$
//	$Date:   15 Sep 1995 10:27:12  $
//	$Log:   R:\darkstar\develop\ts\vcs\ts_mesh.h_v  $
//	

//---------------------------------------------------------------------------

#ifndef _TS_TRANSANIMMESH_H_
#define _TS_TRANSANIMMESH_H_

#include <persist.h>
#include <ts_RenderItem.h>
#include <ts_Material.h>
#include <ts_PointArray.h>
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

	class TransAnimMesh : public Shape::Mesh
		{
      //----------------------------------------------------
      // Persistent Methods:
   public:
      DECLARE_PERSISTENT(TransAnimMesh);
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

      class Frame
         {
      public:
         Int32             fFirstVert; // min vert, max vert, other verts
         };

public:
      Int32                fnVertsPerFrame;

		Vector<TransformedVertex> fCooked;
      Vector<Point2F>      fTextureVerts;
      Vector<Face>         fFaces;
      Vector<Frame>        fFrames;

      RealF                fRadius;    // for clipping sphere

   protected:
      void      validateFace( //RenderContext &rc, 
                           MaterialList const & mats, Face const & face ) const;
      int       faceVisibility( RenderContext &rc, Face const & face, 
                           int first ) const;

   public:
      void      render( RenderContext & rc, const ShapeInstance *shapeInst,
      								  MaterialList const & mats, int frame, int ) const;
		int  importFrame( const Shape::Mesh & fromMesh, int fromFrameIndex);
		void setCommonScale( Shape::Mesh & otherMesh);
		bool collideLineBox( int frameIndex, const Point3F & a, const Point3F & b, float & hitTime) const
				 { return false; }
		bool collideLine( int frameIndex, const Point3F & a, const Point3F & b, CollisionSurface & cs, float minTime) const
				 { return false; }
		bool collideTubeBox( int frameIndex, const Point3F & a, const Point3F & b, float radius, float & hitTime ) const
				 { return false; }
		bool collideTube( int frameIndex, const Point3F & a, const Point3F & b, float radius, CollisionSurface & cs, float minTime ) const
				 { return false; }
		bool collideSphereBox( int frameIndex, const Point3F & center, float radius ) const
				 { return false; }
		bool collideSphere( int frameIndex, const Point3F & center, float radius,
												 CollisionSurfaceList & csl ) const
				 { return false; }
		bool collideShapeBox(int frameIndex, TMat3F & objToOtherShape,
												 objectList &otherOL,
												 objectList & thisOL) const
				 { return false; }
		bool collideBoxBox(int frameIndex, TMat3F &trans, const Point3F & bRadii,
                                   Point3F & aOverlap, Point3F & bOverlap) const
   				 { return false; }

		void getBox( int frameIndex, Box3F & box)
				 	 { frameIndex,box; }

		void getPolys(int frameIndex, int matFrameIndex,
		                      const TMat3F * objToShape, unpackedFaceList & fl) const
				{ objToShape,fl; }

      TransAnimMesh();
      ~TransAnimMesh();
      };


   class TransAnimSubMesh : public Shape::Mesh
      {
      //----------------------------------------------------
      // Persistent Methods:
   public:
      DECLARE_PERSISTENT(TransAnimSubMesh);
      int                    version();
      Persistent::Base::Error read( StreamIO &, int version, int user = 0 );
      Persistent::Base::Error write( StreamIO &, int version, int user = 0 );
   private:
      //
      //----------------------------------------------------

		class SubVertex
		{
		public:
			Vertex 	fVert;
			Int32    fVertIndex;		// into main mesh
			Int32    fWeight;
		};

	public:
		Int32						fMeshIndex;
		Vector<SubVertex>    fVerts;

   public:
      void      render( RenderContext & rc, const ShapeInstance *shapeInst,
      								  MaterialList const & mats, int frame,int ) const;
		int  importFrame( const Shape::Mesh & fromMesh, int fromFrameIndex);
		void setCommonScale( Shape::Mesh & otherMesh);
		bool collideLineBox( int frameIndex, const Point3F & a, const Point3F & b, float & hitTime) const
				 { return false; }
		bool collideLine( int frameIndex, const Point3F & a, const Point3F & b, CollisionSurface & cs, float minTime) const
				 { return false; }
		bool collideTubeBox( int frameIndex, const Point3F & a, const Point3F & b, float radius, float & hitTime ) const
				 { return false; }
		bool collideTube( int frameIndex, const Point3F & a, const Point3F & b, float radius, CollisionSurface & cs, float minTime ) const
				 { return false; }
		bool collideSphereBox( int frameIndex, const Point3F & center, float radius ) const
				 { return false; }
		bool collideSphere( int frameIndex, const Point3F & center, float radius,
												 CollisionSurfaceList & cil ) const
				 { return false; }
		bool collideShapeBox(int frameIndex, TMat3F & objToOtherShape,
												 objectList &otherOL,
												 objectList & thisOL) const
				 { return false; }
		bool collideBoxBox(int frameIndex, TMat3F &trans, const Point3F & bRadii,
                                   Point3F & aOverlap, Point3F & bOverlap) const
				 { return false; }

		void getBox( int frameIndex, Box3F & box)
				 	 { frameIndex,box; }

		void getPolys(int frameIndex, int matFrameIndex,
		                      const TMat3F * objToShape, unpackedFaceList & fl) const 
				{ objToShape,fl; }

      TransAnimSubMesh();
      ~TransAnimSubMesh();
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
