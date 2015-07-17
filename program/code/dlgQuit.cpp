
#include "console.h"
#include "simGuiDelegate.h"

namespace SimGui
{

class quitDelegate : public Delegate
{

private:
   typedef Delegate Parent;
   Control *curGui;
   
public:
   quitDelegate() { curGui = NULL; }
   void onNewContentControl(Control *oldGui, Control *newGui);
   DECLARE_PERSISTENT(quitDelegate);
};

void quitDelegate::onNewContentControl(Control *, Control *newGui)
{
   curGui = newGui;
   Console->evaluate("quit();");
}

IMPLEMENT_PERSISTENT(quitDelegate);


};

