#include "FearGuiArrayCtrl.h"
#include "fear.strings.h"
#include "g_font.h"
#include "simResource.h"

namespace FearGui
{

bool FGHudArrayCtrl::onAdd()
{
   if(!Parent::onAdd())
      return false;

   //set the pba tag
   //pbaTag = IDPBA_SHELL_COLUMNS;
   //loadBitmapArray();
   
   //other misc vars
   columnsSortable = FALSE;
   columnsResizeable = FALSE;
   
   return true;
}

};
