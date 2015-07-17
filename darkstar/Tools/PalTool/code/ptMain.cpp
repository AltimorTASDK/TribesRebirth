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
#include <ptDialog.h>



class PaltoolMain: public GWMain
{
private:
   Paltool *dialog;

public:
   PaltoolMain();
   ~PaltoolMain();
   bool initInstance();   
   void onIdle();
}Main;   



//------------------------------------------------------------------------------
PaltoolMain::PaltoolMain()
{
   AssertSetFlags(ASSERT_NO_DIALOG);
   dialog = NULL;
}

//------------------------------------------------------------------------------
PaltoolMain::~PaltoolMain()
{
   delete dialog;
}

//------------------------------------------------------------------------------
bool PaltoolMain::initInstance()
{
   InitCommonControls();
   dialog = new Paltool;
   if (dialog) dialog->init();
   return (dialog != NULL);
}



//------------------------------------------------------------------------------
void PaltoolMain::onIdle()
{
   if (dialog) dialog->render(); 
}
