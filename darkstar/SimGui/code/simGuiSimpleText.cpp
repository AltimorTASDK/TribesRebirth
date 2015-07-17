#include <g_surfac.h>
#include <string.h>
#include <m_box.h>
#include "simGuiSimpleText.h"
#include "simTagDictionary.h"
#include "darkstar.strings.h"
#include "editor.strings.h"
#include "simResource.h"

namespace SimGui
{

SimpleText::SimpleText()
{
   fontNameTag = IDFNT_FONT_DEFAULT;
   fontNameTagHL = IDFNT_FONT_DEFAULT;
   fontNameTagDisabled = IDFNT_FONT_DEFAULT;
   textTag = IDSTR_STRING_DEFAULT;
   textVPosDelta = 0;

   text[0] = '\0';

   alignment = LeftJustify;
   extent.set(100, 25);

}

const char *SimpleText::getScriptValue()
{
   return getText();
}

void SimpleText::setScriptValue(const char *val)
{
   setText(val);
}

void SimpleText::setFont(Int32 tag)
{
   fontNameTag = tag;
   hFont = SimResource::loadByTag(manager, fontNameTag, true);
}

void SimpleText::setFontHL(Int32 tag)
{
   fontNameTagHL = tag;
   hFontHL = SimResource::loadByTag(manager, fontNameTagHL, true);
}

void SimpleText::setFontDisabled(Int32 tag)
{
   fontNameTagDisabled = tag;
   hFontDisabled = SimResource::loadByTag(manager, fontNameTagDisabled, true);
}

void SimpleText::variableChanged(const char *newValue)
{
   setText(newValue);
}

void SimpleText::setText(const char *txt)
{
   if(txt)
   {
      strncpy(text, txt, MAX_STRING_LENGTH);
      textTag = 0;
   }
   else if(textTag)
   {
      const char *txt = SimTagDictionary::getString(manager, textTag);   
      if(!txt)
         strcpy(text, "bad tag");
      else
         strncpy(text, txt, MAX_STRING_LENGTH);
   }
   text[MAX_STRING_LENGTH] = '\0';
   setUpdate();
} 

void SimpleText::onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect)
{
   if ( (bool)hFont)
   {
      Int32 txt_w;

      txt_w = hFont->getStrWidth(text);

      Point2I localStart;
      switch (alignment)
      {
         case RightJustify:
            localStart.set(extent.x - txt_w, 0);  
            break;
         case CenterJustify:
            localStart.set( (extent.x - txt_w) / 2, 0);
            break;
         default:
            // LeftJustify
            localStart.set(0,0);
      }

      localStart.y += textVPosDelta;
      Point2I globalStart = localToGlobalCoord(localStart);

      sfc->drawText_p(hFont, &globalStart, text);
   }
   renderChildControls(sfc, offset, updateRect);
}

bool SimpleText::onAdd()
{
   if(!Parent::onAdd())
      return false;

   setText();
   setFont(fontNameTag);
   setFontHL(fontNameTagHL);
   setFontDisabled(fontNameTagDisabled);
   return true;
}

void SimpleText::inspectRead(Inspect *insp)
{
   Parent::inspectRead(insp);
   Int32 align;

   insp->read(IDITG_FONTNAME_TAG, NULL, NULL, NULL, fontNameTag);
   insp->read(IDITG_HL_FONTNAME_TAT, NULL, NULL, NULL, fontNameTagHL);
   insp->read(IDITG_DISABLED_FONTNAME_TAG, NULL, NULL, NULL, fontNameTagDisabled);
   insp->read(IDITG_TEXT_TAG, NULL, NULL, NULL, textTag);
   insp->read(IDITG_TEXT, text);
   insp->read(IDITG_ALIGNMENT, align);
   insp->read(IDITG_TEXT_V_POS_DELTA, textVPosDelta);

   setText();
   setFont(fontNameTag);
   setFontHL(fontNameTagHL);
   setFontDisabled(fontNameTagDisabled);
   
   if ( (align >= LeftJustify) && (align <= CenterJustify) )
      alignment = (AlignmentType)align;
}   

void SimpleText::inspectWrite(Inspect *insp)
{
   Parent::inspectWrite(insp);

   insp->write(IDITG_FONTNAME_TAG, true, IDRES_BEG_GUI_FNT, IDRES_END_GUI_FNT, (Int32)fontNameTag);
   insp->write(IDITG_HL_FONTNAME_TAT, true, IDRES_BEG_GUI_FNT, IDRES_END_GUI_FNT, (Int32)fontNameTagHL);
   insp->write(IDITG_DISABLED_FONTNAME_TAG, true, IDRES_BEG_GUI_FNT, IDRES_END_GUI_FNT, (Int32)fontNameTagDisabled);
   insp->write(IDITG_TEXT_TAG, false, IDDAT_BEG_GUI_STR, IDDAT_END_GUI_STR, (Int32)textTag);
   insp->write(IDITG_TEXT, text);
   insp->write(IDITG_ALIGNMENT, (Int32)alignment);
   insp->write(IDITG_TEXT_V_POS_DELTA, (Int32)textVPosDelta);
}   

Persistent::Base::Error SimpleText::write( StreamIO &sio, int version, int user )
{
   DWORD unused = 0;
   sio.write(unused);
   sio.write(unused);

   sio.write(fontNameTagDisabled);
   sio.write(fontNameTag);
   sio.write(fontNameTagHL);
   sio.write(textTag);
   
   //since the inspect string length (80) is shorter than the simple text length, truncate...
   Inspect_Str temp;
   strncpy(temp, text, Inspect::MAX_STRING_LEN);
   temp[Inspect::MAX_STRING_LEN] = '\0';
   sio.write(Inspect::MAX_STRING_LEN + 1, temp);
   
   sio.write((Int32)alignment);
   sio.write(textVPosDelta);

   return Parent::write(sio, version, user);
}

Persistent::Base::Error SimpleText::read( StreamIO &sio, int version, int user)
{
   DWORD temp;

   sio.read(&temp);
   sio.read(&temp);
   
   sio.read(&fontNameTagDisabled);
   sio.read(&fontNameTag);
   sio.read(&fontNameTagHL);
   sio.read(&textTag);
   sio.read(Inspect::MAX_STRING_LEN + 1, text);
   Int32 align;
   sio.read(&align);
   alignment = (AlignmentType)align;
   sio.read(&textVPosDelta);

   return Parent::read(sio, version, user);
}

}; //namespace GUI