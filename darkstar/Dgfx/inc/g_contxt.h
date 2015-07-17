
#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif


struct	GFXEdge;
struct	GFXEdgeList;

struct GFXPolyDrawContext
{
	// step constants
	double dwdx, dwdy, w0;
	double dudx, dudy, u0;
	double dvdx, dvdy, v0;
	double dsdx, dsdy, s0;
	// shadeMap is either a pointer to a shade map
	// for gouraud shading or a pointer to a haze map
	// for hazing a pre-lit texture or color.
	BYTE *shadeMap;
	float shadeScale; // a scale value = 65535 * numShadeLevels
	BYTE *textureMap;
	Int32 textureMapStride;
	Int32 textureMapHeight;
	Int32 color;
	BYTE *rowAddr; // base address of the current scanline.
	Int32 rowStride;
	Int32 currentY;
	Int32 destY;
	Int32 xRight;
	Int32 deltaXR;
	Int32 xLeft;
	Int32 deltaXL;
};

struct GFXRenderContext 
{
	BYTE *rowAddr;
	Int32 stride;
	BYTE *textureMap;
	Int32 textureMapStride;
	void *shadeMap;
	void *translucencyMap;
	Int32 *redMap;
	Int32 *greenMap;
	Int32 *blueMap;
	GFXEdgeList *edgeList;
	GFXEdge **currentEdgeLine;
	GFXEdge **curEdgePtr;
	GFXEdge *freeEdge;
	Int32 color;
	Int32 hazeShift;
	Int32 hazeMask;
	void (__cdecl *rasterLoop)();
	DWORD unused[12];
};

struct GFXSpan
{
	Int32 x;
	Int32 y;
	Int32 cnt;
	Fix16 u;
	Fix16 su;
	Fix16 v;
	Fix16 sv;
	Fix16 fsu;
	Fix16 fsv;
	Fix16 w;
	Fix16 sw;
	Fix16 z;
	Fix16 sz;
	Fix16 s;
	Fix16 ss;
	Fix16 h;
	Fix16 sh;
	Fix16 r;
	Fix16 sr;
	Fix16 g;
	Fix16 sg;
	Fix16 b;
	Fix16 sb;
	Fix16 unused[10];
};

struct GFXPolyRasterContext
{
	Int32 hazeLevel;
	Int32 deltaHaze;
	Int32 currentY;
	Int32 destY;

	Int32 xRight;
	Int32 deltaXR;
	Int32 xLeft;
	Int32 deltaXL;

	Int32 iLeft;
	Int32 deltaIL;
	Int32 iRight;
	Int32 deltaIR;

	float vLeft;
	float deltaVL;
	float vRight;
	float deltaVR;

	float uLeft;
	float deltaUL;
	float uRight;
	float deltaUR;

	float wLeft;
	float deltaWL;
	float wRight;
	float deltaWR;

	Int32 zLeft;
	Int32 deltaZL;
	Int32 zRight;
	Int32 deltaZR;

	Int32 hLeft;
	Int32 deltaHL;
	Int32 hRight;
	Int32 deltaHR;

	Int32 rLeft;
	Int32 deltaRL;
	Int32 rRight;
	Int32 deltaRR;

	Int32 gLeft;
	Int32 deltaGL;
	Int32 gRight;
	Int32 deltaGR;

	Int32 bLeft;
	Int32 deltaBL;
	Int32 bRight;
	Int32 deltaBR;

	float delU;
	float delV;
	float delW;
};

extern "C" GFXRenderContext gfxRC;
extern "C" GFXSpan gfxSpan;
extern "C" GFXPolyRasterContext gfxPRC;
extern "C" GFXPolyDrawContext gfxPDC;

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif
