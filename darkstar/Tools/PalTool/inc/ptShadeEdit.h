//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _PTSHADEEDIT_H_
#define _PTSHADEEDIT_H_

#include <gw.h>
#include <gwDialog.h>



class Paltool;


class ShadeEdit: public GWDialog
{
private:
	typedef GWDialog Parent;
   Paltool *paltool;

public:   
   ShadeEdit();
   void open(HWND hWnd, Paltool *_paltool);


   //--------------------------------------
   BOOL onInitDialog(HWND hwndFocus, LPARAM lParam);
	void onCommand(int id, HWND hwndCtl, UINT codeNotify);
	void onKey(UINT vk, BOOL fDown, int cRepeat, UINT flags);
};   


#endif //_PTSHADEEDIT_H_
