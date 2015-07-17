#include "gwmenu.h"

//----------------------------------------------------------------------------
GWMenu::GWMenu()
{
	menu=NULL;
}

//----------------------------------------------------------------------------
GWMenu::GWMenu(LPCSTR name)
{
	create(name);
}
//----------------------------------------------------------------------------
GWMenu::GWMenu(int id)
{
	create(id);
}

//----------------------------------------------------------------------------
GWMenu::~GWMenu()
{
	destroy();
}

//----------------------------------------------------------------------------
HMENU GWMenu::getMenuHandle()
{
	return menu;
}


//----------------------------------------------------------------------------
//set menu as main menu of a window
bool GWMenu::setMainMenu(HWND hWnd)
{
  if ( ::IsWindow(hWnd) && ::IsMenu(getMenuHandle() ) )
  {
   if ( !::SetMenu(hWnd, getMenuHandle()) )
   {
      AssertFatal(0, "GWMenu::setMainMenu: cannot set main menu");
      //MessageBox(NULL, "cannot set main menu!", NULL, MB_OK);
      return ( false );
   }
  }
  return ( true );
}

//----------------------------------------------------------------------------
//popup a submenu of current menu onto window
bool GWMenu::popupMenu(HWND hWnd, int submenu, int clientx, int clienty)
{
  POINT point;
  point.x = clientx;
  point.y = clienty;

  if ( !::IsWindow(hWnd) )
   return ( false );
  
  ClientToScreen(hWnd, &point);
  HMENU hMenu = GetSubMenu(getMenuHandle(), submenu);
  if ( hMenu )
   ::TrackPopupMenu(hMenu, 0, point.x, point.y, 0, hWnd, NULL);

  return ( true ); 
   
}

//----------------------------------------------------------------------------
bool GWMenu::destroy()
{

  bool b = (::DestroyMenu(menu) == TRUE);
  if (!b)
  {
	 //otherwise menu already destroyed
	  if ( ::IsMenu(getMenuHandle()) )				
	  {
      AssertFatal(0, "GWMenu::destroy: menu not destory");
		//MessageBox(NULL, "menu not destroyed", NULL, MB_OK);
		return (false);
	  }
  }	
  else
  {
	  menu=NULL;
	  return (true);
  }
  return (false);
}
	
//----------------------------------------------------------------------------
bool GWMenu::create(int id)
{
   menu = LoadMenu(0, MAKEINTRESOURCE(id));
   if (!menu)
   {
     AssertFatal(0, "GWMenu::create(id): menu could not load");
    // MessageBox (NULL, "menu(id) not loaded!", NULL, MB_OK );
   }
   return ( menu != NULL);
}   


//----------------------------------------------------------------------------
bool GWMenu::create(LPCSTR name)
{
   menu = LoadMenu(0, name);
   if (!menu)
   {
     AssertFatal(0, "GWMenu::create(name): menu could not load");
     //MessageBox (NULL, "menu not loaded!", NULL, MB_OK );
   }
   return ( menu != NULL);
}

//----------------------------------------------------------------------------
