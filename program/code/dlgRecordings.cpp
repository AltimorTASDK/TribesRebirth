#include "simGuiDelegate.h"
#include "simGuiBitmapCtrl.h"
#include "simResource.h"
#include "simGuiTextEdit.h"
#include "console.h"
#include "fear.strings.h"
#include "FearGuiArrayCtrl.h"
#include "FearGuiRecList.h"

namespace SimGui
{

class RecordingsDelegate : public Delegate
{

private:
   typedef Delegate Parent;
   Control *curGui;
   
   SimTime demoTime;
   
public:
   void onNewContentControl(Control *oldGui, Control *newGui);
   DECLARE_PERSISTENT(RecordingsDelegate);
   RecordingsDelegate() { curGui = NULL; }
};

void RecordingsDelegate::onNewContentControl(Control *, Control *newGui)
{
   curGui = newGui;
}

IMPLEMENT_PERSISTENT(RecordingsDelegate);

};