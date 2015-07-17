//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMGUISIMPLETEXT_H_
#define _SIMGUISIMPLETEXT_H_

#include <inspect.h>
#include "simGuiActiveCtrl.h"
#include "g_font.h"

namespace SimGui
{

// SimpleText holds two fonts, one for displaying text normally
// and one displaying the text highlighted.
// Although SimpleText only uses the normal font for display, 
// the second font is provided so that subclasses of SimpleText
// can take advantage of it.
class DLLAPI SimpleText : public ActiveCtrl
{
private:
   typedef ActiveCtrl Parent;

protected:
   Int32 fontNameTag;
   Int32 fontNameTagHL;
   Int32 fontNameTagDisabled;

   // if textTag is > 0, the text displayed by SimpleText
   // is retrieved through SimTagDictionary 
   Int32 textTag;

   Resource<GFXFont> hFont;
   Resource<GFXFont> hFontHL;
   Resource<GFXFont> hFontDisabled;

public:
	//used to be Inspect::MAX_STRING_LEN  which is only 80.
	//Note all persist methods must still used Inspect::MAX_STRING_LEN
	enum Constants
	{
      MAX_STRING_LENGTH = 255,		
	};

protected:
   char text[MAX_STRING_LENGTH + 1];

   // variable to offset the local vertical position of text 
   Int32 textVPosDelta;

public:   
   enum AlignmentType {
      LeftJustify,
      RightJustify,
      CenterJustify,
   };

   AlignmentType alignment;
   SimpleText();

   const char *getScriptValue();
   void setScriptValue(const char *value);

   virtual void setFont(Int32 tag);
   virtual void setFontHL(Int32 tag);
   virtual void setFontDisabled(Int32 tag);

   inline const Resource<GFXFont> &getFont()         const { return hFont;         };
   inline const Resource<GFXFont> &getFontHL()       const { return hFontHL;       };
   inline const Resource<GFXFont> &getFontDisabled() const { return hFontDisabled; };

   virtual void setText(const char *txt = NULL);

   void setTextVDelta(Int32 txtVDelta) {textVPosDelta = txtVDelta;}
   const char *getText() {return text;}

   void onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect);

   bool onAdd();

   virtual void variableChanged(const char *newValue);

   void inspectRead(Inspect *insp);
   void inspectWrite(Inspect *insp);

   DECLARE_PERSISTENT(SimpleText);
   Persistent::Base::Error write( StreamIO &sio, int, int );
   Persistent::Base::Error read( StreamIO &sio, int, int );
};

};

#endif //_SIMGUISIMPLETEXT_H_
