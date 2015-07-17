#include "console.h"
#include <stdio.h>
#include "resManager.h"

CMDConsole *console;
ResourceManager rm;

const char *c_print(CMDConsole *console, int id, int argc, const char **argv)
{
   for(int i = 1; i < argc; i++)
      printf("%s", argv[i]);
   return "True";
}

char retBuf[256];
class ConOut : public CMDConsumer
{
   virtual void consoleCallback(CMDConsole *, const char *consoleLine);
};

void ConOut::consoleCallback(CMDConsole *, const char *consoleLine)
{
   puts(consoleLine);
}

const char *c_sqrt(CMDConsole *console, int id, int argc, const char **argv)
{
   sprintf(retBuf, "%g", sqrt(atof(argv[1])));
   return retBuf;
}

const char *c_floor(CMDConsole *console, int id, int argc, const char **argv)
{
   sprintf(retBuf, "%d", int(atof(argv[1])));
   return retBuf;
}

void main(int argc, const char **argv)
{
   argc;
   argv;
   ConOut co;
   console = new CMDConsole();

   console->setResourceManager(&rm);
   console->addConsumer(&co);
   rm.setSearchPath(".");
   console->addCommand(0, "print", c_print);
   console->evaluate("exec(\"blah.cs\");");
}