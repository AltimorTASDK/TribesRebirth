#include <g_surfac.h>
#include <string.h>
#include "simTagDictionary.h"
#include "simResource.h"
#include "darkstar.strings.h"
#include "editor.strings.h"
#include "fear.strings.h"
#include "g_bitmap.h"
#include "g_font.h"
#include "simGame.h"
#include "simGuiTextFormat.h"

class FearGuiFormattedText : public SimGui::TextFormat
{
   typedef SimGui::TextFormat Parent;
public:
   void parentResized(const Point2I &oldParentExtent, const Point2I &newParentExtent);
   void setScriptValue(const char *value);
   bool onAdd();
   bool wantsTabListMembership() { return FALSE; }
   bool  becomeFirstResponder() { return FALSE; }
   void onPreRender();
   DECLARE_PERSISTENT(FearGuiFormattedText);
};

IMPLEMENT_PERSISTENT(FearGuiFormattedText);

bool FearGuiFormattedText::onAdd()
{
   fontArray[0].tag = IDFNT_10_STANDARD;
   fontArray[1].tag = IDFNT_10_HILITE;
   fontArray[2].tag = IDFNT_10_SELECTED;

   return Parent::onAdd();
}

void FearGuiFormattedText::onPreRender()
{
   bool ghosted = FALSE;
   SimGui::Control *topDialog = root->getDialogNumber(1);
   if (topDialog && (topDialog != getTopMostParent()) && (topDialog->findControlWithTag(IDCTG_DIALOG)))
   {
      ghosted = TRUE;
   }
   
   if (ghosted)
   {
      setFont(0, IDFNT_10_DISABLED); 
      setFont(1, IDFNT_10_DISABLED); 
      setFont(2, IDFNT_10_DISABLED); 
   }
   else
   {
      setFont(0, IDFNT_10_STANDARD); 
      setFont(1, IDFNT_10_HILITE); 
      setFont(2, IDFNT_10_SELECTED); 
   }
}

void FearGuiFormattedText::setScriptValue(const char *text)
{
   int newXExt = parent ? parent->extent.x : 100;

   formatControlString(text, newXExt, false, true);
   resize(position, Point2I(newXExt, getHeight() + 4));
}

void FearGuiFormattedText::parentResized(const Point2I &, const Point2I &newParentExtent)
{
   formatControlString(NULL, newParentExtent.x, true, false);
   resize(position, Point2I(newParentExtent.x, getHeight()));
}
