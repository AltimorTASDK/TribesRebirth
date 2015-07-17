- 4/24/96
	- Added default OnSysKey handler to the GWCanvas class to
	  handle switching in and out of DirectDraw.
- 4/11/96
	- Added flags (VKey/scancode?) to onChar() event method.

- 4/8/96
	- Added shellapi.h in gwbase.h on request.
	- Added setSize method to GWWindow basse class.
	- Removed setPalette() from GWCanvas.  Palettes should be set
	  directly on the surface.
	- Cleaned up some of the DirectDraw support. This still needs some
	  work, but runs for now.
	- Only example.cpp works at the moment. Whello.cpp uses the dialog
	  class, but I haven't tried to build it in a while.