//============================================================================
//==   
//==  $Workfile:   TV.h  $
//==  $Version$
//==  $Revision:   1.00  $
//==    
//==  DESCRIPTION:
//==        EDITBOX class decleration
//==        
//==  (c) Copyright 1997, Dynamix Inc.   All rights reserved.
//== 
//============================================================================

#ifndef  _EDITBOX_H_
#define  _EDITBOX_H_

#include <types.h>
#include <gw.h>
#include <commctrl.h>   // includes the common control header
#include <m_rect.h>

//----------------------------------------------------------------------------

class GWEditBox : public GWWindow
{
   typedef GWWindow Parent;

public:
   bool  createWin( GWWindow *parent, RectI &r, DWORD exStyle, DWORD style, DWORD id );
   void  __cdecl printf(char *_format, ...);
   void  setText( char *text );
   char* getText();
};

#endif   // _EDITBOX_H_