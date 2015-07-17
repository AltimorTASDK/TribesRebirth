#include "simConsolePlugin.h"
#include "simWinConsolePlugin.h"
#include <stdio.h>
#include "gwmain.h"
#include "simConsoleEvent.h"

enum CallbackID
{
   WinConsoleEnabled = 0,
   DedicatedServer = 1,
};

SimWinConsolePlugin::SimWinConsolePlugin()
{
   for (int iIndex = 0; iIndex < MAX_CMDS; iIndex ++)
      rgCmds[iIndex][0] = '\0';

   iCmdIndex         = 0;
   winConsoleEnabled = false;
}

void SimWinConsolePlugin::printf(const char *s, ...)
{
   static char buffer[512];
   DWORD bytes;
   va_list args;
   va_start(args, s);
   vsprintf(buffer, s, args);
   WriteFile(stdOut, buffer, strlen(buffer), &bytes, NULL);
   FlushFileBuffers( stdOut );
}   


void SimWinConsolePlugin::init()
{
   STARTUPINFO si;

   si.cb = sizeof(si);
   si.lpReserved = NULL;

   GetStartupInfo(&si);
   
   inpos = 0;
   lineOutput = false;
   
   console->addVariable(WinConsoleEnabled, "WinConsoleEnabled", this, "false");
   console->addVariable(DedicatedServer, "dedicated", this);
   console->addConsumer(this);
}

void SimWinConsolePlugin::consoleCallback(CMDConsole *, const char *consoleLine)
{
   if(winConsoleEnabled)
   {
      inbuf[inpos] = 0;
      if(lineOutput)
         printf("%s\n", consoleLine);
      else
         printf("%c%s\n%s%s", '\r', consoleLine, console->getVariable("Console::Prompt"), inbuf);
   }
}

const char *SimWinConsolePlugin::consoleCallback(CMDConsole *, int id, int argc, const char **argv)
{
   argc;
   argv;

   if(id == WinConsoleEnabled)
   {
      winConsoleEnabled = console->getBoolVariable("WinConsoleEnabled");
   
      if(winConsoleEnabled)
      {
         AllocConsole();
         const char *title = CMDConsole::getLocked()->getVariable("Console::WindowTitle");
         if (title && *title)
            SetConsoleTitle(title);
         stdOut = GetStdHandle(STD_OUTPUT_HANDLE);
         stdIn  = GetStdHandle(STD_INPUT_HANDLE);
         stdErr = GetStdHandle(STD_ERROR_HANDLE);

         printf("%s", console->getVariable("Console::Prompt"));
      }
   } 
   else if(id == DedicatedServer)
   {
      GWMain::pMain->checkFocus = !console->getBoolVariable("dedicated");
   }
   return 0;
}

void SimWinConsolePlugin::endFrame()
{
   if(winConsoleEnabled)
   {
      DWORD numEvents;
      GetNumberOfConsoleInputEvents(stdIn, &numEvents);
      if(numEvents)
      {
         INPUT_RECORD rec[20];
         char outbuf[256];
         int outpos = 0;

         ReadConsoleInput(stdIn, rec, 20, &numEvents);
         DWORD i;
         for(i = 0; i < numEvents; i++)
         {
            if(rec[i].EventType == KEY_EVENT)
            {
               KEY_EVENT_RECORD *ke = &(rec[i].Event.KeyEvent);
               if(ke->bKeyDown)
               {
                  switch (ke->uChar.AsciiChar)
                  {
                     // If no ASCII char, check if it's a handled virtual key
                     case 0:
                        switch (ke->wVirtualKeyCode)
                        {
                           // UP ARROW
                           case 0x26 :
                              // Go to the previous command in the cyclic array
                              if ((-- iCmdIndex) < 0)
                                 iCmdIndex = MAX_CMDS - 1;

                              // If this command isn't empty ...
                              if (rgCmds[iCmdIndex][0] != '\0')
                              {
                                 // Obliterate current displayed text
                                 for (int i = outpos = 0; i < inpos; i ++)
                                 {
                                    outbuf[outpos ++] = '\b';
                                    outbuf[outpos ++] = ' ';
                                    outbuf[outpos ++] = '\b';
                                 }

                                 // Copy command into command and display buffers
                                 for (inpos = 0; inpos < (int)strlen(rgCmds[iCmdIndex]); inpos ++, outpos ++)
                                 {
                                    outbuf[outpos] = rgCmds[iCmdIndex][inpos];
                                    inbuf [inpos ] = rgCmds[iCmdIndex][inpos];
                                 }
                              }
                              // If previous is empty, stay on current command
                              else if ((++ iCmdIndex) >= MAX_CMDS)
                              {
                                 iCmdIndex = 0;
                              }
                              
                              break;

                           // DOWN ARROW
                           case 0x28 : {
                              // Go to the next command in the command array, if
                              // it isn't empty
                              if (rgCmds[iCmdIndex][0] != '\0' && (++ iCmdIndex) >= MAX_CMDS)
                                  iCmdIndex = 0;

                              // Obliterate current displayed text
                              for (int i = outpos = 0; i < inpos; i ++)
                              {
                                 outbuf[outpos ++] = '\b';
                                 outbuf[outpos ++] = ' ';
                                 outbuf[outpos ++] = '\b';
                              }

                              // Copy command into command and display buffers
                              for (inpos = 0; inpos < (int)strlen(rgCmds[iCmdIndex]); inpos ++, outpos ++)
                              {
                                 outbuf[outpos] = rgCmds[iCmdIndex][inpos];
                                 inbuf [inpos ] = rgCmds[iCmdIndex][inpos];
                              }
                              }
                              break;

                           // LEFT ARROW
                           case 0x25 :
                              break;

                           // RIGHT ARROW
                           case 0x27 :
                              break;

                           default :
                              break;
                        }
                        break;
                     case '\b':
                        if(inpos > 0)
                        {
                           outbuf[outpos++] = '\b';
                           outbuf[outpos++] = ' ';
                           outbuf[outpos++] = '\b';
                           inpos--;
                        }
                        break;
                     case '\n':
                     case '\r':
                        outbuf[outpos++] = '\r';
                        outbuf[outpos++] = '\n';

                        inbuf[inpos] = 0;
                        outbuf[outpos] = 0;
                        printf("%s", outbuf);

                        SimConsoleEvent::postFocused(inbuf, false, 0);

                        // If we've gone off the end of our array, wrap
                        // back to the beginning
                        if (iCmdIndex >= MAX_CMDS)
                            iCmdIndex %= MAX_CMDS;

                        // Put the new command into the array
                        strcpy(rgCmds[iCmdIndex ++], inbuf);

                        printf("%s", console->getVariable("Console::Prompt"));
                        inpos = outpos = 0;
                        break;
                     default:
                        inbuf[inpos++] = ke->uChar.AsciiChar;
                        outbuf[outpos++] = ke->uChar.AsciiChar;
                        break;
                  }
               }
            }
         }
         if(outpos)
         {
            outbuf[outpos] = 0;
            printf("%s", outbuf);
         }
      }
   }
}