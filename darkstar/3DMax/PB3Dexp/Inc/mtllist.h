#ifndef MTLLIST_H_
#define mTLLIST_H_


//- Material Export -------------------------------------------------------------

// Some 3DS structures

#pragma pack(1)
typedef struct {
	float r,g,b;
	} Color_f;

typedef struct {
	unsigned char r,g,b;
	} Color_24;
#pragma pack()

struct SMtl;

#include "mtldef.h"  // 3ds r4 material defs

//-------------------------------------------------------------------

struct MEntry 
   { 
   SMtl *sm; 
   Mtl *m; 
   };

class MeshMtlList: public Tab<MEntry> 
{
public:
	void AddMtl(Mtl *m);
	void ReallyAddMtl(Mtl *m);
	int FindMtl(Mtl *m);
	int FindSName(char *nam);
	~MeshMtlList() 
      {
		for (int i=0; i<Count(); i++) 
         {
			FreeMatRefs((*this)[i].sm);
			delete (*this)[i].sm;
			}
		}
};

#endif
