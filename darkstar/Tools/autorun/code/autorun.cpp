//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <process.h>
#include <errno.h>
#include <stdio.h>
#include <autorun.h>
#include <asWindows.h>


//------------------------------------------------------------------------------
void warning(const char *msg, ...)
{
   static char buffer[1024];
   va_list pArgs;
   va_start(pArgs, msg);
   vsprintf(buffer, msg, pArgs);

   MessageBox(0, buffer, "Warning !!!", MB_OK );
}   


//------------------------------------------------------------------------------
ASConsole::ASConsole()
{
}  


//------------------------------------------------------------------------------
ASConsole::~ASConsole()
{
   reg.close();
}  


//------------------------------------------------------------------------------ 
void ASConsole::init()
{
   rm.setSearchPath(".");
   setResourceManager(&rm);
   addConsumer(&co);

   //--------------------------------------
   addCommand(NewTitleWindow,    "newTitleWindow", this);
   addCommand(NewButton,         "newButton", this);
   addCommand(NewBitmap,         "newBitmap", this);
   addCommand(ShowWindow,        "showWindow", this);
   addCommand(CenterWindow,      "centerWindow", this);
   addCommand(SetPosition,       "setPosition", this);
   addCommand(SetIcon,           "setIcon", this);
   addCommand(Explore,           "explore", this);
   addCommand(Open,              "open", this);
   addCommand(IsRunning,         "isRunning", this);
   addCommand(Quit,              "quit", this);
   addCommand(RegistryOpen,      "registryOpen", this);
   addCommand(RegistryClose,     "registryClose", this);
   addCommand(RegistryRead,      "registryRead", this);
   addCommand(RegistryWrite,     "registryWrite", this);
   setIntVariable("last::x", 0);
   setIntVariable("last::y", 0);
   setIntVariable("last::w", 110);
   setIntVariable("last::h", 20);
   setIntVariable("spacing", 10);
}   


//------------------------------------------------------------------------------
const char *ASConsole::windowHandle(GWWindow* win)
{
   static char buffer[32];
   sprintf(buffer, "%08d", win);
   return (buffer);   
}   



//------------------------------------------------------------------------------
const char *ASConsole::consoleCallback(CMDConsole *console, int id, int argc, const char *argv[])
{
   switch (id)
   {
      //--------------------------------------
      case NewTitleWindow:
         if (argc == 3 || argc == 7)
         {
            Point2I pos(10,10);
            Point2I size(200,200);
            if (argc == 7)
            {
               pos.set( atoi(argv[3]),  atoi(argv[4]) );
               size.set( atoi(argv[5]), atoi(argv[6]) );
            }

            TitleWindow *title = new TitleWindow();
            title->create(NULL, argv[1], argv[2], 
               0, WS_OVERLAPPED|WS_SYSMENU, &pos, &size, 0, NULL );
            return (windowHandle(title));
         }
         else
            warning("Bad arguments to newWindow(title, name, x,y, w,h);");
         break;

      //--------------------------------------
      case NewButton:
         if (argc == 4 || argc == 8)
         {
            Point2I pos;
            Point2I size;
            if (argc == 8)
            {
               pos.set( atoi(argv[4]),  atoi(argv[5]) );
               size.set( atoi(argv[6]), atoi(argv[7]) );
            }

            GWWindow *parent = BaseWindow::findWindow(argv[1]);
            if (parent)
            {
               ButtonControl *btn = new ButtonControl();

               if (argc == 8)
                  btn->create(parent, argv[2], argv[3], &pos, &size );
               else
                  btn->create(parent, argv[2], argv[3]);
               return (windowHandle(btn));
            }
         }
         warning("Bad arguments to newButton( parent, title, name, x,y, w,h );");
         break;

      //--------------------------------------
      case NewBitmap:
         if (argc == 6 || argc == 9)
         {
            Point2I pos;
            int R,G,B;
            bool zero = false;
            pos.set( atoi(argv[4]),  atoi(argv[5]) );
               
            GWWindow *parent = BaseWindow::findWindow(argv[1]);
            if (parent)
            {
               BitmapControl *ctrl = new BitmapControl();
               if (argc == 9)
               {
                  zero = true;
                  R = atoi(argv[6]);
                  G = atoi(argv[7]);
                  B = atoi(argv[8]);
               }
               ctrl->create(parent, argv[2], argv[3], R,G,B, zero, pos );
               return (windowHandle(ctrl));
            }
         }
         warning("Bad arguments to newBitmap( parent, name, filename, x,y, {r,g,b} );");
         break;

      //--------------------------------------
      case ShowWindow:
         if (argc == 3) 
         {
            GWWindow *win = BaseWindow::findWindow(argv[1]);
            if (win)
            {
               bool show = (stricmp("true", argv[2]) == 0);
               win->show(show ? SW_SHOW : SW_HIDE);
            }
         }
         else
            warning("Bad arguments to showWindow( handle, ture/false );");
         break;

      //--------------------------------------
      case CenterWindow:
         if (argc >=2 && argc <= 4) 
         {
            GWWindow *win = BaseWindow::findWindow(argv[1]);
            if (win)
            {
               bool vert = true, horz = true;
               if (argc >= 3)
                  horz = stricmp(argv[2], "TRUE") == 0;
               if (argc == 4)
                  vert = stricmp(argv[3], "TRUE") == 0;

               Point2I parentSize;
               Point2I parentPos;
               GWWindow *parent = win->getParent();
               if (parent)
               {
                  parentSize = parent->getClientSize();
                  parentPos  = parent->getPosition();
               }
               else
               {

                  parentSize.x = GetSystemMetrics(SM_CXSCREEN);
                  parentSize.y = GetSystemMetrics(SM_CYSCREEN);
                  parentPos.set();
               }
               
               Point2I windowSize = win->getSize();
               Point2I windowPos  = win->getPosition();

               if (parent)
                  ScreenToClient(parent->getHandle(), (POINT*)(&windowPos));

               if (horz && windowSize.x <= parentSize.x)
                  windowPos.x = (parentSize.x-windowSize.x) >> 1;

               if (vert && windowSize.y <= parentSize.y)
                  windowPos.y = (parentSize.y-windowSize.y) >> 1;

               win->setPosition(windowPos);
            }
            else
               warning("Bad arguments to centerWindow( handle, horz, vert );");
         }
         break;

      //--------------------------------------
      case SetPosition:
         if (argc == 4 ) 
         {
            GWWindow *win = BaseWindow::findWindow(argv[1]);
            if (win)
            {
               Point2I pos;
               pos.set( atoi(argv[2]), atoi(argv[3]) );
               win->setPosition(pos);
            }
            else
               warning("Bad arguments to setPosition( handle, x,y );");
         }
         break;

      //--------------------------------------
      case SetIcon:
         if (argc == 3 ) 
         {
            BaseWindow *win = BaseWindow::findWindow(argv[1]);
            if (win)
               win->setIcon(argv[2]);
            else
               warning("Bad arguments to setIcon( handle, iconName );");
         }
         break;

      //--------------------------------------
      case Explore:
         if (argc == 2 ) 
         {
            ShellExecute(GetForegroundWindow(), "explore", argv[1], NULL, NULL, SW_SHOWNORMAL);
         }
         else
            warning("Bad arguments to explore( path );");
         break;

      //--------------------------------------
      case Open:
         if (argc == 2 ) 
         {
            char buffer[1024];
            char cwd[MAX_PATH];
            GetCurrentDirectory(MAX_PATH, cwd);
            strcpy(buffer, argv[1]);
            //strtok(buffer, " ");
            char* p = strrchr(buffer, '\\');
            if (!p)
               p = strrchr(buffer, '/');
            if (p)
               *p = 0;

            const char* t = strrchr(argv[1], '\\');
            if (!t)
               t = strrchr(argv[1], '/');
            if (t)
               t++;
            else
               t = argv[1];

            SetCurrentDirectory(buffer);
            ShellExecute(GetForegroundWindow(), "open", t, NULL, NULL, SW_SHOWNORMAL);
            SetCurrentDirectory(cwd);
         }
         else
            warning("Bad arguments to open( file );");
         break;

      //--------------------------------------
      case IsRunning:
         if (argc == 2 ) 
         {
            bool running = false;
            HANDLE hSem = CreateSemaphore(NULL, 0, 100, argv[1]);
            if (GetLastError() == ERROR_ALREADY_EXISTS)
               running = true;
            CloseHandle(hSem);
            return (running ? "TRUE" : NULL);
         }
         else
            warning("Bad arguments to isRunning( program );");
         break;

      //--------------------------------------
      case Quit:
         PostQuitMessage( 0 );
         break;

      //--------------------------------------
      case RegistryOpen:
         if (argc == 2 || argc == 3)
         {
            bool create = false;
            if (argc == 3)
               create = stricmp(argv[2], "TRUE");
            if (!reg.open(argv[1], create))
               return (NULL);
         }
         else
            warning("Bad arguments to RegistryOpen( key );");
         break;

      //--------------------------------------
      case RegistryClose:
         reg.close();
         break;

      //--------------------------------------
      case RegistryRead:
         if (argc == 2)
         {
            char buffer[1024];
            if (reg.readString(argv[1], buffer, 1024))
               return stringTable.insert(buffer);
            return (NULL);
         }
         else
            warning("Bad arguments to RegistryRead( key );");
         break;

      //--------------------------------------
      case RegistryWrite:
         if (argc == 3)
         {
            if (reg.writeString(argv[1], argv[2]))
               return ("TRUE");
            else
               return (NULL);
         }
         else
            warning("Bad arguments to RegistryWrite( key, value );");
         break;


      //--------------------------------------
      default:
         return ( Parent::consoleCallback(console, id, argc, argv) );
   }
   return ("TRUE");
}   




//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class Main: public GWMain
{
private:
   ASConsole *asConsole;

public:
   Main();
   ~Main();
   bool initApplication();
   int  messageLoop();
}_main;


//------------------------------------------------------------------------------
Main::Main()
{
   asConsole = NULL;
}   


//------------------------------------------------------------------------------
Main::~Main()
{
   delete asConsole;   
}   


//------------------------------------------------------------------------------
bool Main::initApplication()
{
   asConsole = new ASConsole();
   asConsole->init();
   asConsole->evaluate("exec( autorun );");   
   return (true);
}   

int Main::messageLoop()
{
   // We have no idle processing, so just give up control 
   // until we get a message ...
   // Remember, GetMessage can return TRUE, FALSE, or -1 ... so
   // explicitly check for TRUE to avoid errors
   while (GetMessage(&currentMsg, NULL, 0, 0) == TRUE)
   {
      TranslateMessage(&currentMsg);
      DispatchMessage (&currentMsg);
   }
   return (true);
}

