#include <fearguiircactivetextformat.h>
#include <console.h>
#include <g_surfac.h>
#include <fear.strings.h>

namespace FearGui
{
FGIRCActiveTextFormat::FGIRCActiveTextFormat()
{
   szAction[0] = '\0';
   active      = true;
}

FGIRCActiveTextFormat::~FGIRCActiveTextFormat()
{
}

// The "click action" string is basically a console callback - that is,
// some executable text that will be passed to the console and executed
// when this text format is clicked
void FGIRCActiveTextFormat::setClickAction(const char *lpszAction)
{
   strcpy(szAction, lpszAction);
}

bool FGIRCActiveTextFormat::isClickable()
{
   return (szAction[0] != '\0');
}

void FGIRCActiveTextFormat::setStateOver(bool fStateOver)
{
   stateOver = fStateOver;
}

void FGIRCActiveTextFormat::onMouseDown(const SimGui::Event &)
{
   if (stateOver && szAction[0] != '\0')
   {
      Console->evaluatef(szAction);
   }
}

void FGIRCActiveTextFormat::onRender(GFXSurface *sfc, Point2I offset, const Box2I &)
{
   GFXFont *font;

   if (mOrgTextPtr)
   {
      if (stateOver && szAction[0] != '\0')
      {
         sfc->drawRect2d(&RectI(offset.x, offset.y + 1, 
            offset.x + extent.x - 10, offset.y + extent.y - 1), 255);
      }

      offset.x += 4;
      offset.y -= 2;
      
      // Use a local buffer
      char *localBuffer = strnew(mOrgTextPtr);

      Vector<msgFormat>::iterator i;
      for (i = msgLines.begin(); i != msgLines.end(); i++)
      {
         font = fontArray[i->fontType].font;

         AssertFatal(font, "what happened to the font?");
      
         // Find out where to draw the text;
         Point2I textOffset = offset + i->offset;
      
         // Draw the text
         char *textPtr = localBuffer + i->startChar;
         if (i->length > 0)
         {
            char temp = textPtr[i->length];
            textPtr[i->length] = '\0';
         
            sfc->drawText_p(font, &textOffset, textPtr);
         
            textPtr[i->length] = temp;
         }
      }
   
      // Delete the local buffer
      delete [] localBuffer;
   
      // Now draw all the bitmaps
      for (int j = 0; j < mBitmapCount; j++)
      {
         Point2I bmpOffset = offset + mBitmaps[j].offset;
         sfc->drawBitmap2d(mBitmaps[j].bmp, &bmpOffset);
      }
   }
}
};
