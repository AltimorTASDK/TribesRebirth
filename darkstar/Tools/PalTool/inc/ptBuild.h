//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _PTBUILD_H_
#define _PTBUILD_H_



struct COLOR
{
   union { double R; double L; double X; };
   union { double G; double U; double Y; };
   union { double B; double V; double Z; };
   int index;
};


class GFXPalette;
class PaletteMap;

class PaletteBuilder
{
private:

   GFXPalette *palette;
   PaletteMap *map;

   COLOR *table;
   COLOR *htable;
   BYTE  *lookup;

   HWND hProgress;
   int  progressSize;
   int  progressPos;
   int  progressLast;

   void setProgressSize(int sz, char *text);
   void showProgress();
   void clearProgress();

   int  calcShift(int x);
   void calcRGBTables();
   void calcLUVTables();
   void findMatches();
   void findClosest( COLOR *remap, int index, bool doShade );
   void updatePalette();

   float getShadeAdj(int s);
   float getHazeAdj(int h, int hazeLevels);
   COLOR* getTable( COLOR *base, int s, int h, int stride);

   void shadeHazeRGB( COLOR *base, int s, int h, int stride );
   

public:
   PaletteBuilder();
   ~PaletteBuilder();
   void setControl(HWND hWnd);
   bool build(GFXPalette *palette, PaletteMap *map);
};


inline void PaletteBuilder::setControl(HWND hWnd)
{
   hProgress = hWnd;   
}   

#endif //_PTBUILD_H_
