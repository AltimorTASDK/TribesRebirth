//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMGUITEXTEDIT_H_
#define _SIMGUITEXTEDIT_H_

#include "simGuiSimpleText.h"

namespace SimGui
{

class DLLAPI TextEdit : public SimpleText
{
private:
   typedef SimpleText Parent;

   // max string len, must be less then or equal to Inspect::MAX_STRING_LEN
protected:
   Int32 maxStrLen;
   Int32 highlightColor;
      
   // for animating the cursor
   Int32 numFramesElapsed;
   SimTime timeLastCursorFlipped;
	int cursorColor;
   int cursorOn;
   bool insertOn;
   bool scrollOn;
   int mouseDragStart;
   Point2I textOffset;
   RectI textClipRect;
   bool dragHit;
	int scrollDir;

	bool mbNumbersOnly;

   //undo members
   char undoText[SimpleText::MAX_STRING_LENGTH + 1];
   int undoBlockStart;
   int undoBlockEnd;
   int undoCursorPos;
   void saveUndoState(void);

   int setCursorPos(const Point2I &offset);

   int blockStart;
   int blockEnd;
   int cursorPos;

public:   
   TextEdit();

	int getCursorColor(void) { return cursorColor; }
	void setCursorColor(int color) { cursorColor = color; }

   void setHighlightColor(int color) { highlightColor = color;}
                              // draws a background rect around the highlighted text
                              // if highlightColor is nonzero

   void getText(char *dest);  // dest must be of size
                                      // StructDes::MAX_STRING_LEN + 1

   void setText(Int32 tag);
   void setText(const char *txt);

   bool becomeFirstResponder();
   void onKeyDown(const Event &event);
   void onKeyRepeat(const Event &event);
   void onKeyUp(const Event &event);
   void onMouseDown(const Event &event);
   void onMouseDragged(const Event &event);
   void onMouseUp(const Event &event);

	void parentResized(const Point2I &oldParentExtent, const Point2I &newParentExtent);
   void setTextClipRect(const RectI &clipRect);
   void onPreRender();
   bool hasText();

   void onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect);
   virtual void DrawText(GFXSurface *sfc);

   void inspectRead(Inspect *inspector);
   void inspectWrite(Inspect *inspector);

   DECLARE_PERSISTENT(TextEdit);
   Persistent::Base::Error read( StreamIO &sio, int version, int user);
   Persistent::Base::Error write( StreamIO &sio, int version, int user);
};

};

#endif //_SIMGUITTEXTEDIT_H_
