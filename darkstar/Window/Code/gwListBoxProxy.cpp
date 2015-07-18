//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
#include <windows.h>

#include "gwListBoxProxy.h"


Int32  
GWListBoxProxy::getCount() const
{
   HWND lbHWnd = getControlHWnd();
   
   return (Int32)::SendMessage(lbHWnd, LB_GETCOUNT, 0, 0);
}

UInt32 
GWListBoxProxy::getItemData(const int in_index) const
{
   HWND lbHWnd = getControlHWnd();

   return (UInt32)::SendMessage(lbHWnd, LB_GETITEMDATA, in_index, 0);
}

UInt32 
GWListBoxProxy::setItemData(const int    in_index,
                            const UInt32 in_userData)
{
   HWND lbHWnd = getControlHWnd();

   return (UInt32)::SendMessage(lbHWnd, LB_SETITEMDATA, in_index, in_userData);
}

Int32 
GWListBoxProxy::getSelStatus(const int in_index) const
{
   HWND lbHWnd = getControlHWnd();

   return Int32(::SendMessage(lbHWnd, LB_GETSEL, in_index, 0));
}

Int32 
GWListBoxProxy::getItemText(const int in_index, char* in_pBuffer) const
{
   HWND lbHWnd = getControlHWnd();

   return Int32(::SendMessage(lbHWnd, LB_GETTEXT, in_index, (LPARAM)in_pBuffer));
}

Int32 
GWListBoxProxy::clearListBox() const
{
   HWND lbHWnd = getControlHWnd();

   return Int32(::SendMessage(lbHWnd, LB_RESETCONTENT, 0, 0));
}

Int32 
GWListBoxProxy::addString(const char* in_pString)
{
   HWND lbHWnd = getControlHWnd();

   return Int32(::SendMessage(lbHWnd, LB_ADDSTRING, 0, (LPARAM)in_pString));
}

Int32 
GWListBoxProxy::deleteString(const int in_index)
{
   HWND lbHWnd = getControlHWnd();

   return Int32(::SendMessage(lbHWnd, LB_DELETESTRING, in_index, 0));
}

Int32 
GWListBoxProxy::insertString(const int in_index, const char* in_pString)
{
   HWND lbHWnd = getControlHWnd();

   return Int32(::SendMessage(lbHWnd,
                              LB_INSERTSTRING,
                              in_index,
                              (LPARAM)in_pString));
}

Int32 
GWListBoxProxy::getCurSel() const
{
   HWND lbHWnd = getControlHWnd();

   return Int32(::SendMessage(lbHWnd, LB_GETCURSEL, 0, 0));
}

Int32 
GWListBoxProxy::setCurSel(const int in_index)
{
   HWND lbHWnd = getControlHWnd();

   return Int32(::SendMessage(lbHWnd, LB_SETCURSEL, in_index, 0));
}

Int32 
GWListBoxProxy::setSel(const int in_index, const bool in_select)
{
   HWND lbHWnd = getControlHWnd();

   return Int32(::SendMessage(lbHWnd, LB_SETSEL, in_select, in_index));
}

Int32 
GWListBoxProxy::getSelCount() const
{
   HWND lbHWnd = getControlHWnd();

   return Int32(::SendMessage(lbHWnd, LB_GETSELCOUNT, 0, 0));
}

Int32 
GWListBoxProxy::getSelItems(const UInt32 in_maxItems,
                            const int*   in_pItemBuffer) const
{
   HWND lbHWnd = getControlHWnd();

   return Int32(::SendMessage(lbHWnd,
                              LB_GETSELITEMS,
                              in_maxItems,
                              (LPARAM)in_pItemBuffer));
}
 
