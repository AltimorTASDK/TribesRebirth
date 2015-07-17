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
#include <inspect.h>
#include <g_surfac.h>

#include "TestCtrl.h"
#include "main.strings.h"

IMPLEMENT_PERSISTENT_TAG( SimGui::TestCtrl,  FOURCC('s','g','t','c') );

namespace SimGui 
{

void COREAPI TestCtrl::onRender(GFXSurface *sfc, Point2I offset, const Box2I& updateRect)
{
   RectI r(offset.x, offset.y, offset.x + extent.x, offset.y + extent.y);

   switch (background)
   {  
      case BLUE:
         sfc->drawRect2d_f(&r, 252);
         break;
      case RED:
         sfc->drawRect2d_f(&r, 249);
         break;
      case GREEN:
         sfc->drawRect2d_f(&r, 250);
         break;
   };
   Parent::onRender(sfc, offset, updateRect);
}

bool COREAPI TestCtrl::onSimMessageEvent(const SimMessageEvent *event)
{
   if (event->message == HELLO_WORLD)
      setText("Hello World");

   return Parent::onSimMessageEvent(event);
}   
   
bool COREAPI TestCtrl::onSimInspectEvent(const SimInspectEvent *event)
{
   Parent::onSimInspectEvent(event);
   Inspect *insp = event->inspector;
   SimTag newTxt;
   
   insp->getActionButton(IDITG_CHANGE_STRING, NULL, NULL);
   Int32 _background = (Int32)background;
   insp->getSimTagPairList(IDITG_BACKGROUND_COLOR, _background);
   background = SimGui::TestCtrl::COLORS(_background);
   insp->get(IDITG_PRESET_TXT, NULL, NULL, NULL, newTxt);
   insp->getDivider();
   insp->get(IDITG_DESCRIPTION);
   if (newTxt)
      setText(newTxt);
   
   return true;
}   

bool COREAPI TestCtrl::onSimInspectQuery(SimInspectQuery *query)
{
   Parent::onSimInspectQuery(query);
   Inspect *insp = query->inspector;

   insp->addActionButton(IDITG_CHANGE_STRING, this, HELLO_WORLD);
   insp->addSimTagPairList(IDITG_BACKGROUND_COLOR, background, 3, IDSTR_RED, RED,
                                                                  IDSTR_BLUE, BLUE,
                                                                  IDSTR_GREEN, GREEN);
   insp->add(IDITG_PRESET_TXT, false, IDDAT_BEG_PRESET_TXT, IDDAT_END_PRESET_TXT, textTag);
   insp->addDivider();
   insp->add(IDITG_DESCRIPTION);
   return true;   
}   

};