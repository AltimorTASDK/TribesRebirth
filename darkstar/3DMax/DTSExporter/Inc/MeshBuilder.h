#ifndef MESHBUILDER_H_
#define MESHBUILDER_H_

#include <Max.h>
#include <Stdmat.h>

#include <ts_Shape.h>
#include <ts_CelAnimMesh.h>
#include <tvector.h>

#define MESH_CHANGES		1
#define MAT_CHANGES		2
#define VIS_CHANGES		4

// The code controlled by this defined is not well tested, so we'll leave it out of the
//    checked in code for 98.  
#define  LAST_98_ADDITION  0
void sDump(FILE * fp, const char * string, ...);

class MyTransform : public TS::V6Transform
{
public:
	int fMirrored;		// currently this is not used...
	
	void setMatrix( TMat3F *mat ) const
	{
	   mat->set( fRotate, fTranslate );
      mat->preScale( fScale );
	}	
};

//--------------------------------------------------------------

class ShapeBuilder;

class MeshBuilder
{
	protected:
		ShapeBuilder *						fpSB;
		int									fnVertsPerFrame;
		int									fName;
		int 									fFlags;
		int									fNodeIndex;
		int									fFirstSubSequence;
		int									fnSubSequences;
		TMat3F								fObjectOffset;
		Bool									fSmoothMesh;
		int									fFrameStart;
		Bool									fIsShape;

   public:
		virtual void TestSmoothing( INode *pNode, Mesh & mesh ) = 0;
      virtual Bool addTexVerts( Mesh &maxmesh ) = 0;
      virtual void AddKey( FILE *fp, ShapeBuilder & sb, int time, float pos, 
									Bool visibility, int changes ) = 0;
      virtual Bool addCelFrame( INode *pNode, Mesh &maxmesh, TMat3F *mat, 
                                Bool addFaces ) = 0;
		virtual TS::Shape::Mesh * MakeMesh(FILE *fp) = 0;
		virtual void SetupObject( TS::Shape::Object *obj ) = 0;
		virtual int IsShape() = 0;
		virtual void AddSubSequence( ShapeBuilder & sb, int seq, int first, int count ) = 0;
		virtual int sortPassKey() = 0;      // return 0, 1, or 2.  
		int getName() { return fName; }
};

class CelAnimMeshBuilder : public MeshBuilder
{
      struct FaceVert 
      { 
         int v[3]; 
         Bool uses_normal[3]; 
         int vert_offset;
      };
      
      class TempVert 
      { 
         enum { Unsharable = 0x01, Uses_normal = 0x02 };
         int flags; 
      public:            
         TS::Vertex vertex; 
         int uses_normal() { return (flags & Uses_normal); }
         int unsharable() { return (flags & Unsharable); }
         void set_uses_normal( Bool val ) 
         {         
            if ( val )
               flags |= Uses_normal;
            else
               flags &= ~Uses_normal;
         }
         void set_unsharable( Bool val ) 
         {         
            if ( val )
               flags |= Unsharable;
            else
               flags &= ~Unsharable;
         }
      };

		Vector<TS::Vertex>				vertList;
		Vector<TS::CelAnimMesh::Face> faceList;
		Vector<TS::CelAnimMesh::Frame> frameList;
		Vector<Point2F>					textureVertList;
      Vector<TempVert>              tempVerts;
      Vector<FaceVert>              faceVerts;
      Matrix3								mat;
      int                           fTexFrameStart;
      int                           fnTexVertsPerFrame;
      int                           fCountXLucent;

      int AddTVertex( Point3 &tvert );
      int AddVertex( TS::Vertex const &vert );
		Point3F const & ShapeVertex( Mesh & maxmesh, int vi );
		TS::Vertex const & CalcVertex( Mesh & maxmesh, unsigned smGroup, int vi );
      void addVerts( Mesh &maxmesh, Face *maxface, int face, int flags );
      void getVerts( TS::Vertex *verts, Mesh &maxmesh, Face &maxface, 
                     TMat3F *mat, Bool flat );
      void setMinMaxVerts();
      void addFace( Mesh &maxmesh, TS::CelAnimMesh::Face &face, INode *pNode, 
                    int face_id );
      void setShapeVerts( Mesh &maxmesh, Face &maxface, int face_id );

public:
      CelAnimMeshBuilder(ShapeBuilder *pShapeBuilder, 
			int iname, int inode, MyTransform const & oo, Bool IsShape,
			Int16 flags );
      ~CelAnimMeshBuilder();

      virtual Bool addCelFrame( INode *pNode, Mesh &maxmesh, TMat3F *mat, 
                                Bool addFaces );
      virtual Bool addTexVerts( Mesh &maxmesh );
		virtual void TestSmoothing( INode *pNode, Mesh & mesh );
      virtual void AddKey( FILE *fp, ShapeBuilder & sb, int time, float pos, 
									Bool visibility, int changes );
		virtual TS::Shape::Mesh * MakeMesh(FILE *fp);
		virtual void SetupObject( TS::Shape::Object *obj );
		virtual int IsShape();
		virtual int sortPassKey();
		virtual void AddSubSequence( ShapeBuilder & sb, int seq, int first, int count );
};


//--------------------------------------------------------------
#endif
