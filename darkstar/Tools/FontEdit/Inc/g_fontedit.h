//
// g_fontedit.h
//

#ifndef _G_FONTEDIT_H_
#define _G_FONTEDIT_H_

#include <g_font.h>

class GFXOldFont;

class GFXFontEdit : public GFXFont
{
  private:
	bool staticSized;
	int  firstASCII, lastASCII;
	int  maxCharBaseline;
	int  nextAvailableCharTableIndex;

  public:
	//DECLARE_PERSISTENT(GFXFontEdit);

	GFXFontEdit();
	GFXFontEdit(GFXFont *font);
	GFXFontEdit(GFXOldFont *oldFont);
	void defaultInit();

	void pack();
	void unpack(GFXFont *);
	void packBitmaps();
	void deleteChar(int ascii);
	Bool insertBitmap(int lookupValue, GFXBitmap *bitmap, int baseline);
	Bool place(GFXBitmap *bitmap, GFXCharInfo *charInfo);
	Bool appendBitmap(GFXBitmap *bitmap, int baseline = 0);
	void drawChar(GFXSurface *s, int index, Point2I *pos);
	void computeUsefulInformation();
	int  getJustification()  { return(fi.justification); }
	int	 getSpacing()        { return(fi.spacing); }
	int	 getNumChars()        { return(fi.numChars); }
	int  getBaseline()        { return(fi.baseline); }
	int  getMaxCharBaseline() { return(maxCharBaseline); }
	int  getFirstASCII();
	int  getLastASCII();
	void addASCII32(int width = -1);
	GFXCharInfo *getFontCharInfoOnly(int ascii);

	void setFlag(int flag, bool value);
	void toggleJustificationFlag(int flag);
	void setBaseline(int b) { fi.baseline = b;     }
	Bool setBaseline(int ascii, int newBaseline);
	Bool setASCII32Width(int newWidth);
	void setForeground(int color);
	void setBackground(int color);
	void setSpacing(int newSpacing);
	void setPalette(GFXPalette *newPalette);

	int  getCharTableIndex(int ascii);
	int  getCharTableLookupSize()  { return(charTableLookupSize); }
				    
	bool changeASCII(int oldASCII, int newASCII);
	int asciiOfIthChar(int i);
	int asciiOfPrevChar(int asciiOfCurrent, int amount);
	int asciiOfNextChar(int asciiOfCurrent, int amount);
    int findNextAvailableASCII(int ascii, int direction);
	
	int heightMetric() { return(getBaseline() + getMaxCharBaseline()); }
	GFXBitmap *getBitmap(int ascii);
    GFXBitmap *getBitmapAbsolute(int charTableIndex);
	static GFXFontEdit *load(const char *in_name);
	static GFXFontEdit *loadOldFont(const char *in_name);
	Bool GFXFontEdit::save(const char *in_name);
};

#endif