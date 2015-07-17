//----------------------------------------------------------------------------

#ifndef _SIMTOOLWIN_H_
#define _SIMTOOLWIN_H_

#include <sim.h>
#include <gw.h>
#include <TString.h>
#include <talgorithm.h>
#include <tmap.h>

//TODO use loadable bitmap
const int IDB_BITMAP1 = 1010;

#define DEFAULTWINPOS 50;

//----------------------------------------------------------------------------
class TBarCallback {
public:
	virtual void tbarCallback(int id, HWND hwndCtl, UINT codeNotify, 
	   const char *ConCommand) = 0;
};



//----------------------------------------------------------------------------

class SimToolSet: public SimSet
{
public:
	SimToolSet() { id = SimToolSetId; }
};



//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class SimToolWindow: public GWToolWin, public SimObject
{
   typedef SimObject Parent;
   
public:
	enum WindowType {
		Horiz = 1,
		Vert =  2,
	};

   struct PartInfo {
      String VarName;
      String Text;
   };

	struct TWButton {
		int BtnIndex;
		int BmpIndex;
		String ConCommand;
      String ConCommandUp;
		String Help;
	};

private:
	static char *tempstr;
	typedef Map<String,TWButton> BtnList;
	BtnList btnList;

   HBITMAP bitmap;      //bitmap associated with toolbar
   
   //for var names associated with status bar
   typedef Map<int, PartInfo> SbarVarNames;
   SbarVarNames sbarVarNames;

	int currentButton;
   int MinHeight;
   int MinWidth;
	TBarCallback *handler;

	bool ToolBarSize(RECT *r);
	bool RefitWindow();

public:
	int type;

    SimToolWindow();
    ~SimToolWindow();

   void SetHandler(TBarCallback *hndlr) {handler = hndlr;}

   //for menu selection & button click
	void onCommand(int id, HWND hwndCtl, UINT codeNotify);
   //for pop-up help on toolbar
   int onNotify(int id, LPNMHDR pnmhdr);                       


	bool NewWindow(GWWindow *parent, String name, DWORD winStyle = WS_EX_TOOLWINDOW);
   bool CloseWindow();   
   bool PositionWindow(int x, int y);

   bool AddButton(String name, GFXBitmap *pBM,  bool wrapAfter=false,
      bool toggleStyle=false, DWORD bstyle=TBSTYLE_BUTTON);
	bool AddGap(String name);
	bool DeleteButton(String name);

	bool SetButtonConCommand(String name, String conCommand,
         String conCommandUp);

   bool  SetButtonHelp(String name, String help);
	bool  FindButton(String name);
	char* GetBtnInfo();
   bool  isButtonDown(String name);

   bool AddStatus(int NumWidths, LPINT Widths);
   bool DelStatus();
   bool AttachStatVar(int PartNo, String VarName, String PreText);
   bool ClearStatVar(int PartNo);
   int  GetStatVarCount();
   const char* GetStatVar(int PartNo);
   const char* GetStatText(int PartNo);


   //------------------------------------------
	bool onAdd();
};


#endif //_SIMTOOLWIN_H_   

