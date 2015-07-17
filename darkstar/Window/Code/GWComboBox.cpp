//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//               (c) Copyright 1996, Dynamix Inc.   All rights reserved.
//
//------------------------------------------------------------------------------

#include <stdio.h>
#include "GWComboBox.h"

//============================================================================

bool  GWComboBox::createWin( GWWindow *parent, RectI &r, DWORD exStyle, DWORD style )
{
   GWWindowClass gwwc;
   gwwc.setName("COMBOBOX");

   //--------------------------------------------------------------------------------
   if ( Parent::createEx( exStyle, &gwwc, (LPCSTR)NULL,
            WS_VISIBLE | WS_CHILD | CBS_AUTOHSCROLL | CBS_DROPDOWN | CBS_HASSTRINGS | style,
            r.upperL,
            Point2I(r.len_x(),r.len_y()),
            parent, 0) )
   {
      show (SW_SHOW);
      update();
      return ( true );
   }
   else
   {
      AssertFatal(0, "GWComboBox::createWin: could not create window");
      return ( false );        
   }
}   

//------------------------------------------------------------------------------

int GWComboBox::getCurSel()
{
   return (SendMessage( hWnd, CB_GETCURSEL, 0, 0 ));
}

//------------------------------------------------------------------------------

void GWComboBox::clear()
{
   SendMessage( hWnd, CB_RESETCONTENT, 0, 0 );
}

//------------------------------------------------------------------------------

void __cdecl GWComboBox::printf(char *_format, ...)
{
   char buff[512];
   va_list va;
   va_start(va, _format);
   vsprintf( buff, _format, (LPSTR)va );
   va_end(va);
   SendMessage( hWnd, CB_SETCURSEL, 
      SendMessage( hWnd, CB_ADDSTRING, NULL, (LPARAM)buff ),
      NULL);
}

