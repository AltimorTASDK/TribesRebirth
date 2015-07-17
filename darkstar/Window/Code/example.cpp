//----------------------------------------------------------------------------

#include <gw.h>
#include <gwLog.h>
#define GFX   true
#define PALETTE "palette.ppl"    
//NOTE: if you do no load a palette DirectDraw will appear BLACK!
#include "resource.h"


//----------------------------------------------------------------------------

#ifdef GFX
class MainWindow: public GWCanvas
{
	typedef GWCanvas Parent;
#else
class MainWindow: public GWWindow
{
	typedef GWWindow Parent;
#endif

GWMenu mainMenu;
GWMenu apopup;
GWToolWin atoolwin;

public:
    MainWindow();
    ~MainWindow();

	void render();
	void onDestroy();
   void onSysKey(UINT vk, BOOL fDown, int cRepeat, UINT flags);
   void onKey(UINT vk, BOOL fDown, int cRepeat, UINT flags);
	void onMouseMove(int x, int y, UINT keyFlags);
   void onRButtonDown(BOOL fDoubleClick, int x, int y, UINT keyFlags);
   void onCommand(int id, HWND hwndCtl, UINT codeNotify);

	//popup menu handlers
	void onShowToolBar();
	void onHideToolBar();

};


MainWindow::MainWindow()
{
#ifdef GFX
    if (create("GWMain",
#else
    if (create(0,"GWMain",
#endif
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		Point2I(CW_USEDEFAULT,0),
		Point2I(640,480),
		NULL,0)) {
	    show( GWMain::cmdShow );
	    update();
	}
	else {
		AssertFatal(0,"MainWindow:: Could not create main window");
		exit(1);
	}
#ifdef PALETTE
	// Load palette
	GFXPalette* palette = new GFXPalette();
	if (!palette->read(PALETTE)) {
		AssertFatal(0,"Error loading palette");
	   return;
	}
	pSurface->setPalette(palette);
#endif

//initalize the main menu for this window
// we could also have added it to tool window
mainMenu.create("IDM_MENU1");
mainMenu.setMainMenu(getHandle());

//init a popup
apopup.create("IDM_MENU2");

}


void MainWindow::onShowToolBar() 
{

TBBUTTON tbButtons[] = {
   { 0, IDM_NEW, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
   { 1, IDM_OPEN, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
   { 2, IDM_SAVE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
   { 0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, -1},
   { 3, IDM_CUT, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
   { 4, IDM_COPY, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
   { 5, IDM_PASTE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
   { 0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, -1},
   { 6, IDM_PRINT, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
   { 7, IDM_HELP, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
   { 8, IDM_ONHELP, TBSTATE_ENABLED | TBSTATE_WRAP, TBSTYLE_BUTTON, 0L, 0},
   { 6, IDM_PRINT, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
   { 7, IDM_HELP, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
   { 8, IDM_ONHELP, TBSTATE_ENABLED | TBSTATE_WRAP, TBSTYLE_BUTTON, 0L, 0},
};
int numButtons = sizeof(tbButtons)/sizeof(TBBUTTON);

atoolwin.createToolBar(0, tbButtons, numButtons, IDB_BITMAP1);

//now capture the toolbar button messages
// so that this window receives messages when buttons clicked
atoolwin.setMessageWin(this);

}

void MainWindow::onHideToolBar()
{
   atoolwin.destroyToolBar();
}   


void MainWindow::onCommand(int id, HWND hwndCtl, UINT codeNotify)
{

	RECT r;
   int rightEdges[3] = {60, 250, -1,};
   int numEdges = sizeof(rightEdges)/sizeof(rightEdges[0]);
   char menstr[40];

   HWND tbar = atoolwin.getToolBarHandle();
	if ( ::IsWindow(tbar) )
	{
	   if ( hwndCtl == tbar )
	   {
	      strcpy(menstr, "buttonID: ");
         itoa(id, &menstr[strlen(menstr)], 10);
         if ( ::IsWindow (atoolwin.getStatusBarHandle()) )
            atoolwin.setStatusText(2, menstr);
      }
	}

	switch (id)
	{
	case ID_POPUP_WINSHOW:
      atoolwin.destroyWindow();
      r.left = 200;
      r.top = 200;
      r.right = r.left+360;
      r.bottom = r.top + 110;
      atoolwin.createWin("Tools", &r);
	   break;
   case ID_POPUP_WINHIDE:
      atoolwin.destroyWindow();
      break;
	case ID_POPUP_TOOLSHOW:
      onShowToolBar();
	   break;
   case ID_POPUP_TOOLHIDE:
      onHideToolBar();
	   break;
   case ID_POPUP_STATSHOW:
      atoolwin.createStatusBar(0, numEdges, rightEdges);
      atoolwin.setStatusText(1, "test");
      break;
   case ID_POPUP_STATHIDE:
      atoolwin.destroyStatusBar();
      break;
   default:
      if ( atoolwin.getStatusBarHandle() )
      {
         strcpy(menstr," menuID: ");
         itoa(id, &menstr[strlen(menstr)], 10);
         atoolwin.setStatusText(1, menstr); 
      }
      break;
	}

	
}

void MainWindow::onRButtonDown(BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
//	popup the 0th submenu of the popup menu at the cursor position
	apopup.popupMenu(getHandle(), 0, x, y);

}


MainWindow::~MainWindow()
{
}

void MainWindow::onDestroy()
{
	PostQuitMessage(0);
}

void MainWindow::render()
{
#ifdef GFX
	pSurface->clear(2);
	pSurface->drawLine2d(&Point2I(0,0),&Point2I(300,300),7);
#endif
}

void MainWindow::onMouseMove(int x, int y, UINT)
{
#ifdef GFX
	lock();
	pSurface->clear(1);
	pSurface->drawLine2d(&Point2I(0,0),&Point2I(x,y),6);
	unlock();
	flip();
#endif
}

void MainWindow::onSysKey(UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
#ifdef GFX
   if (!fDown)
   {
      if (flags & (1<<13))    //check for alt with syskey
      {
         switch (vk)
         {
            case VK_ESCAPE:
            case VK_TAB:
               setMode(GFX_SM_MODE_WINDOW);
               break;

            case VK_RETURN:
               if (manager.getMode() == GFX_SM_MODE_WINDOW) setMode(GFX_SM_MODE_DIRECT);
               else setMode(GFX_SM_MODE_WINDOW);
               break;
         }
      }
      else
         if (vk == VK_ESCAPE)
            setMode(GFX_SM_MODE_WINDOW);
   }
#endif
   Parent::onSysKey( vk, fDown, cRepeat, flags );
}

void MainWindow::onKey(UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
#ifdef GFX

   if (!fDown)
   {
      switch (vk)                //adjust Direct Draw screen resolution
      {                          //ignored if in DIB section
         case VK_F5:



            return;
         case VK_ADD:
            adjSurfaceSize(+1);
            return;
         case VK_SUBTRACT:
            adjSurfaceSize(-1);
            return;
      }
   }
#endif
   Parent::onKey(vk, fDown, cRepeat, flags);
}   



//----------------------------------------------------------------------------

class CMain: public GWMain
{
   MainWindow* window;
   GWLog *log;

public:
	CMain();
	~CMain();
	virtual bool initInstance();
} Main;

CMain::CMain()
{
	window = 0;
}

CMain::~CMain()
{
   delete window;
   delete log;
}

bool CMain::initInstance()
{
   window = new MainWindow();
   log = new GWLog("My debug listbox...");

   return true;
}



