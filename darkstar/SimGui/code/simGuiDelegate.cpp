//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------


#include "simGuiDelegate.h"

namespace SimGui
{

int DelegateLinkFlag = 0;

void Delegate::preRender()
{
   return;
}

void Delegate::onLoseDelegation()
{
   return;  
}  
 
void Delegate::onGainDelegation()
{
   return;
} 

void Delegate::onRegainTopDelegation()
{
   return;
}  

void Delegate::onDialogPush(Control *oldTopControl, Control *dialogControl, Int32 message)
{
   oldTopControl;
   dialogControl;
   message;
}

Int32 Delegate::onDialogPop(Control *dialog, Int32 message)
{
   dialog;
   message;
   return 0;
}
 
void Delegate::onNewContentControl(Control *, Control *) 
{
   return;   
}   

};
