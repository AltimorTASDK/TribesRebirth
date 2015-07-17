#include <g_surfac.h>
#include <string.h>
#include <m_box.h>
#include "simGuiTextWrap.h"
#include "simTagDictionary.h"
#include "darkstar.strings.h"
#include "editor.strings.h"
#include "simResource.h"

namespace SimGui
{

//------------------------------------------------------------------------------
// comuptes offsets into 'txt' which will word wrap it to 'lineWidth'
// newlines in 'txt' are accounted for during the word wrapping.
//
// There will be an equal number of entries in startLineOffset and lineLen
// Each starLineOffset and lineLen pair represents what text should be 
// displayed to denote a single word wrapped line.  
//
// ie, if startLineOffset is {0, 12} and lineLen is {8, 7}
// then the first line of text displayed should be txt[0] to txt[7]
// and the second line of text displayed should be txt[12] to txt[18] 
// txt[8] to txt[11] are skipped.  This is possible because space and newline 
// characters within txt should not be displayed between lines.
void wrapText(GFXFont *font, const char *txt, Int32 lineWidth, Vector<Int32> &startLineOffset, Vector<Int32> &lineLen)
{
   startLineOffset.clear();
   lineLen.clear();

   if (!txt || !font || lineWidth < 20)
      return;
         
   if (lineWidth < font->getWidth('W')) //make sure the line width is greater then a single character
      return;

   Int32 len = strlen(txt);
   if (!len)
      return;

   char buffer[256];
   Int32 startLine, k; 
   for (int i = 0; i < len;)
   {
      startLine = i;
      startLineOffset.push_back(startLine);

      // loop until the string is too large
      bool needsNewLine = false;
      for (k = 0; i < len; i++)
      {
         buffer[k++] = txt[i];
         AssertFatal(k < 255, "doh, line lenght exceeds buffer.  Increase buffer size");
         buffer[k] = '\0';
         if ( (txt[i] == '\n') || (font->getStrWidth(buffer) > lineWidth) )
         {
            needsNewLine = true;
            break;      
         }
      }

      if (!needsNewLine)
      {
         // we are done!
         lineLen.push_back(k);
         return;
      }

      // now determine where to put the newline
      if (isspace(buffer[k - 1]))
      {
         // if the last character is a space character or a newline,
         // then we need to put a newline here.
         lineLen.push_back(k - 1);
      }
      else
      {
         // else we need to backtrack until we find a either space character 
         // or \\ character to break up the line. 
         int j;
         bool firstSlashFound = false;
         for (j = k - 1; j >= 0; j--)
         {
            if (isspace(buffer[j]))
               break;

            if (firstSlashFound)
            {
               if (buffer[j] != '\\')
               {
                  j++; // we want to break the line at the start of the \\ char
                  break;   
               }
            }
            else
            {
               if (buffer[j] == '\\')
                  firstSlashFound = true;
            }
         }

         if (j < 0)
         {
            // the line consists of a single word!              
            // So, just break up the word
            j = k - 1;
         }
         lineLen.push_back(j);
         i = startLine + j - 1;
      }

      // now we need to increment through any space characters at the
      // beginning of the next line
      for (i++; i < len; i++)
      {
         if (!isspace(txt[i]) || txt[i] == '\n')
            break;      
      }
   }
}   

//------------------------------------------------------------------------------
TextWrap::TextWrap()
{
   fontNameTag = IDFNT_FONT_DEFAULT;
   fontNameTagDisabled = IDFNT_FONT_DEFAULT;
   textTag = IDSTR_STRING_DEFAULT;
   lineSpacing = 5;

   text = NULL;
   extent.set(100, 25);
   alignment = LeftJustify;
   ignoreClick = false;
   enabled = true;
   textVPosDelta = 0;
   inset.set(0,0);
}

//------------------------------------------------------------------------------
bool TextWrap::pointInControl(Point2I &parentCoordPoint)
{
   if (!root->inEditMode() && ignoreClick)
      return false;
   return Parent::pointInControl(parentCoordPoint);
}

//------------------------------------------------------------------------------
void TextWrap::setUpdate()
{
   Parent::setUpdate();
   wrapText(hFont, text, extent.x - inset.x - inset.x, startLineOffset, lineLen);
}  

//------------------------------------------------------------------------------
void TextWrap::editResized()
{
   Parent::editResized();
   setUpdate();
}   
 
//------------------------------------------------------------------------------
void TextWrap::setFont(Int32 tag)
{
   fontNameTag = tag;
   hFont = SimResource::loadByTag(manager, fontNameTag, true);
   setUpdate();
}

//------------------------------------------------------------------------------
void TextWrap::setFontDisabled(Int32 tag)
{
   fontNameTagDisabled = tag;
   hFontDisabled = SimResource::loadByTag(manager, fontNameTagDisabled, true);
}

//------------------------------------------------------------------------------
void TextWrap::setText(const char *txt)
{
   if(text)
      delete[] text;
   text = NULL;
   if (txt)
   {
      totalTextLen = strlen(txt);
      text = new char[totalTextLen + 1];
      strcpy(text, txt);
      for (int i = 0; i < totalTextLen - 2; i++)
      {
         if ((text[i] == '\\') && (text[i + 1] == 'n'))
         {
            text[i] = '\n';
            text[i+1] = ' ';
         }
      }
   }
   setUpdate();
} 

//------------------------------------------------------------------------------
void TextWrap::resize(const Point2I &newPosition, const Point2I &newExtent)
{
   Parent::resize(newPosition, newExtent);
   setUpdate();
}                                                

//------------------------------------------------------------------------------
void TextWrap::onRender(GFXSurface *sfc, Point2I offset, const Box2I &)
{
   GFXFont *useFont = (enabled) ? hFont : hFontDisabled;

   if (!useFont || !text)
      return;

   if (!startLineOffset.size())
      return;

   AssertFatal(startLineOffset.size() == lineLen.size(), "doh!, startLineOffset and lineLen vectors must be same len");
   
   Int32 fontHeight = useFont->getHeight();
   Point2I drawPt(offset);
   drawPt.y += inset.y;
   drawPt.y -= fontHeight - useFont->fi.baseline + textVPosDelta;
   char buffer[256];
   Int32 txt_w;
   for (int i = 0; i < startLineOffset.size(); i++)
   {
      drawPt.x = offset.x + inset.x;
      AssertFatal(lineLen[i] < 255, "doh, line lenght exceeds buffer.  Increase buffer size");
      strncpy(buffer, &text[startLineOffset[i]], lineLen[i]);
      buffer[lineLen[i]] = '\0';
   
      switch (alignment)
      {
         case RightJustify:
            txt_w = useFont->getStrWidth(buffer);
            drawPt.x += (extent.x - inset.x - txt_w);
            break;
         case CenterJustify:
            txt_w = useFont->getStrWidth(buffer);
            drawPt.x += ((extent.x - txt_w) >> 1);
            break;
      }

      sfc->drawText_p(useFont, &drawPt, buffer);
      drawPt.y += (fontHeight + lineSpacing);
   }
}

//------------------------------------------------------------------------------
bool TextWrap::onAdd()
{
   if(!Parent::onAdd())
      return false;

   setFont(fontNameTag);
   setFontDisabled(fontNameTagDisabled);
   if (!text)
   {
      // text was not set by the console variable
      setText(SimTagDictionary::getString(manager, textTag));
   }
   return true;
}

//------------------------------------------------------------------------------
void TextWrap::onRemove()
{
   setText(NULL);

   Parent::onRemove();
}   

//------------------------------------------------------------------------------
void TextWrap::variableChanged(const char *newValue)
{
   setText(newValue);
}   

//------------------------------------------------------------------------------
void TextWrap::inspectRead(Inspect *insp)
{
   Parent::inspectRead(insp);
   Int32 align;

   insp->read(IDITG_FONTNAME_TAG, NULL, NULL, NULL, fontNameTag);
   insp->read(IDITG_DISABLED_FONTNAME_TAG, NULL, NULL, NULL, fontNameTagDisabled);
   insp->read(IDITG_TEXT_TAG, NULL, NULL, NULL, textTag);
   insp->read(IDITG_LINE_SPACING, lineSpacing);
   insp->read(IDITG_ALIGNMENT, align);
   insp->read(IDITG_ALIGNMENT, ignoreClick);

   if ( (align >= LeftJustify) && (align <= CenterJustify) )
      alignment = (AlignmentType)align;

   setText(SimTagDictionary::getString(manager, textTag));
   setFont(fontNameTag);
   setFontDisabled(fontNameTagDisabled);
}   

//------------------------------------------------------------------------------
void TextWrap::inspectWrite(Inspect *insp)
{
   Parent::inspectWrite(insp);

   insp->write(IDITG_FONTNAME_TAG, true, IDRES_BEG_GUI_FNT, IDRES_END_GUI_FNT, (Int32)fontNameTag);
   insp->write(IDITG_DISABLED_FONTNAME_TAG, true, IDRES_BEG_GUI_FNT, IDRES_END_GUI_FNT, (Int32)fontNameTagDisabled);
   insp->write(IDITG_TEXT_TAG, false, IDDAT_BEG_GUI_STR, IDDAT_END_GUI_STR, (Int32)textTag);
   insp->write(IDITG_LINE_SPACING, lineSpacing);
   insp->write(IDITG_ALIGNMENT, (Int32)alignment);
   insp->write(IDITG_ALIGNMENT, ignoreClick);
}   

//------------------------------------------------------------------------------
Persistent::Base::Error TextWrap::write( StreamIO &sio, int version, int user )
{
   Int32 temp = (Int32) ignoreClick;
   sio.write(temp);
   sio.write(fontNameTagDisabled);
   sio.write((DWORD)alignment);
   sio.write(fontNameTag);
   sio.write(textTag);
   sio.write(lineSpacing);

   return Parent::write(sio, version, user);
}

//------------------------------------------------------------------------------
Persistent::Base::Error TextWrap::read( StreamIO &sio, int version, int user)
{
   DWORD align;
   Int32 temp;
   sio.read(&temp);
   ignoreClick = (bool)temp;
   sio.read(&fontNameTagDisabled);
   sio.read(&align);
   alignment = (AlignmentType)align;
   sio.read(&fontNameTag);
   sio.read(&textTag);
   sio.read(&lineSpacing);

   return Parent::read(sio, version, user);
}

}; //namespace GUI       