#pragma pack(4)

#include "3dsftk.h"


int DatabaseOpen( char *filename );
void DatabaseClose( int dbi );
void DatabaseDump( int dbi, char *filename );
int DatabaseGetMaterialCount( int dbi );
material3ds *  DatabaseGetMaterial( int dbi, int index );
void DatabaseReleaseMaterial( material3ds *mat );
int DatabaseGetMeshCount( int dbi );
mesh3ds * DatabaseGetMesh( int dbi, int index );
void DatabaseReleaseMesh( mesh3ds *mesh );

#pragma pack()
