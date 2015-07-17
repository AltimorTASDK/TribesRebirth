#ifndef _H_SIMGUICONSOLEPLUGIN
#define _H_SIMGUICONSOLEPLUGIN

#include <timer.h>
#include <console.h>
#include <g_font.h>
#include <g_surfac.h>
#include <tvector.h>
#include <tstring.h>
#include <simconsoleplugin.h>

#define CONSOLE_CURSORTYPE    "$Console::CursorType"
#define CONSOLE_CURSORLINES   "$Console::CursorLines"
#define CONSOLE_PROMPT        "$Console::Prompt"
#define CONSOLE_HISTORY       "$Console::History"
#define CONSOLE_REPEATSPERSEC "$Console::RepeatsPerSec"
#define MAX_LINES               40
#define MAX_REPEATSPERSEC       10
#define MAX_CMD                512
#define MAX_PROMPT              12

class SimGuiConsolePlugin : public SimConsolePlugin, public CMDConsumer
{
   private :
      int                iPageDir, iLastLog;
      PALETTEENTRY       pe_fore, pe_back;
      bool               colorChanged;
      int                iRepeatsPerSec;
      Timer              timer;
      SimInputEvent     *pEvent;
      bool               fEnabled, fActive;
      bool               fOverwrite;
      char              *lpszPos;
      int                iCurrentCmd;
      char               szCmd[MAX_CMD];
      Resource<GFXFont>  rsFont;
      Vector<String *>   history;
      char              *lpszSuffix;
      char              *lpszHiStart, *lpszHiEnd;
      char               szPrefix[MAX_CMD];
      char               szCompletion[MAX_CMD];
      bool               pagedBack;
      DWORD              lastLineTime;
      static DWORD       lastLineTimeout;

      enum {
         PageUpScrollCount = 4,
         PageDownScrollCount = 4,
      };

   public : 
       SimGuiConsolePlugin();
      ~SimGuiConsolePlugin();

      void        init();
      void        activate(bool);
      void        enable(bool);
      bool        active();
      bool        enabled();
      void        endFrame();
      void        handleInput();
      void        render(GFXSurface *);
      void        renderLine(GFXSurface *, GFXFontInfo *, Point2I &, 
                             const char *, char *lpszPos = NULL, 
                             char *lpszHiStart = NULL, char *lpszHiEnd = NULL);
      bool        onSimInputEvent(const SimInputEvent *);
      const char *consoleCallback(CMDConsole *, int, int, const char *x[]);
      void        consoleCallback(CMDConsole *, const char *);
};

#endif // _H_SIMGUICONSOLEPLUGIN