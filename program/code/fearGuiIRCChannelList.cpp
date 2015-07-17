#include <fearguiircchannellist.h>
#include <fear.strings.h>
#include <console.h>
#include <g_font.h>
#include <m_qsort.h>
#include <fearguishellpal.h>
#include <g_surfac.h>
#include <g_bitmap.h>

namespace FearGui
{
enum 
{
   ICName = 0,
   ICTopic,
   ICCount,
};

static int __cdecl fnChannelNameAscend(const void *lpvOne, const void *lpvTwo)
{
   FGIRCChannelList::Channel *one = *((FGIRCChannelList::Channel **)lpvOne);
   FGIRCChannelList::Channel *two = *((FGIRCChannelList::Channel **)lpvTwo);

   if (one->lpszName == NULL)
   {
      return (1);
   }
   
   if (two->lpszName == NULL)
   {
      return (-1);
   }

   return (stricmp(one->lpszName, two->lpszName));
}

static int __cdecl fnChannelNameDescend(const void *lpvOne, const void *lpvTwo)
{
   FGIRCChannelList::Channel *one = *((FGIRCChannelList::Channel **)lpvOne);
   FGIRCChannelList::Channel *two = *((FGIRCChannelList::Channel **)lpvTwo);

   if (one->lpszName == NULL)
   {
      return (-1);
   }
   
   if (two->lpszName == NULL)
   {
      return (1);
   }

   return (stricmp(two->lpszName, one->lpszName));
}

static int __cdecl fnChannelTopicAscend(const void *lpvOne, const void *lpvTwo)
{
   FGIRCChannelList::Channel *one = *((FGIRCChannelList::Channel **)lpvOne);
   FGIRCChannelList::Channel *two = *((FGIRCChannelList::Channel **)lpvTwo);

   if (one->lpszTopic == NULL)
   {
      return (1);
   }
   
   if (two->lpszTopic == NULL)
   {
      return (-1);
   }

   return (stricmp(one->lpszTopic, two->lpszTopic));
}

static int __cdecl fnChannelTopicDescend(const void *lpvOne, const void *lpvTwo)
{
   FGIRCChannelList::Channel *one = *((FGIRCChannelList::Channel **)lpvOne);
   FGIRCChannelList::Channel *two = *((FGIRCChannelList::Channel **)lpvTwo);

   if (one->lpszTopic == NULL)
   {
      return (1);
   }
   
   if (two->lpszTopic == NULL)
   {
      return (-1);
   }

   return (stricmp(two->lpszTopic, one->lpszTopic));
}

static FGIRCChannelList::ColumnInfo gChannelInfo[ICCount] =
{
   { IDSTR_IRC_CHANNEL_NAME,  10, 500, 0, 0, true, 150, fnChannelNameAscend,  fnChannelNameDescend  },
   { IDSTR_IRC_CHANNEL_TOPIC, 10, 800, 1, 1, true, 235, fnChannelTopicAscend, fnChannelTopicDescend },
};

static FGIRCChannelList::ColumnInfo *gInfoPtrs[ICCount];

FGIRCChannelList::FGIRCChannelList()
{
}

FGIRCChannelList::~FGIRCChannelList()
{
}

bool FGIRCChannelList::cellSelectable(const Point2I &cell)
{
   if (cell.y >= 0 && cell.y < show.size())
   {
      return (true);
   }
   return (false);
}

bool FGIRCChannelList::cellSelected(Point2I cell)
{
   if (cell.y < show.size())
   {
      if(cell.y >= 0)
      {
         // Do specific stuff here
      }
      return Parent::cellSelected(cell);
   }
   return (false);
}

bool FGIRCChannelList::onAdd()
{
   int iWidth = 0;

   if(Parent::onAdd())
   {
      refresh        = true;
	   numColumns     = ICCount;
      gInfoPtrs[0]   = &gChannelInfo[0];
      gInfoPtrs[1]   = &gChannelInfo[1];
	   columnInfo     = gInfoPtrs;
   	columnsResizeable = true;
   	columnsSortable = true;
      sortAscending = true;
      columnToSort = -1;
      size.set(2, 0);

      for (int i = 0; i < ICCount; i ++)
      {
         if (gInfoPtrs[i]->active)
         {
            iWidth += gInfoPtrs[i]->width;
         }
      }
      cellSize.set(iWidth, hFont->getHeight() + 6);
   
      return (true);
   }
   return (false);
}

void FGIRCChannelList::onWake()
{
   setSize(Point2I(1, 0));
   refresh      = true;
   columnToSort = 0;
}

void FGIRCChannelList::addChannel(const char *lpszName, const char *lpszTopic, bool fOwnsStrings)
{
   int      iWidth = 0;
   Channel *channel;

   int i;
   for (i = 0; i < channels.size(); i ++)
   {
      // If the channel name already exists, don't re-enter it
      if (channels[i]->lpszName && stricmp(channels[i]->lpszName, lpszName) == 0)
      {
         return;
      }
   }

   channel = new Channel;

   if (fOwnsStrings)
   {
      channel->lpszName  = strnew(lpszName);
      channel->lpszTopic = strnew(lpszTopic);
   }
   else
   {
      channel->lpszName  = lpszName;
      channel->lpszTopic = lpszTopic;
   }
   channel->fOwnsStrings = fOwnsStrings;
   channels.push_back(channel);

   // Resize to accomodate strings
   if (channel->lpszName)
   {
      gChannelInfo[ICName].width = 
         min((int)gChannelInfo[ICName].maxWidth, 
         max((int)gChannelInfo[ICName].width, (int)hFont->getStrWidth(channel->lpszName)));
   }

   if (channel->lpszTopic)
   {
      gChannelInfo[ICTopic].width = 
         min((int)gChannelInfo[ICTopic].maxWidth, 
         max((int)gChannelInfo[ICTopic].width, (int)hFont->getStrWidth(channel->lpszTopic)));
   }

   for (i = 0; i < ICCount; i ++)
   {
      if (gInfoPtrs[i]->active)
      {
         iWidth += gInfoPtrs[i]->width;
      }
   }
   cellSize.set(iWidth, hFont->getHeight() + 6);
   sort();
}

const char *FGIRCChannelList::getSelectedChannel() const
{
   if (selectedCell.y >= 0 && selectedCell.y < show.size())
   {
      return (show[selectedCell.y]->lpszName);
   }
   return (NULL);
}

void FGIRCChannelList::showMatches(const char *lpszText)
{
   char szOne[256], szTwo[256];

   show.clear();

   for (int i = 0; i < channels.size(); i ++)
   {
      strcpy(szOne, lpszText ? lpszText : "");
      strcpy(szTwo, channels[i]->lpszName ? channels[i]->lpszName : "");

      strupr(szOne);
      strupr(szTwo);

      if (strlen(szOne) == 0 || (strstr(szTwo, szOne)))
      {
         show.push_back(channels[i]);
      }
   }

   sort();
   setSize(Point2I(size.x, show.size()));
}

void FGIRCChannelList::clear()
{
   for (int i = 0; i < channels.size(); i ++)
   {
      if (channels[i]->fOwnsStrings)
      {
         delete [] (char*)channels[i]->lpszName;
         delete [] (char*)channels[i]->lpszTopic;
      }
      delete channels[i];
   }

   channels.clear();
   show.clear();
   setSize(Point2I(size.x, 0));
}

void FGIRCChannelList::sort()
{
   if (columnToSort >= 0 && show.size())
   {
      if (sortAscending)
      {
         if (gInfoPtrs[columnToSort]->sortAscend)
         {
            m_qsort((void *)&show[0], show.size(),
                           sizeof(FGIRCChannelList::Channel *),
                           gInfoPtrs[columnToSort]->sortAscend);
         }
      }
      else if (gInfoPtrs[columnToSort]->sortDescend)
      {
         m_qsort((void *)&show[0], show.size(),
                        sizeof(FGIRCChannelList::Channel *),
                        gInfoPtrs[columnToSort]->sortDescend);
      }
   }
}

char *FGIRCChannelList::getCellText(GFXSurface *, const Point2I &cell, const Point2I &, const Point2I &)
{
   switch (cell.x)
   {
      case ICName :
      {
         if (cell.y >= 0 && cell.y < show.size())
         {
            return (const_cast<char *>(show[cell.y]->lpszName));
         }
         break;
      }

      case ICTopic :
      {
         if (cell.y >= 0 && cell.y < show.size())
         {
            return (const_cast<char *>(show[cell.y]->lpszTopic));
         }
         break;
      }
   }

   return (NULL);
}

void FGIRCChannelList::onPreRender()
{
   if (refresh)
   {
      sort();
      refresh = false;
      setUpdate();
   }
}

void FGIRCChannelList::onRenderCell(GFXSurface *sfc, Point2I offset, Point2I cell, bool sel, bool mouseOver)
{
   Point2I parentOffset = parent->localToGlobalCoord(Point2I(0, 0));
   
   bool ghosted = FALSE;
   if (root)
   {
      SimGui::Control *topDialog = root->getDialogNumber(1);
      if ((! active) || 
          (topDialog && (topDialog != getTopMostParent()) && (topDialog->findControlWithTag(IDCTG_DIALOG))))
      {
         ghosted = TRUE;
      }
   }
   
   //get the bitmaps
   //initialize the font
   GFXFont *font;
   if (ghosted || (! cellSelectable(cell))) font = hFontNA;
   else if (sel) font = hFontHL;
   else if (mouseOver) font = hFontMO;
   else font = hFont;
   
   //initialize the draw offset
   Point2I drawOffset = offset;
   
   //draw the cels first
   for (int i = 0; i < numColumns; i++)
   {
      if (columnInfo[i]->active)
      {
         sfc->drawLine2d(&drawOffset, &Point2I(drawOffset.x, drawOffset.y + cellSize.y - 1), (ghosted ? BOX_GHOST_LAST_PIX : HILITE_COLOR));
         drawOffset.x += columnInfo[i]->width;
      }
   }
   sfc->drawLine2d(&drawOffset, &Point2I(drawOffset.x, drawOffset.y + cellSize.y - 1), (ghosted ? BOX_GHOST_LAST_PIX : HILITE_COLOR));
   
   drawOffset = offset;
   
   for (int j = 0; j < numColumns; j++)
   {
      if (columnInfo[j]->active)
      {
         char *entryText = getCellText(sfc, Point2I(columnInfo[j]->origPosition, cell.y),
                                          drawOffset, Point2I(columnInfo[j]->width, cellSize.y)); 
         if (entryText)
         {
            drawInfoText(sfc, font, entryText, Point2I(drawOffset.x + 7, drawOffset.y - 2),
                                               Point2I(columnInfo[j]->width - 4, cellSize.y), false, false);
         }
         drawOffset.x += columnInfo[j]->width;
      }
   }
   
   sfc->drawRect2d(&RectI(parentOffset.x, offset.y, parentOffset.x + parent->extent.x - 1, offset.y + cellSize.y), (ghosted ? BOX_GHOST_LAST_PIX : HILITE_COLOR));
}

IMPLEMENT_PERSISTENT_TAG(FGIRCChannelList, FOURCC('F','i','c','l'));
};
