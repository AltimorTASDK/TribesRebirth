//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "stdlib.h"
#include "stdio.h"
#include "dirent.h"

#include "g_bitmap.h"

const char version[] = "May 4, 1998";

main ()
{
   printf("\nValidBmp  version: %s\n", version);
   printf("Function: to search the current directory for .bmp files\n");
   printf("with an aspect ration greater than 8x1, which are invalid in hardware.\n\n");
   
   //loop through each file, get the file name
   WIN32_FIND_DATA data;
   HANDLE handle = FindFirstFile("*.bmp", &data);

   if (handle != INVALID_HANDLE_VALUE)
   {
      do
      {
         printf("%s\n", data.cFileName);
         
         //Open the file, and check the width and height
         GFXBitmap *testBmp = GFXBitmap::load(data.cFileName);
         if (! testBmp) printf("INVALID BMP FILE: %s\n", data.cFileName);
         else
         {
            int width = testBmp->getWidth();
            int height = testBmp->getHeight();
            
            if ((min(width, height) * 8) < max(width, height)) printf("ASPECT RATION > 8x1 IN FILE: %s\n", data.cFileName);
            if (width > 256) printf("WIDTH > 256 IN FILE: %s\n", data.cFileName);
            if (height > 256) printf("HEIGHT > 256 IN FILE: %s\n", data.cFileName);
         }
      }
      while (FindNextFile(handle,&data));
      FindClose(handle);
   }
   else printf("No '.BMP' files found.\n\n");
}