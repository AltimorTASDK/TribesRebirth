//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <simTagDictionary.h>
#include <g_surfac.h>
#include "simGuiTextList.h"
#include "simTagDictionary.h"
#include "darkstar.strings.h"
#include "editor.strings.h"
#include <g_font.h>
#include "simResource.h"
#include "m_qsort.h"

namespace SimGui
{

static int __cdecl textCompare(const void *a,const void *b)
{
   TextList::Entry *entry_A = (TextList::Entry *)(a);
   TextList::Entry *entry_B = (TextList::Entry *)(b);
   return (stricmp(entry_A->text, entry_B->text));
} 

TextList::Entry TextList::createEntry(Int32 stdTag, const char *text)
{
   Entry entry;
   
   entry.stdTag = stdTag;
   if (stdTag)
   {
      const char* temp = SimTagDictionary::getString(manager, stdTag);
      entry.text = new char[strlen(temp) + 1];
      strcpy(entry.text, temp);
      return entry;
   }
   
   if (text)   
   {
      entry.text = new char[strlen(text) + 1];      
      strcpy(entry.text, text);
   }
   else
      entry.text = NULL;

   entry.data = NULL;
   return entry;      
}  

void TextList::clearEntry(TextList::Entry *entry)
{
   if (entry->text)
      delete[] entry->text; 
}  
 
void TextList::onPreRender(void)
{
}

void TextList::onRenderCell(GFXSurface *sfc, Point2I offset, Point2I cell, bool selected, bool mouseOver)
{
   //compiler warning
   mouseOver;
   
   Point2I textOffset = offset;
   textOffset.x += ENTRY_H_MARGIN;
   textOffset.y += textVPosDelta;
   
   if (selected)
   {
      sfc->drawRect2d_f(&RectI(offset.x, offset.y,
                                 offset.x + cellSize.x - 1, offset.y + cellSize.y - 1), 249);
      if (bool(hFontHL) && textList[cell.y].text)
      sfc->drawText_p(hFontHL, &textOffset, textList[cell.y].text);
   }
   else
   {
      if (bool(hFontHL) && textList[cell.y].text)
      sfc->drawText_p(hFont, &textOffset, textList[cell.y].text);
   }
   
}   

void TextList::setFont(Int32 tag)
{
   fontNameTag = tag;
   hFont = SimResource::loadByTag(manager, fontNameTag, true);
   if (! bool(hFont)) hFont = SimResource::loadByTag(manager, IDFNT_FONT_DEFAULT, true);
   if(bool(hFont))
      cellSize.set(extent.x, hFont->getHeight() + ENTRY_V_MARGIN + textVPosDelta);
}

void TextList::setFontHL(Int32 tag)
{
   fontNameTagHL = tag;
   hFontHL = SimResource::loadByTag(manager, fontNameTagHL, true);
   if (! bool(hFontHL)) hFontHL = SimResource::loadByTag(manager, IDFNT_FONT_DEFAULT, true);
}

bool TextList::onAdd()
{
   if (! Parent::onAdd()) return FALSE;
   
   setFont(fontNameTag);
   setFontHL(fontNameTagHL);
   
   return TRUE;
}   

void TextList::onWake()
{
   setFont(fontNameTag);
   setFontHL(fontNameTagHL);
   
   setSize(Point2I( 1, 0));
}   

void TextList::clear()
{
   while (textList.size())
      removeEntryBack();
      
   setSelectedCell(Point2I(-1, -1));
} 

Int32 TextList::findEntry(const char *text)
{
   for (int i = 0; i < getNumEntries(); i++)
   {
      if (stricmp(textList[i].text, text) == 0)
         return i;
   }
   return -1;
}  
 
void TextList::sort()
{
   if (getNumEntries() < 2)
      return;

   m_qsort((void *)&(textList[0]), textList.size(), sizeof(Entry), textCompare);
}  


Persistent::Base::Error TextList::write( StreamIO &sio, int version, int user )
{
   sio.write(fontNameTag);
   sio.write(fontNameTagHL);
   sio.write(textVPosDelta);
   
   sio.write(enumerate);
   sio.write(noDuplicates);
   return Parent::write(sio, version, user);
}

Persistent::Base::Error TextList::read( StreamIO &sio, int version, int user)
{
   sio.read(&fontNameTag);
   sio.read(&fontNameTagHL);
   sio.read(&textVPosDelta);
   
   sio.read(&enumerate);
   sio.read(&noDuplicates);
   Persistent::Base::Error result = Parent::read(sio, version, user);
   
   setFont(fontNameTag);
   setFontHL(fontNameTagHL);
   
   return result;
}
   
void TextList::inspectWrite(Inspect *insp)
{
   Parent::inspectWrite(insp);
   
   insp->write(IDITG_FONTNAME_TAG, true, IDRES_BEG_GUI_FNT, IDRES_END_GUI_FNT, (Int32)fontNameTag);
   insp->write(IDITG_HL_FONTNAME_TAT, true, IDRES_BEG_GUI_FNT, IDRES_END_GUI_FNT, (Int32)fontNameTagHL);
   insp->write(IDITG_TEXT_V_POS_DELTA, (Int32)textVPosDelta);

   insp->write(IDITG_ENUMERATE_LIST, enumerate);
   insp->write(IDITG_NO_DUPLICATES, noDuplicates);
}   

void TextList::inspectRead(Inspect *insp)
{
   Parent::inspectRead(insp);

   insp->read(IDITG_FONTNAME_TAG, NULL, NULL, NULL, fontNameTag);
   insp->read(IDITG_HL_FONTNAME_TAT, NULL, NULL, NULL, fontNameTagHL);
   insp->read(IDITG_TEXT_V_POS_DELTA, textVPosDelta);
   
   insp->read(IDITG_ENUMERATE_LIST, enumerate); 
   insp->read(IDITG_NO_DUPLICATES, noDuplicates);
   
   setFont(fontNameTag);
   setFontHL(fontNameTagHL);
   
   cellSize.set(extent.x, hFont->getHeight() + ENTRY_V_MARGIN + textVPosDelta);
}   

void TextList::onRemove()
{
   clear();
   Parent::onRemove();
}   

TextList::TextList()
{
   active = true;
   enumerate = false;
   noDuplicates = true;
   textVPosDelta = 0;
}

Int32 TextList::getNumEntries()
{
   return textList.size();
}

void TextList::addEntryFront(Int32 stdTag)
{
   Entry entry = createEntry(stdTag, NULL);
   if (noDuplicates && findEntry(entry.text) > -1)
   {
      clearEntry(&entry);
      return;
   }

   textList.push_front(entry);
   setSize(Point2I( 1, textList.size()));
}

void TextList::addEntryFront(const char *text)
{
   Entry entry = createEntry(0, text);
   if (noDuplicates && findEntry(entry.text) > -1)
   {
      clearEntry(&entry);
      return;
   }

   textList.push_front(entry);
   setSize(Point2I( 1, textList.size()));
}

void TextList::addEntryBack(Int32 stdTag)
{
   Entry entry = createEntry(stdTag, NULL);
   if (noDuplicates && findEntry(entry.text) > -1)
   {
      clearEntry(&entry);
      return;
   }

   textList.push_back(entry);
   setSize(Point2I( 1, textList.size()));
}

void TextList::addEntryBack(const char *text)
{
   Entry entry = createEntry(0, text); 
   if (noDuplicates && findEntry(entry.text) > -1)
   {
      clearEntry(&entry);
      return;
   }

   textList.push_back(entry);
   setSize(Point2I( 1, textList.size()));
}

void TextList::removeEntryFront()
{
   if (textList.size() == 0)
      return;

   Entry entry = textList.first();
   clearEntry(&entry);
     
   textList.pop_front();
   setSize(Point2I( 1, textList.size()));
   
   setSelectedCell(Point2I(-1, -1));
}                                

void TextList::removeEntryBack()
{
   if (textList.size() == 0)
      return;

   Entry entry = textList.last();
   clearEntry(&entry);
        
   textList.pop_back();
   setSize(Point2I( 1, textList.size()));

   setSelectedCell(Point2I(-1, -1));
}

const char *TextList::getSelectedText()
{
   if (selectedCell.y == -1)
      return NULL;

   return textList[selectedCell.y].text;  
}

void TextList::setScriptValue(const char *val)
{
   int i;
   for(i = 0; i < textList.size(); i++)
   {
      if(!strcmp(textList[i].text, val))
      {
         setSelectedCell(Point2I(0, i));
         return;
      }
   }
}

const char *TextList::getScriptValue()
{
   if(selectedCell.y == -1)
      return "";
   else
      return textList[selectedCell.y].text;
}

Int32 TextList::getSelectedTag()
{
   if (selectedCell.y == -1)
      return 0;

   return textList[selectedCell.y].stdTag;
}

void TextList::deleteSelected()
{
   if (selectedCell.y == -1)
      return;

   Entry entry = textList[selectedCell.y];
   clearEntry(&entry);

   textList.erase(selectedCell.y);
   setSelectedCell(Point2I(-1, -1));
}

bool TextList::becomeFirstResponder()
{
   return (active && getNumEntries());
}

};   

