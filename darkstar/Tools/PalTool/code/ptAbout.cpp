//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------


#include <ptCore.h>
#include <ptDefines.h>
#include <ptDialog.h>
#include <ptAbout.h>


//------------------------------------------------------------------------------
About::About()
{
}

void About::open(HWND hWnd, Paltool *_paltool)
{
   paltool = _paltool;
   create(IDD_ABOUT, hWnd);
}   



//------------------------------------------------------------------------------
void About::onCommand(int id, HWND hwndCtl, UINT codeNotify)
{
   id, hwndCtl, codeNotify;   
}
