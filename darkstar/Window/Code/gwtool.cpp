#include "gwtool.h"

//TODO get system sizes for these things
const int BUTTONWIDTH=24;
const int BUTTONHEIGHT=22;

//-----------------------------------------------------------------------
// create the window which will hold the tool bar and status bar
// pass in name of window and screen rect for it
bool GWToolWin::createWin(GWWindow *parent, LPCSTR name, LPRECT r, DWORD exStyle)
{

   if ( getHandle() && ::IsWindow(getHandle()) )
      destroyWindow();

   if ( Parent::createEx( exStyle, 0, name,
      WS_OVERLAPPED | WS_CLIPCHILDREN,
      Point2I(r->left, r->top),
      Point2I(r->right-r->left, r->bottom-r->top),
      parent, 0, NULL, true, false) )
      {
         show (SW_SHOW);
         update();
         return ( true );
      }
      else
      {
        AssertFatal(0, "GWToolWin::createWind: could not create tool window");
//         MessageBox(NULL, "could not create tool window", NULL, MB_OK);
         return ( false );        
      }


}

//-----------------------------------------------------------------------
bool GWToolWin::windowTitle(LPCSTR title)
{
   if (  getHandle() && ::IsWindow(getHandle()) )
      return (SetWindowText(getHandle(), title) == TRUE);

   return ( false );
}

//-----------------------------------------------------------------------
bool GWToolWin::createToolBar(int tbarID, TBBUTTON *tbButtons, int numButtons, HBITMAP hBM )
{
   if ( getToolBarHandle() && ::IsWindow(getToolBarHandle()) )
      if ( !destroyToolBar() )
         return ( false );

   if ( getHandle() && ::IsWindow(getHandle()) ) 
   {
      show( SW_SHOW );
      update();
      RECT rc;
      GetClientRect(getHandle(), &rc);

      int totButtons = 1;

      //create the toolbar
      HWND toolres;
      toolres = CreateToolbarEx (
         hWnd,
         WS_CHILD | WS_BORDER | WS_VISIBLE | TBSTYLE_TOOLTIPS,
         tbarID,
         totButtons,
//         getHInstance(),
//         bitMapID,
         NULL,       // not using bitmap resource
         (UINT)hBM,  // using button bitmap we created
         tbButtons,
         numButtons, 
         BUTTONWIDTH, BUTTONHEIGHT, BUTTONWIDTH, BUTTONHEIGHT,
         sizeof(TBBUTTON) );
      
      if ( !toolres )
      {
         AssertFatal(0, "GWToolWin::createToolBar: could not create tool bar");
//         MessageBox(NULL, "could not create tool bar", NULL, MB_OK);
         return (false);
      }

      hToolBar = toolres;
      if (!SetWindowPos(hToolBar, NULL, 0, 0, rc.right-rc.left, 
            rc.bottom-rc.top, SWP_NOMOVE | SWP_NOZORDER))
      {
         AssertFatal(0, "GWToolWin::createToolBar:"
                     " could not position tool bar");
        //MessageBox(NULL, "could not position tool window", NULL, MB_OK);
        return ( false );
      }
      return ( true );
   }
   else
   {
     AssertFatal(0, "GWToolWin::createToolBar: tool window not created");
     //MessageBox(NULL, "tool window not created", NULL, MB_OK);
     return ( false );        
   }
}   

//-----------------------------------------------------------------------
bool GWToolWin::destroyToolBar()
{
  if ( hToolBar && ::IsWindow(hToolBar) )
  {
     if (DestroyWindow (hToolBar))
     {
      hToolBar = (HWND)NULL;
      return ( true );
     }
  }
  return ( false );
}

//-----------------------------------------------------------------------
GWToolWin::GWToolWin()
{
   hToolBar = (HWND)NULL;
   hStatusBar = (HWND)NULL;
   gwMessWnd = NULL;
}

//-----------------------------------------------------------------------
GWToolWin::~GWToolWin()
{
   destroyWindow();
}

//-----------------------------------------------------------------------
bool GWToolWin::setMessageWin(GWWindow* gwwindow)
{
   gwMessWnd = dynamic_cast<GWWindow*>(gwwindow);
   return ( gwMessWnd != NULL);
}
//-----------------------------------------------------------------------
void GWToolWin::destroyWindow()
{
   if ( hToolBar  && ::IsWindow(hToolBar) )
      DestroyWindow(hToolBar);

   if ( hStatusBar  && ::IsWindow(hStatusBar) )
      DestroyWindow(hStatusBar);

   Parent::destroyWindow();
}


//-----------------------------------------------------------------------
void GWToolWin::onCommand(int id, HWND hwndCtl, UINT codeNotify)
{
   //forward menu and toolbar button messages
   GWWindow *gwwin = dynamic_cast<GWWindow*>(gwMessWnd);
   if ( gwwin && ::IsWindow(gwwin->getHandle()) )
      gwwin->onCommand(id, hwndCtl, codeNotify);
}   

//-----------------------------------------------------------------------
bool GWToolWin::enableButton(int buttonID, bool enable)
{
   if ( hToolBar  && ::IsWindow(hToolBar) )
      return (SendMessage(hToolBar, TB_ENABLEBUTTON, 
                          buttonID, MAKELONG(enable,0)) != 0);
   else
      return ( false );
}

//-----------------------------------------------------------------------
bool GWToolWin::hideButton(int buttonID, bool hide)
{
   if ( hToolBar  && ::IsWindow(hToolBar) )
      return (SendMessage (hToolBar, TB_HIDEBUTTON, 
                           buttonID, MAKELONG(hide, 0)) != 0);
   else
      return ( false );
}

//-----------------------------------------------------------------------
bool GWToolWin::insertButton(int buttonIndex, LPTBBUTTON tbButton)
{
   if ( hToolBar  && ::IsWindow(hToolBar) )
      return (SendMessage(hToolBar, TB_INSERTBUTTON, 
                          buttonIndex, (long)tbButton) != 0);
   else
      return ( false );
}

//-----------------------------------------------------------------------
bool GWToolWin::deleteButton(int buttonIndex)
{
   if ( hToolBar && ::IsWindow(hToolBar) )
      return ( SendMessage(hToolBar, TB_DELETEBUTTON, buttonIndex, 0) != 0);
   else
      return ( false );
}

//-----------------------------------------------------------------------
// handles the onNotify event to provide popup help for a button
//  help pops up only if there is a string table in the resource with a string
//  with ID identical to the corresponding button's ID
int GWToolWin::onNotify(int id, LPNMHDR pnmhdr)
{
   id;      // unused parameter
      CHAR szBuf[128];

    LPTOOLTIPTEXT lpToolTipText = (LPTOOLTIPTEXT)pnmhdr;
    if (lpToolTipText->hdr.code ==  TTN_NEEDTEXT)
    {
        // If the system needs text, load it from the resource.
        LoadString (getHInstance(), 
            lpToolTipText->hdr.idFrom,    // string ID == cmd ID
            szBuf,
            sizeof(szBuf) );

      lpToolTipText->lpszText = szBuf;
    }
   return ( Parent::onNotify( id, pnmhdr ) );
}   


//-----------------------------------------------------------------------
bool GWToolWin::createStatusBar(int sbarID, int numParts, LPINT rightEdges)
{
   if ( ::IsWindow(getStatusBarHandle()) )
      if ( !destroyStatusBar() )
         return ( false );

   if ( !getHandle() || !::IsWindow(getHandle()) ) 
   {
      AssertFatal(0, "GWToolWin::createStatusBar: tool window not created");
//      MessageBox(NULL, "Tool window not created", NULL, MB_OK);
      return ( false );
   }

   hStatusBar = CreateStatusWindow(WS_CHILD | WS_BORDER | WS_VISIBLE, 
            "", getHandle(), sbarID);
   if ( !hStatusBar )    
   {
    AssertFatal(0, "GWToolWin::createStatusBar: status bar did not create");
    //MessageBox(NULL, "Status bar didn't create", NULL, MB_OK);
    return ( false );
   }
  
   if (!SendMessage(hStatusBar, SB_SETPARTS, numParts, (long)rightEdges))
   {
     AssertFatal(0, "GWToolWin::createStatusBar: status bar parts not set");
     //MessageBox(NULL, "Status bar parts not set", NULL, MB_OK);
     return ( false );
   }
   
    return ( true );
}

//-----------------------------------------------------------------------
bool GWToolWin::destroyStatusBar()
{
  if ( hStatusBar && ::IsWindow(hStatusBar) )
  {
     if (DestroyWindow (hStatusBar))
     {
      hStatusBar = (HWND)NULL;
      return ( true );
     }
  }
  return ( false );
}


//-----------------------------------------------------------------------
bool GWToolWin::setStatusText(int section, LPCSTR text)
{
   if ( hStatusBar && ::IsWindow(hStatusBar) )
   {
      return (SendMessage(hStatusBar, SB_SETTEXT, 
                          MAKEWORD(section, 0), (long)text) != 0);
   }
   return ( false );
}

//-----------------------------------------------------------------------
int GWToolWin::getStatusTextLen(int section)
{
   if ( hStatusBar && ::IsWindow(hStatusBar) )
   {
      long lres = SendMessage(hStatusBar, SB_GETTEXTLENGTH, 
                  MAKEWORD(section, 0), 0);
      return ( LOWORD(lres) );
   }
   return ( false );
}

//-----------------------------------------------------------------------
int GWToolWin::getStatusText(int section, char *buffer)
{
   if ( hStatusBar && ::IsWindow(hStatusBar) )
   {
      long lres = SendMessage(hStatusBar, SB_GETTEXT, 
                  MAKEWORD(section, 0), (long) buffer);
      return ( LOWORD(lres) );
   }
   return ( false );
}

//-----------------------------------------------------------------------
bool GWToolWin::setStatusParts(int numParts, LPINT rightEdges)
{
   if ( hStatusBar && ::IsWindow(hStatusBar) )
   {
      return (SendMessage(hStatusBar, SB_SETPARTS, 
                          MAKEWORD(numParts,0), (long)rightEdges) != 0);
   }
   return ( false );
}

//-----------------------------------------------------------------------
int GWToolWin::getStatusParts(int maxParts, LPINT rightEdges)
{
   if ( hStatusBar && ::IsWindow(hStatusBar) )
   {
      return ( SendMessage(hStatusBar, SB_GETPARTS, 
               MAKEWORD(maxParts, 0), (long) rightEdges) );
   }
   return ( false );
}
