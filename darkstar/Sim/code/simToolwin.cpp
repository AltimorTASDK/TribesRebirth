//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

//----------------------------------------------------------------------------
//-----  the templated classes we need
#if defined(__BORLANDC__) && (__BORLANDC__ >= 0x500)
#pragma option -Jg
#endif

#include <sim.h>
#include <g_bitmap.h>
#include "simToolwin.h"

//----------------------------------------------------------------------------
const int MAX_BTN_NO = 1000;
const int STD_STATBAR_HEIGHT = 12 + 10;  //12 + 2*5 for borders

//----------------------------------------------------------------------------
// init static vars
char *SimToolWindow::tempstr = NULL;

//----------------------------------------------------------------------------
SimToolWindow::SimToolWindow()
{
   currentButton=0;
   MinHeight=0;
   MinWidth=0;
   type = SimToolWindow::Horiz;
   tempstr = NULL;
   handler = NULL;
   bitmap = 0;
}


//----------------------------------------------------------------------------
SimToolWindow::~SimToolWindow()
{
   if (tempstr)
      delete [] tempstr;

   if ( bitmap )
      ::DeleteObject(bitmap);

   CloseWindow();
}


//----------------------------------------------------------------------------
//for menu selection & button click
void SimToolWindow::onCommand(int id, HWND hwndCtl, UINT codeNotify)
{
   if (handler)
   {
      if (id < MAX_BTN_NO)
      {
         //the user clicked a toolbar button, find its console cmd
         for (BtnList::iterator i = btnList.begin();i != btnList.end(); i++)
         {
            if (id == (*i).second.BtnIndex)
            {
               if ( !isButtonDown((*i).first))
                   handler->tbarCallback(id, hwndCtl, codeNotify, 
                  (*i).second.ConCommand.c_str());
                else                                               
                   handler->tbarCallback(id, hwndCtl, codeNotify, 
                  (*i).second.ConCommandUp.c_str());
            }
         }
      }
   }
}

//----------------------------------------------------------------------------
// handle this message to provide pop up help for the buttons
int SimToolWindow::onNotify(int id, LPNMHDR pnmhdr)
{
    LPTOOLTIPTEXT lpToolTipText = (LPTOOLTIPTEXT)pnmhdr;

    if (lpToolTipText->hdr.code ==  TTN_NEEDTEXT)
    {
      BtnList::iterator i;
      // If the system needs text, find the button in the map
      for ( i = btnList.begin(); i!=btnList.end();i++ )
      {
        //supply the help text for the button
        if ( id == (*i).second.BtnIndex)
        {
            lpToolTipText->lpszText = const_cast<char*>
                        ((*i).second.Help.c_str());
				return TRUE;
        }
      }
   }
   return ( GWToolWin::onNotify( id, pnmhdr ) );
}   


//----------------------------------------------------------------------------
// returns a string listing all the button names and their corresponding
// console commands
char* SimToolWindow::GetBtnInfo()
{
   const int MAXLEN = 200;

   if (tempstr)
      delete [] tempstr;

   //first get an approx. string length
   int noLines = (int)(btnList.end()-btnList.begin());
   tempstr = new char[(1+noLines)*MAXLEN];
   if (!tempstr)
      return (NULL);

   tempstr[0]= '\0';

   //now make the string
   sprintf(tempstr, "   %5s: %s, %s; \"%s\"\n", "Name", "#", "Bmp#", 
         "Command");
   
   BtnList::iterator i;
   for ( i = btnList.begin();i != btnList.end(); i++)
   {
      bool b = ( (*i).second.ConCommand.length() > 60 );
      char oneLine[MAXLEN];
      oneLine[0]= '\0';
      if ( b )
         sprintf(oneLine, "   %10s: %2d, %2d; \"%60s\"\n", 
            (*i).first.c_str(),
            (*i).second.BtnIndex,
            (*i).second.BmpIndex,
            (*i).second.ConCommand.c_str());
      else
           sprintf(oneLine, "   %10s: %2d, %2d; \"%s\"\n", 
            (*i).first.c_str(),
            (*i).second.BtnIndex,
            (*i).second.BmpIndex,
            (*i).second.ConCommand.c_str());

      if (strlen(oneLine))
         strcat(tempstr, oneLine);
   }

   
   return tempstr;
}


//----------------------------------------------------------------------------
bool SimToolWindow::FindButton(String name)
{
   BtnList::iterator i = btnList.find(name);
   if (i != btnList.end())
   {
      return (true);
   }
   else
      return (false);
}


//----------------------------------------------------------------------------
bool SimToolWindow::DeleteButton(String name)
{
   //check for null name or name not found
   if (!name.length() || !FindButton(name))
      return (false);

   BtnList::iterator i = btnList.find(name);

   //check if last button
   if (currentButton==1)
   {
      if (!GWToolWin::destroyToolBar())
         return (false);
   }
   else
   {
      if (!GWToolWin::deleteButton( (*i).second.BtnIndex ))
         return (false);
   }

   //remove from map
   btnList.erase(i);

   currentButton--;
   if (currentButton==0)
      return (true);
   
    //resize the window
   if (!RefitWindow())
     return (false);

   return (true);
}

//----------------------------------------------------------------------------
bool SimToolWindow::SetButtonConCommand(String name, String conCommand,
      String conCommandUp )
{
   //check for null name or name not found
   if (!name.length() || !FindButton(name))
      return (false);

   BtnList::iterator i = btnList.find(name);
   (*i).second.ConCommand = conCommand;
   (*i).second.ConCommandUp = conCommandUp;
   return (true);
}

//----------------------------------------------------------------------------
bool SimToolWindow::SetButtonHelp(String name, String help)
{
   //check for null name of name not found
   if ( !name.length() || !FindButton(name) )
      return ( false );

   BtnList::iterator i = btnList.find(name);
   (*i).second.Help = help;
   return ( true );
}

//----------------------------------------------------------------------------
bool SimToolWindow::isButtonDown(String name)
{
   //check if name found
   if ( !name.length() || !FindButton(name) )
      return ( false );

   BtnList::iterator i = btnList.find(name);
   int index = (*i).second.BtnIndex;
     DWORD res = SendMessage(getToolBarHandle(), TB_GETSTATE, WORD(index), 0);

   return ( (res & TBSTATE_PRESSED) || (res & TBSTATE_CHECKED) );
}


//----------------------------------------------------------------------------
bool SimToolWindow::NewWindow(GWWindow *parent, String name, DWORD winStyle)
{

   RECT winrect;
   winrect.left = DEFAULTWINPOS;
   winrect.top = DEFAULTWINPOS;

   POINT cursorPos;
   if ( ::GetCursorPos(&cursorPos) )
   {
      winrect.left = cursorPos.x;
      winrect.top = cursorPos.y;
   }

   winrect.right = winrect.left + 30;
   winrect.bottom = winrect.top + 30;

   if (!GWToolWin::createWin(parent, name.c_str(), &winrect, winStyle))
      return (false);

   return ( true );

}

//----------------------------------------------------------------------------
bool SimToolWindow::CloseWindow()
{
   //delete temp string if necessary
   if (tempstr)
      delete [] tempstr;

   GWToolWin::destroyWindow();

   return (true);
}

//----------------------------------------------------------------------------
bool SimToolWindow::PositionWindow(int x, int y)
{
   if ( (x < 0) || (x > ::GetSystemMetrics(SM_CXSCREEN)) )
      return ( false );

   if ( (y < 0) || (y > ::GetSystemMetrics(SM_CYSCREEN)) )
      return ( false );

   if ( getHandle() && ::IsWindow(getHandle()) )
   {
      return (::SetWindowPos(getHandle(), HWND(NULL), x, y,
       0, 0, SWP_NOSIZE | SWP_NOZORDER) );
   }
   return ( false ); 
   
}
//----------------------------------------------------------------------------
// return a bounding rectangle for the toolbar
bool SimToolWindow::ToolBarSize(RECT *r)
{
   r->left =0;
   r->right = 0;
   r->bottom = 0;
   r->right = 0;

   if ( !getToolBarHandle() )
      return ( true );

   if ( !::IsWindow(getHandle()) || !::IsWindow(getToolBarHandle()) )    
      return (false);

   int num = SendMessage(getToolBarHandle(), TB_BUTTONCOUNT, 0, 0);
   HWND tbarh = getToolBarHandle();

   for ( int i=0; i<num; i++ )
   {
      RECT thisR;
      if ( SendMessage(tbarh, TB_GETITEMRECT, WORD(i), (long) &thisR ))
      {
         r->right = max(r->right, thisR.right);
         r->bottom = max(r->bottom, thisR.bottom);
      }
   }

   r->left=0;
   r->top =0;

   return (true);

}


//----------------------------------------------------------------------------
// resizes the window to hold its elements
bool SimToolWindow::RefitWindow()
{
   RECT toolwin;
   toolwin.left = toolwin.right = 0;
   toolwin.top = toolwin.bottom =0;

   //get tool bar rectangle
   if (!ToolBarSize(&toolwin))
     return (false);
 
   //make sure window is at least min sizes
   toolwin.bottom = max(toolwin.bottom, (long)MinHeight);  
   toolwin.right = max(toolwin.right, (long)MinWidth);      


   //get the height of the title bar here
   RECT winrect;
   POINT cpoint = {0,0};

   if ( !::GetWindowRect(getHandle(), &winrect ))
      return ( false );
   if ( !::ClientToScreen(getHandle(), &cpoint ))
      return ( false );
   int border = cpoint.x-winrect.left;

   //border << 2;

   toolwin.bottom += cpoint.y-winrect.top+border;
   toolwin.right += border;

   int sbarht = 0;
   if ( getStatusBarHandle() )
   {
      toolwin.bottom += STD_STATBAR_HEIGHT;
      sbarht+= STD_STATBAR_HEIGHT;
   }

     //resize the window
   if (!::SetWindowPos(getHandle(), HWND(NULL), 0, 0,
    toolwin.right-toolwin.left, 
    toolwin.bottom-toolwin.top,
    SWP_NOMOVE | SWP_NOZORDER) )
      return (0);

   if ( getStatusBarHandle() )
      if (!::SetWindowPos(getStatusBarHandle(), HWND(NULL), 
       0, toolwin.bottom,
       toolwin.right-toolwin.left, 
       STD_STATBAR_HEIGHT,
       SWP_NOZORDER) )
         return (0);


    //resize the toolbar accordingly
   if ( getToolBarHandle() )
      if (!::SetWindowPos(getToolBarHandle(), HWND(NULL), 0, 0,
       toolwin.right-toolwin.left, 
       toolwin.bottom-toolwin.top-sbarht,
       SWP_NOMOVE | SWP_NOZORDER) )
         return (0);

   return (1);
}   


//----------------------------------------------------------------------------
bool SimToolWindow::AddGap(String name)
{
   return (AddButton(name, 0, false, false, TBSTYLE_SEP));
}


//----------------------------------------------------------------------------
bool SimToolWindow::AddButton(String name, GFXBitmap *pBM, bool wrapAfter,
   bool toggleStyle, DWORD bstyle)
{

  // check for valid window
  if ( !(getHandle() && ::IsWindow(getHandle())) )
   return ( false );

  //check if name already taken or null length
  if (!name.length() || FindButton(name))
     return (0);

  //check if limit reached
  if ( currentButton >= MAX_BTN_NO)
     return (false);


   GFXPalette p;
   p.setWindowsColors();

   HBITMAP hBM;
   if (pBM)
   {
      struct   // BITMAPINFOHEADER
      {
         BITMAPINFOHEADER  bmiHeader;
         RGBQUAD           bmiColors[256];
      }bi;

      bi.bmiHeader.biSize           = sizeof( BITMAPINFOHEADER );
      bi.bmiHeader.biWidth          = pBM->width;
      bi.bmiHeader.biHeight         = -pBM->height;
      bi.bmiHeader.biPlanes         = 1;
      bi.bmiHeader.biBitCount       = 8;
      bi.bmiHeader.biCompression    = BI_RGB;
      bi.bmiHeader.biSizeImage      = pBM->imageSize;
      bi.bmiHeader.biXPelsPerMeter  = 0;
      bi.bmiHeader.biYPelsPerMeter  = 0;
      bi.bmiHeader.biClrUsed        = 256;
      bi.bmiHeader.biClrImportant   = 0;

      for ( int i=0; i<256; i++ )
      {
         bi.bmiColors[i].rgbRed     = p.palette[0].color[i].peRed;
         bi.bmiColors[i].rgbGreen   = p.palette[0].color[i].peGreen;
         bi.bmiColors[i].rgbBlue    = p.palette[0].color[i].peBlue;
         bi.bmiColors[i].rgbReserved = 0;
      }

      HWND hw = getToolBarHandle();
      HDC dc = GetDC(hw);

      //gfxBitmapInvert( pBM->pBits, pBM->pBits+((pBM->height-1)*pBM->stride), pBM->stride);
      hBM = CreateDIBitmap( dc, &bi.bmiHeader, CBM_INIT, pBM->pBits, (LPBITMAPINFO)&bi, DIB_RGB_COLORS );

      ReleaseDC(hw, dc);
   }
   else 
      hBM = NULL;




   TBADDBITMAP tbAddBM;

   tbAddBM.hInst = NULL;
   tbAddBM.nID   = (UINT)hBM;

   TBBUTTON tbButton;
   tbButton.idCommand   = currentButton;
   tbButton.fsState     = TBSTATE_ENABLED;
   tbButton.fsStyle     = bstyle;      //TBSTYLE_BUTTON;
   tbButton.dwData      = 0L;
   tbButton.iString     = 0;

  if ( (type == SimToolWindow::Vert) || (wrapAfter) )
      tbButton.fsState |= TBSTATE_WRAP;

  if ( toggleStyle )
      tbButton.fsStyle = TBSTYLE_CHECK;

   BOOL b;
   if (currentButton == 0)
   {
      tbButton.iBitmap = 0;
      b = GWToolWin::createToolBar(0, &tbButton, 1, hBM );
   }
   else
   {
      (bool)::SendMessage(getToolBarHandle(), TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), (long)0); 
      if (pBM)  
         tbButton.iBitmap = SendMessage(getToolBarHandle(), TB_ADDBITMAP, WORD(1), (long)&tbAddBM);   
      else
         tbButton.iBitmap = 0;
      b = (bool)::SendMessage(getToolBarHandle(), TB_ADDBUTTONS, WORD(1), (long)&tbButton);   
   }

  if (!b)
     return (0);

  if (!RefitWindow())
     return (0);

  //insert the button into the map
  TWButton btn;
  btn.BtnIndex = currentButton;
  btn.BmpIndex = tbButton.iBitmap;
  btn.ConCommand = "";

  btnList.insert(name, btn);
  currentButton++;

  //if there is a status bar, then resize it
  if ( getStatusBarHandle() && ::IsWindow(getStatusBarHandle()) )
     if ( !UpdateWindow(getStatusBarHandle()) )
        return ( false );

  return (true);
}

//----------------------------------------------------------------------------
bool SimToolWindow::DelStatus()
{
   if ( getStatusBarHandle() )
   {
      MinHeight=0;      //reset min desired window dims
      MinWidth =0;

      if (!GWToolWin::destroyStatusBar())
         return (false);

      if ( !RefitWindow() )  
         return ( false );
   }

   return ( true );
}

//----------------------------------------------------------------------------
bool SimToolWindow::AddStatus(int NumWidths, LPINT Widths)
{
  // check for valid window
  if ( !(getHandle() && ::IsWindow(getHandle())) )
   return ( false );

  //have to resize the window for sbar to fit in
  int imax=1;
  for ( int i=0; i< NumWidths; i++)
     if ( Widths[i] > imax )
        imax = Widths[i];

 //get rect of toolbar if any
 RECT trect = {0,0,0,0};
 if (!ToolBarSize(&trect))
     return (false);

/* //get the height of the title bar here
 RECT winrect = {0,0,0,0};
 POINT cpoint = {0,0};

 if ( !::GetWindowRect(getHandle(), &winrect ))
   return ( false );
 if ( !::ClientToScreen(getHandle(), &cpoint ))
   return ( false );
 int titleheight = cpoint.y-winrect.top;


//set desired min dims of window 
  MinHeight = trect.bottom-trect.top+STD_STATBAR_HEIGHT+titleheight;*/
  MinWidth = imax;

//  if ( !RefitWindow() )  
//     return ( false );

  //if bar exists, destroy it so it is resized correctly
  if ( getStatusBarHandle() && ::IsWindow(getStatusBarHandle()) )
   GWToolWin::destroyStatusBar();
  
  //set last part to variable width (in case window resized later)
  Widths[NumWidths-1] =-1;

  if ( !GWToolWin::createStatusBar(0, NumWidths, Widths) ) 
   return ( false );

  if ( !RefitWindow() )  
    return ( false );

  return ( true );
}

//----------------------------------------------------------------------------
bool SimToolWindow::AttachStatVar(int PartNo, String VarName, String PreText)
{
   //don't chck for valid sbar, since can set this anytime for later use

   SbarVarNames::iterator i = sbarVarNames.find(PartNo);
   if (i != sbarVarNames.end())
      sbarVarNames.erase(i);

   PartInfo pi;
   pi.Text = PreText;
   pi.VarName = VarName;
   
   sbarVarNames.insert(PartNo, pi);

   return ( true );
}

//----------------------------------------------------------------------------
bool SimToolWindow::ClearStatVar(int PartNo)
{

   SbarVarNames::iterator i = sbarVarNames.find(PartNo);
   if (i != sbarVarNames.end())
      sbarVarNames.erase(i);

   return ( true );
}

//----------------------------------------------------------------------------
const char* SimToolWindow::GetStatVar(int PartNo)
{
   SbarVarNames::iterator i = sbarVarNames.find(PartNo);
   if (i != sbarVarNames.end())
      return ( (*i).second.VarName.c_str() );

   return ( NULL );
}

//----------------------------------------------------------------------------
const char* SimToolWindow::GetStatText(int PartNo)
{
   SbarVarNames::iterator i = sbarVarNames.find(PartNo);
   if (i != sbarVarNames.end())
      return ( (*i).second.Text.c_str() );

   return ( NULL );
}

//----------------------------------------------------------------------------
int SimToolWindow::GetStatVarCount()
{
   if (getStatusBarHandle() && ::IsWindow(getStatusBarHandle()))
      return ( ::SendMessage(getStatusBarHandle(), SB_GETPARTS, 
         MAKEWORD(500,0), (long) 0) );

   return (0);
}

//----------------------------------------------------------------------------
bool SimToolWindow::onAdd()
{
	if (!Parent::onAdd())
		return false;

   if (!manager->findObject(SimToolSetId))
   {
      SimToolSet* set = new SimToolSet;
      manager->addObject(set);
      manager->assignId(set, SimToolSetId);
      manager->assignName(set, "SimToolSet");
   }
   addToSet(SimToolSetId);
	return true;
}	