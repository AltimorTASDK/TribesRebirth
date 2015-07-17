//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <sim.h>
#include "simCanvas.h"
#include "simResource.h"
//----------------------------------------------------------------------------

#define HANDLE_CON_CMD(cmd, errMsg) \
   case (cmd): if (!F##cmd(argc, argv)) \
   console->printf(ETERROR errMsg); break;

//----------------------------------------------------------------------------

#include "simConsolePlugin.h"
#include "toolPlugin.h"

#define ETERROR "ToolPlugin Error: " 

#define RESULTVAR "dlgResult"
#define CANCEL_STR "[cancel]"
#define OK_STR "[ok]"
#define YES_STR "[yes]"
#define NO_STR "[no]"

const int MAXEDGES = 20;
const int MAXLINE = 1024;
const char grpname[] = "ToolSet";

const char *tempfilter[] = {"Material Lists", "*.dml", NULL, NULL,};

//----------------------------------------------------------------------------

GWCanvas *ToolPlugin::mainCanvas = NULL;

//----------------------------------------------------------------------------

GWWindow* ToolPlugin::getMainWindow()
{
   CMDConsole *con = CMDConsole::getLocked();
   if ( con )
   {
      if ( !mainCanvas )
         con->printf("  must set main main window first!");
   }

   return ( (GWWindow*)mainCanvas );
}

//----------------------------------------------------------------------------

void ToolPlugin::init()
{
   console->printf("ToolPlugin");
   //-------------------- add the commands
   console->addCommand(newToolWindow,     "newToolWindow",     this);
   console->addCommand(newToolStrip,      "newToolStrip",      this);
   console->addCommand(listToolButtons,   "listToolButtons",   this);   
   console->addCommand(listToolWindows,   "listToolWindows",   this);

   console->addCommand(hideToolWin,       "hideToolWin",       this);
   console->addCommand(showToolWin,       "showToolWin",       this);
   console->addCommand(showToolWinAll,    "showToolWinAll",    this);
   console->addCommand(hideToolWinAll,    "hideToolWinAll",    this);
   console->addCommand(setToolWinPos,     "setToolWinPos",     this);
   
   console->addCommand(addToolButton,     "addToolButton",     this);
   console->addCommand(delToolButton,     "delToolButton",     this);
   console->addCommand(addToolGap,        "addToolGap",        this);
   console->addCommand(setToolCommand,    "setToolCommand",    this); 
   console->addCommand(setButtonHelp,     "setButtonHelp",     this);
   console->addCommand(isButtonDown,      "isButtonDown",      this);

   console->addCommand(addStatusBar,      "addStatusBar",      this);
   console->addCommand(delStatusBar,      "delStatusBar",      this);
   console->addCommand(setStatusField,    "setStatusField",    this);
   console->addCommand(getStatusField,    "getStatusField",    this);
   console->addCommand(clearStatusField,  "clearStatusField",  this);

   console->addCommand(setMainWindow,     "setMainWindow",     this);
   console->addCommand(editVar,           "editBox",           this);
   console->addCommand(browseBox,         "browseBox",         this);
   console->addCommand(edit2Vars,         "edit2Box",          this);
   console->addCommand(confirmBox,        "confirmBox",        this);
   console->addCommand(openFile,          "openFile",          this);
   console->addCommand(saveFileAs,        "saveFileAs",        this);
}

//----------------------------------------------------------------------------

void ToolPlugin::startFrame()
{
   
}

//----------------------------------------------------------------------------

void ToolPlugin::endFrame()
{
    if( !manager )
        return;
   //-----------After each frame, update all the status bars
   char Line[MAXLINE];
   SimToolSet* set;
   if ( (set = dynamic_cast<SimToolSet*>
      (manager->findObject(SimToolSetId))) != NULL )
      {
         SimToolSet::iterator i;
         for (i = set->begin(); i != set->end(); i++)
         {
            SimToolWindow *tw = dynamic_cast<SimToolWindow*>(*i);
            if ( !tw )
               continue;
            if ( tw->getStatusBarHandle() && 
                  ::IsWindow(tw->getStatusBarHandle()) )
            {
               int count = tw->GetStatVarCount();
               for (int j=0; j< count; j++)
               {
                  Line[0] = '\0';
                  const char *vartext = tw->GetStatText(j);
                  const char *varname = tw->GetStatVar(j);

                  if (vartext)
                  {
                     strcpy(Line, vartext);
                     strcat(Line, ": ");
                  }

                  if ( varname )
                  {
                     const char *varval= console->getVariable(varname);
                     if (varval) 
                        strcat(Line, varval);
                  }
                  
                  tw->setStatusText(j, Line);
               }
            }
         }
      }

}

//----------------------------------------------------------------------------

ToolPlugin::~ToolPlugin()
{
}

//----------------------------------------------------------------------------

const char *ToolPlugin::consoleCallback(CMDConsole*, int id, 
            int argc, const char *argv[])
{
   switch ( id )
   {
     HANDLE_CON_CMD(newToolWindow, "cannot create window.");
     HANDLE_CON_CMD(newToolStrip, "cannot create tool strp.");
     HANDLE_CON_CMD(listToolButtons, "cannot list buttons.");
     HANDLE_CON_CMD(listToolWindows, "cannot list windows.");
     HANDLE_CON_CMD(addToolButton, "cannot add button.");
     HANDLE_CON_CMD(delToolButton, "cannot delete button.");
     HANDLE_CON_CMD(addToolGap, "cannot add a gap.");
     HANDLE_CON_CMD(setToolCommand, "cannot set that command.");
     HANDLE_CON_CMD(setButtonHelp, "cannot set the help.");
     HANDLE_CON_CMD(isButtonDown, "cannot determine that.");
     HANDLE_CON_CMD(showToolWin, "cannot show the window.");
     HANDLE_CON_CMD(hideToolWin, "cannot hide the window.");
     HANDLE_CON_CMD(hideToolWinAll, "cannot hide the windows.");
     HANDLE_CON_CMD(showToolWinAll, "cannot show the windows.");
     HANDLE_CON_CMD(setToolWinPos, "cannot position the window.");
     HANDLE_CON_CMD(addStatusBar, "cannot add the status bar.");
     HANDLE_CON_CMD(delStatusBar, "cannot delete the status bar.");
     HANDLE_CON_CMD(setStatusField, "cannot set the status bar field.");
     HANDLE_CON_CMD(getStatusField, "cannot get the status bar field.");
     HANDLE_CON_CMD(clearStatusField, "cannot clear the status bar field.");

     HANDLE_CON_CMD(setMainWindow, "cannot set the main window.");
     HANDLE_CON_CMD(editVar, "cannot edit the console variable.");
     HANDLE_CON_CMD(browseBox, "cannot edit the variable.");
     HANDLE_CON_CMD(edit2Vars, "cannot edit the console variables.");
     HANDLE_CON_CMD(confirmBox, "cannot display the dialog.");
     HANDLE_CON_CMD(openFile, "cannot display the dialog.");
     HANDLE_CON_CMD(saveFileAs, "cannot display the dialog.");
   }
   return ( 0 );
}

//----------------------------------------------------------------------------
// callback when a toolbar button is clicked
// Toolbar commands are strings to be evaluated by the console. A backslash or
// newline character separate multiple commands in a single string
void ToolPlugin::tbarCallback(int id, HWND hwndCtl, UINT codeNotify, 
         const char *ConCommand)
{
   //for now we'll take anything with a valid conCommand, so ignore some params
   // which may be used later
   if ( (id || !id) && (hwndCtl || !hwndCtl) && (codeNotify || !codeNotify))
      if ( ConCommand && (strlen(ConCommand) > 0) )
      {
         //allow multiline commands by tokenizing string
         char seps[]   = "\n\\";
         char *token;

         char *concmd = new char[strlen(ConCommand)+1];
         if ( !concmd )           return;
         strcpy(concmd, ConCommand);

         token = strtok( concmd, seps );
         while( token != NULL )
         {
            console->evaluate( token, false );
            token = strtok( NULL, seps );
         }
         delete [] concmd;
      }
}

//----------------------------------------------------------------------------
// checks that the main window was set
bool ToolPlugin::checkMainCanvas(const char *fnname)
{
   if ( !mainCanvas )
   {
      console->printf("%s%s" , fnname, "must set main main window first!");
      return ( false );
   }
   return ( true );
}

//----------------------------------------------------------------------------
//sets resultVar accordingly. returns false if a cancel
bool ToolPlugin::resultCheck(const char *result)
{
   CMDConsole *con = CMDConsole::getLocked();
   if ( !result )
   {
      con->printf("Dialog box problem.");
      return ( false );
   }

   if ( !strcmp(result, CANCEL_STR) )
   {
      con->setVariable(RESULTVAR, CANCEL_STR);
      return ( false );
   }
   else
      con->setVariable(RESULTVAR, OK_STR);

   return ( true );
}

//----------------------------------------------------------------------------
// checks for the correct # of params
bool ToolPlugin::Checkparams(bool valid, const char *fnname, 
         const char* correct)
{
   //check # of params
   if (!valid)
         console->printf("%s%s", fnname, correct);

   return (valid);
}

//----------------------------------------------------------------------------
//returns ptr to toolwindow  if window name found 
SimToolWindow* ToolPlugin::WindowCheck(String name, const char *fnname)
{
   //check if window exists
   SimToolWindow* obj = dynamic_cast<SimToolWindow*>
         (manager->findObject(name.c_str()));
   
   if ( !obj )
      console->printf("%s%s", fnname, "Window name not found");

   return ( obj );
}


//----------------------------------------------------------------------------
//returns true if the given button for the given entry 
bool ToolPlugin::ButtonCheck(String winname, String name, const char *fnname)
{
   //check if window exists
   SimToolWindow* tw = WindowCheck(winname, fnname);
   if (!tw)
      return (false);

   //check if the button exists
   if ( !tw->FindButton(name) )
   {
      console->printf("%s%s", fnname, "Button name not found in that window");
      return (false);
   }
   return (true);
}


//----------------------------------------------------------------------------
bool ToolPlugin::ShowToolWindowAll(int CmdShow)
{
   if ( !((CmdShow == SW_HIDE) || (CmdShow == SW_SHOW)) )
      return (false);

   SimToolSet* set;
   if ( (set = dynamic_cast<SimToolSet*>
      (manager->findObject(SimToolSetId))) != NULL )
      {
         SimToolSet::iterator i;
         for (i = set->begin(); i != set->end(); i++)
         {
            SimToolWindow *tw = dynamic_cast<SimToolWindow*>(*i);
            if ( !tw )
               continue;

            HWND hwnd = tw->getHandle();

            if (!IsWindow(hwnd))
               continue;

            ::ShowWindow( hwnd, CmdShow);
         }
         return ( true );
      }

   return (false);

}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Callback function handlers. Return true if either error handled or fn
//  successful
// TODO: move argument parsing up to case statement using scanf so these 
//  routines take params instead of arc & argv
//----------------------------------------------------------------------------
bool ToolPlugin::FnewToolWindow (int argc, const char *argv[])
{
   GWWindow *parent;
   const char fnname[] = "newToolWindow: ";

   //check # of params
   if ( !Checkparams( (argc>1 && argc<5), fnname, "WindowName ParentCanvas [floating?]"))
      return (true);

   //check if name used
   if ( dynamic_cast<SimToolWindow*>(manager->findObject(argv[1])) )
   {
      console->printf("%s%s%s", fnname, "Window name already used",argv[1]);
      return (true);
   }

   SimCanvas *sc = dynamic_cast<SimCanvas*>(manager->findObject(argv[2]));
   if ( !sc )
   {
      console->printf("%s%s%s", fnname, "Cannot find parent window",argv[2]);
      return (true);
   }
   parent = sc;

   SimToolWindow* wt = new SimToolWindow;
   if (!wt)
      return (false);

   bool b;
   if ( argc > 3 )
      b= wt->NewWindow(parent, argv[1], WS_EX_PALETTEWINDOW);
   else
      b= wt->NewWindow(parent, argv[1]);

   if ( !b )
   {
      delete wt;
       return (false);
   }

   
   wt->type = SimToolWindow::Horiz;

   //add to manager
   manager->addObject((SimObject*)wt);
   manager->assignName((SimObject*)wt, argv[1]);

   //set handler so we get callbacks on button clicks
   wt->SetHandler(this);
   return (true);
}

//----------------------------------------------------------------------------
bool ToolPlugin::FnewToolStrip (int argc, const char *argv[])
{
   GWWindow *parent;
   const char fnname[] = "newToolStrip: ";

   //check # of params
   if ( !Checkparams((argc>2 && argc<6), fnname, "WindowName ParentWindow bitmapFile [floating?]"))
      return (true);

   //check if name used
   if (    dynamic_cast<SimToolWindow*>(manager->findObject(argv[1])) )
   {
      console->printf("%s%s", fnname, "Window name already used");
      return (true);
   }

   SimCanvas *sc = dynamic_cast<SimCanvas*>(manager->findObject(argv[2]));
   if ( !sc )
   {
      console->printf("%s%s%s", fnname, "Cannot find parent window",argv[2]);
      return (true);
   }
   parent = sc;

   SimToolWindow* wt = new SimToolWindow;
   if (!wt)
      return (false);


   bool b;
   if (  argc > 3 )
      b= wt->NewWindow(parent, argv[1], WS_EX_PALETTEWINDOW);
   else
      b= wt->NewWindow(parent, argv[1]);

   if ( !b )
   {
      delete wt;
       return (false);
   }
   
   wt->type = SimToolWindow::Vert;

   //add to map
   manager->addObject((SimObject*)wt);
   manager->assignName((SimObject*)wt, argv[1]);

   //set handler so we get callbacks on button clicks
   wt->SetHandler(this);
   return (true);
}

//----------------------------------------------------------------------------
bool ToolPlugin::FlistToolButtons(int argc, const char *argv[]) 
{
   const char fnname[] = "listToolButtons: ";

   //check # of params
   if ( !Checkparams(argc==2, fnname, "WindowName"))
      return (true);

   //check if window exists
   SimToolWindow *tw= WindowCheck(argv[1], fnname);
   if (!tw)
      return (true);

   char *btninfo = tw->GetBtnInfo();
   if (!btninfo)
      return (false);

   //now print out token-seperated sections of string
   char seps[]   = "\n";
   char *token;

   token = strtok( btninfo, seps );
   while( token != NULL )
   {
      console->printf( " %s", token );
      token = strtok( NULL, seps );
   }

   return (true);
}

//----------------------------------------------------------------------------
bool ToolPlugin::FlistToolWindows(int argc, const char *argv[]) 
{
   argc, argv;
   const char fnname[] = "listToolWindows: ";


   SimToolSet* set = dynamic_cast<SimToolSet*>
      (manager->findObject(SimToolSetId));

   if ( !set )
      return ( false );

   if ( set->begin() == set->end())
   {
      console->printf("%s%s", fnname, "There are no tool windows or strips.");
      return (true);
   }

   //now print the window names
   for (SimToolSet::iterator i = set->begin();i != set->end(); i++)
   {
      char cmd[10] = "Hidden";

      SimToolWindow *tw = dynamic_cast<SimToolWindow*>(*i);
      if ( tw != NULL)
      {
         HWND hwnd = tw->getHandle();
         if ( (hwnd != NULL) && (::IsWindow(hwnd)) )
         {
            if (!::ShowWindow(hwnd, SW_SHOWNA))
               ::ShowWindow(hwnd, SW_HIDE);
            else
              strcpy(cmd, "Visible");

            int l = ::GetWindowTextLength(hwnd);
            char *txt = new char[l+2];
            if ( !txt ) return ( false );
            *txt= '\0';
            ::GetWindowText(hwnd, txt, l+1);


            console->printf("   %s: %6s (%s)",  
               txt, 
               tw->type==SimToolWindow::Horiz ? "Window" : "Strip",
               cmd);

            delete [] txt;
            
         }
      }

   }


   return (true);
}


//----------------------------------------------------------------------------
bool ToolPlugin::FaddToolButton (int argc, const char *argv[])
{
   
   const char fnname[] = "addToolButton: ";

   //check # of params
   if ( !Checkparams( (argc >=4) && (argc <=6), fnname, "WindowName ButtonName"
      " BitmapFile [wrapAfter] [toggleButton]"))
      return (true);
   
   //check if window exists
   SimToolWindow* tw = WindowCheck(argv[1], fnname);
   if (!tw)
      return (true);

   //check if button already exists
   if ( tw->FindButton(argv[2]))
   {
      console->printf("%s%s", fnname, "Button name already used.");
      return (true);
   }

   bool wrapAfter = ( (argc >=5) && (atoi(argv[4])) );
   bool toggleStyle = ( (argc >=6) && (atoi(argv[5])) );

   ResourceManager *rm = SimResource::get(manager);
   Resource <GFXBitmap> pBM = rm->load( argv[3]);
   if ( !(bool)pBM )
      console->printf("%s%s%s", fnname, "Could not load bitmap: ", argv[3] );

   //add button to window
   if ( !tw->AddButton(argv[2], pBM, wrapAfter, toggleStyle) )
      return (false);

   return (true);
}

//----------------------------------------------------------------------------
bool ToolPlugin::FdelToolButton (int argc, const char *argv[])
{
   
   const char fnname[] = "delToolButton: ";

   //check # of params
   if ( !Checkparams(argc==3, fnname, "WindowName ButtonName"))
      return (true);
   
   //check if window exists
   SimToolWindow* tw = WindowCheck(argv[1], fnname);
   if (!tw)
      return (true);

   //check if button already exists
   if (!tw->FindButton(argv[2]))
   {
      console->printf("%s%s", fnname, "Button name not found in that window.");
      return (true);
   }


   //delete button from window
   if ( !tw->DeleteButton(argv[2]) )
      return (false);

   return (true);
}

//----------------------------------------------------------------------------
bool ToolPlugin::FaddToolGap (int argc, const char *argv[])
{
   
   const char fnname[] = "addToolGap: ";

   //check # of params
   if ( !Checkparams(argc==3, fnname, "WindowName GapName"))
      return (true);
   
   //check if window exists
   SimToolWindow* tw = WindowCheck(argv[1], fnname);
   if (!tw)
      return (true);

   //check window type
   if ( tw->type == SimToolWindow::Vert )
   {
      console->printf("%s%s", fnname, "Cannot add gap to tool strips.");
      return (true);
   }

   //check if button already exists
   if ( tw->FindButton(argv[2]))
   {
      console->printf("%s%s", fnname, "Name already used in that window.");
      return (true);
   }


   //add button to window
   if ( !tw->AddGap(argv[2]) )
      return (false);

   return (true);
}

//----------------------------------------------------------------------------
bool ToolPlugin::FsetToolCommand(int argc, const char *argv[])
{
   const char fnname[] = "setToolCommand: ";

   //check # of params
   if ( !Checkparams( (argc==4) || (argc==5), fnname, "WindowName ButtonName"
      " ConsoleCommand [ConsoleCommandUp]"))
      return (true);

   //check if window exists
   SimToolWindow* tw = WindowCheck(argv[1], fnname);
   if (!tw)
      return (true);

   //check if button exists
   if (!ButtonCheck(argv[1], argv[2], fnname))
      return (true);

   return tw->SetButtonConCommand(argv[2], argv[3], 
         argc==5 ? argv[4]:"");
}

//----------------------------------------------------------------------------
bool ToolPlugin::FsetButtonHelp(int argc, const char *argv[])
{
   const char fnname[] = "setButtonHelp: ";
   if ( !Checkparams(argc==4, fnname, "WindowName ButtonName HelpString"))
      return ( true );
   SimToolWindow *tw = WindowCheck(argv[1], fnname);
   if ( !tw )
      return ( true );
   if ( !ButtonCheck(argv[1], argv[2], fnname))
      return ( true );

   return ( tw->SetButtonHelp(argv[2], argv[3]));
}

//----------------------------------------------------------------------------
bool ToolPlugin::FisButtonDown(int argc, const char *argv[])
{
   const char fnname[] = "isButtonDown: ";
   if ( !Checkparams( (argc==3) || (argc==4), fnname, "WindowName"
         " ButtonName [consoleVar]"))
      return ( true );
   SimToolWindow *tw = WindowCheck(argv[1], fnname);
   if ( !tw )
      return ( true );
   if ( !ButtonCheck(argv[1], argv[2], fnname))
      return ( true );

   char answer[4] = "no";
   if ( tw->isButtonDown(argv[2]) )
      strcpy(answer, "yes");

   if ( argc==4 && argv[3] )
      console->setVariable(argv[3], answer);
   else
      console->printf("   %s", answer);

   return ( true );
}

//----------------------------------------------------------------------------
bool ToolPlugin::FhideToolWin(int argc, const char *argv[])
{
   const char fnname[] = "hideToolWin: ";

   //check # of params
   if ( !Checkparams(argc==2, fnname, "WindowName"))
      return (true);
   
   //check if window exists
   SimToolWindow* tw= WindowCheck(argv[1], fnname);
   if (!tw)
      return (true);

   HWND hwnd = tw->getHandle();

   if (!IsWindow(hwnd))
      return (false);

   ::ShowWindow( hwnd, SW_HIDE);
   return (true);
}

//----------------------------------------------------------------------------
bool ToolPlugin::FsetToolWinPos(int argc, const char *argv[])
{
   const char fnname[] = "hideToolWin: ";

   //check # of params
   if ( !Checkparams(argc==4, fnname, "WindowName x y"))
      return (true);
   
   //check if window exists
   SimToolWindow* tw = WindowCheck(argv[1], fnname);
   if (!tw)
      return (true);

   //position the window
   return tw->PositionWindow(atoi(argv[2]), atoi(argv[3]));
}

//----------------------------------------------------------------------------
bool ToolPlugin::FshowToolWin(int argc, const char *argv[])
{
   const char fnname[] = "showToolWin: ";

   //check # of params
   if ( !Checkparams(argc==2, fnname, "WindowName"))
      return (true);
   
   //check if window exists
   SimToolWindow* tw= WindowCheck(argv[1], fnname);
   if (!tw)
      return (true);

   HWND hwnd = tw->getHandle();

   if (!IsWindow(hwnd))
      return (false);

   ::ShowWindow( hwnd, SW_SHOW);
   return (true);
}

//----------------------------------------------------------------------------
bool ToolPlugin::FhideToolWinAll(int argc, const char *argv[])
{
   argc, argv;
   return ShowToolWindowAll(SW_HIDE);
}

//----------------------------------------------------------------------------
bool ToolPlugin::FshowToolWinAll(int argc, const char *argv[])
{
   argc, argv;
   return ShowToolWindowAll(SW_SHOW);
}

//----------------------------------------------------------------------------
bool ToolPlugin::FaddStatusBar(int argc, const char *argv[])
{
   const char fnname[] = "setStatusBar: ";

     //check # of params
   if ( !Checkparams(argc > 2, fnname, "WindowName "
            " width1, width2, ... widthn"))
      return (true);
   
     //check upper bound of params
   if ( !Checkparams(argc < MAXEDGES, fnname, 
            "Too many parts to fit on status bar"))
      return (true);

   //check if window exists
   SimToolWindow* tw = WindowCheck(argv[1], fnname);
   if (!tw)
      return (true);

     //check window type
   if ( tw->type == SimToolWindow::Vert )
   {
      console->printf("%s%s", fnname, 
               "Cannot add a status bar to tool strips.");
      return (true);
   }

   //build an array of right edges
   int Widths[MAXEDGES];
   for ( int i= 2; i < argc ;i++ )
   {
      int partwidth = atoi(argv[i]);

      //-1 is a valid width (== full width of statbar)
      if ( (partwidth <= 0) && (partwidth != -1) ) 
      {
         console->printf("%s%s", fnname, "Cannot have nil or "
                  "negative widths.");
         return ( true );
      }

      if ( partwidth >= ::GetSystemMetrics(SM_CXSCREEN) )
      {
         console->printf("%s%s", fnname, "Cannot have a status bar "
                     "wider than screen");
         return ( true );
      }

      Widths[i-2]= partwidth;
   }

   //now add the status bar to the window
   return ( tw->AddStatus(argc-2, Widths) );

}

//----------------------------------------------------------------------------
bool ToolPlugin::FdelStatusBar(int argc, const char *argv[])
{
   const char fnname[] = "delStatusBar: ";

   //check # of params
   if ( !Checkparams(argc==2, fnname, "WindowName"))
      return (true);
   
   //check if window exists
   SimToolWindow* tw = WindowCheck(argv[1], fnname);
   if (!tw)
      return (true);

   return ( tw->DelStatus() );
}

//----------------------------------------------------------------------------
bool ToolPlugin::FsetStatusField(int argc, const char *argv[])
{
   const char fnname[] = "setStatusPart: ";

   //check # of params
   if ( !Checkparams(argc==5, fnname, "WindowName StatusPart VarName Text"))
      return (true);
   
   //check if window exists
   SimToolWindow* tw = WindowCheck(argv[1], fnname);
   if (!tw)
      return (true);

     //check window type
   if ( tw->type == SimToolWindow::Vert )
   {
      console->printf("%s%s", fnname, "Strips don't have status bars.");
      return (true);
   }
  
   if ( atoi(argv[2]) < 0 )
   {
      console->printf("%s%s", fnname, "Cannot have negative part #'s");
      return ( true );
   }
    
   return ( tw->AttachStatVar(atoi(argv[2]), argv[3], argv[4] ));
}

//----------------------------------------------------------------------------
bool ToolPlugin::FgetStatusField(int argc, const char *argv[])
{
   const char fnname[] = "getStatusPart: ";

   //check # of params
   if ( !Checkparams(argc==3, fnname, "WindowName StatusPart"))
      return (true);
   
   //check if window exists
   SimToolWindow* tw= WindowCheck(argv[1], fnname);
   if (!tw)
      return (true);

     //check window type
   if ( tw->type == SimToolWindow::Vert )
   {
      console->printf("%s%s", fnname, "Strips don't have status bars.");
      return (true);
   }

   if ( atoi(argv[2]) < 0 )
   {
      console->printf("%s%s", fnname, "Cannot have negative part #'s");
      return ( true );
   }

   const char *varname=tw->GetStatVar(atoi(argv[2]));
   const char *vartext=tw->GetStatText(atoi(argv[2]));

   if (varname || vartext)
   console->printf("%s var: %s; text:%s", fnname,
      varname ? varname : "",
       vartext ? vartext : "");

   return ( true );
}

//----------------------------------------------------------------------------
bool ToolPlugin::FclearStatusField(int argc, const char *argv[])
{
   const char fnname[] = "clearStatusPart: ";

   //check # of params
   if ( !Checkparams(argc==3, fnname, "WindowName StatusPart"))
      return (true);
   
   //check if window exists
   SimToolWindow* tw = WindowCheck(argv[1], fnname);
   if (!tw)
      return (true);

     //check window type
   if ( tw->type == SimToolWindow::Vert )
   {
      console->printf("%s%s", fnname, "Strips don't have status bars.");
      return (true);
   }
  
   if ( atoi(argv[2]) < 0 )
   {
      console->printf("%s%s", fnname, "Cannot have negative part #'s");
      return ( true );
   }
    
   return ( tw->ClearStatVar(atoi(argv[2])) );
}

//----------------------------------------------------------------------------
bool ToolPlugin::FsetMainWindow(int argc, const char *argv[])
{
   const char fnname[] = "setMainWindow: ";
   if (!Checkparams(argc==2, fnname, "canvasName"))
      return ( true );

   //find the specified canvas
   SimCanvas *canvas = dynamic_cast<SimCanvas*>
         (manager->findObject(argv[1]));

   if ( !canvas )      return ( false );

   mainCanvas = canvas;
   return ( true );
}   

//----------------------------------------------------------------------------
bool ToolPlugin::FeditVar(int argc, const char *argv[])
{
   const char fnname[] = "editBox: ";
   if ( !Checkparams(argc==4, fnname, "Title Question conVar:ToEdit"))
      return ( true );
   EditBox ebox;
   const char *ival = console->getVariable(argv[3]);
   ebox.setTexts(argv[1], argv[2], ival);
   
   if ( !checkMainCanvas(fnname) )  return ( true );
   ebox.showIt( (GWWindow*)mainCanvas );

   if ( resultCheck(ebox.result) )
   {
      console->setVariable(RESULTVAR, OK_STR);
      console->setVariable(argv[3], ebox.result);
   }
   return ( true );
}

//----------------------------------------------------------------------------
bool ToolPlugin::FbrowseBox(int argc, const char *argv[])
{
   const char fnname[] = "browseBox";
   if ( !Checkparams( ((argc>=6)&& !(argc & 0x1)), fnname, "title question"
     " conVar::Result filterName filter [filterName filter...]") )
            return ( true );

   if ( !checkMainCanvas(fnname) )  return ( true );

   char **filter = new char*[1+argc-1];
   if ( !filter )    return ( false );
   memset(filter, sizeof(char*), 1+argc-1);

   for ( int i=4; i< argc; i++ )
      filter[i-4] = (char *)argv[i];
   filter[argc-4]=0;

   BrowseBox ebox;
   const char *ival = console->getVariable(argv[3]);
   ebox.setTexts(argv[1], argv[2], ival);
   ebox.setFilter(filter);
   
   ebox.showIt( (GWWindow*)mainCanvas );
   if ( resultCheck(ebox.result) )
   {
      console->setVariable(RESULTVAR, OK_STR);
      console->setVariable(argv[3], ebox.result);
   }
   return ( true );
   
}  

//----------------------------------------------------------------------------
bool ToolPlugin::Fedit2Vars(int argc, const char *argv[])
{
   const char fnname[] = "edit2Box: ";
   if ( !Checkparams(argc==7, fnname, "Title Question"
    " QforVal1 QforVal2 VarToEdit1 VarToEdit2" ))
      return ( true );

   Edit2Box ebox;
   const char *ival = console->getVariable(argv[5]);
   const char *ival2 = console->getVariable(argv[6]);
   ebox.setTexts(argv[1], argv[2], ival, ival2, argv[3], argv[4]);
   if ( !checkMainCanvas(fnname) )  return ( true );

   ebox.showIt( (GWWindow*)mainCanvas );
   if ( resultCheck(ebox.result) )
   {
      console->setVariable(argv[5], ebox.result);
      console->setVariable(argv[6], ebox.result2);
   }
   return ( true ); 
}

//----------------------------------------------------------------------------
bool ToolPlugin::FconfirmBox(int argc, const char *argv[])
{
   const char fnname[] = "confirmBox: ";
   if ( !Checkparams(argc==3, fnname, "title question") )
      return ( true );

   //display the dialog
   if ( !checkMainCanvas(fnname) )  return ( true );
   int res = MessageBox(mainCanvas->getHandle(), argv[2], argv[1], MB_YESNOCANCEL);

   char result[] = CANCEL_STR;
   if ( res==IDNO )
      strcpy(result, NO_STR);
   else if ( res == IDYES )
      strcpy(result, YES_STR);
   
   console->setVariable(RESULTVAR, result);
   return ( true );
}

//----------------------------------------------------------------------------
// internal routine to setup an open file name structure
void ToolPlugin::setupOFN(OPENFILENAME& ofn, int fNameLen, 
      char *filenamebuf, char *filterbuf, HWND owner)
{
   memset(&ofn,0,sizeof(OPENFILENAME));
   memset(filenamebuf,0,fNameLen);

     ofn.lStructSize       = sizeof (OPENFILENAME) ;
     ofn.hwndOwner        = owner;       
     ofn.hInstance         = NULL ;
     ofn.lpstrFilter      = filterbuf;
     ofn.lpstrCustomFilter = NULL ;
     ofn.nMaxCustFilter    = 0 ;
     ofn.nFilterIndex      = 0 ;
     ofn.lpstrFile        = filenamebuf;
     ofn.nMaxFile         = fNameLen;
     ofn.lpstrFileTitle    = NULL ;          // Set in Open and Close functions
     ofn.nMaxFileTitle     = _MAX_FNAME + _MAX_EXT ;
     ofn.lpstrInitialDir   = NULL ;
     ofn.lpstrTitle        = NULL ;
     ofn.Flags            = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST |
                           OFN_HIDEREADONLY | OFN_EXPLORER;// | OFN_NOCHANGEDIR;
     ofn.nFileOffset       = 0 ;
     ofn.nFileExtension    = 0 ;
     ofn.lpstrDefExt       = "txt" ;
     ofn.lCustData         = 0L ;
     ofn.lpfnHook          = NULL ;
     ofn.lpTemplateName    = NULL ;

//   ofn.nFilterIndex     = 1;
}

//----------------------------------------------------------------------------
bool ToolPlugin::FopenFile(int argc, const char *argv[])
{
   const char fnname[] = "openFile: ";
   return FopenOrSave(argc, argv, fnname, true);
}

//----------------------------------------------------------------------------
bool ToolPlugin::FsaveFileAs(int argc, const char *argv[])
{
   const char fnname[] = "saveFileAs: ";
   return FopenOrSave(argc, argv, fnname, false);
}

//----------------------------------------------------------------------------
bool ToolPlugin::FopenOrSave(int argc, const char *argv[], 
   const char *fnname, bool Open)
{
   if ( !Checkparams( ((argc>=4)&& !(argc & 0x1)), fnname, "conVar::Result "
     "filterName filter [filterName filter...]") )
            return ( true );

	int filterLen = 0;
	for( int ii = 2; ii < argc; ii++ )
		filterLen += strlen( argv[ii] );
	filterLen += argc; // add enough extra for null terminators required for filter
	                   // (one betwen each arg, and two at the end)

	char *filter = new char[filterLen]; 
	if( !filter )
		return( false );

	memset( filter, '\0', filterLen );
	char *ptr = &filter[0];
	for( int jj = 2; jj < argc; jj++ )
	{
		strcpy( ptr, argv[jj] );
		ptr += strlen( argv[jj] );
		*ptr++ = '\0'; // separate each element by NULL
	}
	*ptr = '\0'; // add second NULL for required double NULL termination

	
   static OPENFILENAME ofn;
   char filename[512];
   if ( !checkMainCanvas(fnname) )  return ( true );
   setupOFN(ofn, 511, filename, filter, mainCanvas->getHandle());

   // remember current working directory
   char cwd[512];
   GetCurrentDirectory(512,cwd);
   // change to open/save current working directory
   const char * openSaveDir = console->getVariable("Tool::openSaveDirectory");
   if (strlen(openSaveDir)!=0)
      SetCurrentDirectory(openSaveDir);

   bool res;
   if ( Open )
      res = GetOpenFileName(&ofn);
   else
      res = GetSaveFileName(&ofn);

   // remember open/save working directory
   char newOSD[512];
   GetCurrentDirectory(512,newOSD);
   console->setVariable("Tool::openSaveDirectory",newOSD);
   // back to original working directory
   SetCurrentDirectory(cwd);

   if ( res )
   {
      console->setVariable(argv[1], filename);
      console->setVariable(RESULTVAR, OK_STR);
   }
   else
      console->setVariable(RESULTVAR, CANCEL_STR);

	delete filter;
   return ( true );
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// dialogs and list boxes
void ToolPlugin::BaseBox::setTexts(const char *_title, const char *_question)
{
   title = (char *)_title;
   question = (char *)_question;
}

//----------------------------------------------------------------------------
void ToolPlugin::BaseBox::showIt(GWWindow *parent)
{
   //create the dialog modally
   if ( parent )
      GWDialog::create(dboxname, parent, true);
}

//----------------------------------------------------------------------------
LRESULT ToolPlugin::BaseBox::windowProc(UINT message, WPARAM wParam, 
   LPARAM lParam)
{
    HWND ctrlwnd;
    char buf[256];
    memset(buf, 0, 256);
    const char *cncl = CANCEL_STR;
    Point2I size;

    switch (message) {
      case WM_INITDIALOG:
        //center the dialog on the screen
        size = Point2I(::GetSystemMetrics(SM_CXSCREEN), 
                  ::GetSystemMetrics(SM_CYSCREEN));
        size -= getSize(); //subtract dims of dialog box
        SetWindowPos(getHandle(), 
            HWND_TOP, 
            size.x/2, 
            size.y/2, 
            0, 0,          // ignores size arguments 
            SWP_NOSIZE); 

         //set the title and the question
         ::SetWindowText(getHandle(), title);
         ctrlwnd = GetDlgItem(getHandle(), TDBOX_Q);
         ::SendMessage(ctrlwnd, WM_SETTEXT, WORD(0), (LONG)question); 
         break;
      case WM_COMMAND:
         switch (wParam) {
            case IDCANCEL:
            case IDOK:
               result = new char[strlen(cncl)+1];
               strcpy(result, cncl);
                 ::EndDialog(::GetActiveWindow(), 0);
               destroyWindow();
               break;
            default:
               return GWDialog::windowProc(message, wParam, lParam);
         }
         break;
      default:
         return GWDialog::windowProc(message, wParam, lParam);
   }
   return 0;
}

//----------------------------------------------------------------------------
void ToolPlugin::EditBox::setTexts(const char *_title, const char *_question,
        const char *_initialval)
{
   initial = (char *)_initialval;
   BaseBox::setTexts(_title, _question);
}

//----------------------------------------------------------------------------
LRESULT ToolPlugin::EditBox::windowProc(UINT message, WPARAM wParam,
       LPARAM lParam)
{
    HWND ctrlwnd = GetDlgItem(getHandle(), TDEDIT_EDIT);
    char buf[256];
    memset(buf, 0, 256);

    switch (message) {
      case WM_INITDIALOG:
         //set the initial value
         ::SetWindowText (ctrlwnd, initial);
         return BaseBox::windowProc(message, wParam, lParam);
      case WM_COMMAND:
         switch (wParam) {
            case IDOK:
               ::GetWindowText(ctrlwnd, buf, 255);
               result = new char[strlen(buf)+1];
               strcpy(result, buf);
                 ::EndDialog(::GetActiveWindow(), 0);
               destroyWindow();
               break;
            default:
               return BaseBox::windowProc(message, wParam, lParam);
         }
         break;
      default:
         return BaseBox::windowProc(message, wParam, lParam);
   }
   return 0;
}

//----------------------------------------------------------------------------
LRESULT ToolPlugin::BrowseBox::windowProc(UINT message, WPARAM wParam,
      LPARAM lParam)
{
   HWND ctrlwnd = GetDlgItem(getHandle(), TDEDIT_EDIT);
   switch ( message )
   {
      case WM_COMMAND:
         switch ( wParam ) {
            case TDBROWSE:
               static OPENFILENAME ofn;
               char filename[512];
               ToolPlugin::setupOFN(ofn, 511, filename, filter[0], 
                     ToolPlugin::getMainWindow()->getHandle());
               if ( GetOpenFileName(&ofn) )
                  ::SetWindowText(ctrlwnd, filename);
               break;
             default:
               return EditBox::windowProc(message, wParam, lParam);
         }
         break;
      default:
         return EditBox::windowProc(message, wParam, lParam);
   }
   return 0;
}   

//----------------------------------------------------------------------------
void ToolPlugin::Edit2Box::setTexts(const char *_title, const char *_question,
      const char *_initialval1, const char *_initialval2,
      const char *_qforval1, const char *_qforval2)
{
   initial2 = (char *)_initialval2;
   qval1 = (char *)_qforval1;
   qval2 = (char *)_qforval2;

   EditBox::setTexts(_title, _question, _initialval1);
}   

//----------------------------------------------------------------------------
LRESULT ToolPlugin::Edit2Box::windowProc(UINT message, WPARAM wParam, 
   LPARAM lParam)
{
    HWND ctrlwnd2 = GetDlgItem(getHandle(), TDEDIT2_EDIT2);
    HWND wqval1 = GetDlgItem(getHandle(), TDEDIT2_Q2);
    HWND wqval2 = GetDlgItem(getHandle(), TDEDIT2_Q3);
    char buf2[256];
    memset(buf2, 0, 256);

    switch (message) {
      case WM_INITDIALOG:
         //set the initial value
         ::SetWindowText (ctrlwnd2, initial2);
         ::SetWindowText (wqval1, qval1);
         ::SetWindowText (wqval2, qval2);
         return EditBox::windowProc(message, wParam, lParam);
      case WM_COMMAND:
         switch (wParam) {
            case IDOK:
               ::GetWindowText(ctrlwnd2, buf2, 255);
               result2 = new char[strlen(buf2)+1];
               if ( !result2 ) return 0;
               strcpy(result2, buf2);
               return ( EditBox::windowProc(message, wParam,
                      lParam ));
            default:
               return EditBox::windowProc(message, wParam, lParam);
         }
      default:
         return EditBox::windowProc(message, wParam, lParam);
   }
}

//----------------------------------------------------------------------------
void ToolPlugin::ListBox::addItem(const char *item)
{
   items.push_back(item);
}

//----------------------------------------------------------------------------
ToolPlugin::ListBox::ListBox(bool multi)
{
   isMulti= multi;                              
   if (multi)
      dboxname = "IDD_MULTIBOX";
   else
      dboxname = "IDD_LISTBOX";
}

//----------------------------------------------------------------------------
LRESULT ToolPlugin::ListBox::windowProc(UINT message, WPARAM wParam, 
   LPARAM lParam)
{
    HWND listwnd = GetDlgItem(getHandle(), TDLIST_LIST);
    Items::iterator i;
    int cursel;
    int len;
   const int maxSels = 100;
   int numSel, selItems[maxSels];
   char *temp;

    switch (message) {
      case WM_INITDIALOG:
         //add all the items to the listbox
         for ( i = items.begin(); i != items.end();i++ )
            SendMessage(listwnd, LB_ADDSTRING, NULL, (LPARAM)(*i));

         return BaseBox::windowProc(message, wParam, lParam);
      case WM_COMMAND:
         switch (wParam) {
            case IDOK:
            result = new char[1];
            if (!result)  return 0 ;
            *result = 0;
            if (isMulti)
            {
              //build a space separated string of all selected items   
              numSel = SendMessage(listwnd, LB_GETSELITEMS, WORD(maxSels), 
                  LONG(selItems));
              if (numSel != LB_ERR)
              {
                 for (int i=0; i<numSel; i++)
                 {
                    len = SendMessage(listwnd, LB_GETTEXTLEN,
                        WORD(selItems[i]), LONG(0));
                    temp = new char[len+strlen(result)+2];
                    if (!temp) return 0;
                    *temp=0;
                    if (i!=0) {
                     strcpy(temp, result);
                     strcat(temp, " ");
                    }
                    SendMessage(listwnd, LB_GETTEXT, WORD(selItems[i]),
                       LONG(&temp[strlen(temp)]));
                    delete [] result;
                    result = temp;
                 }
              }
            }
            else
            {
               cursel = SendMessage(listwnd, LB_GETCURSEL, WORD(0), LONG(0));
               if ( cursel != LB_ERR )
               {
                 len = SendMessage(listwnd, LB_GETTEXTLEN,
                    WORD(cursel), LONG(0));
                result = new char [len+1];
                if ( !result )  break;
                SendMessage(listwnd, LB_GETTEXT, (WORD)cursel, (LONG)result);
                intResult = cursel;
               }
            }
              ::EndDialog(::GetActiveWindow(), 0);
            destroyWindow();
            break;                                          

            default:
               return BaseBox::windowProc(message, wParam, lParam);
         }
         break;
      default:
         return BaseBox::windowProc(message, wParam, lParam);
   }
   return 0;
}

