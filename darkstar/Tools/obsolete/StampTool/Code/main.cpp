#include <gw.h>

#include "gridWindow.h"

class CMain : public GWMain
{
	GridWindow    *window;

public:
	CMain();
	~CMain();
	virtual bool initInstance();
   virtual void onIdle();
} Main;

//--------------------------------------------------------------------------------

CMain::CMain()
{
	window = 0;
}
 
CMain::~CMain()
{
	delete window;
}

bool CMain::initInstance()
{
   window = new GridWindow();
   if (window) 
   {
      if (window->init(cmdLine))
         return true;
   }
   return false; 
}

void CMain::onIdle()
{
   if (window) window->render();
}
