//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMGUIACTIVECTRL_H_
#define _SIMGUIACTIVECTRL_H_


#include "simGuiCtrl.h"

namespace SimGui
{

// abstract class to serve primarmy needs of  buttons
class DLLAPI ActiveCtrl : public Control
{
private:
   typedef Control Parent;

protected:
   // runtime states
   bool stateDepressed; // set when mouse or '\n' button is being pressed down 
   bool stateOver;      // set when mouse is over the control

   Int32 onSelectedCursorTag; // cursor displayed when active control is selected

public:   
   // persistent states
   bool active;
   DWORD message;

   ActiveCtrl();
   virtual bool inHitArea(const Point2I &pt);
   virtual void onAction();
	void setMessage(DWORD msg);
	DWORD getMessage(void);

   void setSelectedCursorTag(Int32 cursorTag) {onSelectedCursorTag = cursorTag;}

   void onMouseMove(const Event &event);
   void onMouseDown(const Event &event);
   void onMouseUp(const Event &event);
   void onMouseDragged(const Event &event);
   void onMouseLeave(const Event &event);
   void onKeyUp(const Event &event);
   void onKeyDown(const Event &event);

   void inspectRead(Inspect *insp);
   void inspectWrite(Inspect *insp);

   bool wantsTabListMembership();
   bool loseFirstResponder();
   bool becomeFirstResponder();

   DECLARE_PERSISTENT(ActiveCtrl);
   Persistent::Base::Error write( StreamIO &sio, int, int );
   Persistent::Base::Error read( StreamIO &sio, int, int );
};

};

#endif //_SIMGUIACTIVECTRL_H_
