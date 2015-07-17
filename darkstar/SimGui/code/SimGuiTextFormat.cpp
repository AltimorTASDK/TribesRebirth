//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
#include <g_surfac.h>
#include <string.h>
#include "simTagDictionary.h"
#include "simResource.h"
#include "darkstar.strings.h"
#include "editor.strings.h"
#include "g_bitmap.h"
#include "g_font.h"
#include "simGame.h"
#include "simGuiTextFormat.h"

namespace SimGui
{

TextFormat::TextFormat(void)
{
   init();
   mOrgTextPtr = NULL;
   mbUseLocalCopy = FALSE;
   
   for (int i = 0; i < MAX_NUM_FONTS; i++)
   {
      fontArray[i].font = NULL;
      fontArray[i].tag = 0;
   }
}

void TextFormat::init()
{
  //initialize the structure
   mWidth = 0;
   mMinWidth = 0;
   mHeight = 0;
   mBitmapCount = 0;
   msgLines.clear();
}   

bool TextFormat::onAdd()
{
   if (!Parent::onAdd()) return false;

   // auto load fonts that subclasses may set
   // or persist read  
   for (int i = 0; i < MAX_NUM_FONTS; i++)
   {
      if (fontArray[i].tag)
         setFont(i, fontArray[i].tag);
   }
   
   return true;
}   

TextFormat::~TextFormat()
{
   if (mbUseLocalCopy)
   {
      delete[] (char*)mOrgTextPtr;
      mbUseLocalCopy = FALSE;
   }
   mOrgTextPtr = NULL;
}

void TextFormat::setFont(int index, Int32 tag)
{
   AssertFatal(index >= 0 && index < MAX_NUM_FONTS, "Font index must be in [0..9]");
   fontArray[index].font = SimResource::loadByTag(SimGame::get()->getManager(), tag, true);
   fontArray[index].tag = tag;
   AssertFatal(bool(fontArray[index].font), avar("Failed to load font tag: %d", tag));
}

void TextFormat::setFont(int index, const char *fontName)
{
   if (! (fontName && fontName[0])) return;
   AssertFatal(index >= 0 && index < MAX_NUM_FONTS, "Font index must be in [0..9]");
   fontArray[index].font = SimResource::get(SimGame::get()->getManager())->load(fontName);
   fontArray[index].tag = -1;
   AssertFatal(bool(fontArray[index].font), avar("Failed to load font: %s", fontName));
}

bool TextFormat::beyondBitmap()
{
   if (bmpLeftBottom > 0 && curOffset.y < bmpLeftBottom) return FALSE;
   if (bmpRightBottom > 0 && curOffset.y < bmpRightBottom) return FALSE;
   return TRUE;
}

void TextFormat::carriageReturn(int width)
{
   width;
   curOffset.y += (int)(((float)spacing / 2.0f) * (float)curFont->getHeight());
   
   //check the right and left edges...
   if (bmpLeftBottom > 0 && curOffset.y >= bmpLeftBottom)
   {
      bmpLeftBottom = -1;
      leftEdge = hardLeftEdge;
   }
   if (bmpRightBottom > 0 && curOffset.y >= bmpRightBottom)
   {
      bmpRightBottom = -1;
      rightEdge = hardRightEdge;
   }
   
   curOffset.x = leftEdge;
}

void TextFormat::formatControlString(const char *text, int width, bool reformatOldOnly, bool copyString)
{
   setUpdate();
   
   //sanity checks
   if (! bool(fontArray[0].font)) setFont(0, IDFNT_FONT_DEFAULT);
   AssertFatal(bool(fontArray[0].font), "The default font (font #0) must be set.");
   
   int minWidth = fontArray[0].font->getWidth('W');
   int i;   
   for (i = 1; i < MAX_NUM_FONTS; i++)
   {
      if (bool(fontArray[i].font))
         minWidth = max(minWidth, int(fontArray[i].font->getWidth('W')));
   }

   if (width < minWidth)
   {
      AssertFatal(0, "width is too narrow to contain characters with the current font set");
      return;
   }
   
   //initialize the structure
   init();
   mWidth = width;

   if (! reformatOldOnly)
   {
      if (mbUseLocalCopy)
      {
         delete [] (char*)mOrgTextPtr;
         mbUseLocalCopy = FALSE;
      }
      mOrgTextPtr = NULL;

      // if no new text, we are done
      if (text && copyString)
      {
         mOrgTextPtr = strnew(text);
         mbUseLocalCopy = TRUE;
      }
      else
      {
         mOrgTextPtr = text;
      }
   }   

   //make sure we have a valid string
   if (! mOrgTextPtr)
   {
      return;
   }
   
   //since we might not be using a local copy, copy the string into a local buffer and parse it there
   char *localBuffer = strnew(mOrgTextPtr);

   char *startParsePtr = localBuffer;
   char *curParsePtr;
   
   int curJustType = JUST_LEFT;
   int curFontType = 0;
   
   //initialize formatting vars
   curFont = fontArray[0].font;
   curOffset.set(0, 0);
   spacing = 2;
   hardLeftEdge = 0;
   hardRightEdge = width - 1;
   leftEdge = hardLeftEdge;
   rightEdge = hardRightEdge;
   bmpLeftBottom = -1;
   bmpRightBottom = -1;
   
   int spaceWidth = fontArray[0].font->getWidth(' ');
   int tabWidth = spaceWidth * 3;
   
   while (*startParsePtr != '\0')
   {
      curParsePtr = startParsePtr;
      
      bool processText = FALSE;
      char *endOfArgPtr;
      while (! processText)
      {
         switch (*curParsePtr)
         {
            case '<':
               endOfArgPtr = strchr(startParsePtr, '>');
               if (curParsePtr == startParsePtr && endOfArgPtr)
               {
                  //find the arg and place it into the arg buffer
                  char argBuffer[256];
                  if (curParsePtr[1] == '<')
                  {
                     //increment to the second '<' and process the text, causing the '<' to be drawn
                     curParsePtr += 2;
                     startParsePtr += 1;
                     processText = TRUE;
                     break;
                  }
                  else
                  {
                     startParsePtr++;
                     char *tempPtr = strchr(startParsePtr, '>');
                     if (! tempPtr)
                     {
                        //AssertWarn(0, "unable to locate the end of arg character '>'");
                        delete [] localBuffer;
                        return;
                     }
                     int length = tempPtr - startParsePtr;
                     if (length > 255)
                     {
                        //AssertWarn(0, "arg > 255 chars");
                        delete [] localBuffer;
                        return;
                     }
                     strncpy(argBuffer, startParsePtr, length);
                     argBuffer[length] = '\0';
                     
                     //now advance the pointers
                     curParsePtr = tempPtr + 1;
                     startParsePtr = tempPtr + 1;
                  }
                  
                  //here we handle the parameters...
                  switch (argBuffer[0])
                  {
                     case 's':
                     case 'S':
                        spacing = max(2, atoi(&argBuffer[1]));
                        break;
                        
                     case 'n':
                     case 'N':
                        if (curOffset.x > leftEdge)
                        {
                           carriageReturn(width);
                        }
                        while (! beyondBitmap())
                        {
                           carriageReturn(width);
                        }
                        curOffset.x = leftEdge;
                        break;
                        
                     case 'f':
                     case 'F':                     
                        curFontType = atoi(&argBuffer[1]);
                        curFontType = min(MAX_NUM_FONTS - 1, max(0, curFontType));
                        if (! bool(fontArray[curFontType].font)) curFontType = 0;
                        curFont = fontArray[curFontType].font;
                        break;
                        
                     case 'j':
                     case 'J':
                        if (argBuffer[1] == 'c' || argBuffer[1] == 'C') curJustType = JUST_CENTER; 
                        else if (argBuffer[1] == 'r' || argBuffer[1] == 'R') curJustType = JUST_RIGHT; 
                        else curJustType = JUST_LEFT;
                        break;
                        
                     case 'l':
                     case 'L':
                     case 'r':
                     case 'R':
                     {   
                        int spaceCount = max(0, atoi(&argBuffer[1]));
                        if (argBuffer[0] == 'l' || argBuffer[0] == 'L')
                        {
                           hardLeftEdge = spaceWidth * spaceCount;
                           if (bmpLeftBottom > 0 && curOffset.y < bmpLeftBottom)
                           {
                              leftEdge = max(leftEdge, hardLeftEdge);
                           }
                           else
                           {
                              leftEdge = hardLeftEdge;
                           }
                           if (curOffset.x < leftEdge) curOffset.x = leftEdge;
                        }
                        else
                        {
                           hardRightEdge = width - 1 - (spaceWidth * spaceCount);
                           if (bmpRightBottom > 0 && curOffset.y < bmpRightBottom)
                           {
                              rightEdge = min(rightEdge, hardRightEdge);
                           }
                           else
                           {
                              rightEdge = hardRightEdge;
                           }
                           if (curOffset.x > rightEdge - curFont->getWidth('W'))
                           {
                              carriageReturn(width);
                           }
                        }
                        if (rightEdge - leftEdge < minWidth)
                        {
                           //AssertWarn(0, "LeftMargin and RightMargin are too close to print text...");
                           break;
                        }
                        break;
                     }
                     
                     case 'v':
                     case 'V':
                     {
                        //we substitute the value of the variable into the copy of the string 
                        if (mbUseLocalCopy)
                        {
                           char argLength = strlen(argBuffer) + 2;  //the arg + '<' + '>'
                           if (argLength > 1)
                           {
                              const char *consoleVar = Console->getVariable(&argBuffer[1]);
                              char *startPtr = (char*)mOrgTextPtr + int(startParsePtr) - int(localBuffer) - argLength;
                              char *endPtr = startPtr + argLength;
                              int varLength = strlen(consoleVar);
                              
                              //build the new string
                              char *tempBuf = new char[strlen(mOrgTextPtr) + 1 + varLength - argLength];
                              char *tempBufPtr = &tempBuf[0];
                              int firstPart = int(startPtr) - int(mOrgTextPtr);
                              strncpy(tempBufPtr, mOrgTextPtr, firstPart);
                              tempBufPtr += firstPart;
                              strcpy(tempBufPtr, consoleVar);
                              tempBufPtr += varLength;
                              strcpy(tempBufPtr, endPtr);
                              
                              //delete the old, and replace with the new
                              delete [] (char*)mOrgTextPtr;
                              mOrgTextPtr = tempBuf;
                              
                              //do the same for the local buf
                              tempBuf = strnew(mOrgTextPtr);
                              delete [] localBuffer;
                              localBuffer = tempBuf;
                              startParsePtr = localBuffer + int(firstPart);
                              curParsePtr = startParsePtr;
                           }
                        }
                        
                        break;
                     }
                     
                     case 'b':
                     case 'B':
                     {
                        if (mBitmapCount == MAX_NUM_BITMAPS)
                        {
                           //AssertWarn(0, "Only 8 bitmaps per objective...");
                           break;
                        }
                        
                        //check for spacing around the bitmap.  Default is 4 pix on each side, and top and bottom
                        //eg.  <B0,12:bitmap.bmp> if you want no spaces top and bottom, and 12 pix on each side
                        char *argPtr = &argBuffer[1];
                        Point2I bmpPadding(4, 0);
                        char *bmpParamPtr = strchr(argBuffer, ':');
                        if (bmpParamPtr)
                        {
                           char *tempPtr = strchr(&argBuffer[1], ',');
                           if (tempPtr)
                           {
                              *tempPtr = '\0';
                              bmpPadding.x = atoi(argPtr);
                              *bmpParamPtr = '\0';
                              bmpPadding.y = atoi(&tempPtr[1]);
                           }
                           
                           //set the argPtr past the ':'
                           argPtr = bmpParamPtr + 1;
                        }
                        
                        //load the bmp
                     	ResourceManager *rm = SimResource::get(SimGame::get()->getManager());
                        mBitmaps[mBitmapCount].bmp = rm->load(argPtr);
                        
                        if (! bool(mBitmaps[mBitmapCount].bmp))
                        {
                           //AssertWarn(0, "Unable to load .bmp");
                           break;
                        }
                        
                        //now deal with the bmp...
                        GFXBitmap *bmp = mBitmaps[mBitmapCount].bmp;
                        
                        //set the params
                        mBitmaps[mBitmapCount].justification = curJustType;
                        mBitmaps[mBitmapCount].width = bmp->getWidth() + (2 * bmpPadding.x);
                        
                        //first make sure the bitmap can fit
                        if (mBitmaps[mBitmapCount].width > width) break;
                        while (mBitmaps[mBitmapCount].width > rightEdge - curOffset.x + 1)
                        {
                           carriageReturn(width);
                        }
                        
                        //now check for margin adjustments
                        if (curJustType == JUST_LEFT)
                        {
                           //set the bmp offset
                           mBitmaps[mBitmapCount].offset = curOffset;
                           mBitmaps[mBitmapCount].offset += bmpPadding;
                        
                           //update the offset
                           curOffset.x += mBitmaps[mBitmapCount].width;
                           mMinWidth = max((long)mMinWidth, curOffset.x);
                           
                           //set the bottom of the bitmap
                           bmpLeftBottom = max((int)bmpLeftBottom, int(curOffset.y + bmp->getHeight() + (2 * bmpPadding.y)));
                           leftEdge = curOffset.x;
                        }
                        else if (curJustType == JUST_CENTER)
                        {
                           //set the bmp offset
                           mBitmaps[mBitmapCount].offset = Point2I(leftEdge, curOffset.y);
                           mBitmaps[mBitmapCount].offset += bmpPadding;
                           
                           //set the bottom of the bitmap
                           bmpLeftBottom = max((int)bmpLeftBottom, int(curOffset.y + bmp->getHeight() + (2 * bmpPadding.y)));
                           leftEdge += mBitmaps[mBitmapCount].width;
                           mMinWidth = max((long)mMinWidth, long(mBitmaps[mBitmapCount].width));
                        }
                        else if (curJustType == JUST_RIGHT)
                        {
                           //set the bottom of the bitmap
                           bmpRightBottom = max((int)bmpRightBottom, int(curOffset.y + bmp->getHeight() + (2 * bmpPadding.y)));
                           rightEdge -= mBitmaps[mBitmapCount].width;
                           
                           //set the bmp offset
                           mBitmaps[mBitmapCount].offset = Point2I(rightEdge, curOffset.y);
                           mBitmaps[mBitmapCount].offset += bmpPadding;
                           
                        }
                        
                        mBitmapCount++;
                        
                        break;   
                     }
                        
                     default:
                        //AssertFatal(0, "Unknown parameter!");
                        break;
                  }
               }
               else if (endOfArgPtr)
               {
                  processText = TRUE;
               }
               else
               {
                  curParsePtr++;
               }
               break;
               
            case '\n':
               if (curParsePtr == startParsePtr)
               {
                  //carriage return, and on to the next character
                  carriageReturn(width);
                  
                  startParsePtr++;
                  curParsePtr++;
               }
               else
               {
                  processText = TRUE;
               }
               break;
            
            case '\t':
               if (curParsePtr == startParsePtr)
               {
                  //find the next tab position
                  int tabPosition = leftEdge;
                  while (tabPosition <= curOffset.x) tabPosition += tabWidth;
                  curOffset.x = tabPosition;
                  
                  startParsePtr++;
                  curParsePtr++;
               }
               else
               {
                  processText = TRUE;
               }
               break;
            
            case '\0':
               processText = TRUE;
               break;
               
            default:
               curParsePtr++;
               break;
         }
      }
      
      //make the text fit
      if (processText)
      {
         //first make sure curOffset.x didn't get pushed too close to the right edge
         if (rightEdge - curOffset.x + 1 < curFont->getWidth('W'))
         {
            carriageReturn(width);
         }
         
         //sub curParsePtr with '\0' temporarily
         char eolTemp = *curParsePtr;
         *curParsePtr = '\0';
         
         int lengthLeft = strlen(startParsePtr);
         char *tempPtr, *noSpacePtr;
         char temp;
         
         while (lengthLeft > 0)
         {
         
            //set the tempPtr
            tempPtr = curParsePtr;
            temp = '\0';
            noSpacePtr = NULL;
            
            //if the string is too long to fit, find a space on which to break
            while (1)
            {
               bool forceNewLine = FALSE;
               //if the string fits
               if (curFont->getStrWidth(startParsePtr) <= rightEdge - curOffset.x + 1)
               {
                  //if we found a space, break and print the string
                  if (temp == ' ' || temp == '\0')
                  {
                     break;
                  }
                  else
                  {
                     //noSpacePtr points to the rightmost char in case there are no spaces at all
                     if (! noSpacePtr) noSpacePtr = tempPtr;
                     
                     //see if we've at the beginning of the string, print up to noSpacePtr
                     if (tempPtr <= startParsePtr + 1)
                     {
                        //first see if we can do a carriage return, and continue
                        if (curOffset.x > leftEdge + curFont->getWidth('W'))
                        {
                           if (tempPtr == startParsePtr + 1 && *startParsePtr == ' ')
                           {
                              startParsePtr++;
                              lengthLeft--;
                           }
                           *tempPtr = temp;
                           tempPtr = curParsePtr;
                           temp = '\0';
                           noSpacePtr = NULL;
                           carriageReturn(width);
                           forceNewLine = TRUE;
                        }
                        else
                        {
                           *tempPtr = temp;
                           tempPtr = noSpacePtr;
                           temp = *tempPtr;
                           *tempPtr = '\0';
                           break;
                        }
                     }
                  }
               }
               
               //at this point, we haven't found a space break, so back up a char
               if (! forceNewLine)
               {
                  *tempPtr = temp;
                  tempPtr--;
                  temp = *tempPtr;
                  *tempPtr = '\0';
               }
            }
            
            //we have now found a string which fits - set the vars
            msgFormat newLine;
            newLine.fontType = curFontType;
            newLine.justification = curJustType;
            newLine.offset = curOffset;
            newLine.startChar = startParsePtr - localBuffer;
            newLine.length = tempPtr - startParsePtr;
            if (curJustType == JUST_RIGHT)
            {
               newLine.lineWidth = rightEdge - curOffset.x;
            }
            else
            {
               newLine.lineWidth = rightEdge - leftEdge;
            }
            
            //add the line...
            msgLines.push_back(newLine);
            
            //update the lengthLeft
            lengthLeft -= strlen(startParsePtr);
            
            //if there is still more to go, reset the offset to the next line
            if (lengthLeft)
            {
               //update the offset
               curOffset.x += curFont->getStrWidth(startParsePtr);
               mMinWidth = max((long)mMinWidth, curOffset.x);
               
               carriageReturn(width);
               
               //replace the missing character and set startParsePtr to be the next string portion
               *tempPtr = temp;
               startParsePtr = tempPtr;
            
               //it also means we broke on a space...  skip the spaces at the start of the line
               while (lengthLeft && (*startParsePtr == ' '))
               {
                  startParsePtr++;
                  lengthLeft--;
               }
               
               if (! lengthLeft) *tempPtr = temp;
            }
            else
            {
               //update the offset
               curOffset.x += curFont->getStrWidth(startParsePtr);
               mMinWidth = max((long)mMinWidth, curOffset.x);
               
               //replace the missing character
               *tempPtr = temp;
            }
         }
      
         //finished parsing this segment, replace the eol character, and reset startParsePtr
         *curParsePtr = eolTemp;
         startParsePtr = curParsePtr;
      }
   }
   
   //always carriage return if there is still text;
   if (curOffset.x > leftEdge) 
   {
      carriageReturn(width);
   }
   
   //make sure we're also past the end of the bitmaps
   int maxBottom = max(bmpLeftBottom, bmpRightBottom);
   if (! beyondBitmap())
   {
      curOffset.y = maxBottom;
   }
   
   mHeight = curOffset.y + 2;
   
   //now make a pass through and center/right justify all the offsets
   i = 0;
   bool finished = FALSE;
   if (i >= msgLines.size()) finished = TRUE;
   while (! finished)
   {
      //first search for a string that is not left justified
      msgFormat *msg = &msgLines[i];
      if (msg->justification != JUST_LEFT)
      {
         int tempBegin = i;
         BYTE curJust = msg->justification;
         int curWidth = 0;
         int curHeight = msg->offset.y;

         do
         {
            //get the font
            GFXFont *font = fontArray[msg->fontType].font;
            if (font && msg->length > 0)
            {
               //get the string width
               char *textPtr = localBuffer + msg->startChar;
               char temp = textPtr[msg->length];
               textPtr[msg->length] = '\0';
               curWidth += font->getStrWidth(textPtr);
               textPtr[msg->length] = temp;
            }
            i++;
            msg = &msgLines[i];
         } while(i < msgLines.size() && msg->offset.y == curHeight && msg->justification == curJust);
         
         //now loop through and ajust all  the offsets
         for (; tempBegin < i; tempBegin++)
         {
            msgFormat *tempMsg = &msgLines[tempBegin];
            if (curJust == JUST_CENTER) tempMsg->offset.x += (tempMsg->lineWidth - curWidth) / 2;
            else tempMsg->offset.x += tempMsg->lineWidth - curWidth;
         }
      }
      else i++;
      if (i >= msgLines.size()) finished = TRUE;
   }
   
   //now readjust all the bitmaps
   i = 0;
   finished = FALSE;
   if (i >= mBitmapCount) finished = TRUE;
   while (! finished)
   {
      if (mBitmaps[i].justification == JUST_CENTER)
      {
         int tempBegin = i;
         BYTE curJust = mBitmaps[i].justification;
         int curWidth = 0;
         int curHeight = mBitmaps[i].offset.y;
         
         do
         {
            //add the width
            curWidth += mBitmaps[i].width;
            i++;
         } while(i < mBitmapCount && mBitmaps[i].offset.y == curHeight && mBitmaps[i].justification == curJust);
         
         //now loop through and adjust all  the offsets
         for (; tempBegin < i; tempBegin++)
         {
            if (curJust == JUST_CENTER) mBitmaps[tempBegin].offset.x += (width - curWidth) / 2;
            else mBitmaps[tempBegin].offset.x += width - curWidth;
         }
      }
      else i++;
      if (i >= mBitmapCount) finished = TRUE;
   }
   
   //delete the local buffer before returning
   delete [] localBuffer;
}

void TextFormat::onRender(GFXSurface *sfc, Point2I offset, const Box2I &)
{
   if (!mOrgTextPtr)
      return;
      
   //use a local buffer
   char *localBuffer = strnew(mOrgTextPtr);

   Vector<msgFormat>::iterator i;
   for (i = msgLines.begin(); i != msgLines.end(); i++)
   {
      //get the font
      GFXFont *font = fontArray[i->fontType].font;
      AssertFatal(font, "what happened to the font?");
      
      //find out where to draw the text;
      Point2I textOffset = offset + i->offset;
      
      //draw the text
      char *textPtr = localBuffer + i->startChar;
      if (i->length > 0)
      {
         char temp = textPtr[i->length];
         textPtr[i->length] = '\0';
         
         sfc->drawText_p(font, &textOffset, textPtr);
         
         textPtr[i->length] = temp;
      }
   }
   
   //delete the local buffer
   delete [] localBuffer;
   
   //now draw all the bitmaps
   for (int j = 0; j < mBitmapCount; j++)
   {
      Point2I bmpOffset = offset + mBitmaps[j].offset;
      sfc->drawBitmap2d(mBitmaps[j].bmp, &bmpOffset);
   }
}

int TextFormat::getWidth(void)
{
   return mWidth;
}

int TextFormat::getMinWidth(void)
{
   return mMinWidth;
}

int TextFormat::getHeight(void)
{
   return mHeight;
}

Persistent::Base::Error TextFormat::write( StreamIO &sio, int a, int b)
{
   return Parent::write(sio, a, b);
}

Persistent::Base::Error TextFormat::read( StreamIO &sio, int a, int b)
{
   return Parent::read(sio, a, b);
}

};