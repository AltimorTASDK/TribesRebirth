//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "shellOpenPlugin.h"
#include <shellapi.h>
#include "simCanvas.h"

void ShellOpenPlugin::init()
{
   console->printf("ShellOpenPlugin");
#ifdef DEBUG
   console->addCommand(ShellOpen, "shellOpen", this);
   console->addCommand(ShellOpenAndGoWin, "shellOpenAndGoWin", this);
#endif
   console->addCommand(HTMLOpen, "HTMLOpen", this);
   console->addCommand(HTMLOpenAndGoWin, "HTMLOpenAndGoWin", this);
   console->addCommand(Notepad, "Notepad", this);
}

const char *ShellOpenPlugin::consoleCallback(CMDConsole* /**/,
                                 int         id,
                                 int         argc,
                                 const char* argv[])
{
   switch(id) 
   {
#ifdef DEBUG
      case ShellOpen: {
         if ((argc == 2) || (argc == 3)){
            ShellExecute(NULL,            // Parent Window
                         "open",
                         argv[1],
                         (argc == 3) ? argv[2] : NULL,
                         NULL,
                         SW_SHOWNORMAL);
         } else {
            console->printf("shellOpen <filename>" );
            break;
         }
         return "True";
      }
      case ShellOpenAndGoWin: {
         if ((argc == 4) || (argc == 3))
         {
             SimCanvas *sc = dynamic_cast<SimCanvas *>( manager->findObject(argv[1]) );
             if (sc)
             {
                sc->setFullScreen(false);
                ShellExecute(NULL,            // Parent Window
                            "open",
                            argv[2],
                            (argc == 4) ? argv[3] : NULL,
                            NULL,
                            SW_SHOWNORMAL);
                return "True";
            }
         }
         console->printf("shellOpenAndGoWin <simCanvas> <filename>" );
         break;
      }
#endif
      case HTMLOpen: {
         if (argc == 2) {

            if (strnicmp(argv[1], "http://", 7) != 0) {
               console->printf("HTMLOpen: can only open properly prefixed HTML URLs.");
               break;
            }

            ShellExecute(NULL,            // Parent Window
                         "open",
                         argv[1],
                         NULL,
                         NULL,
                         SW_SHOWNORMAL);
         } else {
            console->printf("HTMLOpen(<filename>);" );
            break;
         }
         return "True";
      }
      case Notepad: {
         if (argc == 2)
         {
            //make sure the statement is a single word, followed by .txt
            const char *tmp = argv[1];
            while (*tmp != '\0')
            {
		if (*tmp == ' ')
                {
                   Console->printf("Notepad: can only open .txt files");
                   break;
                }
                tmp++;
            }
            tmp = strrchr(argv[1], '.');
            if (! tmp)
            {
                Console->printf("Notepad: can only open .txt files");
                break;
            }
            if (stricmp(tmp, ".txt"))
            {
                Console->printf("Notepad: can only open .txt files");
                break;
            }

            //open the file
            ShellExecute(NULL,            // Parent Window
                         "open",
                         argv[1],
                         NULL,
                         NULL,
                         SW_SHOWNORMAL);
         }
         else
         {
            Console->printf("Notepad(<filename>);");
         }
         return "True";
      }
      case HTMLOpenAndGoWin: {
         if (argc == 3)
         {
            if (strnicmp(argv[2], "http://", 7) != 0) {
               console->printf("HTMLOpenAndGoWin: can only open properly prefixed HTML URLs.");
               break;
            }

             SimCanvas *sc = dynamic_cast<SimCanvas *>( manager->findObject(argv[1]) );
             if (sc)
             {
                sc->setFullScreen(false);
                ShellExecute(NULL,            // Parent Window
                            "open",
                            argv[2],
                            NULL,
                            NULL,
                            SW_SHOWNORMAL);
                return "True";
            }
         }
         console->printf("HTMLOpenAndGoWin(<simCanvas>, <filename>);" );
         break;
      }
   }
   return "False";
}
