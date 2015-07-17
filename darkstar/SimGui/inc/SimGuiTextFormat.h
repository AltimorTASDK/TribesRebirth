//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMGUI_TEXT_FORMAT_H_
#define _SIMGUI_TEXT_FORMAT_H_

#include "g_font.h"
#include "simGuiActiveCtrl.h"

namespace SimGui
{

// abstract class to serve primary needs of buttons
class TextFormat : public ActiveCtrl
{
   private:
      typedef ActiveCtrl Parent;

   protected:
		#define MAX_NUM_FONTS 10
		struct FontItem {
         Resource<GFXFont> font;
         Int32 tag;
      };

		FontItem fontArray[MAX_NUM_FONTS];

		int mWidth;
		int mMinWidth;
		int mHeight;

		//formatting vars
	   GFXFont *curFont;
	   Point2I curOffset;
	   int spacing;
	   int hardLeftEdge;
	   int hardRightEdge;
	   int leftEdge;
	   int rightEdge;
	   int bmpLeftBottom;
	   int bmpRightBottom;

		void carriageReturn(int width);
		bool beyondBitmap();


	public:
		enum justification
		{
			JUST_LEFT = 0,
			JUST_CENTER,
			JUST_RIGHT
		};

		struct msgFormat
		{
			BYTE fontType;
			BYTE justification;
			Point2I offset;
			int lineWidth;
			int startChar;
			int length;
		};

	protected:
		bool mbUseLocalCopy;
		const char *mOrgTextPtr;
		Vector<msgFormat> msgLines;

		#define MAX_NUM_BITMAPS 8
		int mBitmapCount;

		struct bmpFormat
		{
			Resource<GFXBitmap> bmp;
			Point2I offset;
			BYTE justification;
			int width;
		};
		bmpFormat mBitmaps[MAX_NUM_BITMAPS];

   protected:
      bool onAdd();
      void init();

	public:
		TextFormat(void);
      ~TextFormat();

		void setFont(int index, Int32 tag);
		void setFont(int index, const char *fontName);

		void formatControlString(const char *text, int width, bool reformatOldOnly = false, bool copyString = TRUE);

		int getWidth(void);
		int getMinWidth(void);
		virtual int getHeight(void);

	   void onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect);

	   DECLARE_PERSISTENT(TextFormat);
		Persistent::Base::Error write( StreamIO &sio, int a, int b);
		Persistent::Base::Error read( StreamIO &sio, int a, int b);

};

};

#endif //_SIMGUI_TEXT_FORMAT_H_
