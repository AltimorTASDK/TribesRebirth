#include "SimGuiActiveCtrl.h"
#include "FearGuiHudCtrl.h"
#include "FearGuiDialog.h"
#include "Fear.Strings.h"
#include "simTagDictionary.h"
#include "darkstar.strings.h"
#include "editor.strings.h"
#include "inspect.h"

#include <g_bitmap.h>
#include <g_surfac.h>
#include <simResource.h>
#include "g_font.h"
#include "console.h"

namespace FearGui
{

class FearGuiAbortDlg : public FearGuiDialog
{
   typedef FearGuiDialog Parent;
   
   Resource<GFXFont> mFont;
   
   Int32 fontNameTag;

   // if textTag is > 0, the text displayed by SimpleText
   // is retrieved through SimTagDictionary 
   Inspect_Str text;
   Int32 textTag;
   void setFont(void);
   void setText(void);
   
public:
   FearGuiAbortDlg();
   
   bool onAdd(void);
   void onKeyDown(const SimGui::Event &event);
   void onRender(GFXSurface *, Point2I, const Box2I &);

	void inspectWrite(Inspect* insp);
	void inspectRead(Inspect *insp);

   DECLARE_PERSISTENT(FearGuiAbortDlg);
   Persistent::Base::Error write( StreamIO &sio, int, int );
   Persistent::Base::Error read( StreamIO &sio, int, int );
};

FearGuiAbortDlg::FearGuiAbortDlg(void)
{
   fontNameTag = IDFNT_FONT_DEFAULT;
   textTag = IDSTR_STRING_DEFAULT;
}

void FearGuiAbortDlg::setFont(void)
{
   //load the font
   mFont = SimResource::loadByTag(manager, fontNameTag, true);
   if (! (bool)mFont)
   {
      //make sure we have a font
      mFont = SimResource::get(manager)->load("if_w_10.pft");
   }
}

void FearGuiAbortDlg::setText(void)
{
   if (textTag > 0)
   {
      const char *txt = SimTagDictionary::getString(manager, textTag);   
      if(!txt)
         strncpy(text, "bad tag", Inspect::MAX_STRING_LEN - 1);
      else
         strncpy(text, txt, Inspect::MAX_STRING_LEN - 1);
   }
   text[Inspect::MAX_STRING_LEN - 1] = '\0';
}

bool FearGuiAbortDlg::onAdd(void)
{
   if (! Parent::onAdd())
      return FALSE;
      
   setFont();
   setText();
      
   return TRUE;
}

void FearGuiAbortDlg::onKeyDown(const SimGui::Event &event)
{
   switch(event.diKeyCode)
   {
      case DIK_ESCAPE:
      case DIK_N:
         root->makeFirstResponder(NULL);
         root->popDialogControl();
         return;
         
      case DIK_Y:
         root->makeFirstResponder(NULL);
         root->popDialogControl();
         Console->evaluate("EndGame();", false);
         break;
   }
}

void FearGuiAbortDlg::onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect)
{
   //avoid the compiler warning
   updateRect;
   
   //first draw the background dialog
   Parent::onRender(sfc, offset, updateRect);
   
   //draw the text
   Point2I msgOffset;
   msgOffset.x = offset.x + (extent.x - mFont->getStrWidth(text)) / 2;
   msgOffset.y = offset.y  - 2 + (extent.y - mFont->getHeight()) / 2;
   sfc->drawText_p(mFont, &msgOffset, text);
   
   //draw any chil'en
   renderChildControls(sfc, offset, updateRect);
}

void FearGuiAbortDlg::inspectWrite(Inspect* insp)
{
   Parent::inspectWrite(insp);
   insp->write(IDITG_FONTNAME_TAG, true, IDRES_BEG_GUI_FNT, IDRES_END_GUI_FNT, (Int32)fontNameTag);
   insp->write(IDITG_TEXT_TAG, false, IDDAT_BEG_GUI_STR, IDDAT_END_GUI_STR, (Int32)textTag);
}

void FearGuiAbortDlg::inspectRead(Inspect *insp)
{
   Parent::inspectRead(insp);
   insp->read(IDITG_FONTNAME_TAG, NULL, NULL, NULL, fontNameTag);
   insp->read(IDITG_TEXT_TAG, NULL, NULL, NULL, textTag);
   setFont();
   setText();
}   

Persistent::Base::Error FearGuiAbortDlg::write( StreamIO &sio, int a, int b)
{
   sio.write(fontNameTag);
   sio.write(textTag);
   return Parent::write(sio, a, b);
}

Persistent::Base::Error FearGuiAbortDlg::read( StreamIO &sio, int a, int b)
{
   sio.read(&fontNameTag);
   sio.read(&textTag);
   return Parent::read(sio, a, b);
}


IMPLEMENT_PERSISTENT_TAG( FearGuiAbortDlg, FOURCC('F','G','a','d'));

};