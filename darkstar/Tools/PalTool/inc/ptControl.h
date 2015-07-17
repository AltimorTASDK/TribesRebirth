//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _PTCONTROL_H_
#define _PTCONTROL_H_


#include <ptDefines.h>
#include <gw.h>
#include <tBitVector.h>


struct Selection
{
   BitVector from;
   BitVector from_copy;
   BitVector to;
   BitVector to_copy;
   int from_anchor;
   int to_anchor;
};   



#define SHADE_MODE 0
#define HAZE_MODE  1


//------------------------------------------------------------------------------
class PTControl
{
protected:
   bool focus;

public:
   PTControl()  { focus = false; }
   virtual ~PTControl() {}
   virtual void render(GFXSurface *surface) {}
	virtual void onMouseMove(int x, int y, UINT keyFlags) {}
	virtual void onLButtonUp(int x, int y, UINT keyFlags) {}
	virtual void onRButtonUp(int x, int y, UINT keyFlags) {}
	virtual void onLButtonDown(BOOL fDoubleClick, int x, int y, UINT keyFlags) {}
	virtual void onRButtonDown(BOOL fDoubleClick, int x, int y, UINT keyFlags) {}
};   



//------------------------------------------------------------------------------
class PaletteGrid: public PTControl
{
protected:
   enum Constants {
   };
   bool focus;
   int iOver;
   int iShadeHaze;
   int fromColor;
   int toColor;
   int shColor;
   bool lbutton;
   bool rbutton;

   int getOver(int x, int y);

public:
   Selection selection;

   PaletteGrid();
   
   void zeroSelection();
   void setToSelection(BitVector &to)     { selection.to = to; }
   void setFromSelection(BitVector &from) { selection.from = from; }
   bool rangeDefined()  { return selection.from.test() && selection.to.test(); }
   void setOver(int sh) { if (sh != -1) iShadeHaze = sh; }
   int  getOver()       { return ((iOver!=-1) ? iOver : iShadeHaze); }
   int  getAnchor()     { return (selection.from_anchor == -1) ? 0 : selection.from_anchor; }

   void render(GFXSurface *surface);
	void onMouseMove(int x, int y, UINT keyFlags);
	void onLButtonUp(int x, int y, UINT keyFlags);
	void onRButtonUp(int x, int y, UINT keyFlags);
	void onLButtonDown(BOOL fDoubleClick, int x, int y, UINT keyFlags);
	void onRButtonDown(BOOL fDoubleClick, int x, int y, UINT keyFlags);
};   



//------------------------------------------------------------------------------
class PaletteShade: public PTControl
{
protected:
   enum Constants {
   };
   bool focus;
   Point2I ptOver;
   int iOver;
   int iSelect;
   int iSelectColor;
   bool doHaze;
   bool doShade;
   bool change;
   GFXPalette *pal;
   
   void onOver(int, int y);

public:
   PaletteShade();
   void setSelect(int select);
   void setSelectColor(int select);
   void setShade(bool b);
   void setHaze(bool b);
   int  getOver() { return (iOver); }
   void render(GFXSurface *surface);
	void onMouseMove(int x, int y, UINT keyFlags);
	void onLButtonUp(int x, int y, UINT keyFlags);
	void onRButtonUp(int x, int y, UINT keyFlags);
	void onLButtonDown(BOOL fDoubleClick, int x, int y, UINT keyFlags);
	void onRButtonDown(BOOL fDoubleClick, int x, int y, UINT keyFlags);
};   

inline void PaletteShade::setSelect(int select) 
{ 
   change  |= (iSelect != select);
   iSelect = select; 
}
inline void PaletteShade::setSelectColor(int select) 
{ 
   change  |= (iSelect != select);
   iSelectColor = select; 
}

inline void PaletteShade::setShade(bool b)      
{ 
   change  |= (doShade != b);
   doShade = b; 
}

inline void PaletteShade::setHaze(bool b)       
{ 
   change |= (doHaze != b);
   doHaze = b; 
}      



//------------------------------------------------------------------------------
class SelectBar: public PTControl
{
protected:
   enum Constants {
      COLOR = 249,
   };
   bool focus;
   int iSelect;

public:
   SelectBar();
   int  getSelect() { return iSelect; }
   void render(GFXSurface *surface);
	void onMouseMove(int x, int y, UINT keyFlags);
	void onLButtonUp(int x, int y, UINT keyFlags);
	void onLButtonDown(BOOL fDoubleClick, int x, int y, UINT keyFlags);
};   



#endif //_PTCONTROL_H_
