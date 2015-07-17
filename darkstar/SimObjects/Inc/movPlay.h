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
//------------------------------------------------------------------------------

#ifndef _MOVPLAY_H_
#define _MOVPLAY_H_


#include <windows.h>
#include <sim.h>

#include <strmif.h>  // active movie header
#include <control.h> // active movie header

class MovPlay : public SimObject
{
	typedef SimObject Parent;

private:
   static Int32 numInstantiated; // number times this object 
                                 // has been instantiated
public:
   enum State {
      NO_DEVICE, // active movie is not installed
      NO_MOVIE,  // no movie is opened
      STOPPED,   // movie is rewound to the start and ready to play
      PLAYING,
      PAUSED,
      COMPLETED, // movie has been played to the end or aborted by user
                 // by pressing ESC key.
   };

private:
   HMODULE hDll; // handle to the active movie dll (quartz.dll)
   IGraphBuilder *lpAMovie; // pointer to active movie COM object
   IMediaEvent *lpMEvent;   // pointer to active movie event queue
   State curState;

   HWND foregroundWin; // the window from which movis is opened

public:
   MovPlay();
   ~MovPlay();

   bool  isPlaying()   { return (getStatus() == PLAYING); }
   bool  isStopped()   { return (getStatus() == STOPPED); }
   bool  isPaused()    { return (getStatus() == PAUSED); }
   bool  isCompleted() { return (getStatus() == COMPLETED); }
   State getStatus();   

   bool openMovie(const char *movName); // puts the movie in STOPPED 
                                               // state if successful
   void closeMovie();
   
   void play();   // movie is played only if curState is PAUSED or STOPPED
   void playToCompletion(); 
   void pause();  // movie is paused only if curState is PLAYING
   void stop();   // movie is stopped and rewound to beginning

	bool onAdd();
};


#endif //_MOVPLAY_H_
