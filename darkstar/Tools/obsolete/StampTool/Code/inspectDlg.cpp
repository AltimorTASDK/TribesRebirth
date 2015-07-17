//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "inspectDlg.h"
#include "grdblock.h"

//------------------------------------------------------------------------------

InspectDlg::InspectDlg(GridWindow *_parent)
{
   parent = NULL;

   if (! create(IDD_INSPECT_DLG, _parent->getHandle()))
      AssertFatal(0, "Could not open dialog.  Missing resource?");

   parent = _parent;
}

//--------------------------------------------------------------------------------

void InspectDlg::onCommand(int id, HWND, UINT)
{
   if (parent == NULL)
      return;

   switch( id )
   {
      case BTN_STAMP_APPLY:
         onApplyStamp();
         break;

      case BTN_TILE_APPLY:
         onApplyTile();
         break;
   }
}   

//------------------------------------------------------------------------------

void InspectDlg::updateView()
{
   Point2I *tile = parent->getSelectedTile();
   TextureInfo *tex = parent->getTileTexture(*tile);

   if (!tex)
   {
      SetDlgItemText(getHandle(),EDT_TILE_INDEX, "none");  
      SendMessage(GetDlgItem(getHandle(),CHK_ROTATE), BM_SETCHECK, 0, 0L);
      SendMessage(GetDlgItem(getHandle(),CHK_FLIP_X), BM_SETCHECK, 0, 0L);
      SendMessage(GetDlgItem(getHandle(),CHK_FLIP_Y), BM_SETCHECK, 0, 0L);
   }
   else 
   {
      switch (tex->textureID)
      {
         case -1:
            SetDlgItemText(getHandle(),EDT_TILE_INDEX, "hole");   
            break;
              
         default:
            SetDlgItemText(getHandle(),EDT_TILE_INDEX, avar("%d",tex->textureID));  
      }
      SendMessage(GetDlgItem(getHandle(),CHK_ROTATE), BM_SETCHECK, tex->flags & GridBlock::Material::Rotate, 0L);
      SendMessage(GetDlgItem(getHandle(),CHK_FLIP_X), BM_SETCHECK, tex->flags & GridBlock::Material::FlipX, 0L);
      SendMessage(GetDlgItem(getHandle(),CHK_FLIP_Y), BM_SETCHECK, tex->flags & GridBlock::Material::FlipY, 0L);
   }

   SetDlgItemText(getHandle(),EDT_NAME, parent->getStampName()); 
   SetDlgItemText(getHandle(),EDT_DML_NAME, parent->getDML_Name()); 
   SetDlgItemText(getHandle(),EDT_CLAMP, avar("%d",parent->getStampClampValue()));
   SendMessage(GetDlgItem(getHandle(),CHK_FLAT), BM_SETCHECK, parent->isTileFlat(*tile), 0L);
}   

//------------------------------------------------------------------------------

void InspectDlg::onApplyTile()
{
   char buffer[1024];
   int num;
   TextureInfo tex;
   Point2I *curTile;

   buffer[0] = '\0';

   // get the texture id
   GetDlgItemText(getHandle(), EDT_TILE_INDEX, buffer, 1024);
   if (stricmp(buffer,"none") == 0)
   {
      num = -2;
   }
   else
   {
      if (stricmp(buffer,"hole") == 0)
      {
         num = -1;
      }
      else
      {
         num = atoi(buffer);
         if (num < -2)
            num = -2;
         if (num > parent->getDML_MaxTexId())
            num = parent->getDML_MaxTexId();   
      }
   }
   tex.textureID = num;

   tex.flags = 0;
   if (tex.textureID >= 0)
   {
      // get the flip and rotate flags
      tex.flags = 0;
      if (SendMessage(GetDlgItem(getHandle(),CHK_ROTATE), BM_GETCHECK, 0, 0L))
         tex.flags |=  GridBlock::Material::Rotate;

      if (SendMessage(GetDlgItem(getHandle(),CHK_FLIP_X), BM_GETCHECK, 0, 0L))
         tex.flags |= GridBlock::Material::FlipX;

      if (SendMessage(GetDlgItem(getHandle(),CHK_FLIP_Y), BM_GETCHECK, 0, 0L))
         tex.flags |= GridBlock::Material::FlipY;
   }


   // update the selected tile's texture
   curTile = parent->getSelectedTile();
   tex.tile = *curTile;
   parent->setTileTexture(*curTile, tex);

   // update the selected tile's flat property
   parent->setTileFlat(*curTile, SendMessage(GetDlgItem(getHandle(),CHK_FLAT), BM_GETCHECK, 0, 0L)); 
   updateView();
}   

//------------------------------------------------------------------------------

void InspectDlg::onApplyStamp()
{
   char buffer[1024];
   int clamp;
   TextureInfo tex;
   Point2I *curTile;

   buffer[0] = '\0';
   // get the texture id
   GetDlgItemText(getHandle(), EDT_CLAMP, buffer, 1024);
   clamp = atoi(buffer);
   if (clamp < 0)
      clamp = 0;
   if (clamp > 7)
      clamp = 7;   

   parent->setStampClampValue(clamp);
   updateView();
}   