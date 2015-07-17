#include "g_cds.h"
#include "fn_table.h"
#include "bitview.h"
#include <commdlg.h>


GFXSurface *sfc = NULL;
GFXPalette *pal;
GFXBitmap  *bkgnd;
GFXBitmap  *dest = NULL;
Point2  imgPoint(0,0);

char AppPath[1048];
char IniFile[1048];

static HWND       hwndApp;
static HMENU      hmenuApp;
static HINSTANCE  hInstApp;
Int32 curShadeIndex;

void Redraw()
{
   if(sfc) {
      sfc->drawBitmap2d(bkgnd, &Point2(0,0));
      if(dest)
         sfc->drawBitmap2d(dest, &Point2(imgPoint.x - dest->getWidth()/2, 
                   imgPoint.y - dest->getHeight()/2));
      sfc->update();
   }
}

void LoadBitmap(char *fn)
{
  if(dest)
   delete dest;
   
  if(fn[0])
  {
   dest = GFXBitmap::load(fn);
   dest->attribute = BMA_TRANSLUCENT | BMA_TRANSPARENT;
  }
  else
  {
   dest = NULL;
  }
}


void OpenBitmap(void)
{
   char file[1024];
   
   
   OPENFILENAME ofn;
   memset((void *) &ofn, 0, sizeof(ofn));
   ofn.hwndOwner = hwndApp;
   
   ofn.lStructSize = sizeof(ofn);
   ofn.lpstrFilter = "Bitmap Files\0*.bmp;*.dib\0\0";
   ofn.nFilterIndex = 1;
   ofn.lpstrFile = file;
   ofn.nMaxFile = sizeof(file);
   file[0] = 0;

   ofn.lpstrInitialDir = ".";
   ofn.lpstrDefExt = "bmp";

   if(GetOpenFileName(&ofn))
      LoadBitmap(file);
   Redraw();
}

HWND paletteWindow = NULL;

BOOL CALLBACK PaletteDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   BOOL  msgProcessed;
   RECT  r;
   HWND  hCtrl;

   msgProcessed = FALSE;
   switch (message) 
   {
      case WM_INITDIALOG:
         GetWindowRect(hDlg, &r);
         r.left = (GetSystemMetrics(SM_CXSCREEN) - (r.right - r.left)) / 2;
         r.top  = (GetSystemMetrics(SM_CYSCREEN) - (r.bottom - r.top)) / 2;
         SetWindowPos(hDlg, NULL, r.left,r.top,0,0, SWP_NOSIZE | SWP_NOZORDER);
      
         hCtrl = GetDlgItem(hDlg, IDC_SC_TRANSLUCENCY);
         SetScrollRange(hCtrl, SB_CTL, 0, pal->getMaxTranslucent(), TRUE);
         SetScrollPos(hCtrl, SB_CTL, pal->getTransLevel(), TRUE);

         hCtrl = GetDlgItem(hDlg, IDC_SC_HAZE);
         SetScrollRange(hCtrl, SB_CTL, 0, pal->getMaxHaze(), TRUE);
         SetScrollPos(hCtrl, SB_CTL, pal->getHazeIndex(), TRUE);
         
         hCtrl = GetDlgItem(hDlg, IDC_SC_SHADE);
         SetScrollRange(hCtrl, SB_CTL, 0, pal->getMaxShade(), TRUE);
         SetScrollPos(hCtrl, SB_CTL, curShadeIndex, TRUE);
         
         msgProcessed = TRUE;
         break;

      case WM_COMMAND:            
         switch(LOWORD(wParam))
         {
                      /*
            case IDCANCEL:
            case IDM_PUSHOK:
               EndDialog(hDlg, wParam);         
               msgProcessed = TRUE;
               
               break;  
                        */
            default:
               break;
         }
      
         break;
      case WM_HSCROLL:
         int shift;
         int newPos;
         
         switch(LOWORD(wParam))
         {
            case SB_PAGELEFT:
            case SB_LINELEFT:
               shift = -1;
               break;
            case SB_PAGERIGHT:
            case SB_LINERIGHT:
               shift = 1;
               break;
            case SB_THUMBTRACK:
            case SB_THUMBPOSITION:
               shift = 0;
               newPos = HIWORD(wParam);
               break;
            default:
               return 1;
         }
         hCtrl = (HWND) LOWORD(lParam);
         long ctrlIdx;
         ctrlIdx = GetWindowLong(hCtrl, GWL_ID);
         
         if(shift)
         {
            int min, max;
            newPos = GetScrollPos(hCtrl, SB_CTL);
            newPos += shift;
            GetScrollRange(hCtrl, SB_CTL, &min, &max);
            if(newPos < min)
               newPos = min;
            else if(newPos > max)
               newPos = max;
         }
         SetScrollPos(hCtrl, SB_CTL, newPos, TRUE);
         
         switch(ctrlIdx)
         {
            case IDC_SC_TRANSLUCENCY:
               hCtrl = GetDlgItem(hDlg, IDC_SC_TRANSLUCENCY);
               pal->setTransLevel(GetScrollPos(hCtrl, SB_CTL));
               break;
            case IDC_SC_HAZE:
               
               break;
            case IDC_SC_SHADE:
               break;
         }
         Redraw();
         return 0;
      case WM_CLOSE:
         DestroyWindow(hDlg);
         paletteWindow = 0;
         break;
   }
   return (msgProcessed);               
}

LONG CALLBACK AppWndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
  RECT Rect;
  int key;
  switch (msg)
  {
    case WM_COMMAND:
      switch(LOWORD(wParam))
      {
         case CM_FILEEXIT:
            DestroyWindow(hwndApp);
            break;
         case CM_FILEOPEN:
            OpenBitmap();
            break;
         case CM_PALETTE:
            if(!paletteWindow)
               paletteWindow = CreateDialog(hInstApp, "PaletteControl", hwnd, (DLGPROC) PaletteDialog);
            break;
      }
      break;
    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_CREATE:
      break;

    case WM_PALETTECHANGED:
      break;
    case WM_QUERYNEWPALETTE:
    {
      BOOL changed;
      HDC hDC = GetDC(hwnd);
      if(sfc) sfc->setPalette(NULL);
      changed = RealizePalette(hDC);
      if(changed)
      {
         InvalidateRect(hwnd, NULL, TRUE);
         //((GFXCDSSurface *)sfc)->setHWND(hwnd);
      }
      return changed;
    }
    case WM_SETCURSOR:
      break;

    case WM_ENTERMENULOOP:
      break;

    case WM_EXITMENULOOP:
      break;

    case WM_INITMENUPOPUP:
        break;

    case WM_DESTROY:
      hwndApp = NULL;
      PostQuitMessage(0);
      break;

    case WM_PAINT:
      LPPAINTSTRUCT lpPaint;
      //BeginPaint(hwnd, lpPaint);
      if(sfc) sfc->update();
      //EndPaint(hwnd, lpPaint);
      return 1;
      break;

    case WM_MOVE:
    case WM_SIZE:
      break;

    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
      if(GetKeyState(VK_LBUTTON) < 0)
      {
         imgPoint.x = LOWORD(lParam);
         imgPoint.y = HIWORD(lParam);
         Redraw();
      }
      break;
  }

  return DefWindowProc(hwnd,msg,wParam,lParam);
}

BOOL AppInit(HINSTANCE hInst,HINSTANCE hPrev,int sw,LPSTR szCmdLine)
{
  WNDCLASS cls;

  /* Save instance handle for DialogBoxes */
  hInstApp = hInst;

  if (!hPrev)
  {
    //***  Register a class for the main application window
    cls.hCursor        = LoadCursor(0,IDC_ARROW);

    //*** Just for fun, we'll draw our own spinning cube icon.
    cls.hIcon          = LoadIcon(hInst, "AppIcon");
    cls.lpszMenuName   = "AppMenu";
    cls.lpszClassName  = "MyWindowClass";
    cls.hbrBackground  = NULL;
    cls.hInstance      = hInst;
    cls.style          = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
    cls.lpfnWndProc    = (WNDPROC)AppWndProc;
    cls.cbClsExtra     = 0;
    cls.cbWndExtra     = 0;

    if (!RegisterClass(&cls))
      return FALSE;
  }
  
  RECT r;
  SetRect(&r, 0, 0, bkgnd->getWidth(), bkgnd->getHeight());
  
  AdjustWindowRectEx(&r, WS_POPUP | WS_CAPTION, TRUE, 0);
  
  hwndApp = CreateWindowEx(
                  0,
                  "MyWindowClass",           // Class name
                  "GFX Bitmap Viewer",           // Caption
                  WS_POPUP |
                  WS_SYSMENU |
                  WS_CAPTION,
                  100, 100, 
                  
                  r.right - r.left,
                  r.bottom - r.top,             // Size
                  0,                   // Parent window (no parent)
                  0,                   // use class menu
                  hInst,               // handle to window instance
                  0                    // no params to pass on
                  );
   hmenuApp = LoadMenu(hInst, "MainMenu");
   SetMenu(hwndApp, hmenuApp);
  
   GetWindowRect(hwndApp, &r);

   r.left = (GetSystemMetrics(SM_CXSCREEN) - (r.right - r.left)) / 2;
   r.top  = (GetSystemMetrics(SM_CYSCREEN) - (r.bottom - r.top)) / 2;
   SetWindowPos(hwndApp, NULL, r.left,r.top,0,0, SWP_NOSIZE | SWP_NOZORDER);
   ShowWindow(hwndApp,sw);
   UpdateWindow(hwndApp);
   return TRUE;
}

/**************************************************************************
  WinMain

  Description:
    The main procedure for the App.  After initializing, it just goes
  into a message-processing loop until it gets a WM_QUIT message.
 **************************************************************************/

void ProcessReadyEvents(void)
{
   MSG msg;
   int done = 0;
   while(!done)
   while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
   {
		if(msg.message == WM_QUIT)
      {
         done = 1;
      }
      TranslateMessage(&msg);
      DispatchMessage(&msg);
   }
}


int PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmdLine, int sw)
{
  //*** Call initialization procedure
  GetModuleFileName(NULL, AppPath, 1048);
  *(strrchr(AppPath, '\\') + 1) = 0;
  strcpy(IniFile, AppPath);
  strcat(IniFile, "bitview.ini");
  
  char fnBuf[1048];
  
  GetPrivateProfileString("Bitview", "palette", "pal.pal", 
            fnBuf, sizeof(fnBuf), IniFile);
  
  pal = GFXPalette::load(fnBuf);
  curShadeIndex = pal->getMaxShade() / 2;
  
  GetPrivateProfileString("Bitview", "backgnd", "back.bmp", 
            fnBuf, sizeof(fnBuf), IniFile);
  
  bkgnd = GFXBitmap::load(fnBuf);
 
  LoadBitmap(szCmdLine);
  
  if (!AppInit(hInst,hPrev,sw,szCmdLine))
    return FALSE;

    //*** Polling messages from event queue until quit
  
  GFXCDSSurface::setFunctionTable(&rclip_table);
  
  GFXSurface::createRasterList(3000);
  GFXCDSSurface::create(sfc, TRUE, bkgnd->getWidth(), bkgnd->getHeight(), hwndApp);
  sfc->setPalette(pal);
  pal->setTransLevel(1);
  sfc->lock();
  Redraw();
  ProcessReadyEvents();
  return 0;
}

