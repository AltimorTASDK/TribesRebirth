//
// vinfo.cpp
//


#include <typeinfo.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <stdio.h>
#include <iostream.h>
#include <streams.h>
#include <g_bitmap.h>

/////////////////////////////////////////////////////////////////////////////

class VolumeInfoStream : public VolumeRStream
{
 public:
   Vector<VolumeItem> &getVolumeItems() { return(volumeItems); }
   int getStringBlockOffset()           { return(stringBlockOffset); }
};

/////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
   if (argc < 2)
   {
      fprintf(stderr, "huh?\n");
      exit(0);
   }
   
   VolumeInfoStream vol; 
   if (!vol.openVolume(argv[1]))
   {
      fprintf(stderr, "unable to open \"%s\"\n", argv[1]);
      exit(0);
   }

   Vector<VolumeItem> &volumeItems = vol.getVolumeItems();
   fprintf(stderr, "Volume %s: stringBlockOffset = %d\n", argv[1], 
           vol.getStringBlockOffset());
                                      
   for (int i = 0; i < volumeItems.size(); i++)
      fprintf(stderr, 
              "%3d: %20s at offset %d for %d bytes\n",
              i,
              volumeItems[i].string,
              volumeItems[i].blockOffset,
              volumeItems[i].size);
   fprintf(stderr, "\n");
}



















