//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
#include <console.h>
#include <simConsolePlugin.h>
#include <cdPlayerPlugin.h>


void CDPlayerPlugin::init()
{
	console->printf("CDPlayerPlugin");
	console->addCommand(CDPlayer, "CDPlayer", this);
}

void CDPlayerPlugin::endFrame()
{
}

const char *CDPlayerPlugin::consoleCallback(CMDConsole*,int id,int argc,const char *argv[])
{
   argv;

	switch(id)
	{
      case CDPlayer:
      {
			if (argc == 1)
			{
		      CDPlayerDlg *cdpDlg = new CDPlayerDlg();
            manager->addObject(cdpDlg);
         }
         else
				console->printf("CDPlayer <obj id>" );
         break;
      }
	}
	return 0;
}

