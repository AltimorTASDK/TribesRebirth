#ifndef _DETAIL_H_
#define _DETAIL_H_

#include <ts.h>
#ifdef MESH2
#include <ts_mesh2.h>
#else
#include <ts_mesh.h>
#endif
#include <ts_bsp.h>
#include "except.h"
#include "polyhedr.h"
#include "couple.h"
#include "3ds.h"

#define MAT_LIST_EXT    "DML"

#define MAX_CELLS     ('z' - 'a' + 1)
#define MAX_SEQUENCES ('z' - 'a' + 1)
#define MAX_MATS     100
#define MAX_DETAILS  10
#define MAX_FACES    2000
#define MAX_VERTS    6000
#define MAX_PARTS    100

//--------------------------------------------------------------------

class DetailList;

//--------------------------------------------------------------------

class Material
{
public:
   char fName[18];
   char fRawTextureFile[256];
   char fTextureFile[256];
   char fAlphaFile[256];
   UInt8 fR,fG,fB;
   int fUnlit;
   int fSmooth;
   int fTextured;
   int fTwoSided;
   int fTransparent;
   int fTextureHasAlpha;
   RealF fTranslucency;
   int fPerspectiveCorrect;
	int fUsePaletteIndex;
	int fPaletteIndex;
   int fTSMaterialIndex;

   Material();
   Material::Material( material3ds *mat3ds, const char * pPath, Int32 detailIndex );
   int operator==( const Material& m );
	void Dump();
   TSMaterial * MakeTSMaterial( TSMaterial *TSMat ) const;
};

//--------------------------------------------------------------------

class Mesh
{
public:
#ifdef MESH2
   TSMesh2        fTSMesh;
#else
   TSMesh         fTSMesh;
#endif
   Point3F        fCenter;
   RealF          fRadius;
   int            fPart;
   int            fSeq;
   int            fCell;
   char           fName[11];
   Polyhedron     fPolyhedron;
   Int32          fDetailIndex;  // needed to keep materials straight

   Int32          fnVerts;
   TSVertex       faVerts[MAX_VERTS];
   Int32          fnTexVerts;
   Point2F        faTexVerts[MAX_VERTS];
   Int32          fnFaces;
   Point3F        faFaceNormals[MAX_FACES];
#ifdef MESH2
   TSMesh2::Face  faFaces[MAX_FACES];
#else
   TSMeshFace     faFaces[MAX_FACES];
#endif

	int			CompareFaces( int a, int b );
	void			SortFaces();

   Bool        GetCellInfoFromName( char * name );
   void        CalcCenterAndRadius( mesh3ds *p3dsMesh );
   Point3F &   CalcFaceNormal( int faceIndex, mesh3ds *p3dsMesh );
   Point3F &   VertexNormal( int vertexIndex, int faceIndex, 
                  mesh3ds *p3dsMesh );
   Point3F *   CalcFaceCenter( Point3F *pCenter, face3ds *pFace, 
                  mesh3ds *p3dsMesh );
   void        ConvertFace( int index, DetailList *pDL, mesh3ds *pMesh3ds );
   void        ConvertFaces( DetailList * pDL, mesh3ds * p3dsMesh );

   Int16       AddTexture( textvert3ds & text );
   Int16       AddVertex( const Point3F & loc, const Point3F & normal, 
						RealF pointTolerance, RealF normalTolerance );
   Int16       AddVertex( point3ds & floc, Point3F & fnormal,
						RealF pointTolerance, RealF normalTolerance );
	Bool			FaceIsConvex( TSMeshFace const & face );
   void        AddTri( Int16 v0, Int16 v1, Int16 v2,Int16 center,
                  Int16 t0, Int16 t1, Int16 t2, Int16 mat,
						RealF normalTolerance );

   TSPartBase *    MakeTSMesh();

   Mesh( mesh3ds *p3dsMesh, DetailList *pDL, RealF shrink, Int32 detailIndex );
   ~Mesh();
};

//--------------------------------------------------------------------

class Part
{
public:
   char        fName[11];
   int         fSeq;
   int         fnParts;
   int         fnCells;
   Mesh *      faParts[MAX_CELLS];
   Int32       faPartLabels[MAX_CELLS];
   Int32       faCells[MAX_CELLS];
   Polyhedron  fPolyhedron;

   Part();
   ~Part();

   void        AddCell( Mesh * );
   void        ValidateCells();
   TSPartBase * MakeTSCellAnimPart();
   void        Reset();
   void     GetSequenceLengths( Int32 * pLengths );
};

//--------------------------------------------------------------------

class Detail
{
public:
   char     fName[128];
	char		fActualName[256];
   RealF    fShrink;
   RealF    fScale;
   RealF    fDetailSize;
   Int32    fSort;

   //----------------------------------------------------
   // used for building TSShape:

   int			fnFaces;
	int			fnVerts;
   int         fDatabase;
   int         fnParts;
   Part        faParts[MAX_PARTS];
   Point3F     fCenter;
   RealF       fRadius;
   Bool        fSortNow;
   TSBSPPartNode faNodes[MAX_PARTS];
   int         fnNodes;
   TSPartBase * fpTSPart;
   Int32       fMaxSeq;
   Int32       fDetailIndex;

   void        ShallowCopy( const Detail & );
   Bool        LoadDatabase( DetailList * pDL );
   void        ReleaseDatabase();
   Bool        GetMaterials( DetailList * pDL );
   void        InsertMesh( Mesh * pMesh );
   void        ValidateCells();
   void        GetMeshes( DetailList * pDL );
   void        GetBoundingSphere();
   void        ApplyShrinkage();
   void        CheckCollisions();
   void        SortParts();
   TSPartBase * MakeTSPart();
   void        Reset();
   void        ScaleMesh3ds( mesh3ds *pMesh );

   void        read( StreamIO& sio );
   void        write( StreamIO& sio );

	Detail();
   ~Detail();

   //--------------------------------------------------------------------

public:
   class PList
   {
      int   fnParts;
      int   faParts[MAX_PARTS];
   public:
      PList()  { fnParts = 0; };
      void  AddPart( int part )  
         {
         AssertFatal( fnParts < MAX_PARTS, "Too many parts" );
         faParts[fnParts++] = part; 
         }
      int   RemovePart()
         {
         AssertFatal( fnParts > 0, "No part to remove" );
         return faParts[--fnParts];
         }
      int   Count() const
         {
         return fnParts;
         }
      int   GetPart( int index ) const
         {
         AssertFatal( index < fnParts, "Index out of range"  );
         return faParts[index];
         }
   };
   Point3F  fLastSplitNormal;
   RealF    fLastSplitCoeff;
   PList    fLastSplitLeft;
   PList    fLastSplitRight;

   Bool     CanSplit( PList const & left, PList const & right );
   Bool     TrySplit( PList left, PList right, PList more );
   int      BuildSortTree( int* pNodeIndex, PList & parts );

   void     GetSequenceLengths( Int32 * pLengths );
};

//--------------------------------------------------------------------


//--------------------------------------------------------------------

class DetailList : public ClassItem
{
   //----------------------------------------------------
   // ClassIO Methods:
public:
	ClassType COREAPI	classType();
	UInt32 COREAPI		classSize();
	UInt32 COREAPI		classMemSize();
	void COREAPI		classRead(StreamIO&,UInt32);
	void COREAPI		classWrite(StreamIO&,UInt32);
	void COREAPI		classDump(Int32 detailLevel,Int32 indent);
   //
   //----------------------------------------------------
   
   //----------------------------------------------------
   // instance data:
public:
	CDocument * fpDoc;
protected:
   Int32       fnDetails;
   Detail      faDetails[MAX_DETAILS];
public:
	RealF			fMergePointTolerance;
	RealF			fMergeNormalTolerance;
   Int32       fAlphaNumLevels;
   Int32       fAlphaNumColors;

   //----------------------------------------------------
   // used for building TSShape:

   char        fBaseName[256];
   char        fPath[256];
   Int32       faSequenceLengths[MAX_SEQUENCES];
   Mesh *      fpMeshes;

   int         fnMats;
   Material    faMats[MAX_MATS];
   int         fnTSMats;
   TSMaterial  faTSMats[MAX_MATS];

   Point3F     fCenter;
   RealF       fRadius;
   Int32       fMaxSeq;

   //
   //----------------------------------------------------

   //----------------------------------------------------
   // instance methods:
public:
   DetailList();

   void        Reset();
   void        BuildReset();
   void        SetBaseName( const char * );
   const char * GetPath() const { return fPath; }
   int         GetCount()  const;
   Detail &		operator[](int index) { return faDetails[index]; }
   int			InsertDetail( int index, const Detail & d );
   int			DeleteDetail( int index );
   Bool			SortDetails();

   void        BuildShape();

   Bool			LoadDatabases();
   void			ReleaseDatabases();
   void			ProcessDatabases();

   Int32 *     GetSequenceLengths();

   void        GetMeshes();
   TSPartBase * MakeTSDetailPart();
   void        MakeTSShape();


   void Alphatize( GFXBitmap *src, GFXBitmap *alpha );
	void AddMaterial( Material &m );
	void GetMaterials();
	void SaveMaterials();
   int  FindMaterial( char *name );
   int  FindMaterial( int face, mesh3ds *mesh, Int32 detailIndex );
   const Material & GetMaterial( int index );
   Int16 GetMaterialTSIndex( int index );
   //
   //----------------------------------------------------
};

enum ClassDLType
{
	CLS_DetailListOld = 	ClassNumber(CLS_APP_RANGE, 33),
	CLS_DetailList = 	ClassNumber(CLS_APP_RANGE, 34),
};


void DLInit();

extern ClassItem* COREAPI loadDetailListItemOld(StreamIO&,UInt32);
extern ClassItem* COREAPI loadDetailListItem(StreamIO&,UInt32);

#ifdef DL_TABLE_INSTANCE
ClassTypeEntry ClassDLTypeList[] =
{
	{ CLS_DetailListOld,	loadDetailListItemOld  , 		"OldDetailList"			},
	{ CLS_DetailList,  		loadDetailListItem  , 		"DetailList"			},
};

int ClassDLTypeListSize = sizeof(ClassDLTypeList) / sizeof(ClassTypeEntry);
#endif

#endif
