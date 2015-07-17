//============================================================================
//==   
//==  $Workfile:   TV.h  $
//==  $Version$
//==  $Revision:   1.00  $
//==    
//==  DESCRIPTION:
//==        ComboBox class decleration
//==        
//==  (c) Copyright 1997, Dynamix Inc.   All rights reserved.
//== 
//============================================================================

#ifndef  _COMBOBOX_H_
#define  _COMBOBOX_H_

#include <types.h>
#include <gw.h>
#include <commctrl.h>   // includes the common control header
#include <m_rect.h>

//----------------------------------------------------------------------------

class GWComboBox : public GWWindow
{
   typedef GWWindow Parent;

public:
   bool  createWin( GWWindow *parent, RectI &r, DWORD exStyle, DWORD style );
   int   getCurSel();
   void  clear();
   void  __cdecl printf(char *_format, ...);
};

#endif   // _COMBOBOX_H_