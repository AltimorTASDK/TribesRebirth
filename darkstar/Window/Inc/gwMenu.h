#include <gw.h>

#ifndef _GWMENU_H_
#define _GWMENU_H_


//----------------------------------------------------------------------------
class GWMenu
{
   
   HMENU menu;

   public:
	  //constructor, destructor
	  GWMenu();
	  GWMenu(LPCSTR name);
	  GWMenu(int id);
	  ~GWMenu();

	  //creation, destruction
     bool create(LPCSTR name);
     bool create(int id);
	  bool destroy();

	  //data
	  HMENU getMenuHandle();
      
     //menu functions
     bool setMainMenu(HWND hWnd);
     bool popupMenu(HWND hWnd, int submenu, int clientx, int clienty); 

}; 



#endif
