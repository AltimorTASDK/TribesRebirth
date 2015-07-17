#include <g_surfac.h>
#include <g_pal.h>
#include <string.h>
#include <m_box.h>
#include <simWorld.h>
#include "FearGuiPaletteCtrl.h"
#include "FearGuiShellPal.h"
#include "simGame.h"

namespace FearGui
{

void FGPaletteCtrl::loadPalette()
{
   Parent::loadPalette();
   /*
   hPal = SimResource::loadByTag(manager, palTag, true);
   if ((bool)hPal)
   {
      hPal->palette[0].color[117].peRed = 0;  
      hPal->palette[0].color[117].peGreen = 48;  
      hPal->palette[0].color[117].peBlue = 2;  
      
      hPal->palette[0].color[118].peRed = 0;  
      hPal->palette[0].color[118].peGreen = 101;  
      hPal->palette[0].color[118].peBlue = 3;  
      
      hPal->palette[0].color[119].peRed = 5;  
      hPal->palette[0].color[119].peGreen = 137;  
      hPal->palette[0].color[119].peBlue = 10;  
      SimGame::get()->getWorld()->setPalette(hPal);
   }
   */
   
   //if (bool(hPal)) FearGui::InitializePaletteTable(hPal);
}  
 
IMPLEMENT_PERSISTENT_TAG(FGPaletteCtrl, FOURCC('F','G','p','a'));

}; //namespace FearGui