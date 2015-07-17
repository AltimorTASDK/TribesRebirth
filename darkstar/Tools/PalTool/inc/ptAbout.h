//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _PTABOUT_H_
#define _PTABOUT_H_

#include <gw.h>
#include <gwDialog.h>



class Paltool;


class About: public GWDialog
{
private:
	typedef GWDialog Parent;
   Paltool *paltool;

public:   
   About();
   void open(HWND hWnd, Paltool *_paltool);

   //--------------------------------------
	void onCommand(int id, HWND hwndCtl, UINT codeNotify);
};   



#endif //_PTABOUT_H_
