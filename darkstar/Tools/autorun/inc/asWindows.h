//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _ASWINDOWS_H_
#define _ASWINDOWS_H_


#include <gw.h>



//--------------------------------------
class BaseWindow: public GWWindow
{
private:
   typedef GWWindow Parent;
   static VectorPtr<BaseWindow*> window;

protected:
   const char *name;
   HICON hIcon;

public:
   BaseWindow();
   ~BaseWindow();
   void setIcon(const char*);
   static BaseWindow* findWindow(const char *name);
};


//--------------------------------------
class TitleWindow: public BaseWindow
{
private:
   typedef BaseWindow Parent;

public:
   TitleWindow();

   bool create(GWWindow* parent, const char* title, const char* name, 
      DWORD exstyle, DWORD style,  
      const Point2I *pos, const Point2I *size, int id, HMENU hMenu);
   virtual void onCommand(int id, HWND hwndCtl, UINT codeNotify);
   virtual void onDrawItem(const DRAWITEMSTRUCT * lpDrawItem);
   virtual void onPaint();
};


//--------------------------------------
class ButtonControl: public BaseWindow
{
private:
   typedef BaseWindow Parent;

   const char *function;
   const char *title;
   HFONT hFont;

public:
   ButtonControl();
   ~ButtonControl();

   bool create(GWWindow* parent, const char *title, const char *name, 
      const Point2I *pos=NULL, const Point2I *size=NULL);
   void execute();

   virtual void setPosition(Point2I pos);
};


//--------------------------------------
class BitmapControl: public BaseWindow
{
private:
   typedef BaseWindow Parent;

   const char *filename;
   BITMAPFILEHEADER fileHeader;
   BITMAPINFOHEADER infoHeader;
   BYTE *bits;
   BYTE *transparent;
   Point2I size;   
   int  imageSize;
   BYTE transparentR;
   BYTE transparentG;
   BYTE transparentB;
   HPALETTE hPalette;

   bool load(const char *filename, int R, int G, int B, bool zero);
   void unload();

public:
   BitmapControl();
   ~BitmapControl();

   bool create(GWWindow* parent, const char *name, const char *bitmap,  
      int R, int G, int B, bool zero, const Point2I &pos);

   virtual void onPaint();
};







#endif //_ASWINDOWS_H_
