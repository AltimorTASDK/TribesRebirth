// Must include after sim stuff
//----------------------------------------------------------------------------

#ifndef _TOOLPLUGIN_H_
#define _TOOLPLUGIN_H_

#include <sim.h>

#include <TString.h>
#include <simtoolwin.h>
#include <commdlg.h>
#include <gw.h>


//----------------------------------------------------------------------------
// defines for the toolPlugin.res file
#define TDBOX_Q              102
#define TDEDIT_EDIT          110
#define TDLIST_LIST          103

#define TDEDIT2_EDIT2        111
#define TDEDIT2_Q2           105
#define TDEDIT2_Q3           106 

#define TDBROWSE             555

#define EDITBOXNAME          "IDD_EDITBOX"
#define EDIT2BOXNAME         "IDD_EDIT2BOX"
#define BROWSENAME           "IDD_BROWSE"


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

class ToolPlugin: public SimConsolePlugin, TBarCallback
{
   private:
      class BaseBox;
      class EditBox;
      class Edit2Box;
      class BrowseBox;


   enum CallbackID
   {
//---------------------creation/destruction
      newToolWindow,
      newToolStrip,

//---------------------data
	  listToolButtons,
	  listToolWindows,

//---------------------hide/show
      hideToolWin,
      showToolWin,
      showToolWinAll,
      hideToolWinAll,
      setToolWinPos,

//---------------------buttons
      addToolButton,
      delToolButton,
	   addToolGap,
	   setToolCommand,
      setButtonHelp,
      isButtonDown,

//---------------------status bar
      addStatusBar,
      delStatusBar,
      setStatusField,
      getStatusField,
      clearStatusField,

//--------------------- dialog box and list box commands
      setMainWindow,
      editVar,
      edit2Vars,
      confirmBox,

      openFile,
      saveFileAs,
      browseBox,
   };

   static GWCanvas *mainCanvas;

   //general fns
   SimToolWindow* WindowCheck(String name, const char *fnname);
   bool ButtonCheck(String winname, String name, const char *fnname);
   bool ToolPlugin::Checkparams(bool valid, const char *fnname, 
         const char* correct);
   bool ShowToolWindowAll(int CmdShow);
   bool checkMainCanvas(const char *fnname);
   
   //handlers
   bool FnewToolWindow(int argc, const char *argv[]) ;
   bool FnewToolStrip(int argc, const char *argv[]) ;
   bool FlistToolButtons(int argc, const char *argv[]) ;
   bool FlistToolWindows(int argc, const char *argv[]);
   bool FaddToolButton(int argc, const char *argv[]) ;
   bool FdelToolButton(int argc, const char *argv[]);
   bool FaddToolGap(int argc, const char *argv[]);
   bool FsetToolCommand(int argc, const char *argv[]);
   bool FsetButtonHelp(int argc, const char *argv[]);
   bool FisButtonDown(int argc, const char *argv[]);
   bool FhideToolWin(int argc, const char *argv[]);
   bool FshowToolWin(int argc, const char *argv[]);
   bool FhideToolWinAll(int argc, const char *argv[]);
   bool FshowToolWinAll(int argc, const char *argv[]);
   bool FsetToolWinPos(int argc, const char *argv[]);

   bool FaddStatusBar(int argc, const char *argv[]);
   bool FdelStatusBar(int argc, const char *argv[]);
   bool FsetStatusField(int argc, const char *argv[]);
   bool FgetStatusField(int argc, const char *argv[]);
   bool FclearStatusField(int argc, const char *argv[]);

   bool FsetMainWindow(int argc, const char *argv[]);
   bool FeditVar(int argc, const char *argv[]);
   bool Fedit2Vars(int argc, const char *argv[]);
   bool FconfirmBox(int argc, const char *argv[]);
   bool FbrowseBox(int argc, const char *argv[]);
   
   bool FopenOrSave(int argc, const char *argv[], 
      const char *fnname, bool Open);
   bool FopenFile(int argc, const char *argv[]);
   bool FsaveFileAs(int argc, const char *argv[]);


   public:
	  ToolPlugin() {};
	  ~ToolPlugin();

      class ListBox;
   
      static bool resultCheck(const char *result);
      static GWWindow* getMainWindow();

      static void setupOFN(OPENFILENAME& ofn, int fNameLen,char *filenamebuf, 
         char *filterbuf, HWND owner);

      void init();
      void startFrame();
      void endFrame();
      const char *consoleCallback (CMDConsole*, int id, int argc, const char *argv[]);
	   void tbarCallback(int id, HWND hwndCtl, UINT codeNotify, const char *ConCommand);

};

//----------------------------------------------------------------------------
//base dialog box which allows the setting of window title and one question
class ToolPlugin::BaseBox : public GWDialog
{
   protected:
      char *dboxname;
      char *title;
      char *question;

   public:
      char *result;

      BaseBox() {result=NULL;};
      ~BaseBox() {delete [] result;};
      void setTexts(const char *_title, const char *_question);
      void showIt(GWWindow *parent);
      LRESULT windowProc(UINT message, WPARAM wParam, LPARAM lParam);
};

//----------------------------------------------------------------------------
//edit box for editing a console variable
class ToolPlugin::EditBox : public ToolPlugin::BaseBox
{
   protected:
      char *initial;
   public:
      EditBox() {dboxname = EDITBOXNAME;};
      void setTexts(const char *_title, const char *_question,
            const char *_initialval);
      LRESULT windowProc(UINT message, WPARAM wParam, LPARAM lParam);
};


//----------------------------------------------------------------------------
//edit box for editing a console variable with a browse button
class ToolPlugin::BrowseBox : public ToolPlugin::EditBox
{
   protected:
      char *initial;
      char **filter;
   public: 
      BrowseBox() {dboxname = BROWSENAME;};
      void setFilter(char **_filter) {filter=_filter;};
      LRESULT windowProc(UINT message, WPARAM wParam, LPARAM lParam);
};   


//----------------------------------------------------------------------------
//edit2 box for editing 2 console vars at once
class ToolPlugin::Edit2Box :public ToolPlugin::EditBox
{
   protected:
      char *initial2;
      char *qval1;
      char *qval2;

   public:
      char *result2;
      
      Edit2Box() {dboxname = EDIT2BOXNAME;};
      void setTexts(const char *_title, const char *_question,
            const char *_initialval1, const char *_initialval2,
            const char *_qforval1, const char *_qforval2);
      LRESULT windowProc(UINT message, WPARAM wParam, LPARAM lParam);
};   

//----------------------------------------------------------------------------
class ToolPlugin::ListBox : public BaseBox
{
   typedef Vector<const char *> Items;
   Items items;
   bool isMulti;
     
   public:
      int intResult;

      ListBox(bool multi);
      void addItem(const char *item);
      LRESULT windowProc(UINT message, WPARAM wParam, LPARAM lParam);
};



#endif //_EDITPLUGIN_H_  