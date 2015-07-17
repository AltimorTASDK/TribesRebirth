#include "g_surfac.h"
#include "simGuiHelpCtrl.h"

namespace SimGui
{

HelpCtrl::HelpCtrl(void)
{
   mHelpTag     = 0;
   lpszHelpText = NULL;
}

HelpCtrl::~HelpCtrl()
{
   if (lpszHelpText)
   {
      delete [] (char*)lpszHelpText;
      lpszHelpText = NULL;
   }
}

void HelpCtrl::render(GFXSurface *sfc)
{
   sfc;
}

void HelpCtrl::setHelpText(const char *lpszText, float /* rElapsedTime */, bool /*mouseClicked*/)
{
   if (lpszHelpText)
   {
      delete [] (char*)lpszHelpText;
      lpszHelpText = NULL;
   }

   if (lpszText)
   {
      lpszHelpText = strnew(lpszText);
      mHelpTag = 0;
   }
}

void HelpCtrl::setHelpTag(Int32 iHelpTag, float /* rElapsedTime */, bool /*mouseClicked*/)
{
   mHelpTag = iHelpTag;

   if (lpszHelpText)
   {
      delete [] (char*)lpszHelpText;
      lpszHelpText = NULL;
   }
}
   
};