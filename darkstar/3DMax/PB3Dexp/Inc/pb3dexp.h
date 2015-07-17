// Some 3DS structures


// Node list structure

//struct WkObjList;
typedef struct {
	TriObject *object;
	TCHAR name[11];
	int used;
	void *next;
	} WkObjList;

//struct WkNodeList;
typedef struct {
	ImpNode *node;
	short id;
	TCHAR name[11];
	Mesh *mesh;
	ImpNode *parent;
	Matrix3 tm;
	void *next;
	} WkNodeList;

// 3DS Key structures

/*
struct color_24
{
uchar r;
uchar g;
uchar b;
};
typedef struct color_24 Color_24;
*/

