#include <timer.h>
#include <fearguiircactivetextedit.h>
#include <fearguiircchannellist.h>

namespace FearGui
{
FGIRCActiveTextEdit::FGIRCActiveTextEdit()
: fListGenerated(false), objList(NULL)
{
}

FGIRCActiveTextEdit::~FGIRCActiveTextEdit()
{
}

void FGIRCActiveTextEdit::onWake()
{
   timer.reset();
}

void FGIRCActiveTextEdit::associateList(FGIRCChannelList *list)
{
   objList = list;
}

void FGIRCActiveTextEdit::onPreRender()
{
   if (timer.getElapsed() >= 0.5f && fListGenerated == false && objList)
   {
      fListGenerated = true;
      objList->showMatches(text);
   }
}

void FGIRCActiveTextEdit::onKeyDown(const SimGui::Event &event)
{
   timer.reset();
   fListGenerated = false;

   Parent::onKeyDown(event);
}

IMPLEMENT_PERSISTENT_TAG(FGIRCActiveTextEdit, FOURCC('F','i','a','e'));
};