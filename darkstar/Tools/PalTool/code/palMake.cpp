//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <core.h>
#include <ptBuild.h>

#include <g_bitmap.h>
#include <g_pal.h>
#include <palMap.h>

#define STR_EQ(s1,s2) (stricmp((s1),(s2))==0)

//------------------------------------------------------------------------------
int showHelp()
{
   printf("PalMake  -  Generates palette shade and haze tables\n");
   printf("USAGE: PalMake [opts] <palette/bitmap> {.pmp} {-o outputPalette}\n");
   printf("OPTIONS:\n");
   printf("  -h,-?                This Help. (how useful)\n");
   printf("  -o outputPalette     save result to ouputPalette\n");
   printf("  -haze (r,g,b)        override hazeRGB in .PMP file\n");
   printf("  -r (min,max)          override RGBmatcth in .PMP file\n");
   return (0);
}   


//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
   char namePAL[MAX_PATH];
   char nameBMP[MAX_PATH];
   char namePMP[MAX_PATH];
   char outPAL[MAX_PATH];
   bool overrideHaze = false;
   int  R,G,B;
   #define  MAX_RANGES  10    // only allow 10 range settings on palette
   struct range
   {
      int   min;
      int   max;
   }palRange[MAX_RANGES];
   int rangeIndex = 0;
   GFXPalette *palette;

   namePAL[0] = 0;
   namePMP[0] = 0;
   outPAL[0] = 0;

   AssertSetFlags(ASSERT_NO_DIALOG|ASSERT_NO_LOG);

   if (argc == 1)
      return (showHelp());

   for (argc--,argv++; argc; argc--,argv++)
   {
      // command line 'dash' arguments
      if (**argv == '-' || **argv == '/')
      {
         char *flag = (*argv)+1;
         if (STR_EQ(flag, "?") || STR_EQ(flag, "h") )
            return (showHelp());
         else if (STR_EQ(flag, "o"))
         {
            if (!argc)
            {
               printf("Error no ouput palette specified.\n");
               return (1);
            }
            argc--;
            argv++;
            strcpy(outPAL, *argv);
         
         }
         else if (STR_EQ(flag, "haze"))
         {
            if (!argc)
            {
               printf("Error no (R,G,B) specified.\n");
               return (1);
            }
            argc--;
            argv++;
            sscanf(*argv, "(%d,%d,%d)", &R, &G, &B);
            overrideHaze = true;         
         }
         else if (STR_EQ(flag, "r"))
         {
            if (!argc)
            {
               printf("Error no (min,max) specified.\n");
               return (1);
            }
            if ( rangeIndex > MAX_RANGES )
            {
               printf(avar("Max RGBMatch range limit(%i) exeeded.\n",MAX_RANGES));
               return (1);
            }
            argc--;
            argv++;
            sscanf(*argv, "(%d,%d)", &palRange[rangeIndex].min, &palRange[rangeIndex].max);
            rangeIndex++;
         }
      }
      else
      {
         char *dot = strrchr(*argv, '.');
         if (!dot)
         {
            printf("Error: bad filename '%s'\n", argv);
            return (1);
         }
         if (STR_EQ(dot, ".ppl") || STR_EQ(dot, ".pal"))
            strcpy(namePAL, *argv);
         else if (STR_EQ(dot, ".dib") || STR_EQ(dot, ".bmp"))
            strcpy(nameBMP, *argv);
         else if (STR_EQ(dot, ".pmp"))
            strcpy(namePMP, *argv);
         else
         {
            printf("Error: unknown filename extension '%s'\n", argv);
            return (1);
         }
      }
   }

   if (!(*namePAL || *nameBMP))
   {
      printf("Error: no .ppl/.pal/.dib/.bmp specified\n");
      return (1);
   }

   if (!*namePMP)
   {
      strcpy(namePMP, namePAL);
      char *dot = strrchr(namePMP, '.');
      strcpy(dot, ".pmp");
   }

   if (!*outPAL)
   {
      if (*namePAL)
         strcpy(outPAL, namePAL);
      else
         strcpy(strrchr(nameBMP, '.'), ".ppl");
   }

   //-------------------------------------- LOAD PALETTE
   if (*namePAL)
   {
      palette = GFXPalette::load( namePAL );
      if (!palette)
      {
         printf("Error: unable to load palette '%s'.", namePAL);
         return (1);
      }
   }
   else
   {
      GFXBitmap *bmp = GFXBitmap::load(nameBMP, BMF_INCLUDE_PALETTE);
      if (bmp) 
      {
         if (bmp->pPalette)   
         {
            palette = bmp->pPalette;
            bmp->pPalette = NULL;
            delete bmp;   
         }
         else
         {
	         printf( "Error bitmap file does not contain a palette\"%s\".", nameBMP);
            return (1);
         }
      }
      else
      {
	      printf( "Error loading bitmap file \"%s\".", nameBMP);
         return (1);
      }
   }

   //-------------------------------------- LOAD PALETTE MAP
   Persistent::Base::Error err;
   FileRStream sio( namePMP );
   PaletteMap bogus;
   PaletteMap *map = (PaletteMap*)Persistent::Base::load(sio, &err);
   if (err != Persistent::Base::Ok)
   {
      printf("Error: unable to load paletteMap '%s'.", namePMP);
      return (1);
   }
   map->version();
   if (overrideHaze)
   {
      map->hazeR = R;
      map->hazeG = G;
      map->hazeB = B;
   }
   sio.close();

   //-------------------------------------- BUILD PALETTE
   PaletteBuilder builder;
   builder.build(palette, map);

   if (rangeIndex)
   {
      palette->allowedColorMatches.zero();
      for ( int i=0; i<rangeIndex; i++ )
         palette->allowedColorMatches.set(palRange[i].min,palRange[i].max);
   }

   //-------------------------------------- SAVE PALETTE
   if (!palette->write( outPAL ) )
   {
      printf("Error: unable to Write palette '%s'.", outPAL);
      return (1);
   }

   return (0);   
}   