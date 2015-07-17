//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <gwWinSX.h>

#include <ptCore.h>
#include <ptDefines.h>
#include <ptDialog.h>
#include <ptShadeEdit.h>


//------------------------------------------------------------------------------
ShadeEdit::ShadeEdit()
{
}


void ShadeEdit::open(HWND hWnd, Paltool *_paltool)
{
   paltool = _paltool;
   create(IDD_SHADEHAZE, hWnd);
}


//------------------------------------------------------------------------------
BOOL ShadeEdit::onInitDialog(HWND hwndFocus, LPARAM lParam)
{
   CheckDlgButton(getHandle(), IDC_DO_SHADE, 1);
   CheckDlgButton(getHandle(), IDC_DO_HAZE,  1);

   DWORD state = paltool->getState();
   Button_SetCheck(GetDlgItem(getHandle(), IDC_EDIT_HAZE),  (state & STATE_HAZE) ?  BST_CHECKED:BST_UNCHECKED);
   Button_SetCheck(GetDlgItem(getHandle(), IDC_EDIT_SHADE), (state & STATE_SHADE) ? BST_CHECKED:BST_UNCHECKED);
   Button_SetCheck(GetDlgItem(getHandle(), IDC_EDIT_HAZE_TO), (state & STATE_HAZE_TO) ? BST_CHECKED:BST_UNCHECKED);
   Button_SetCheck(GetDlgItem(getHandle(), IDC_VIEW), (state & STATE_EDIT) ? BST_UNCHECKED:BST_CHECKED);

   return Parent::onInitDialog(hwndFocus, lParam);
}


//------------------------------------------------------------------------------
void ShadeEdit::onCommand(int id, HWND hwndCtl, UINT codeNotify)
{
   hwndCtl, codeNotify;   
   switch (id)
   {
      case IDC_GENERATE:
         paltool->onGenerate();
         break;

      case IDC_APPLY:
         paltool->onApply();
         break;

      case IDC_EDIT_HAZE:
         paltool->setState( STATE_HAZE | (paltool->getState() & ~STATE_MASK) );
         Button_SetCheck(GetDlgItem(getHandle(), IDC_EDIT_HAZE),     BST_CHECKED);
         Button_SetCheck(GetDlgItem(getHandle(), IDC_EDIT_SHADE),    BST_UNCHECKED);
         Button_SetCheck(GetDlgItem(getHandle(), IDC_EDIT_HAZE_TO),  BST_UNCHECKED);
         Button_SetCheck(GetDlgItem(getHandle(), IDC_EDIT_RGB_MATCH),BST_UNCHECKED);
         break;

      case IDC_EDIT_SHADE:
         paltool->setState( STATE_HAZE | (paltool->getState() & ~STATE_MASK) );
         Button_SetCheck(GetDlgItem(getHandle(), IDC_EDIT_HAZE),     BST_UNCHECKED);
         Button_SetCheck(GetDlgItem(getHandle(), IDC_EDIT_SHADE),    BST_CHECKED);
         Button_SetCheck(GetDlgItem(getHandle(), IDC_EDIT_HAZE_TO),  BST_UNCHECKED);
         Button_SetCheck(GetDlgItem(getHandle(), IDC_EDIT_RGB_MATCH),BST_UNCHECKED);
         break;

      case IDC_EDIT_HAZE_TO:
         paltool->setState( STATE_HAZE_TO | (paltool->getState() & ~STATE_MASK) );
         Button_SetCheck(GetDlgItem(getHandle(), IDC_EDIT_HAZE),     BST_UNCHECKED);
         Button_SetCheck(GetDlgItem(getHandle(), IDC_EDIT_SHADE),    BST_UNCHECKED);
         Button_SetCheck(GetDlgItem(getHandle(), IDC_EDIT_HAZE_TO),  BST_CHECKED);
         Button_SetCheck(GetDlgItem(getHandle(), IDC_EDIT_RGB_MATCH),BST_UNCHECKED);
         break;

      case IDC_EDIT_RGB_MATCH:
         paltool->setState( STATE_RGB_MATCH | (paltool->getState() & ~STATE_MASK) );
         Button_SetCheck(GetDlgItem(getHandle(), IDC_EDIT_HAZE),     BST_UNCHECKED);
         Button_SetCheck(GetDlgItem(getHandle(), IDC_EDIT_SHADE),    BST_UNCHECKED);
         Button_SetCheck(GetDlgItem(getHandle(), IDC_EDIT_HAZE_TO),  BST_UNCHECKED);
         Button_SetCheck(GetDlgItem(getHandle(), IDC_EDIT_RGB_MATCH),BST_CHECKED);
         break;


      case IDC_VIEW:
         paltool->setState( paltool->getState() ^ STATE_EDIT);
         Button_SetCheck(GetDlgItem(getHandle(), IDC_VIEW), 
            (paltool->getState() & STATE_EDIT) ? BST_UNCHECKED:BST_CHECKED);
         break;

      case IDC_DO_SHADE:
         paltool->paletteShade.setShade(IsDlgButtonChecked(getHandle(), IDC_DO_SHADE));
         break;

      case IDC_DO_HAZE:
         paltool->paletteShade.setHaze(IsDlgButtonChecked(getHandle(), IDC_DO_HAZE));
         break;
   }
}


//------------------------------------------------------------------------------
void ShadeEdit::onKey(UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
   fDown, cRepeat, flags;
   switch (vk)
   {
      case VK_ESCAPE:
         if (paltool->getState() & STATE_EDIT)
            paltool->paletteGrid.zeroSelection();   
         break;

      case VK_F1:
         paltool->setState( paltool->getState() ^ STATE_EDIT);
         Button_SetCheck(GetDlgItem(getHandle(), IDC_VIEW), 
            (paltool->getState() & STATE_EDIT) ? BST_UNCHECKED:BST_CHECKED);
         break;
   }
}   
