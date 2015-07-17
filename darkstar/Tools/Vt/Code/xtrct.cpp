#include <typeinfo.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <streams.h>

VolumeRStream * pVol = 0;

int __cdecl main(int argc, char **argv)
{
   char *volfile, *filename, *newfile;
   if(argc != 3 && argc != 4)
   {
      printf("extract v1.0\n");
      printf("  Usage: extract <volfile> <filename> (optional) <newfile>.\n");
      return 0;
   }
   
   volfile = argv[1];
   filename = argv[2];
   newfile = argv[3];
   
   if(argc == 3)
      newfile = filename;
   
   pVol = new VolumeRStream();
   if(!pVol->openVolume(volfile))
      return 0;
   
   FileRWStream *f = new FileRWStream(newfile);
   
   pVol->open(filename);
   
   Int8 c;
   
   while(pVol->read(&c))
      f->write(c);
   
   pVol->close();
   f->close();
   
   pVol->closeVolume();
   return 0;
}