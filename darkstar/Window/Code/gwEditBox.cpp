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
#include "GWEditBox.h"

//============================================================================

bool  GWEditBox::createWin( GWWindow *parent, RectI &r, DWORD exStyle, DWORD style, DWORD id )
{
   GWWindowClass *gwwc = new GWWindowClass();
   gwwc->setName("EDIT");

   //--------------------------------------------------------------------------------
   if ( Parent::createEx( exStyle, gwwc, (LPCSTR)NULL,
            WS_VISIBLE | WS_CHILD |  ES_MULTILINE | WS_BORDER | style,
            r.upperL,
            Point2I(r.len_x(),r.len_y()),
            parent, 0, (HMENU)id) )
   {
      show (SW_SHOW);
      update();
      delete gwwc;
      return ( true );
   }
   else
   {
      delete gwwc;
      AssertFatal(0, "GWEditBox::createWin: could not create window");
      return ( false );        
   }
}   

//------------------------------------------------------------------------------

void __cdecl GWEditBox::printf(char *_format, ...)
{
   char buff[512];
   va_list va;
   va_start(va, _format);
   vsprintf( buff, _format, (LPSTR)va );
   va_end(va);
   SetWindowText( hWnd, buff );
}

//------------------------------------------------------------------------------

void GWEditBox::setText( char *text )
{
   SetWindowText( hWnd, text );
}   

//------------------------------------------------------------------------------

char* GWEditBox::getText()
{
   static   char buff[256];
   GetWindowText( hWnd, buff, 256 );
   return ( buff );
}