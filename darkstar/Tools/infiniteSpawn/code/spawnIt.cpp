#include <windows.h>
#include <limits.h>

static const char *szAppName = "InfiniteSpawn v1.0";
static const char *szClassName = "InfiniteSpawn";
static HINSTANCE hInstance = NULL;
static HWND hWnd = NULL;
static HWND hButton  = NULL;
static HWND hProgram = NULL;
static HWND hTotal   = NULL;
static HWND hCurrent = NULL;
static HWND hMinMax  = NULL;
static HWND hAve     = NULL;
static HWND hRestart = NULL;
static HWND hNoresponse = NULL;

static int numWindows = 0;

STARTUPINFO si = {0};
PROCESS_INFORMATION pi = {0};

static int numHandles = 0;
char program[2048] = {0};
char process[2048] = {0};

bool debugInfoOn = false;
FILETIME lastKernalTime;
FILETIME lastUserTime;
int delta = 0;
int restarts = 0;
int noresponse = 0;

DWORD totalTime = 0;    // in minutes
DWORD totalTicks = 0;   // in milliseconds
DWORD totalCount = 0;   // in milliseconds
DWORD curTime = 0;      // in minutes
DWORD curTicks = 0;     // in milliseconds
DWORD curCount = 0;     // in milliseconds
DWORD noresponseTime = 0;
int maxTime = LONG_MIN;


//------------------------------------------------------------------------------
void DisplayError()
{
   
   char *lpMsgBuf;
   FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
               NULL, 
               GetLastError(),
               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),   // Default language
               (LPTSTR) &lpMsgBuf,    
               0, NULL );                                   
   MessageBox( NULL, lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );
   LocalFree( lpMsgBuf );
}


//------------------------------------------------------------------------------
char *formatTime(DWORD minutes)
{
   static char buf[100];
   int day  = minutes/(60*24);
   minutes %= (60*24);
   int hour = minutes/60;
   minutes %= 60;

   if (day)
   {
      wsprintf(buf, "%d day %d hr %d min", day, hour, minutes);
      return (buf);
   }

   if (hour)
   {
      wsprintf(buf, "%d hr %d min", hour, minutes);
      return (buf);
   }

   wsprintf(buf, "%d min", minutes);
   return (buf);

}   

//------------------------------------------------------------------------------
BOOL CALLBACK enumWindows( HWND hwnd, LONG lParam )  
{
   hwnd, lParam;
   numWindows++;
   return (true);
}


//------------------------------------------------------------------------------
void updateWindow()
{
   int ticksnow = GetTickCount();
   totalTicks += (ticksnow-totalCount);
   curTicks   += (ticksnow-curCount);

   totalCount = ticksnow;
   curCount = ticksnow;

   #define MINUTES  (1000*60)
   int minutes = (totalTicks/MINUTES);
   totalTicks -= (minutes*MINUTES);
   totalTime  += minutes;

   minutes   = (curTicks/MINUTES);
   curTicks -= (minutes*MINUTES);
   curTime  += minutes;

   if (numHandles)
   {
      maxTime = ((int)curTime > maxTime) ? curTime : maxTime;
   }

   if (debugInfoOn)
   {
      char buf[100];
      wsprintf(buf, "Total: %s", formatTime(totalTime));
      SendMessage(hTotal, WM_SETTEXT, 0, (LPARAM)buf);

      wsprintf(buf, "Time: %s", formatTime(curTime));
      SendMessage(hCurrent, WM_SETTEXT, 0, (LPARAM)buf);

      if (maxTime == LONG_MIN)
         strcpy(buf, "Max: ?");
      else
         wsprintf(buf, "Max: %s", formatTime(maxTime));
      SendMessage(hMinMax, WM_SETTEXT, 0, (LPARAM)buf);

      int aveTime = totalTime / (restarts+noresponse+1);
      wsprintf(buf, "Ave: %s", formatTime(aveTime));
      SendMessage(hAve, WM_SETTEXT, 0, (LPARAM)buf);

      wsprintf(buf, "Restarts: %d", restarts);
      SendMessage(hRestart, WM_SETTEXT, 0, (LPARAM)buf);

      wsprintf(buf, "NoResponse: %d", noresponse);
      SendMessage(hNoresponse, WM_SETTEXT, 0, (LPARAM)buf);
   }   
}   


//------------------------------------------------------------------------------
bool startProcess()
{
   si.cb = sizeof(STARTUPINFO);
   si.lpReserved = NULL;
   si.lpReserved2 = NULL;
   si.cbReserved2 = 0;
   si.lpDesktop = NULL;
   si.dwFlags = 0;

   if (CreateProcess( NULL,               // str executable
                  process,                // str commandline
                  NULL,                   // process security attributes
                  NULL,                   // thread security attributes
                  false,                   // handle inheritance flag
                  NORMAL_PRIORITY_CLASS,  // creation flags
                  NULL,                   // ptr new environment block
                  NULL,                   // str current directory name
                  &si,                    // STARTUPINFO
                  &pi ))                  // PROCESS_INFORMATION
   {
      numHandles = 1;
      FILETIME ct, et;
      GetProcessTimes(pi.hProcess, &ct, &et, &lastKernalTime, &lastUserTime);

      curTime  = 0;
      curTicks = 0;
      curCount  = GetTickCount();
   }
   return (numHandles);
}   


//------------------------------------------------------------------------------
int subTime(FILETIME &a, FILETIME &b)
{
   return (b.dwLowDateTime - a.dwLowDateTime);
}   


//------------------------------------------------------------------------------
void killProcess()
{
   if (numHandles)
   {
      TerminateProcess(pi.hProcess, 0);   
      CloseHandle(pi.hProcess);
      noresponseTime = 0;
   }
}   


//------------------------------------------------------------------------------
void checkResponse()
{
   //--------------------------------------
   // this is a total hack!
   // If the server thread creates ANY other windows we detect it and
   // ASSUME it's something bad like an ATAN, SQRT error dlg
   // and restart the program.
   numWindows = 0;
   EnumThreadWindows(pi.dwThreadId, enumWindows, 0);
   if (numWindows)
   {
      if (noresponseTime == 0)
         noresponseTime = GetTickCount();
      else
      {
         // don't kill it unless it's been questionable for at least 15 seconds
         if ((GetTickCount()-noresponseTime) > (15*1000))
         {
            noresponse++;
            noresponseTime = 0;
            killProcess();
            startProcess();
         }
      }
   }
   else
      noresponseTime = 0;


   //-------------------------------------- the brute force way
//   FILETIME ct, et, kt, ut;
//   GetProcessTimes(pi.hProcess, &ct, &et, &kt, &ut);
//
//   delta  = subTime(lastKernalTime, kt);
//   delta += subTime(lastUserTime, ut);
//   
//   lastKernalTime = kt;
//   lastUserTime   = ut;
//
//   if (delta == 0)
//   {
//      if (noresponseTime == 0)
//         noresponseTime = GetTickCount();
//      else
//      {
//         // don't kill it unless it's been dead for at least 1 minute
//         if ((GetTickCount()-noresponseTime) > (1*60*1000))
//         {
//            noresponse++;
//            killProcess();
//            startProcess();
//         }
//      }
//   }
//   else
//      noresponseTime = 0;
}   


//------------------------------------------------------------------------------
bool initWindow(HWND hWnd)
{
   RECT rcClient;
   //int cyVScroll = GetSystemMetrics(SM_CYVSCROLL); 
   GetClientRect(hWnd, &rcClient);        

   hProgram = CreateWindow("STATIC", 
      program, 
      WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP, 
      rcClient.left+10, 15,
      rcClient.right-10, 22,
      hWnd, 
      0, 
      hInstance, 
      NULL);

   hButton = CreateWindow("BUTTON", 
      "Quit",
      WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
      rcClient.right-70, 45,
      60, 30,
      hWnd, 
      0, 
      hInstance, 
      NULL);

   if (debugInfoOn)
   {
      hTotal = CreateWindow("STATIC", 
         "Total:", 
         WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP, 
         rcClient.left+10, 35,
         200, 20,
         hWnd, 
         0, 
         hInstance, 
         NULL);

      hCurrent = CreateWindow("STATIC", 
         "Time:", 
         WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP, 
         rcClient.left+10, 50,
         200, 20,
         hWnd, 
         0, 
         hInstance, 
         NULL);

      hMinMax = CreateWindow("STATIC", 
         "Max:", 
         WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP, 
         rcClient.left+10, 65,
         200, 20,
         hWnd, 
         0, 
         hInstance, 
         NULL);

      hAve = CreateWindow("STATIC", 
         "Ave:", 
         WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP, 
         rcClient.left+10, 80,
         200, 20,
         hWnd, 
         0, 
         hInstance, 
         NULL);

      hRestart = CreateWindow("STATIC", 
         "Restart:", 
         WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP, 
         rcClient.left+200, 35,
         100, 20,
         hWnd, 
         0, 
         hInstance, 
         NULL);

      hNoresponse = CreateWindow("STATIC", 
         "NoResponse:", 
         WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP, 
         rcClient.left+200, 50,
         100, 20,
         hWnd, 
         0, 
         hInstance, 
         NULL);
   }

   UpdateWindow(hWnd);
   return (true);
}   


//------------------------------------------------------------------------------
long FAR PASCAL WndProc( HWND hWnd, unsigned message, WPARAM wParam, LPARAM lParam)
{
   switch (message)
   {
      case WM_CREATE:
         if (!initWindow(hWnd)) 
            return (-1);
         break;

      case WM_COMMAND:{
         if ((HWND)lParam == hButton)   
            PostQuitMessage( 0 );
         }
         break;

      case WM_DESTROY:
         PostQuitMessage( 0 );
         break;
   
      default:
         return DefWindowProc( hWnd, message, wParam, lParam );
   }
   return(0L);
}   


//------------------------------------------------------------------------------
bool initClass( HINSTANCE hInst )
{
   WNDCLASSEX wc;
   hInstance = hInst;
   wc.cbSize         = sizeof(WNDCLASSEX);
   wc.style          = CS_HREDRAW | CS_VREDRAW;
   wc.lpfnWndProc    = WndProc;
   wc.cbClsExtra     = 0;
   wc.cbWndExtra     = 0;
   wc.hInstance      = hInstance;
   wc.hIcon          = LoadIcon( hInstance, MAKEINTRESOURCE(1));
   wc.hCursor        = LoadCursor( NULL, IDC_ARROW );
   wc.hbrBackground  = (HBRUSH)(COLOR_WINDOW);
   wc.lpszMenuName   = NULL;
   wc.lpszClassName  = szClassName;
   wc.hIconSm        = LoadIcon( hInstance, MAKEINTRESOURCE(2));

   return RegisterClassEx( &wc );
}


//------------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR lpCmdLine, int nCmdShow)
{
   nCmdShow;

   if (!initClass(hInstance))
      return (false);

   //--------------------------------------

   if (*lpCmdLine == '*')
   {
      debugInfoOn = true;
      lpCmdLine++;
   }

   wsprintf(program, "Program: %s", lpCmdLine);
   strcpy(process, lpCmdLine);
   totalCount = GetTickCount();

   startProcess();

   //--------------------------------------
   hWnd = CreateWindow(
               szClassName,
               szAppName,
               WS_OVERLAPPEDWINDOW,
               CW_USEDEFAULT,    
               CW_USEDEFAULT,    
               450, 130,     
               NULL,             // hWnd parent   
               NULL,             // hMenu
               hInstance,        // this instance
               NULL              
               );
   if (!hWnd) 
      return (false);

   ShowWindow( hWnd, SW_SHOW );
   updateWindow();


   //--------------------------------------
   while (true)    
   {       
      MSG msg ;         
      while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))         
      { 
         // If it's a quit message, we're out of here.
         if (msg.message == WM_QUIT)
         {
            killProcess();
            return 1; 
         }
         DispatchMessage(&msg);         
      } 

      // Wait for any message sent or posted to this queue 
      // or for one of the passed handles be set to signaled.
      // timeout ever 10 seconds and see if the app is still alive
      DWORD result = MsgWaitForMultipleObjects(numHandles, &pi.hProcess, FALSE, (1*1000), QS_ALLINPUT); 
      switch( result )
      {
         case WAIT_OBJECT_0:
            if (numHandles)
               startProcess();
               restarts++;
            break;

         case WAIT_OBJECT_0+1:
            // New messages have arrived. 
            // Continue to the top of the always while loop to 
            // dispatch them and resume waiting.            
            continue;        

         case WAIT_TIMEOUT:
            checkResponse();
            break;
      }
      updateWindow();
   } 
}

