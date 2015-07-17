//------------------------------------------------------------------------------
// Description: Base class for classes that stand between a control and the
//               program, i.e. GWButtonProxy...
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "gwControlProxy.h"


GWControlProxy::GWControlProxy()
 : m_attached(false),
   m_parentHWnd(NULL),
   m_ctrlHWnd(NULL),
   m_ctrlId(0)
{
   // NULL
}



GWControlProxy::~GWControlProxy()
{
   m_attached = false;

   m_parentHWnd = NULL;
   m_ctrlHWnd   = NULL;
   m_ctrlId     = 0;
}


bool 
GWControlProxy::attach(const HWND   in_parentHWnd,
                       const UInt16 in_ctrlId)
{
   AssertFatal(in_parentHWnd != NULL, "Error, bogus parent HWND");
   AssertFatal(m_attached == false, "Error, tried to reattach!");
   
   m_ctrlHWnd = GetDlgItem(in_parentHWnd, in_ctrlId);
   if (m_ctrlHWnd == NULL) {
      AssertWarn(0, "Unabled to get valid handle to control...");
      return false;
   }
   
   m_parentHWnd = in_parentHWnd;
   m_ctrlId     = in_ctrlId;
   m_attached   = true;
   
   return true;
}


void 
GWControlProxy::setEnable(const bool in_enable) const
{
   AssertFatal(m_attached == true, "Error, not attached...");
   
   EnableWindow(m_ctrlHWnd, BOOL(in_enable));
}


void 
GWControlProxy::getWindowText(char* out_pString,
                              const Int32 in_bufSize)
{
   HWND controlHWnd = getControlHWnd();
   
   GetWindowText(controlHWnd, out_pString, in_bufSize);
}


void 
GWControlProxy::setWindowText(const char* in_pString)
{
   HWND controlHWnd = getControlHWnd();
   
   SetWindowText(controlHWnd, in_pString);
}
