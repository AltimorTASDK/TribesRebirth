//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMGUITEXTWRAPCTRL_H_
#define _SIMGUITEXTWRAPCTRL_H_

#include <inspect.h>
#include "simGuiActiveCtrl.h"
#include <g_font.h>

namespace SimGui
{

void wrapText(GFXFont *font, const char *txt, Int32 lineWidth, Vector<Int32> &startLineOffset, Vector<Int32> &lineLen);

class DLLAPI TextWrap : public ActiveCtrl
{
private:
   typedef ActiveCtrl Parent;

protected:
   Int32 fontNameTag;
   Int32 fontNameTagDisabled;

   // if textTag is > 0, the text displayed by textWrap
   // is retrieved through SimTagDictionary 
   Int32 textTag;

   Resource<GFXFont> hFont;
   Resource<GFXFont> hFontDisabled;

   char *text;
   Int32 totalTextLen;
   Int32 lineSpacing;

   Vector<Int32> startLineOffset;
   Vector<Int32> lineLen;

   bool enabled;
   bool ignoreClick;

   Point2I inset;

   void variableChanged(const char *newValue);

public:   
   enum AlignmentType {
      LeftJustify,
      RightJustify,
      CenterJustify,                              
   };

   AlignmentType alignment;
   Int32 textVPosDelta;

   TextWrap();

   bool pointInControl(Point2I &parentCoordPoint);

   void ignoreMouseClick(bool tf) {ignoreClick = tf;}
   void enable(bool tf) { enabled = tf; setUpdate();}

   void setFont(Int32 tag);
   void setFontDisabled(Int32 tag);

   void setText(const char *txt = NULL);
   char *getText() { return text; }
   void setLineSpacing(Int32 spacing) {lineSpacing = spacing;}

   Int32 getFontTag() { return fontNameTag; }
   Int32 getMinYExtent() { int numLines = startLineOffset.size();
                           if (!numLines || !(bool)hFont) return 0;
                           return (numLines * hFont->getHeight() + (numLines - 1)*lineSpacing) + inset.y + inset.y; }

   void onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect);
   void setUpdate();
   void editResized();

   bool onAdd();
   void onRemove();
   void resize(const Point2I &newPosition, const Point2I &newExtent);

   void inspectRead(Inspect *insp);
   void inspectWrite(Inspect *insp);

   DECLARE_PERSISTENT(TextWrap);
   Persistent::Base::Error write( StreamIO &sio, int, int );
   Persistent::Base::Error read( StreamIO &sio, int, int );
};

};

#endif //_SIMGUITEXTWRAPCTRL_H_
