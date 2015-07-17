//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <string.h>
#include <g_surfac.h>
#include <inspect.h>
#include "simGuiTextEdit.h"
#include "r_clip.h"
#include "editor.strings.h"

namespace SimGui
{

TextEdit::TextEdit()
{
   active = true;
   maxStrLen = SimpleText::MAX_STRING_LENGTH;
   highlightColor = 0;

   insertOn = TRUE;
   blockStart = 0;
   blockEnd = 0;
   cursorPos = 0;
   cursorColor = 0;

   dragHit = FALSE;
	scrollDir = 0;
   
   mbNumbersOnly = FALSE;

   undoText[0] = '\0';
   undoBlockStart = 0;
   undoBlockEnd = 0;
   undoCursorPos = 0;

	textOffset.x = 65535;
}

void TextEdit::getText(char *dest)
{
   if (dest)
      strcpy(dest, text);   
}  
 
void TextEdit::setText(Int32 tag)
{
   textTag = tag;
   Parent::setText();
   cursorPos = strlen(text);
}

void TextEdit::setText(const char *txt)
{
   Parent::setText(txt);
   cursorPos = strlen(text);
}

bool TextEdit::becomeFirstResponder()
{
   numFramesElapsed = 0;
   timeLastCursorFlipped = manager->getCurrentTime();
   cursorOn = 1;
   return Parent::becomeFirstResponder();   
}

int TextEdit::setCursorPos(const Point2I &offset)
{
    int charCount = strlen(text);
    int charLength = 0;
    int curX;
    
    curX = offset.x - textClipRect.upperL.x;
    setUpdate();

    //if the cursor is too far to the left
    if (curX < 0) return -1;
    
    //if the cursor is too far to the right
    if (curX > (textClipRect.lowerR.x - textClipRect.upperL.x)) return -2;
    
    curX = offset.x - textOffset.x;
    int pos = 0;
    while (pos < charCount)
    { 
        charLength += hFont->getWidth(text[pos]) + 1;
        if (charLength > curX)
        {
            break;
        }
        pos++;
    }
    
    return pos;
}

void TextEdit::onMouseDown(const Event &event)
{
   dragHit = false;

   //undo any block function
   blockStart = 0;
   blockEnd = 0;

   //find out where the cursor should be
   int pos = setCursorPos(event.ptMouse);

   //if the position is to the left
   if (pos == -1) cursorPos = 0;
   //else if the position is to the right
   else if (pos == -2) cursorPos = strlen(text);
   //else set the cursorPos
   else cursorPos = pos;

   //save the mouseDragPos
   mouseDragStart = cursorPos;

   //lock the mouse
   mouseLock();

   //set the drag var
   dragHit = TRUE;
   
   //let the parent get the event
   Parent::onMouseDown(event);
}

void TextEdit::onMouseDragged(const SimGui::Event &event)
{
    int pos = setCursorPos(event.ptMouse);
    
    //if the position is to the left
    if (pos == -1)
    {
       scrollDir = -1;
    }
    //else if the position is to the right
    else if (pos == -2)
    {
      scrollDir = 1;
    }
    //else set the new cursor position
    else
    {
        scrollDir = 0;
        cursorPos = pos;
    }
    
    //update the block
    blockStart = min(cursorPos, mouseDragStart);
    blockEnd = max(cursorPos, mouseDragStart);
    if (blockStart < 0) blockStart = 0;
    
    if (blockStart == blockEnd)
    {
        blockStart = blockEnd = 0;
    }
    
   //let the parent get the event
   Parent::onMouseDragged(event);
}

void TextEdit::onMouseUp(const SimGui::Event &event)
{
   event;
   dragHit = FALSE;
   scrollDir = 0;
   mouseUnlock();
   
   //let the parent get the event
   Parent::onMouseUp(event);
}

void TextEdit::saveUndoState(void)
{
    //save the current state
    strcpy(undoText, text);
    undoBlockStart = blockStart;
    undoBlockEnd = blockEnd;
    undoCursorPos = cursorPos;
}

void TextEdit::onKeyUp(const Event &event)
{
   event;
   setVariable(text);
   setUpdate();
   onAction();
}

void TextEdit::onKeyRepeat(const Event &event)
{
   onKeyDown(event);   
}   

void TextEdit::onKeyDown(const Event &event)
{
   int stringLen = strlen(text);
   setUpdate();
   
    if (event.modifier & SI_SHIFT)
    {
        switch(event.diKeyCode)
        {
             case DIK_HOME:
                blockStart = 0;
                blockEnd = cursorPos;
                cursorPos = 0;
                return;
                
             case DIK_END:
                blockStart = cursorPos;
                blockEnd = stringLen;
                cursorPos = stringLen;
                return;
            
            case DIK_LEFT:
                if ((cursorPos > 0) & (stringLen > 0))
                {
                    //if we already have a selected block
                    if (cursorPos == blockEnd)
                    {
                        cursorPos--;
                        blockEnd--;
                        if (blockEnd == blockStart)
                        {
                            blockStart = 0;
                            blockEnd = 0;
                        }
                    }
                    else {
                        cursorPos--;
                        blockStart = cursorPos;
                        
                        if (blockEnd == 0)
                        {
                            blockEnd = cursorPos + 1;
                        }
                    }
                }
                return;
                
            case DIK_RIGHT:
                if (cursorPos < stringLen)
                {
                    if ((cursorPos == blockStart) && (blockEnd > 0))
                    {
                        cursorPos++;
                        blockStart++;
                        if (blockStart == blockEnd)
                        {
                            blockStart = 0;
                            blockEnd = 0;
                        }
                    }
                    else
                    {
                        if (blockEnd == 0)
                        {
                            blockStart = cursorPos;
                            blockEnd = cursorPos;
                        }
                        cursorPos++;
                        blockEnd++;
                    }
                }
                return;
        }
    }
   else if (event.modifier & SI_CTRL)
   {
      switch(event.diKeyCode)
      {
         case DIK_C:
         case DIK_X:
            if (blockEnd > 0)
            {
                //save the current state
                saveUndoState();
                
                char buf[SimpleText::MAX_STRING_LENGTH + 1], temp;
                
                //copy to the clipboard    
                if (!OpenClipboard(getCanvas()->getHandle()))
                    return;
                LPTSTR  lptstrCopy; 
                HGLOBAL hglbCopy;
                hglbCopy = GlobalAlloc(GMEM_DDESHARE, (blockEnd - blockStart + 1)); 
                if (hglbCopy == NULL) 
                { 
                    CloseClipboard(); 
                    return; 
                } 

                // Lock the handle and copy the text to the buffer. 
                temp = text[blockEnd];
                text[blockEnd] = '\0';
                lptstrCopy = (char*)(GlobalLock(hglbCopy)); 
                strcpy(lptstrCopy, &text[blockStart]); 
                text[blockEnd] = temp;

                GlobalUnlock(hglbCopy);

                // Place the handle on the clipboard. 
                SetClipboardData(CF_TEXT, hglbCopy); 
                
                CloseClipboard();
                
                if (event.diKeyCode == DIK_X)
                {
                    strcpy(buf, &text[blockEnd]);
                    cursorPos = blockStart;
                    strcpy(&text[blockStart], buf);
                }
                
                blockStart = 0;
                blockEnd = 0;
            }
            return;
            
         case DIK_V:
            char buf[SimpleText::MAX_STRING_LENGTH + 1];
            
            //delete anything hilited
            if (blockEnd > 0)
            {
                //save the current state
                saveUndoState();
                
                strcpy(buf, &text[blockEnd]);
                cursorPos = blockStart;
                strcpy(&text[blockStart], buf);
                blockStart = 0;
                blockEnd = 0;
            }
      
            HGLOBAL   hglb; 
            LPTSTR    lptstr; 
            if (!IsClipboardFormatAvailable(CF_TEXT)) 
                return; 
            if (! OpenClipboard(getCanvas()->getHandle()))
                return; 
     
            hglb = GetClipboardData(CF_TEXT); 
            if (hglb != NULL) 
            { 
                lptstr = (char*)GlobalLock(hglb); 
                if (lptstr != NULL) 
                { 
                    int pasteLen = strlen(lptstr);
                    if ((stringLen + pasteLen) > maxStrLen)
                    {
                        pasteLen = maxStrLen - stringLen;
                    }
                                             
                    if (cursorPos == stringLen)
                    {
                        strncpy(&text[cursorPos], lptstr, pasteLen);
                        text[cursorPos + pasteLen] = '\0';
                    }
                    else
                    {
                        strcpy(buf, &text[cursorPos]);
                        strncpy(&text[cursorPos],lptstr, pasteLen);
                        strcpy(&text[cursorPos + pasteLen], buf);
                    }
                    cursorPos += pasteLen;
                    GlobalUnlock(hglb); 
                } 

            } 
            CloseClipboard();
            return; 
        
         case DIK_U:
            if (! dragHit)
            {
                char buf[SimpleText::MAX_STRING_LENGTH + 1];
                int tempBlockStart;
                int tempBlockEnd;
                int tempCursorPos;
                
                //save the current
                strcpy(buf, text);
                tempBlockStart = blockStart;
                tempBlockEnd = blockEnd;
                tempCursorPos = cursorPos;
                
                //restore the prev
                strcpy(text, undoText);
                blockStart = undoBlockStart;
                blockEnd = undoBlockEnd;
                cursorPos = undoCursorPos;
                
                //update the undo
                strcpy(undoText, buf);
                undoBlockStart = tempBlockStart;
                undoBlockEnd = tempBlockEnd;
                undoCursorPos = tempCursorPos;
                
                return;
             }
                
         case DIK_DELETE:
         case DIK_BACK:
            //save the current state
            saveUndoState();
            
            //delete everything in the field
            text[0] = '\0';
            cursorPos = 0;
            blockStart = 0;
            blockEnd = 0;
            return;
      } //switch
   }
   else
   {
      switch(event.diKeyCode)
      {
		 case DIK_LEFT:
            blockStart = 0;
            blockEnd = 0;
            if (cursorPos > 0)
            {
                cursorPos--;
            }
            return;
            
		 case DIK_RIGHT:
            blockStart = 0;
            blockEnd = 0;
            if (cursorPos < stringLen)
            {
                cursorPos++;
            }
            return;
                     
         case DIK_BACK:
            if (blockEnd > 0)
            {
                char buf[SimpleText::MAX_STRING_LENGTH + 1];
                strcpy(buf, &text[blockEnd]);
                cursorPos = blockStart;
                strcpy(&text[blockStart], buf);
                blockStart = 0;
                blockEnd = 0;
            }
            else if (cursorPos > 0)
            {
                char buf[SimpleText::MAX_STRING_LENGTH + 1];
                strcpy(buf, &text[cursorPos]);
                cursorPos--;
                strcpy(&text[cursorPos], buf);
            }
            return;
            
         case DIK_DELETE:
            //save the current state
            saveUndoState();
                
            if (blockEnd > 0)
            {
                char buf[SimpleText::MAX_STRING_LENGTH + 1];
                strcpy(buf, &text[blockEnd]);
                cursorPos = blockStart;
                strcpy(&text[blockStart], buf);
                blockStart = 0;
                blockEnd = 0;
            }
            else if (cursorPos < stringLen)
            {
                char buf[SimpleText::MAX_STRING_LENGTH + 1];
                strcpy(buf, &text[cursorPos + 1]);
                strcpy(&text[cursorPos], buf);
            }
            return;
         
         case DIK_INSERT:
            insertOn = !insertOn;
            return;
         
         case DIK_HOME:
            blockStart = 0;
            blockEnd = 0;
            cursorPos = 0;
            return;
            
         case DIK_END:
            blockStart = 0;
            blockEnd = 0;
            cursorPos = stringLen;
            return;
            
         }
   }
   
   switch (event.diKeyCode)
   {
      case DIK_UP:
      case DIK_DOWN:
      case DIK_TAB:
      case DIK_ESCAPE:
         Parent::onKeyDown(event);
         return;
   }
   
   //if ((! mbNumbersOnly && isprint(event.ascii)) || (mbNumbersOnly && event.ascii >= '0' && mbNumbersOnly && event.ascii <= '9'))
   if (isprint(event.ascii))
   {
      //see if it's a number field
      if (mbNumbersOnly)
      {
         if (event.ascii == '-')
         {
            //a minus sign only exists at the beginning, and only a single minus sign
            if (cursorPos != 0) return;
            if (insertOn && (text[0] == '-')) return;
         }
         else if (event.ascii < '0' || event.ascii > '9')
         {
            return;
         }
      }
      
      //save the current state
      saveUndoState();
                
      //delete anything hilited
      if (blockEnd > 0)
      {
          char buf[SimpleText::MAX_STRING_LENGTH + 1];
          strcpy(buf, &text[blockEnd]);
          cursorPos = blockStart;
          strcpy(&text[blockStart], buf);
          blockStart = 0;
          blockEnd = 0;
      }
      
      if ((insertOn && (stringLen < maxStrLen)) ||
          ((! insertOn) && (cursorPos < maxStrLen)))
      {
         if (cursorPos == stringLen)
         {
            text[cursorPos++] = event.ascii;
            text[cursorPos] = '\0';
         }
         else
         {
            if (insertOn)
            {
                char buf[SimpleText::MAX_STRING_LENGTH + 1];
                strcpy(buf, &text[cursorPos]);
                text[cursorPos] = event.ascii;
                cursorPos++;
                strcpy(&text[cursorPos], buf);
            }
            else
            {
                text[cursorPos++] = event.ascii;
                if (cursorPos > stringLen)
                {
                   text[cursorPos] = '\0';
                }
            }
         }
      }
      return;
   }
}

void TextEdit::parentResized(const Point2I &oldParentExtent, const Point2I &newParentExtent)
{
   Parent::parentResized(oldParentExtent, newParentExtent);
   textOffset.x = 65535;
}

void TextEdit::setTextClipRect(const RectI &clipRect)
{
   textClipRect.upperL.x = clipRect.upperL.x;
   textClipRect.upperL.y = clipRect.upperL.y;
   textClipRect.lowerR.x = clipRect.lowerR.x;
   textClipRect.lowerR.y = clipRect.lowerR.y;
}

void TextEdit::onRender(GFXSurface *sfc, Point2I offset, const Box2I & /*updateRect*/)
{
   RectI textClip(offset.x, offset.y, offset.x + extent.x - 1, offset.y + extent.y - 1);
   setTextClipRect(textClip);
   sfc->drawRect2d_f(&textClip, 255);
   DrawText(sfc);
}

void TextEdit::onPreRender()
{
   bool focused =  (root->getFirstResponder() == this);

   if(focused)
   {
      SimTime timeElapsed = manager->getCurrentTime() - timeLastCursorFlipped;
      numFramesElapsed++;
      if ((timeElapsed > SimTime(0.5f)) && (numFramesElapsed > 3) )
      {
         cursorOn = 1 - cursorOn;
         timeLastCursorFlipped = manager->getCurrentTime();   
         numFramesElapsed = 0;
         setUpdate();
      }
   }
}    

void TextEdit::DrawText(GFXSurface *sfc)
{
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
                  
   bool focused =  (root->getFirstResponder() == this);

   GFXFont *normFont = (!active && (bool)hFontDisabled) ? hFontDisabled : hFont;

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
      
      //also verify the block start/end
      if ((blockStart > int(strlen(text))) || (blockEnd > int(strlen(text))) || (blockStart > blockEnd))
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
         sfc->drawText_p(hFontHL, &tempOffset, &text[blockStart]);
         tempOffset.x += highlightWidth + 1;
         text[blockEnd] = temp;
      }
      
      //draw the portion after the highlite
      sfc->drawText_p(normFont, &tempOffset, &text[blockEnd]);
      
      //draw the cursor      
      if (focused && cursorOn)
      {
         sfc->drawLine2d(&cursorStart, &cursorEnd, cursorColor);
      }
   }
}

void TextEdit::inspectRead(Inspect *inspector)
{
   Parent::inspectRead(inspector);
   
   inspector->read(IDITG_TEXT_NUMBERS_ONLY, mbNumbersOnly);
   
   Int32 newLen;
   inspector->read(IDITG_MAX_STR_LEN, newLen);
   maxStrLen = min((int)SimpleText::MAX_STRING_LENGTH, (int)newLen);
}   

void TextEdit::inspectWrite(Inspect *inspector)
{
   Parent::inspectWrite(inspector);
   inspector->write(IDITG_TEXT_NUMBERS_ONLY, mbNumbersOnly);
   inspector->write(IDITG_MAX_STR_LEN, (Int32)maxStrLen);
}   

bool TextEdit::hasText()
{
   return ((bool)(strlen(text) > 0));
}

Persistent::Base::Error TextEdit::write( StreamIO &sio, int version, int user)
{
   DWORD numbersOnly = mbNumbersOnly;
   sio.write(numbersOnly);
   
   DWORD unused = 0;
   sio.write(unused);
   sio.write(unused);

   sio.write(maxStrLen);
   return Parent::write(sio, version, user);
}

Persistent::Base::Error TextEdit::read( StreamIO &sio, int version, int user)
{
   DWORD numbersOnly;
   sio.read(&numbersOnly);
   mbNumbersOnly = numbersOnly;
   
   DWORD temp;
   sio.read(&temp);
   sio.read(&temp);

   sio.read(&maxStrLen);
   return Parent::read(sio, version, user);
}   

};