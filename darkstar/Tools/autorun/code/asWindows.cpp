//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <asWindows.h>
#include <console.h>

void warning(const char *msg, ...);


////////////////////////////////////////////////////////////////////////////////
VectorPtr<BaseWindow*> BaseWindow::window;

//------------------------------------------------------------------------------
BaseWindow::BaseWindow()
{
   window.push_back(this);   
   hIcon = 0;
}  

//------------------------------------------------------------------------------
BaseWindow::~BaseWindow()
{
   VectorPtr<BaseWindow*>::iterator i = window.begin();   
   for (; i != window.end(); i++)
      if (*i == this)
      {
         window.erase(i);
         return;
      }
   if(hIcon)
      DestroyIcon(hIcon);
}  

//------------------------------------------------------------------------------
void BaseWindow::setIcon(const char* name)
{
   if (hIcon)   
   {
      DestroyIcon(hIcon);
      hIcon = 0;
   }

   hIcon = (HICON)LoadImage(hInstance, name, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE|LR_LOADFROMFILE);
   if (hIcon)   
   {
      SetClassLong(hWnd, GCL_HICONSM, (LONG)hIcon);
   }
}   

//------------------------------------------------------------------------------
BaseWindow* BaseWindow::findWindow(const char *name)
{
   VectorPtr<BaseWindow*>::iterator i = window.begin();   
   for (; i != window.end(); i++)
      if (stricmp(name, (*i)->name) == 0)
         return (*i);
   return (NULL);
}   



////////////////////////////////////////////////////////////////////////////////
TitleWindow::TitleWindow()
{
   postQuit = true;
}   


//------------------------------------------------------------------------------
bool TitleWindow::create(GWWindow* parent, const char *title, const char *_name, 
      DWORD exstyle, DWORD style, 
      const Point2I *pos, const Point2I *size, int id, HMENU hMenu)
{
   GWWindowClass gwwc;
   gwwc.registerClass(CS_BYTEALIGNCLIENT | CS_HREDRAW | CS_VREDRAW,
      0,LoadCursor(NULL,IDC_ARROW),GetSysColorBrush(COLOR_MENU));

   name = stringTable.insert(_name);
   return Parent::createEx(exstyle, &gwwc, title, style, *pos, *size, parent, id, hMenu);   
}   


//------------------------------------------------------------------------------
void TitleWindow::onCommand(int id, HWND hwndCtl, UINT codeNotify)
{
   ButtonControl *btn = dynamic_cast<ButtonControl*>(GWMap::getWindow(hwndCtl));
   if (btn)
      btn->execute();

   Parent::onCommand(id, hwndCtl, codeNotify);
}   


//------------------------------------------------------------------------------
void TitleWindow::onDrawItem(const DRAWITEMSTRUCT * lpDrawItem)
{
   Parent::onDrawItem(lpDrawItem);
}   

void TitleWindow::onPaint()
{
   InvalidateRgn(hWnd, NULL, FALSE);
   Parent::onPaint();
}

////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
ButtonControl::ButtonControl()
{
   function = NULL;
   hFont    = NULL;
}   

ButtonControl::~ButtonControl()
{
   if (hFont)
   {
      DeleteObject(hFont);
      hFont = NULL;
   }
}


//------------------------------------------------------------------------------
bool ButtonControl::create(GWWindow* parent, const char *_title, const char *_name, const Point2I *_pos, const Point2I *_size)
{
   Point2I pos;
   Point2I size;   
   if (_pos)
      pos = *_pos;
   else
   {
      pos.x = Console->getIntVariable("last::x");
      pos.y = Console->getIntVariable("last::y") +
              Console->getIntVariable("last::h") +      
              Console->getIntVariable("spacing");       
   }
   Console->setIntVariable("last::x", pos.x);
   Console->setIntVariable("last::y", pos.y);

   if (_size)
   {
      size = *_size;
   }
   else
   {
      size.x = Console->getIntVariable("last::w");
      size.y = Console->getIntVariable("last::h");
   }
   Console->setIntVariable("last::w", size.x);
   Console->setIntVariable("last::h", size.y);

   title = stringTable.insert(_title);
   name = stringTable.insert(_name);
   function = stringTable.insert(avar("%s::onPress", name), false);

   GWWindowClass gwwc;
   gwwc.setName("BUTTON");
   
   if (Parent::createEx(0, 
                        &gwwc, 
                        _title, 
                        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 
                        pos, 
                        size, 
                        parent, 
                        0, 
                        NULL))
   {
      // Create font
      hFont = CreateFont(12,                       // logical height
                         0,                        // logical avg char width
                         0,                        // angle of escapement
                         0,                        // base-line orientation angle
                         FW_BOLD,                  // font weight
                         0,                        // italic attribute flag
                         0,                        // underline attribute flag
                         0,                        // strikeout attribute flag
                         ANSI_CHARSET,             // character set ID
                         OUT_DEFAULT_PRECIS,       // output precision
                         CLIP_DEFAULT_PRECIS,      // clipping precision
                         DEFAULT_QUALITY,          // output quality
                         DEFAULT_PITCH | FF_SWISS, // pitch and family
                         "MS SANS SERIF");         // typeface name

      if (hFont)
      {
         // Set the font
         SendMessage(hWnd, 
                     WM_SETFONT, 
                     (WPARAM)hFont, 
                     MAKELPARAM(TRUE, 0));
      }
      return (true);
   }
   return (false);
}   


//------------------------------------------------------------------------------
void ButtonControl::execute()
{
   if(Console->isFunction(function))
      Console->executef(1, function);
}   


//------------------------------------------------------------------------------
void ButtonControl::setPosition(Point2I pos)
{
   Console->setIntVariable("last::x", pos.x);
   Console->setIntVariable("last::y", pos.y);
   Parent::setPosition(pos);
}   

////////////////////////////////////////////////////////////////////////////////
BitmapControl::BitmapControl()
{
   bits = NULL;
   transparent = NULL;
   imageSize = 0;
   hPalette = NULL;
}   


BitmapControl::~BitmapControl()
{
   unload();
}   

//------------------------------------------------------------------------------
bool BitmapControl::create(GWWindow* parent, const char *_name, const char *filename,
   int R, int G, int B, bool zero, const Point2I &pos)
{
   name = stringTable.insert(_name);
   load(stringTable.insert(filename), R, G, B, zero);

   GWWindowClass gwwc;
   gwwc.setName("STATIC");
   
   return Parent::createEx(0, &gwwc, name, WS_CHILD | WS_VISIBLE | SS_GRAYFRAME, pos, size, parent, 0, NULL);   
}   


//------------------------------------------------------------------------------
void BitmapControl::unload()
{
   delete bits;
   delete transparent;
   bits = NULL;
   transparent = NULL;
   imageSize   = 0;

   if (hPalette)
   {
      DeleteObject(hPalette);
   }
}   


//------------------------------------------------------------------------------
bool BitmapControl::load(const char *_filename, int R, int G, int B, bool zero)
{
   HDC hRefDC;
   int iStride, iSize;

   if (filename != _filename)
   {
      FileRStream fs(_filename);

      if (fs.getStatus() == STRM_OK)
      {
         transparentR = R;
         transparentG = G;
         transparentB = B;

         fs.read(sizeof(fileHeader), &fileHeader);
         fs.read(sizeof(infoHeader), &infoHeader);

         if (infoHeader.biSize != sizeof(infoHeader) || infoHeader.biBitCount != 24)
         {
            warning("Bitmaps must be 24-bit Uncompressed Microsoft bitmaps (%s)", name);
            unload();
            return (false);
         }

         iStride = ((3 * infoHeader.biWidth) + 3) & ~3;
         iSize   = iStride * infoHeader.biHeight;

         size.set(infoHeader.biWidth, infoHeader.biHeight);
         
         if (bits)
         {
            if (imageSize && imageSize != iSize)
            {
               delete [] bits;
               bits = new BYTE[iSize];
            }
         }
         else
         {
            bits = new BYTE[iSize];
         }

         fs.read(iSize, bits);
         fs.close();

         if (zero)
         {  // create a black/white blit mask so windows can draw a transparent bitmap...
            if (transparent)
            {
               if (imageSize && imageSize != iSize)
               {
                  delete [] transparent;
                  transparent = new BYTE[iSize];
               }
            }
            else
            {
               transparent = new BYTE[iSize];
            }

            memcpy(transparent, bits, iSize);

            for (int h = 0; h < infoHeader.biHeight; h ++)
            {
               BYTE *b = transparent + (h * iStride);
               BYTE *g = b+1;
               BYTE *r = g+1;

               for (int w = 0; w < infoHeader.biWidth; w ++, r += 3, g += 3, b += 3)
               {
                  if (*r == transparentR && 
                      *g == transparentG && 
                      *b == transparentB)
                  {
                     *r = 0xff, *g = 0xff, *b = 0xff;
                  }
                  else
                  {
                     *r = 0x00, *g = 0x00, *b = 0x00;
                  }
               }
            }     
         }
         else
         {
            if (imageSize != iSize)
            {
               delete [] transparent;
               transparent = NULL;
            }
         }

         hRefDC   = GetDC(NULL);
         hPalette = CreateHalftonePalette(hRefDC);
         ReleaseDC(NULL, hRefDC);

         filename  = _filename;
         imageSize = iSize;
         return (true);
      }
   }
   return (false);
}   

//------------------------------------------------------------------------------
void BitmapControl::onPaint()
{
   PAINTSTRUCT ps;
   HDC         hDC, hMemDC;
   HPALETTE    hOldPalette;

   if (bits)
   {
      hDC         = BeginPaint(hWnd, &ps); 
      hMemDC      = CreateCompatibleDC(hDC);
      hOldPalette = SelectPalette(hDC, hPalette, FALSE);
      RealizePalette(hDC);

      if (transparent)
      {
         // BLT the transparency
         StretchDIBits(hDC, 
            0,0,                       // dest upper left
            infoHeader.biWidth,        // width,height
            infoHeader.biHeight,
            0,0,                       // src upper left
            infoHeader.biWidth,        // width,height
            infoHeader.biHeight,
            transparent,
            (BITMAPINFO *)&infoHeader,
            DIB_RGB_COLORS,
            SRCAND
            );

         // BLT the bitmap
         StretchDIBits(hDC, 
            0,0,                       // dest upper left
            infoHeader.biWidth,        // width,height
            infoHeader.biHeight,
            0,0,                       // src upper left
            infoHeader.biWidth,        // width,height
            infoHeader.biHeight,
            bits,
            (BITMAPINFO *)&infoHeader,
            DIB_RGB_COLORS,
            SRCPAINT
            );
      }
      else
      {
         // BLT the bitmap
         StretchDIBits(hDC, 
            0,0,                       // dest upper left
            infoHeader.biWidth,        // width,height
            infoHeader.biHeight,
            0,0,                       // src upper left
            infoHeader.biWidth,        // width,height
            infoHeader.biHeight,
            bits,
            (BITMAPINFO *)&infoHeader,
            DIB_RGB_COLORS,
            SRCCOPY
            );
      }

      SelectPalette(hMemDC, hOldPalette, FALSE);
      DeleteDC(hMemDC);
      EndPaint(hWnd, &ps); 
   }

   Parent::onPaint();
}   


