//---------------------------------------------------------------------------

//	$Workfile:   ts_mesh.h  $
//	$Revision:   2.8  $
// $Version$
//	$Date:   15 Sep 1995 10:27:12  $
//	$Log:   R:\darkstar\develop\ts\vcs\ts_mesh.h_v  $
//	

//---------------------------------------------------------------------------

#ifndef _TS_VERTEX_H_
#define _TS_VERTEX_H_

#include <ml.h>
#include <ts_types.h>

namespace TS
{
   //---------------------------------------------------------------------------

   class RenderContext;

   //---------------------------------------------------------------------------

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

   class PackedVertex;

   class Vertex
      {
   public:
      Point3F           fPoint;
      Point3F           fNormal;
      void      interpolate( Vertex const &src1, Vertex const &src2, 
                           RealF interpParam );
      void      set( PackedVertex const & pv, Point3F const &scale, 
                           Point3F const &origin );
      };

   class VertexIndexPair
      {
   public:
      Int32    fVertexIndex;
      Int32    fTextureIndex;
      };

   class TransformedVertex
      {
   public:
      enum VertexStatus 
         {
         Lit         = 0x1000,
         Projected   = 0x2000,
      	};
   	int            fStatus;       // clip codes + VertexStatus flags
      Point3F        fPoint;        // projected point
      Point4F        fTransformedPoint; // pre-projection transformed point.
      Point2F        fTextureCoord;
      float          fDist;         // euclidian distance from camera point
      GFXColorInfoF  fColor;        // light intensity at a point

      void calcLighting( RenderContext &rc, Vertex const &vert );
	   };

   class PackedVertex
      {
      static Point3F    fNormalTable[];

   public:
      UInt8              x,y,z;         // scaled point
      UInt8             normal;        // encoded vertex normal

      void      set( Vertex const &vert, Point3F const &scale, 
                           Point3F const &origin );

		void setPoint( const Point3F &p, Point3F const & scale,
									Point3F const &origin );

		void getPoint( Point3F &p, Point3F const & scale,
									Point3F const &origin ) const;


      Point3F const & decodeNormal() const;
      UInt8           encodeNormal( Point3F const &norm );
      };

	struct Face
	{
		VertexIndexPair fVIP[3];
		Int32 material;
	};

	struct unpackedFaceList
	{
		Vector<Face>		fFaces;
		Vector<Point3F>	fPoints;
		Vector<Point2F>	fTextures;
		void clear() { fFaces.clear(); fPoints.clear(); fTextures.clear(); }
	};



#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

   //---------------------------------------------------------------------------

   inline void Vertex::interpolate( Vertex const &src1, 
      Vertex const &src2, RealF interpParam )
      {
      fPoint.interpolate( src1.fPoint, src2.fPoint, interpParam );
      fNormal.interpolate( src1.fNormal, src2.fNormal, interpParam );
      }

   //---------------------------------------------------------------------------

   inline void Vertex::set( PackedVertex const & pv, 
      Point3F const & scale, Point3F const & origin )
      {
      fPoint.x = pv.x * scale.x + origin.x;
      fPoint.y = pv.y * scale.y + origin.y;
      fPoint.z = pv.z * scale.z + origin.z;
      fNormal = pv.decodeNormal();
      }

   //---------------------------------------------------------------------------

   inline Point3F const & PackedVertex::decodeNormal() const
      {
      return fNormalTable[normal];
      }

   //---------------------------------------------------------------------------

   inline void PackedVertex::set( Vertex const &vert,
         Point3F const &scale, Point3F const & origin )
      {
      Point3F temp = vert.fPoint;
      temp -= origin;
      temp /= scale;
      x = UInt8(temp.x);
      y = UInt8(temp.y);
      z = UInt8(temp.z);
      normal = encodeNormal( vert.fNormal );
      }

   inline void PackedVertex::setPoint( const Point3F & p, 
         Point3F const &scale, Point3F const & origin )
	{
      Point3F temp = p;
      temp -= origin;
      temp /= scale;
      x = UInt8(temp.x);
      y = UInt8(temp.y);
      z = UInt8(temp.z);
	}

   inline void PackedVertex::getPoint( Point3F & p,
         Point3F const &scale, Point3F const & origin ) const
	{
      p.x = x * scale.x + origin.x;
      p.y = y * scale.y + origin.y;
      p.z = z * scale.z + origin.z;
	}

   //---------------------------------------------------------------------------
};


#endif
