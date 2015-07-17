//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "fearGuiIRCTextEdit.h"
#include "fear.strings.h"
#include "console.h"


namespace FearGui
{

IMPLEMENT_PERSISTENT_TAG( FearGui::FGIRCTextEdit, FOURCC('F','G','i','t') );

#define HISTORY_LIMIT 20
static char gTextHistory[(HISTORY_LIMIT + 1) * 256];
static int gLastHistoryIndex = -1;
static int gCurHistoryIndex = -1;
static bool gInitialized = FALSE;

static const char * getHistory(int index)
{
   if (index >= 0 && index < HISTORY_LIMIT + 1)
   {
      return &gTextHistory[index * 256];   
   }
   return "";
}

static void setHistory(int index, const char *txt)
{
   if (index >= 0 && index < HISTORY_LIMIT + 1)
   {
      char *ptr = &gTextHistory[index * 256];
      if (txt) strncpy(ptr, txt, 255);
      else *ptr = '\0';
      ptr[255] = '\0';
   }
}

static void UpdateHistory(const char *txt)
{
   if (txt && txt[0])
   {
      gLastHistoryIndex++;
      if (gLastHistoryIndex >= HISTORY_LIMIT) gLastHistoryIndex = 0;
      setHistory(gLastHistoryIndex, txt);
      setHistory(HISTORY_LIMIT, "");
      gCurHistoryIndex = -1;
   }
}

void FGIRCTextEdit::onKeyDown(const SimGui::Event &event)
{
   bool ghosted = FALSE;
   SimGui::Control *topDialog = root->getDialogNumber(1);
   if ((! active) ||
       (topDialog && (topDialog != getTopMostParent()) && (topDialog->findControlWithTag(IDCTG_DIALOG))))
   {
      ghosted = TRUE;
   }
   
   if (ghosted) return;
   
   switch (event.diKeyCode)
   {
      case DIK_UP:
      case DIK_DOWN:
         if (gLastHistoryIndex >= 0)
         {
            if (gCurHistoryIndex < 0) gCurHistoryIndex = gLastHistoryIndex;
            else
            {
               if (event.diKeyCode == DIK_UP)
               {
                  gCurHistoryIndex--;
                  if (gCurHistoryIndex < 0)
                  {
                     const char *temp = getHistory(HISTORY_LIMIT - 1);
                     if (temp[0]) gCurHistoryIndex = HISTORY_LIMIT - 1;
                     else gCurHistoryIndex = gLastHistoryIndex;
                  }
               }
               else
               {
                  gCurHistoryIndex++;
                  if (gCurHistoryIndex > HISTORY_LIMIT - 1) gCurHistoryIndex = 0;
                  else if (gCurHistoryIndex > gLastHistoryIndex)
                  {
                     const char *temp = getHistory(gCurHistoryIndex);
                     if (! temp[0]) gCurHistoryIndex = 0;
                  }
               }
               if (gCurHistoryIndex == gLastHistoryIndex) gCurHistoryIndex = -1;
            }
            if (gCurHistoryIndex >= 0)
            {
               setText(getHistory(gCurHistoryIndex));
               setHistory(HISTORY_LIMIT, getHistory(gCurHistoryIndex));
            }
            else
            {
               setText("");
               setHistory(HISTORY_LIMIT, "");
            }
         }
         return;
         
      case DIK_ESCAPE:
         setText("");
         setHistory(HISTORY_LIMIT, "");
         gCurHistoryIndex = -1;
         return;
         
      case DIK_RETURN:
         if (text[0])
         {
            mbCommandReady = TRUE;
            
            //update the history
            char buf[256];
            getText(buf);
            UpdateHistory(buf);
            
            Parent::onKeyDown(event);
            return;
         }
         break;
   }
   Parent::onKeyDown(event);
   
   //update the history
   char buf[256];
   getText(buf);
   setHistory(HISTORY_LIMIT, buf);
}

void FGIRCTextEdit::getIRCText(char *buf)
{
   getText(buf);
   setText("");
   mbCommandReady = FALSE;
}

bool FGIRCTextEdit::becomeFirstResponder()
{
   return TRUE;
}

bool FGIRCTextEdit::loseFirstResponder()
{
   //return Parent::loseFirstResponder();
   return FALSE;
}

bool FGIRCTextEdit::onAdd(void)
{
   if (! Parent::onAdd()) return FALSE;
   
   //initialize the history
   if (! gInitialized)
   {
      gInitialized = TRUE;
      for (int i = 0; i < HISTORY_LIMIT + 1; i++)
      {
         setHistory(i, "");
      }
   }
   return TRUE;
}
   
void FGIRCTextEdit::onWake(void)
{
   root->makeFirstResponder(this);
   
   setText(getHistory(HISTORY_LIMIT));
   
}

};