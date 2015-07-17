//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <g_pal.h>
#include <palMap.h>

#include <ptCore.h>
#include <ptBuild.h>
#include <luvConv.h>



//------------------------------------------------------------------------------
PaletteBuilder::PaletteBuilder()
{
   hProgress = NULL;
   table  = NULL;   
   htable = NULL;   
}   


//------------------------------------------------------------------------------
PaletteBuilder::~PaletteBuilder()
{
   delete [] table;
   delete [] htable;
}   


//------------------------------------------------------------------------------
int
PaletteBuilder::calcShift(int x)
{
   int result = -1;
   do {
      x >>= 1;
      result++;
   }while(x);
   return (result);
}   


//------------------------------------------------------------------------------
bool PaletteBuilder::build(GFXPalette *_palette, PaletteMap *_map)
{
   palette = _palette;
   map = _map;
   if (map->useLUV) calcLUVTables(); 
   else calcRGBTables(); 
   findMatches();
   updatePalette();
   return (true);

  // convertRGBtoLUV(table, 256 * map->shade.levels * map->haze.levels);

}   


float PaletteBuilder::getShadeAdj(int s)
{
   float  shadeAdj = 1.0-float(s)/float(map->shade.levels)*map->shade.max; 
   return pow(shadeAdj, map->gamma);       
}   

float PaletteBuilder::getHazeAdj(int h, int hazeLevels)
{
   return float(h)/float(hazeLevels)*map->haze.max;
}   


COLOR* PaletteBuilder::getTable( COLOR *base, int s, int h, int stride)
{
   return base+( 256*((h*stride)+s) );
}   


//------------------------------------------------------------------------------
void PaletteBuilder::shadeHazeRGB( COLOR *base, int s, int h, int stride )
{
   COLOR *remap = getTable(base, s, h, stride);

   //--------------------------------------
   if ( s == 0 && h == 0 )
   {
      // the first remap table is just a copy of original RGB's
      for (int i = 0; i<256; i++, remap++)
      {
         remap->R = palette->palette[0].color[i].peRed;
         remap->G = palette->palette[0].color[i].peGreen;
         remap->B = palette->palette[0].color[i].peBlue;
      }
   }
   //--------------------------------------
   else if ( h == 0 )
   {
      // JUST SHADE
      float shadeAdj = getShadeAdj(s);
      for (int i = 0; i<256; i++, base++, remap++)
      {
         // shade the color
         remap->R = base->R*shadeAdj;   
         remap->G = base->G*shadeAdj;   
         remap->B = base->B*shadeAdj;   
      }
   }
   //--------------------------------------
   else
   {
      // HAZE and SHADE
      float shadeAdj = getShadeAdj(s);
      float hazeAdj  = getHazeAdj(h, stride == 1 ? map->shade.levels : map->haze.levels);
      float hazeAdjInv = 1.0-hazeAdj;
      COLOR hazeComponent;

      hazeComponent.R = float(map->hazeR)*hazeAdj;
      hazeComponent.G = float(map->hazeG)*hazeAdj;
      hazeComponent.B = float(map->hazeB)*hazeAdj;
      for (int i = 0; i<256; i++, base++, remap++)
      {
         // shade the color
         remap->R = base->R*shadeAdj;   
         remap->G = base->G*shadeAdj;   
         remap->B = base->B*shadeAdj;   

         // haze the color
         remap->R = remap->R*hazeAdjInv + hazeComponent.R;   
         remap->G = remap->G*hazeAdjInv + hazeComponent.G;   
         remap->B = remap->B*hazeAdjInv + hazeComponent.B;   
      }
   }
}   

void PaletteBuilder::calcLUVTables()
{
}   


//------------------------------------------------------------------------------
void PaletteBuilder::calcRGBTables()
{
   delete [] table;
   delete [] htable; htable = NULL;
   table  = new COLOR[ 256 * map->shade.levels * map->haze.levels ];
   htable = new COLOR[ 256 * map->shade.levels ];
   
   setProgressSize(map->shade.levels * map->haze.levels + map->shade.levels, "calc tables:");

   // generate shade/haze tables
   for (int h=0; h<map->haze.levels; h++)
      for (int s=0; s<map->shade.levels; s++)
      {
         shadeHazeRGB(table, s, h, map->shade.levels);
         showProgress();
      }

   // generate haze table
   for (int h=0; h<map->shade.levels; h++)
   {
      shadeHazeRGB(htable, 0, h, 1);
      showProgress();
   }

   clearProgress();
}   


//------------------------------------------------------------------------------
void PaletteBuilder::findMatches()
{
   lookup = new BYTE[(256*256*256)];
   memset(lookup, 0, (256*256*256));

   //--------------------------------------
   // shade==haze==0 special case no lookup necessary
   setProgressSize(map->shade.levels * map->haze.levels, "256 reduction:");
   for (int i=0; i<256; i++)
      table[i].index = i;
   if (htable)
      for (int i=0; i<256; i++)
         htable[i].index = i;
   showProgress();

   //--------------------------------------
   for (int s=1; s < map->shade.levels; s++)
   {
      COLOR *base  = table;
      COLOR *remap = table+(256*s);
      for (int i = 0; i<256; i++, base++, remap++)
         findClosest( remap, base->index, true );
      showProgress();
   }

   //--------------------------------------
   for (int h=1; h < map->haze.levels; h++)
   {
      for (int s=0; s < map->shade.levels; s++)
      {
         COLOR *base  = table+(256*s);
         COLOR *remap = table+(256*((h*map->shade.levels)+s));

         for (int i = 0; i<256; i++, base++, remap++)
            findClosest( remap, base->index, false );
         showProgress();
      }
   }

   //--------------------------------------
   if (map->haze.levels != map->shade.levels)
      for (int h=1; h < map->shade.levels; h++)
      {
         COLOR *base  = htable;
         COLOR *remap = htable+(256*h);

         for (int i = 0; i<256; i++, base++, remap++)
            findClosest( remap, base->index, false );
      }
   clearProgress();

   delete [] lookup;
}   


//------------------------------------------------------------------------------
void PaletteBuilder::findClosest( COLOR *remap, int index, bool doShade )
{
   BitVector *inclusion = (doShade) ? &map->shade.inclusion[index] : &map->haze.inclusion[index];
   int    closestIndex = 0;
   double closestDist  = 10E30;

   int r = remap->R+0.5;
   int g = remap->G+0.5;
   int b = remap->B+0.5;
   int lindex = (r<<16) + (g<<8) + b;
   lindex &= ~0x00010001;

   COLOR *base = table;
   if (inclusion->test())
   {
      remap->index = lookup[lindex];
      if (remap->index) 
         return;

      base += 10;
      for (int i=10; i<236; i++, base++)
         if (inclusion->test(i))
         {
            COLOR c;
            c.R = remap->R - base->R;
            c.G = remap->G - base->G;
            c.B = remap->B - base->B;
            // Add weighted Euclidean - for intensity matching.
            double dist = (c.R*c.R)+(c.G*c.G)+(c.B*c.B);
            if (dist < closestDist)
            {
               closestDist  = dist;
               closestIndex = i;
            }
         }
      remap->index   = closestIndex;
      lookup[lindex] = closestIndex;
   }
   else
      remap->index = 0; //index;

}   


//------------------------------------------------------------------------------
void PaletteBuilder::updatePalette()
{
//   int shadeSize = 256 * map->shade.levels * map->haze.levels;
//   int hazeSize  = 256 * map->shade.levels;
//	palette->shadeShift	= calcShift(map->shade.levels);  
//	palette->hazeShift	= calcShift(map->haze.levels);  
//   PALETTEENTRY *pe = (PALETTEENTRY *) &palette->hazeColor;
//   pe->peRed = map->hazeR;
//   pe->peBlue = map->hazeB;
//   pe->peGreen = map->hazeG;
//
//	palette->transLevels = 0;
//	delete [] palette->shadeMapBase;
//	palette->shadeMapBase= new BYTE[shadeSize];
//	palette->shadeMap	   = palette->shadeMapBase;
//
//	delete [] palette->hazeMapBase;
//	palette->hazeMapBase = new BYTE[hazeSize];
//	palette->hazeMap		= palette->hazeMapBase;
//
//   // copy shade map
//   for (int h=0; h<map->haze.levels; h++)
//      for (int s=0; s<map->shade.levels; s++)
//      {
//         COLOR *base = table+(256*( (h*map->shade.levels)+(map->shade.levels-s-1) ));
//         BYTE  *remap= palette->shadeMap+(256*((h*map->shade.levels)+s));
//         for (int i=0; i<256; i++, base++, remap++)
//            *remap = base->index;
//      }
//
//   // copy haze map
//   if (map->haze.levels != map->shade.levels)
//   {
//      for (int h=0; h<map->shade.levels; h++)
//      {
//         COLOR *base = htable+(256*h);
//         BYTE  *remap= palette->hazeMap+(256*h);
//         for (int i=0; i<256; i++, base++, remap++)
//            *remap = base->index;
//      }
//   }
//   else
//   {
//      for (int h=0; h<map->shade.levels; h++)
//      {
//         BYTE *base = palette->shadeMap+(256*map->shade.levels*h);
//         BYTE *remap= palette->hazeMap+(256*h);
//         memcpy(remap, base, 256);
//      }
//   }
}   


//------------------------------------------------------------------------------
void PaletteBuilder::setProgressSize(int sz, char *text)
{
   progressSize = sz;
   progressLast = 0; 
   progressPos = 0; 
   if (hProgress)  
   {
      SendMessage(hProgress, PBM_SETRANGE, 0, MAKELPARAM(0, progressSize));
      SendMessage(hProgress, PBM_SETPOS, 0, 0);
      SendMessage(hProgress, PBM_SETSTEP, 1, 0);
   }
   else
      printf("%15s", text);
}   


//------------------------------------------------------------------------------
void PaletteBuilder::showProgress()
{
   progressPos++;
   if (hProgress)  
      SendMessage(hProgress, PBM_STEPIT, 0, 0);
   else
   {
      int n = (progressPos * 50)/progressSize;
      int d = n-progressLast;
      while (d--)
         printf(".");
      progressLast = n;
   }
}   


//------------------------------------------------------------------------------
void PaletteBuilder::clearProgress()
{
   if (hProgress)  
      SendMessage(hProgress, PBM_SETPOS, 0, 0);
   else
      printf("\n\r");
}   
