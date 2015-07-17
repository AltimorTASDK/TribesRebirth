//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
// MovPlay is a wrapper around Active Movie 1.0
//
// Currently, MovPlay will take over the entire screen and play
// the specified movie until the movie is completed, the user
// presses Esc, or the user presses the Alt-Tab combo.  When the
// movie is completed or aborted by the user, MovPlay releases the
// screen and gives control back to the caller process.
//
// MovPlay should not be called when the caller process is in
// DirectDraw mode.
//
//------------------------------------------------------------------------------

#include <movPlay.h>

#include <evcode.h> // active movie header
#include "console.h"

// FilterGraph e436ebb3-524f-11ce-9f53-0020af0ba770      
const GUID CLSID_FilterGraph           = {0xe436ebb3,0x524f,0x11ce,0x9f,0x53,0x00,0x20,0xaf,0x0b,0xa7,0x70};
// IID_IGraphBuilder 56a868a9-0ad4-11ce-b03a-0020af0ba770
const GUID IID_IGraphBuilder           = {0x56a868a9,0x0ad4,0x11ce,0xb0,0x3a,0x00,0x20,0xaf,0x0b,0xa7,0x70};
// IID_IMediaControl 56a868B1-0ad4-11ce-b03a-0020af0ba770
const GUID IID_IMediaControl           = {0x56A868B1,0x0AD4,0x11CE,0xB0,0x3A,0x00,0x20,0xAF,0x0B,0xA7,0x70};
// IID_IMediaPosition 56a868B2-0ad4-11ce-b03a-0020af0ba770
const GUID IID_IMediaPosition          = {0x56A868B2,0x0AD4,0x11CE,0xB0,0x3A,0x00,0x20,0xAF,0x0B,0xA7,0x70};
// IID_IVideoWindow 56a868B4-0ad4-11ce-b03a-0020af0ba770
const GUID IID_IVideoWindow            = {0x56A868B4,0x0AD4,0x11CE,0xB0,0x3A,0x00,0x20,0xAF,0x0B,0xA7,0x70};
// IID_IMediaEvent 56a868B6-0ad4-11ce-b03a-0020af0ba770
const GUID IID_IMediaEvent             = {0x56A868B6,0x0AD4,0x11CE,0xB0,0x3A,0x00,0x20,0xAF,0x0B,0xA7,0x70};


//------------------------------------------------------------------------------
Int32 MovPlay::numInstantiated = 0;

//------------------------------------------------------------------------------
MovPlay::MovPlay()
{
   curState = NO_DEVICE;
   lpAMovie = NULL;
   foregroundWin = NULL;
   if (numInstantiated == 0)
   {
      if (! SUCCEEDED(CoInitialize(NULL)) )
         AssertFatal(0,"MovPlay::MovPlay() - could not initialize COM sub system");
   }
   numInstantiated++;
}   

//------------------------------------------------------------------------------
MovPlay::~MovPlay()
{
   closeMovie();
   numInstantiated--;
   if (numInstantiated == 0)
   {
      CoUninitialize();
   }
}   

//------------------------------------------------------------------------------
bool MovPlay::onAdd()
{
	if (!Parent::onAdd())
		return false;
   hDll = LoadLibrary("quartz.dll");
   if ( hDll )
   {
      // Determine if ActiveMovie is installed
      IGraphBuilder *lpgb;
      HRESULT hr = CoCreateInstance(CLSID_FilterGraph,0,CLSCTX_INPROC_SERVER,IID_IGraphBuilder,(void **)&lpgb);

      if (FAILED(hr)) 
      {
         Console->printf("Active Movie Is NOT Installed!!!");
         return true;
      }

      lpgb->Release();
      curState = NO_MOVIE;
	   return true;
   }
	Console->printf("MovPlay::onAdd: Could not load quatz.dll - active movie component");
	return false;
}   

//------------------------------------------------------------------------------
bool MovPlay::openMovie(const char *movName) 
{
   if (curState != NO_MOVIE)
      return false;
   
   HRESULT hr;   

   // get interface for the Active Movie graph builder   
   hr = CoCreateInstance(CLSID_FilterGraph,0,CLSCTX_INPROC_SERVER,IID_IGraphBuilder,(void **)&lpAMovie);
   if (FAILED(hr)) 
      return false;
   
   // init event notification
   hr = lpAMovie->QueryInterface(IID_IMediaEvent, (void **) &lpMEvent);
   if (FAILED(hr))
      return false;

   // open the movie
   WCHAR wPath[MAX_PATH];

   MultiByteToWideChar( CP_ACP, 0, movName, -1, wPath, MAX_PATH );
   hr = lpAMovie->RenderFile(wPath, NULL);  
   if (FAILED(hr))
      return false;

   // remember the current foreground window so we
   // can set focus on it again after we finish the
   // movie
   foregroundWin = GetForegroundWindow();
   
   // go full screen!  
   IVideoWindow *pivw;

   hr = lpAMovie->QueryInterface(IID_IVideoWindow, (void **)&pivw);
   if (FAILED(hr)) 
      return false;
      
   pivw->put_FullScreenMode(-1);
   pivw->Release();

   curState = STOPPED;

   return true;
}   

//------------------------------------------------------------------------------
void MovPlay::closeMovie()
{
   if (lpAMovie == NULL)
      return;

   stop();

   // reduce from full screen
   IVideoWindow *pivw;
   HRESULT hr = lpAMovie->QueryInterface(IID_IVideoWindow, (void **)&pivw);
   if (SUCCEEDED(hr)) 
   {
      pivw->put_FullScreenMode(0);
      pivw->Release();
   }

   // release movie
   curState = NO_MOVIE;

   if (lpMEvent) 
   {
      lpMEvent->Release();
      lpMEvent = NULL;
   }

   if (lpAMovie) 
   {
      lpAMovie->Release();
      lpAMovie = NULL;
   }
   
   // restore state of the window which last had focus
   if (foregroundWin)
   {
      SetForegroundWindow(foregroundWin);
      foregroundWin = NULL;
   }
}   

//------------------------------------------------------------------------------
MovPlay::State MovPlay::getStatus()
{
   if ( (curState == NO_DEVICE) || (curState == NO_MOVIE) || (curState == COMPLETED) )
   {
      return curState;
   }

   // check if the movie has been played to the end or
   // if the user has aborted it by presseing ESC key.
   long lEventCode, lParam1, lParam2;
         
   HRESULT hr = lpMEvent->GetEvent(&lEventCode, &lParam1, &lParam2, 0);
      
   if (SUCCEEDED(hr))
   {
      switch(lEventCode)
      {
         case EC_COMPLETE:
         case EC_USERABORT:
         case EC_ERRORABORT:
         case EC_FULLSCREEN_LOST: 
            stop();
            curState = COMPLETED;
      }
   }

   if (0x01 && GetAsyncKeyState(VK_ESCAPE))
   {
      stop();
      curState = COMPLETED;
   }

   return curState;
}

//------------------------------------------------------------------------------
void MovPlay::play()
{
   if ((getStatus() != STOPPED) && (getStatus() != PAUSED))
      return;

	IMediaControl *pMC;
	HRESULT hr = lpAMovie->QueryInterface(IID_IMediaControl, (void **) &pMC);

   // try to play the movie
   if (SUCCEEDED(hr))
   {
      hr = pMC->Run();
	   pMC->Release();

      if (SUCCEEDED(hr))
         curState = PLAYING;
   }

   if (curState != PLAYING)
   {
      AssertWarn(0, "MovPlay::play() - could not play movie");
      curState = COMPLETED;
   }
}

//------------------------------------------------------------------------------
void MovPlay::playToCompletion()
{
   play();
   if (curState == PLAYING)
   {
      while (getStatus() != COMPLETED)
         ;
   }
}   
 
//------------------------------------------------------------------------------
void MovPlay::pause()
{
   if (! isPlaying())
      return;

	IMediaControl *pMC;
	HRESULT hr = lpAMovie->QueryInterface(IID_IMediaControl, (void **) &pMC);

   if (SUCCEEDED(hr))
   {
      pMC->Pause();
	   pMC->Release();

      curState = PAUSED;
   }
   else
   {
      AssertWarn(0, "MovPlay::pause() - can't pause the movie");
   }
}

//------------------------------------------------------------------------------
void MovPlay::stop()
{
   if ((getStatus() == NO_DEVICE) || (getStatus() == NO_MOVIE))
      return;

	IMediaControl *pMC;
	HRESULT hr = lpAMovie->QueryInterface(IID_IMediaControl, (void **) &pMC);

   if (SUCCEEDED(hr))
   {
      pMC->Stop();
	   pMC->Release();

      curState = STOPPED;

      // rewind the movie back to the beginning
      IMediaPosition * pMP;
	   hr = lpAMovie->QueryInterface(IID_IMediaPosition, (void**) &pMP);
	   if (SUCCEEDED(hr)) {
		   pMP->put_CurrentPosition(0);
		   pMP->Release();
      }
   }
   else
   {
      AssertWarn(0, "MovPlay::stop() - can't stop the movie");
   }
}

