#ifndef _H_SIMGUI_HELPCTRL
#define _H_SIMGUI_HELPCTRL

#include "simGuiCtrl.h"

namespace SimGui
{

class HelpCtrl : public Control
{

private:
   typedef Control Parent;

protected:
	Int32		   mHelpTag;		
   const char *lpszHelpText;

public:
    HelpCtrl();
   ~HelpCtrl();

   virtual void setHelpText(const char *text, float timeElapsed, bool mouseClicked = FALSE);
	virtual void setHelpTag(Int32 helpTag, float timeElapsed, bool mouseClicked = FALSE);
   virtual void render(GFXSurface *sfc);

   DECLARE_PERSISTENT( HelpCtrl );
};

};

#endif