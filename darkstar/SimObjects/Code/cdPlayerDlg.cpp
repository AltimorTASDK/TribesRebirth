//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "cdPlayerDlg.h"
#include "cdPlayerDlg_defs.h"
#include "console.h"
//------------------------------------------------------------------------------
CDPlayerDlg::CDPlayerDlg()
{
   pRb = NULL;
}

//--------------------------------------------------------------------------------
void CDPlayerDlg::onDestroy()
{
   if (pRb)
      manager->deleteObject(pRb);
   manager->deleteObject(this);
}

//------------------------------------------------------------------------------
void CDPlayerDlg::render()
{
   if (pRb == NULL)
      return;

   // update the track and time display
   SetDlgItemText(getHandle(), TXT_DISPLAY, avar("Track %d, Time %f", curTrack, pRb->getTrackPosition()));
   // update from the volume bar
   pRb->setVolume( (Flt32)SendDlgItemMessage(getHandle(), VOLUME_BAR, TBM_GETPOS, 0, 0) / 1000.0f);
}

//--------------------------------------------------------------------------------
void CDPlayerDlg::onCommand( int id, HWND hwndCtl, UINT codeNotify )
{
   UInt32 numTracks;

   hwndCtl;
   codeNotify;
   
   if (pRb == NULL)
      return;

   if (pRb->getStatus() != Redbook::NO_DEVICE)
   {
      numTracks = pRb->getTrackCount();

      curTrack = (curTrack < 1) ? 1 : curTrack;
      curTrack = (UInt32(curTrack) > numTracks) ? 1 : curTrack;
      
      switch( id )
      {
         case BTN_STOP:
            pRb->stop();
            break;
         
         case BTN_PLAY:
            if (pRb->getStatus() == Redbook::PAUSED)
               pRb->resume();
            else
               pRb->play(curTrack);
            break;

         case BTN_PREV:
            curTrack--;
            curTrack = (curTrack < 1) ? numTracks : curTrack;
            if (pRb->getStatus() == Redbook::PLAYING)
               pRb->play(curTrack);
            else
               pRb->stop();
            break;

         case BTN_NEXT:
            curTrack++;
            curTrack = (UInt32(curTrack) > numTracks) ? 1 : curTrack;
            if (pRb->getStatus() == Redbook::PLAYING)
               pRb->play(curTrack);
            else
               pRb->stop();
            break;
         
         case BTN_PAUSE:
            if (pRb->getStatus() == Redbook::PAUSED)
               pRb->resume();
            else
               pRb->pause();
            break;

         case BTN_EJECT:
            if (pRb->getStatus() == Redbook::TRAY_OPENED)
               pRb->retract();
            else
               pRb->eject();
            break;
      }
   }

   render();
}   

bool CDPlayerDlg::processEvent(const SimEvent *event)
{
   switch (event->type) {
      case SimMessageEventType:
         // update displays every 0.3 seconds 
         render();
         SimMessageEvent::post(this, manager->getCurrentTime() + SimTime(0.3f), 0);
			return true;
	}
   return Parent::processEvent(event);
}  
 
bool CDPlayerDlg::onAdd()
{
	if (!Parent::onAdd())
		return false;
   if ( create("IDD_CDPLAYER_DLG", NULL ) )
   {
      pRb = new Redbook;

      manager->addObject(pRb);
      pRb->open(NULL);
      curTrack = 1;

      // set the range of the volume bar to 0-1000
      SendDlgItemMessage(getHandle(), VOLUME_BAR, TBM_SETRANGE, 1, MAKELONG(0,1000));
      SendDlgItemMessage(getHandle(), VOLUME_BAR, TBM_SETPOS, 1, (LONG)(pRb->getVolume() * 1000.0f));

      // send SimMessageEventType in 0.3 seconds to update the displays
      SimMessageEvent::post(this, manager->getCurrentTime() + SimTime(0.3f), 0);
		return true;
   }
	Console->printf("CDPlayerDlg::onAdd: Could not create main window (missing cdPlayer.RC?)");
	return false;
 }	
