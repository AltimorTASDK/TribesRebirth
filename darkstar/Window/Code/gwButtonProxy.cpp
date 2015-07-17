//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "gwButtonProxy.h"


//------------------------------------------------------------------------------
//--------------------------------------GWButtonProxy
bool 
GWButtonProxy::getCheck() const
{
   // base class getCtHWnd() check to make sure we are attached...
   HWND parentHWnd = getParentHWnd();
   UInt16 ctrlId = getControlId();
   
   UINT state = IsDlgButtonChecked(parentHWnd, ctrlId);

   return ((state == BST_CHECKED) ? true : false);
}


void 
GWButtonProxy::setCheck(const bool in_state) const
{
   HWND parentHWnd = getParentHWnd();
   UInt16 ctrlId   = getControlId();
   
   UInt32 state = (in_state == true) ? BST_CHECKED : BST_UNCHECKED;
   BOOL success = CheckDlgButton(parentHWnd, ctrlId, state);
   AssertWarn(success == TRUE, "Error, failed to set check...");
}


//------------------------------------------------------------------------------
//--------------------------------------GWRadioGroup

GWRadioGroup::GWRadioGroup()
 : m_buttonsAttached(false),
   m_parentHWnd(NULL),
   m_pButtons(NULL),
   m_firstId(0),
   m_lastId(0)
{
   // NULL
}



GWRadioGroup::~GWRadioGroup()
{
   delete [] m_pButtons;
   m_pButtons = NULL;
   
   m_buttonsAttached = false;
   m_parentHWnd      = NULL;
   m_firstId         = 0;
   m_lastId          = 0;
}


bool 
GWRadioGroup::attach(const HWND   in_parentHWnd,
                     const UInt16 in_firstId,
                     const UInt16 in_lastId)
{
   AssertFatal(in_parentHWnd != NULL, "Error, bad parent HWnd!");
   AssertFatal(in_lastId >= in_firstId, "Error, bad range");
   AssertFatal(m_buttonsAttached == false, "Error, tried to reattach");
   
   // OK, allocate and attach the button proxies.
   m_pButtons = new GWButtonProxy[in_lastId - in_firstId + 1];
   for (int i = 0; i <= in_lastId - in_firstId; i++) {
      bool success = m_pButtons[i].attach(in_parentHWnd, (i + in_firstId));
      if (success == false) {
         delete [] m_pButtons;
         m_pButtons = NULL;
         return false;
      }
   }
   
   m_parentHWnd = in_parentHWnd;
   m_firstId    = in_firstId;
   m_lastId     = in_lastId;

   m_buttonsAttached = true;
   return true;
}


UInt16 
GWRadioGroup::getCheckedId() const
{
   AssertFatal(m_buttonsAttached == true, "Error, proxies not attached");

   for (int i = 0; i <= (m_lastId - m_firstId); i++) {
      if (m_pButtons[i].getCheck() == true)
         return (i + m_firstId);
   }
   
   return 0;
}

   
bool 
GWRadioGroup::setChecked(const UInt16 in_id) const
{
   AssertFatal(m_buttonsAttached == true, "Error, proxies not attached");
   AssertFatal(in_id >= m_firstId && in_id <= m_lastId, "Error, out of range");
   
   return (CheckRadioButton(m_parentHWnd, m_firstId, m_lastId, in_id) == TRUE);
}


void 
GWRadioGroup::setEnable(const bool in_enable) const
{
   AssertFatal(m_buttonsAttached == true, "Error, proxies not attached");

   for (int i = 0; i <= (m_lastId - m_firstId); i++)
      m_pButtons[i].setEnable(in_enable);
}

