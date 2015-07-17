//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#include <gw.h>
#include <bwcc.h>

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


#include <ts.h>
#include <ts_table.h>
#include <ts_bsp.h>


//----------------------------------------------------------------------------

class GridDialog: public GWDialog
{
	typedef GWDialog Parent;

	int state;
public:
	GridDialog(GWWindow* parent);
	bool create(GWWindow* parent);

	BOOL onInitDialog(HWND hwndFocus, LPARAM lParam);
	void onCommand(int id, HWND hwndCtl, UINT codeNotify);
};

GridDialog::GridDialog(GWWindow* parent)
{
	create(parent);
}

bool GridDialog::create(GWWindow* parent)
{
	return GWDialog::create(1024,parent);
}

BOOL GridDialog::onInitDialog(HWND, LPARAM)
{
	state = 0;
	return false;
}

void GridDialog::onCommand(int id, HWND hwndCtl, UINT codeNotify)
{
	switch (id) {
		case IDCANCEL:
			destroyWindow();
			break;
		case 101:
			state = 1;
			break;
		case 102:
			state = 2;
			break;
		default:
			Parent::onCommand(id,hwndCtl,codeNotify);
	}
}


//----------------------------------------------------------------------------

class DrawWindow: public GWCanvas
{
	typedef GWCanvas Parent;
public:
	void render();
	void onMouseMove(int x, int y, UINT keyFlags);
};

void DrawWindow::render()
{
	// Locking is taken care of
	pSurface->clear(0);
	pSurface->drawLine2d(&Point2(0,0),&Point2(300,300),7);
}

void DrawWindow::onMouseMove(int x, int y, UINT)
{
	// Must lock here
	lock();
	pSurface->drawLine2d(&Point2(0,0),&Point2(x,y),6);
	unlock();
	flip();
}


//----------------------------------------------------------------------------

class MainWindow: public GWCanvas //GWWindow
{
	typedef GWCanvas Parent;
	GridDialog* gdlg;
	DrawWindow* canvas;

public:
   MainWindow();
   ~MainWindow();

	void render();
	void onDestroy();
	void onChar(TCHAR ch, int cRepeat);
	void onMouseMove(int x, int y, UINT keyFlags);
};

MainWindow::MainWindow()
{
	gdlg = 0;
	GWWindowClass* wClass = new GWWindowClass;
	wClass->registerClass(CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
		0,0,HBRUSH(HOLLOW_BRUSH));

    if (create(wClass,"GWMain",
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		Point2(CW_USEDEFAULT,0),
		Point2(640,480),
		NULL,0)) {
	    show( GWMain::cmdShow );
	    update();
	}
	else {
		AssertFatal(0,"MainWindow:: Could not create main window");
		exit(1);
	}

	// Load palette
	GFXPalette* palette = new GFXPalette();
	if (!palette->read("palette.ppl")) {
		AssertFatal(0,"Error loading palette");
	   return;
	}
	setPalette(palette);

//	canvas = new DrawWindow;
//	canvas->setPalette(palette);
//	canvas->attach(this->getHandle());
//	canvas->create(this);
}

MainWindow::~MainWindow()
{
	delete gdlg;
//	delete canvas;
}

void MainWindow::onDestroy()
{
	PostQuitMessage(0);
}

void MainWindow::render()
{
	pSurface->clear(2);
	pSurface->drawLine2d(&Point2(0,0),&Point2(300,300),7);
}

void MainWindow::onMouseMove(int x, int y, UINT)
{
	lock();
	pSurface->clear(1);
	pSurface->drawLine2d(&Point2(0,0),&Point2(x,y),6);
	unlock();
	flip();
}

void MainWindow::onChar(TCHAR ch, int repeat)
{
	switch (ch) {
		case 'd':
			if (gdlg) {
				if (!gdlg->getHandle())
					// Window was deleted
					gdlg->create(this);
				else
					gdlg->show(SW_RESTORE);
			}
			else
				gdlg = new GridDialog(this);
			break;
		case 'k':
			delete gdlg;
			gdlg = 0;
			break;
		case 'w': {
			SetWindowPos(getHandle(),0,0,0,640,480,SWP_NOZORDER);
			setMode(GFX_SM_MODE_DIRECT);
//			canvas->setMode(GFX_SM_MODE_DIRECT);
			break;
		}
		case 'W':
			setMode(GFX_SM_MODE_WINDOW);
			//canvas->setMode(GFX_SM_MODE_WINDOW);
			SetWindowPos(getHandle(),0,0,0,640,480,SWP_NOZORDER);
			break;
	    default:
			Parent::onChar(ch,repeat);
	}
}



//----------------------------------------------------------------------------

class CMain: public GWMain
{
	HINSTANCE bcclib;
	MainWindow* window;

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
	FreeLibrary(bcclib);
	delete window;
}

bool CMain::initInstance()
{
	// Load borland custom control lib
	bcclib = LoadLibrary("bwcc32.dll");
	AssertFatal(bcclib != 0,"WinMain:: Could not load Borland dll");
	BWCCRegister(hInstance);

	//
	window = new MainWindow();
	return true;
}


