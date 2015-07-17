//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#include "g_surfac.h"
#include "fear.strings.h"
#include "fearGlobals.h"
#include "fearHudPalTable.h"

namespace FearGui
{

//----------------------------------------------------------------------------

HudColors gPaletteColors[_NUM_HUD_COLORS] = 
{
   { _BLACK,          1,   1,   1 },   
   { _RED,          220,   0,   0 },   
   { _GREEN_40,       0,  40,   0 },
   { _GREEN_78,       0,  78,   0 },
   { _GREEN_132,      0, 132,   0 },
   { _GREEN,          0, 255,   0 },
   { _BLUE,           0,   0, 255 },   
   { _YELLOW,       255, 255,   0 },
   { _BLUE_OUTLINE,  60, 140, 140 },
   { _BLUE_HILITE,   87, 255, 255 },
   { _WHITE,        255, 255, 255 },   
};

static GFXPalette *gPalette = NULL;

void FGHPaletteTable::InitializeTable(GFXPalette *pal)
{
   if (gPalette) return;
   for (int i = 0; i < _NUM_HUD_COLORS; i++)
   {
      gPaletteColors[i].colorIdx = pal->GetNearestColor(gHudColors[i].r,
                                                    gHudColors[i].g,
                                                    gHudColors[i].b );
   }
   
   gPalette = pal;
}

//----------------------------------------------------------------------------

void onWake(void)
{
   gPalette = NULL;
}

void FGHPaletteTable::onRender(GFXSurface *sfc, Point2I, const Box2I &)
{
   InitializeTable(sfc->getPalette());
}

IMPLEMENT_PERSISTENT_TAG(FGHPaletteTable, FOURCC('F','G','p','t'));

};