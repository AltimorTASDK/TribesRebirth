//---------------------------------------------------------------------------
//
//	$Workfile:   VTList.cpp  $
// $Version$
// $Version$   1.0
//	Initial revision. Louie McCrady
//
//---------------------------------------------------------------------------

#include <sim.h>
#include <dir.h>

int main(int argc, char *argv[], char *)
{
   if ( argc < 3 )
   {
      printf ("VTList  (V1.00)  A utility to search volumes for files\n" );
      printf ("  Usage: VTList \<expression\> name1.vol [name2.vol] [*.vol] \n\n" );
      printf ("    Where \<expression\> is a filename, Wildcards *,? are allowed\n" );
      printf ("    At least one volume must be specified.  Wildcards *?.vol allowed\n");
   }
   else
   {
      VolumeRStream vol;
      FindMatch   fm( argv[1], 1024 );
      for ( int i=2; i<argc; i++ )
      {
         struct ffblk ffblk;
         bool done = findfirst(argv[i],&ffblk,0);
         while ( !done )
         {
            vol.openVolume( (ffblk.ff_name) );
            fm.clear();
            vol.findMatches( &fm );
            for ( int j=0; j<fm.numMatches(); j++ )
			{
               if ( ! j )
                  printf( "%s\n", (ffblk.ff_name) );
					
               printf( "  %s\n",fm.matchList[j] );
			}
			if ( fm.isFull () )
				printf ( "... match list full, %s may have have more entries ..\n",  
											ffblk.ff_name);
            done = findnext(&ffblk);
         }
      }
   }
}   