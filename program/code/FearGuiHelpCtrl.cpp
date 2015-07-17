#include "g_surfac.h"
#include "g_bitmap.h"
#include "inspect.h"
#include "editor.strings.h"
#include "fear.strings.h"
#include "r_clip.h"
#include "console.h"
#include "simResource.h"
#include "simGuiTextFormat.h"
#include "fearGuiShellPal.h"
#include "fearGuiHelpCtrl.h"

namespace FearGui
{

FGHelpCtrl::FGHelpCtrl(void)
{
   helpPanel.setFont(0, IDFNT_9_STATIC);
   helpPanel.setFont(1, IDFNT_9_STANDARD);
   mHelpText = NULL;
   
   //create the pref
   if (! stricmp(Console->getVariable("pref::HelpPopups"), ""))
   {
      Console->setVariable("pref::HelpPopups", "TRUE");
   }
}

FGHelpCtrl::~FGHelpCtrl(void)
{
   if (mHelpText)
   {
      delete [] (char*)mHelpText;
      mHelpText = NULL;
   }
}

void FGHelpCtrl::setHelpTag(Int32 helpTag, float timeElapsed, bool mouseClicked)
{
   //make sure we have your root canvas
   if (! root) return;
   
   //see if help should be repressed
   if (! stricmp(Console->getVariable("pref::HelpPopups"), "FALSE"))
   {
      mHelpTag = 0;
      return;
   }
   
   if (mouseClicked)
   {
      helpTag = 0;
   }
   
   //make sure at least 500ms have elapsed
   if (timeElapsed < 1069) helpTag = 0;
   
   //see if the help tag changed
   if (helpTag != mHelpTag)
   {
      //erase the old help tag panel
      if (mHelpTag)
      {
         if (mHelpText)
         {
            delete [] (char*)mHelpText;
            mHelpText = NULL;
         }
         setUpdate();
      }
      
      if (helpTag)
      {
         //setup the help panel
         const char *helpText = SimTagDictionary::getString(manager, helpTag);
         
         if ((! helpText) || (! helpText[0]))
         {
            helpTag = 0;
         }
         else
         {
            char buf[8];
            const char *temp = strchr(helpText, ',');
            if (temp && temp[1])
            {
               int length = min(7, temp - helpText);
               strncpy(buf, helpText, length);
               buf[length] = '\0';
               
               int tempWidth = atoi(buf);
               if (tempWidth < 0) tempWidth = 200;
               else tempWidth = min(tempWidth, 256);
               
               //format the string
               helpPanel.formatControlString(&temp[1], tempWidth - 6, FALSE, FALSE);
               tempWidth = min(tempWidth, helpPanel.getMinWidth() + 6);
               
               //set the position and extent
               extent.set(tempWidth, min(192, helpPanel.getHeight() + 6));
               
               Point2I pos = root->getCursorPos();
               GFXBitmap *bmp = root->getCursorBmp();
               GFXSurface *sfc = root->getSurface();
               if (bmp) pos.y += bmp->getHeight();
               
               //make sure it doesn't draw off the bottom edge of the screen
               if (sfc)
               {               
                  if (pos.x + extent.x > (sfc->getWidth() - 3))
                  {
                     pos.x -= (pos.x + extent.x - (sfc->getWidth() - 3));
                  }
                  if (pos.y + extent.y > (sfc->getHeight() - 3))
                  {
                     pos.y -= (pos.y + extent.y - (sfc->getHeight() - 3));
                  }
               }
               position.set(pos.x, pos.y);
            }
            else helpTag = 0;
         }
      }
      
      //set the prev
      mHelpTag = helpTag;
   }
   
   //make sure the update region gets set
   if (helpTag)
   {
      setUpdate();
   }
}

void FGHelpCtrl::setHelpText(const char *text, float /*timeElapsed*/, bool /*mouseClicked*/)
{
   Point2I     pos = root->getCursorPos();
   GFXBitmap  *bmp = root->getCursorBmp();
   GFXSurface *sfc = root->getSurface();

   if (mHelpText)
   {
      delete [] (char*)mHelpText;
      mHelpText = NULL;
   }

   if (text)
   {
      mHelpTag = 0;
      mHelpText = strnew(text);

      helpPanel.formatControlString(mHelpText, 250);

      if (bmp)
      {
         pos.y += bmp->getHeight();
      }

      if (sfc)
      {               
         if (pos.x + extent.x > (sfc->getWidth() - 3))
         {
            pos.x -= (pos.x + extent.x - (sfc->getWidth() - 3));
         }
         if (pos.y + extent.y > (sfc->getHeight() - 3))
         {
            pos.y -= (pos.y + extent.y - (sfc->getHeight() - 3));
         }
      }
      position.set(pos.x, pos.y);
      extent.set(helpPanel.getMinWidth() + 6, helpPanel.getHeight() + 6);
   }
   setUpdate();
}

void FGHelpCtrl::render(GFXSurface *sfc)
{
   if ((! mHelpTag) && ((! mHelpText) || (! mHelpText[0]))) return;
   
   // Opaque
   sfc->drawRect2d_f(&RectI(position, 
      Point2I(position.x + extent.x - 1, 
      position.y + extent.y - 1)), GREEN_40);

   // Border
   sfc->drawRect2d(&RectI(position, 
      Point2I(position.x + extent.x - 1, 
      position.y + extent.y - 1)), GREEN_78);
         
   //render the help panel
   Point2I tempPos = position;
   tempPos += 3;
   
   helpPanel.onRender(sfc, tempPos, Box2I(&position, &extent));
}

IMPLEMENT_PERSISTENT_TAG( FearGui::FGHelpCtrl, FOURCC('F','G','h','x') );

};