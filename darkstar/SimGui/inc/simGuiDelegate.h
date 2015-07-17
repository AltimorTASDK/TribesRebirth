//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMGUIDELEGATE_H_
#define _SIMGUIDELEGATE_H_

#include <simGuiBase.h>
#include <simGuiCtrl.h>

namespace SimGui 
{

class DLLAPI Delegate : public SimObject, public Responder
{
private:
   typedef SimObject Parent;

protected:
	bool mbDeleteOnLoseContent;
   
public:
	bool deleteOnLoseContent(void) { return mbDeleteOnLoseContent; }
	void deleteOnLoseContent(bool value) { mbDeleteOnLoseContent = value; }

   virtual void preRender(); // content control is about to be rendered.

   virtual void onLoseDelegation();
   virtual void onGainDelegation();
	virtual void onRegainTopDelegation();

   virtual void onDialogPush(Control *oldTopGui, Control *dialogGui, Int32 message);
   virtual Int32 onDialogPop(Control *dialogGui, Int32 message); // return -1 to prevent the dialog from closing
   virtual void onNewContentControl(Control *oldGui, Control *newGui);

	virtual void setHelpTag(Int32 /*helpTag*/, float /*timeElapsed*/, bool /*mouseClicked*/) { }
   virtual void setHelpText(const char *, float,bool /*mouseClicked*/) { }
      // note: either oldGui, newGui or both may be NULL
};

}

#endif //_SIMGUIDELEGATE_H_
