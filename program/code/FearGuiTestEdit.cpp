//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "string.h"
#include "g_surfac.h"
#include "inspect.h"
#include "console.h"
#include "FearGuiTestEdit.h"
#include "FearDcl.h"
#include "g_bitmap.h"
#include "g_surfac.h"
#include "r_clip.h"
#include "FearGuiShellPal.h"
#include "fear.strings.h"
#include "simResource.h"
#include "fearGuiDlgBox.h"


namespace FearGui
{

IMPLEMENT_PERSISTENT_TAG( FearGui::TestEdit, FOURCC('F','G','t','e') );

void TestEdit::loadBitmapArray()
{
   const char *pbaName = SimTagDictionary::getString(manager, pbaTag);
   AssertFatal(pbaName, "Invalid PBA name");
   bma = SimResource::get(manager)->load(pbaName, true);
   AssertFatal((bool)bma, "Unable to load bma");
   for (int i = 0; i < BmpCount; i++)
   {
      GFXBitmap *bmp = bma->getBitmap(i);
      bmp->attribute |= BMA_TRANSPARENT;
   }
}

bool TestEdit::loseFirstResponder(void)
{
   //execute the alt console command, probably a verification function
   if (altConsoleCommand[0])
   {
      Console->evaluate(altConsoleCommand, FALSE);
   }
   return TRUE;
}

Int32 TestEdit::getMouseCursorTag(void)
{
   return IDBMP_CURSOR_IBEAM;
}


void TestEdit::onKeyDown(const SimGui::Event &event)
{
   Parent::onKeyDown(event);
   
   //now execute the console command if it exists
   if (consoleCommand[0])
   {
      Console->evaluate(consoleCommand, FALSE);
   }
   
   //now set the console var if exists
   if (consoleVariable[0])
   {
      Console->setVariable(consoleVariable, text);
   }
   
   if (event.diKeyCode == DIK_RETURN && altConsoleCommand[0])
   {
      Console->evaluate(altConsoleCommand, FALSE);
   }
   
   //if it was either the return, or the escape,
   FearGui::FGDlgBox *dlg = NULL;
   SimGui::Control *ctrl = NULL;
   SimGui::Control *topParent = const_cast<SimGui::Control*>(getTopMostParent());
   if (topParent) ctrl = topParent->findControlWithTag(IDCTG_DIALOG);
   if (ctrl) dlg = dynamic_cast<FearGui::FGDlgBox*>(ctrl);
   if (event.diKeyCode == DIK_RETURN && dlg)
   {
      dlg->handleReturnKey();
   }
   else if (event.diKeyCode == DIK_ESCAPE && dlg)
   {
      dlg->handleEscapeKey();
   }
}

void TestEdit::setText(const char *txt)
{
   Parent::setText(txt);
   
   //now execute the console command if it exists
   if (consoleCommand[0])
   {
      Console->evaluate(consoleCommand, FALSE);
   }
   
   //now set the console var if exists
   if (consoleVariable[0])
   {
      Console->setVariable(consoleVariable, text);
   }
}

void TestEdit::onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect)
{
   //compiler warning
   updateRect;
   
   //set the cursor color
   setCursorColor(HILITE_COLOR);
   
   bool ghosted = FALSE;
   SimGui::Control *topDialog = root->getDialogNumber(1);
   if ((! active) ||
       (topDialog && (topDialog != getTopMostParent()) && (topDialog->findControlWithTag(IDCTG_DIALOG))))
   {
      ghosted = TRUE;
   }

   int colorTable[] =
   {
   	BOX_INSIDE,
   	BOX_OUTSIDE,
   	BOX_LAST_PIX,
   	BOX_FRAME,
   	BOX_GHOST_INSIDE,
   	BOX_GHOST_OUTSIDE,
   	BOX_GHOST_LAST_PIX,
   	BOX_GHOST_FRAME,   
   };
   
   int colorOffset = (ghosted ? 4 : 0);
   Point2I tl = offset;
   Point2I br(offset.x + extent.x - 1, offset.y + extent.y - 1);
   
   //offset to match the pulldown menu
   tl.x += 3;
   br.x -= 3;
   
   //bottom edge
   sfc->drawLine2d(&Point2I(tl.x, br.y - 1),
                   &Point2I(br.x, br.y - 1),
                   colorTable[0 + colorOffset]);
   sfc->drawLine2d(&Point2I(tl.x + 1, br.y),
                   &Point2I(br.x - 1, br.y),
                   colorTable[1 + colorOffset]);
   sfc->drawPoint2d(&Point2I(tl.x, br.y - 1), colorTable[2 + colorOffset]);
   sfc->drawPoint2d(&Point2I(br.x, br.y - 1), colorTable[2 + colorOffset]);
                   
   //sfc->drawLine2d(&Point2I(offset.x, offset.y + extent.y - 2),
   //                  &Point2I(offset.x + extent.x - 1, offset.y + extent.y - 2), SELECT_COLOR);
      
   // now draw the text
   RectI textClip(offset.x + 6, offset.y,
                  offset.x + extent.x - 1, offset.y + extent.y - 3);
   setTextClipRect(textClip);
   DrawText(sfc);
}

void TestEdit::DrawText(GFXSurface *sfc)
{
   bool ghosted = FALSE;
   SimGui::Control *topDialog = root->getDialogNumber(1);
   if ((! active) ||
       (topDialog && (topDialog != getTopMostParent()) && (topDialog->findControlWithTag(IDCTG_DIALOG))))
   {
      ghosted = TRUE;
   }
   
   //first update the cursor if the text is scrolling
   if (dragHit)
   {
      if ((scrollDir < 0) && (cursorPos > 0))
      {
         cursorPos--;
      }
      else if ((scrollDir > 0) && (cursorPos < (int)strlen(text)))
      {
         cursorPos++;
      }
   }

   //set the clip rect
   // to the intersection of the current clip rect and the 
   // text clip rect
   RectI clipi = textClipRect;

   if(!rectClip(&clipi, sfc->getClipRect()))
      return;

   sfc->setClipRect(&clipi);
                  
   bool focused = (! ghosted) && (root->getFirstResponder() == this);

   GFXFont *normFont = ghosted ? hFontDisabled : hFont;

   if ((bool)normFont)
   {
      // now draw the text
      Int32 txt_w = normFont->getStrWidth(text) + 3;
      Int32 txt_h = normFont->getHeight();
      Point2I cursorStart, cursorEnd;
      textOffset.y = textClipRect.upperL.y + (((textClipRect.lowerR.y - textClipRect.upperL.y) - txt_h) >> 1) + textVPosDelta;
      if ((textOffset.x == 65535) || (txt_w < (textClipRect.lowerR.x - textClipRect.upperL.x)))
      {
         textOffset.x = textClipRect.upperL.x + 3;
      }
      
      // calculate the cursor
      if (focused)
      {
         int tempWidth;
         char temp = text[cursorPos];
         text[cursorPos] = '\0';
         tempWidth = normFont->getStrWidth(text);
         cursorStart.x = textOffset.x + tempWidth;
         text[cursorPos] = temp;
         cursorEnd.x = cursorStart.x;
         cursorStart.y = textClipRect.upperL.y;
         //cursorEnd.y = cursorStart.y + (textClipRect.lowerR.y - textClipRect.upperL.y) - 8;
         cursorEnd.y = textClipRect.lowerR.y;
         
         if (cursorStart.x < (textClipRect.upperL.x))
         {
            //textOffset.x += (3 + textClipRect.upperL.x - cursorStart.x);
            cursorStart.x = 3 + textClipRect.upperL.x;
            textOffset.x = cursorStart.x - tempWidth;
            cursorEnd.x = cursorStart.x;
         }
         else if (cursorStart.x > textClipRect.lowerR.x)
         {
            //textOffset.x -= (3 + cursorStart.x - textClipRect.lowerR.x);
            cursorStart.x = textClipRect.lowerR.x - 3;
            textOffset.x = cursorStart.x - tempWidth;
            cursorEnd.x = cursorStart.x;
         }
      }
      
      //draw the text
      if ((! (bool)hFontHL) || (! focused))
      {
         blockStart = blockEnd = 0;
      }
      
      char temp;
      Point2I tempOffset = textOffset;
      //draw the portion before the highlite
      if (blockStart > 0)
      {
         temp = text[blockStart];
         text[blockStart] = '\0';
         if (bool(hFontShadow)) sfc->drawText_p(hFontShadow, &Point2I(tempOffset.x - 1, tempOffset.y + 1), text); 
         sfc->drawText_p(normFont, &tempOffset, text); 
         tempOffset.x += normFont->getStrWidth(text) + 1;
         text[blockStart] = temp;
      }
      
      //draw the hilited portion
      if (blockEnd > 0)
      {
         temp = text[blockEnd];
         text[blockEnd] = '\0';
         Int32 highlightWidth = hFontHL->getStrWidth(&text[blockStart]);
         
         if (highlightColor)
         {
            RectI hr(tempOffset.x, textClipRect.upperL.y, tempOffset.x + highlightWidth, textClipRect.lowerR.y);
            sfc->drawRect2d_f(&hr, highlightColor);
         }
         if (bool(hFontShadow)) sfc->drawText_p(hFontShadow, &Point2I(tempOffset.x - 1, tempOffset.y + 1), &text[blockStart]); 
         sfc->drawText_p(hFontHL, &tempOffset, &text[blockStart]);
         tempOffset.x += highlightWidth + 1;
         text[blockEnd] = temp;
      }
      
      //draw the portion after the highlite
      if (bool(hFontShadow)) sfc->drawText_p(hFontShadow, &Point2I(tempOffset.x - 1, tempOffset.y + 1), &text[blockEnd]); 
      sfc->drawText_p(normFont, &tempOffset, &text[blockEnd]);
      
      //draw the cursor      
      if (focused && cursorOn)
      {
         sfc->drawLine2d(&cursorStart, &cursorEnd, cursorColor);
      }
   }
}

bool TestEdit::onAdd(void)
{
   if (! Parent::onAdd()) return FALSE;
   
   pbaTag = IDPBA_TEXT_EDIT;
   loadBitmapArray();
   
   //set the fonts
   setFont(IDFNT_10_STANDARD);
   setFontHL(IDFNT_10_SELECTED);
   setFontDisabled(IDFNT_10_DISABLED);
   hFontShadow = SimResource::loadByTag(manager, IDFNT_10_BLACK, true);
   extent.y = hFont->getHeight() + 6; 
   
   return TRUE;
}

};