#include "stdafx.h"
#include <stdio.h>
#include <base.h>
#include "3ds.h"

#define MAX_DB 5

database3ds *gadb[MAX_DB];
byte3ds ftkerr3ds = 0;
extern "C" byte3ds _ignoreftkerr3ds = 1;

void DatabaseDump( int dbi, char *filename )
{
   SetDumpLevel3ds( MaximumDump3ds );
   FILE *fp = fopen( filename, "w" );
   if( fp != NULL )
      {
      _ignoreftkerr3ds = 1;
      ClearErrList3ds();
      DumpDatabase3ds( fp, gadb[dbi] );
      _ignoreftkerr3ds = 0;
      fprintf( fp, "\n\n\n" );
      fprintf( fp, "ObjectNodeCount: %d\n",GetObjectNodeCount3ds( gadb[dbi] ) );
      for( int i = 0; i < GetObjectNodeCount3ds( gadb[dbi] ); i++ )
         {
         kfmesh3ds *obj = 0;
         GetObjectMotionByIndex3ds( gadb[dbi], i, &obj );

         fprintf( fp, "Object %d ---------------\n", i );
         fprintf( fp, "name: %s\n", obj->name );
         fprintf( fp, "parent: %s\n", obj->parent );
         fprintf( fp, "instance: %s\n", obj->instance );
         fprintf( fp, "number of pos keys: %d\n", obj->npkeys );
         fprintf( fp, "position track options: %x\n", obj->npflag );
         fprintf( fp, "number of rot keys: %d\n", obj->nrkeys );
         fprintf( fp, "rotation track options: %x\n", obj->nrflag );
         fprintf( fp, "number of scale keys: %d\n", obj->nskeys );
         fprintf( fp, "scale track options: %x\n", obj->nsflag );
         fprintf( fp, "number of morph keys: %d\n", obj->nmkeys );
         fprintf( fp, "morph track options: %x\n", obj->nmflag );

         ReleaseObjectMotion3ds( &obj );

         }
      fclose( fp );
      }
}

int DatabaseOpen( char *filename )
{
   for( int i = 0; i < MAX_DB; i++ )
      if( gadb[i] == NULL )
         break;
   if( i == MAX_DB )
      return -1;

   file3ds *file = OpenFile3ds( filename, "r" );
   if( file )
      {
      InitDatabase3ds( &(gadb[i]) );
      if( !ftkerr3ds )
         {
            CreateDatabase3ds( file, gadb[i] );
            if( !ftkerr3ds )
               {
               if( MeshFile == GetDatabaseType3ds( gadb[i] ) )
                  DisconnectDatabase3ds( gadb[i] );
               else
                  {
                  ReleaseDatabase3ds( &(gadb[i]) );
                  i = -1;
                  }
               }

         }
      }
   else
      i = -1;
   CloseAllFiles3ds();
   return i;
}

void DatabaseClose( int dbi )
{
   AssertFatal( dbi >= 0 && dbi < MAX_DB && gadb[dbi],
      "Invalid Database index!" );

   ReleaseDatabase3ds( &(gadb[dbi]) );
   gadb[dbi] = NULL;
   AssertFatal( !ftkerr3ds, "ReleaseDatabase3ds failed!" );
}

int DatabaseGetMaterialCount( int dbi )
{
   AssertFatal( dbi >= 0 && dbi < MAX_DB && gadb[dbi],
      "Invalid Database index!" );

   return GetMaterialCount3ds( gadb[dbi] );
}

material3ds * DatabaseGetMaterial( int dbi, int index )
{
   AssertFatal( dbi >= 0 && dbi < MAX_DB && gadb[dbi],
      "Invalid Database index!" );

   material3ds * mat = 0;
   GetMaterialByIndex3ds( gadb[dbi], index ,&mat);
   return mat;
}

void DatabaseReleaseMaterial( material3ds *mat )
{
   ReleaseMaterial3ds( &mat );
}

int DatabaseGetMeshCount( int dbi )
{
   AssertFatal( dbi >= 0 && dbi < MAX_DB && gadb[dbi],
      "Invalid Database index!" );

   return GetMeshCount3ds( gadb[dbi] );
}

mesh3ds * DatabaseGetMesh( int dbi, int index )
{
   AssertFatal( dbi >= 0 && dbi < MAX_DB && gadb[dbi],
      "Invalid Database index!" );

   mesh3ds * mesh = 0;
   GetMeshByIndex3ds( gadb[dbi], index ,&mesh);
   return mesh;
}

void DatabaseReleaseMesh( mesh3ds *mesh )
{
   RelMeshObj3ds( &mesh );
}
