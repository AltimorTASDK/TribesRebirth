#ifndef _GWTOOLWIN_H_
#define _GWTOOLWIN_H_

#include "gw.h"


//-----------------------------------------------------------

class GWToolWin : public GWWindow
{
	typedef GWWindow Parent;
	HWND hToolBar;
   HWND hStatusBar;
   GWWindow* gwMessWnd; 

public:
   GWToolWin();
	~GWToolWin();

	//override from gwBase
	void destroyWindow();


	//onWindows Members
  //for menu selection & button click
   virtual void onCommand(int id, HWND hwndCtl, UINT codeNotify);
   //for pop-up help on toolbar
   virtual int onNotify(int id, LPNMHDR pnmhdr);                       


	//gwToolWin members
   //creates window 
   bool createWin(GWWindow *parent, LPCSTR name, LPRECT r, DWORD exStyle = WS_EX_PALETTEWINDOW);
   //set title of window
   bool windowTitle(LPCSTR title);                                      

   //set window to which onParentNotify & onCommand messages are forwarded to
   bool setMessageWin(GWWindow* gwwindow);            
                                                                        
                                                                        
   GWWindow* getMessageWin() {return ( gwMessWnd );}

   //members relating to the toolbar
   HWND getToolBarHandle() {return ( hToolBar );}
   //creates toolbar in window
   bool createToolBar(int tbarID, TBBUTTON *tbButtons, int numButtons, HBITMAP hbm );
   bool destroyToolBar();

   bool enableButton(int buttonID, bool enable);
   bool hideButton(int buttonID, bool hide);
   bool insertButton(int buttonIndex, LPTBBUTTON tbButton);
   bool deleteButton(int buttonIndex);

   //members relating to the status bar
   HWND getStatusBarHandle() {return ( hStatusBar );}
   //creates status bar in window
   bool createStatusBar(int sbarID, int numParts, LPINT rightEdges);   
   bool destroyStatusBar();

   bool setStatusText(int section, LPCSTR text);
   //length of text in a given section
   int getStatusTextLen(int section);                                   
   //the current text of a section (returns len)
   int getStatusText(int section, char *buffer);                        

  //set the # of sections & their widths
   bool setStatusParts(int numParts, LPINT rightEdges);                 
   //gets parts up to max of numParts into array returns #parts
   int getStatusParts(int maxParts, LPINT rightEdges);                  
                                                               

};

#endif //_GWTOOLWIN_H_

