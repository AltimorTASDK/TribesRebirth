#include <console.h>
#include <g_surfac.h>
#include <simResource.h>
#include <fear.strings.h>
#include <fearguitextlist.h>

namespace SimGui {
extern Control *findControl(const char *name);
};

namespace FearGui
{

static bool gScriptFunctionsAdded = FALSE;

static const char *FGTextListSort(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 2)
   {
      Console->printf("%s(control);", argv[0]);
      return "false";
   }
   SimGui::Control *ctrl = SimGui::findControl(argv[1]);
   FGTextList *tl = NULL;
   if (ctrl) tl = dynamic_cast<FGTextList *>(ctrl);

   if (! tl)
   {
      Console->printf("%s - invalid control %s.", argv[0], argv[1]);
      return "false";
   }
   tl->sort();
   return "TRUE";
}

FGTextList::FGTextList()
{
}

FGTextList::~FGTextList()
{
}

Int32 FGTextList::getSelectedPos()
{
   return (selectedCell.y);
}

void FGTextList::setSelection(Int32 iPos)
{
   const char *lpszVar;

   if ((iPos >= -1) && (iPos < textList.size()))
   {
      selectedCell.y = iPos;

      if ((lpszVar = getSelectedText()) != NULL)
      {
         setVariable(lpszVar);
      }

      setUpdate();
   }
}

bool FGTextList::cellSelected(Point2I cell)
{
   bool result = Parent::cellSelected(cell);
   if (consoleCommand[0])
   {
      Console->evaluate(consoleCommand, FALSE);
   }
   return result;
}

bool FGTextList::onAdd()
{
   if (! Parent::onAdd()) return FALSE;
   
   if (! gScriptFunctionsAdded)
   {
      gScriptFunctionsAdded = TRUE;
      Console->addCommand(0, "FGTextList::sort", FGTextListSort);
   }
   
   //load the fonts - force all in Fear to look the same
   hFont = SimResource::get(manager)->load("sf_orange214_10.pft");
   AssertFatal(hFont.operator bool(), "Unable to load sf_orange214_10.pft.");
   hFontHL = SimResource::get(manager)->load("sf_white_10.pft");
   AssertFatal(hFontHL.operator bool(), "Unable to load sf_white_10.pft");
   hFontDisabled = SimResource::get(manager)->load("sf_grey100_10b.pft");
   AssertFatal(hFontDisabled.operator bool(), "Unable to load sf_grey100_10b.pft.");
   
   //set the cell dimensions
   cellSize.set(640, hFont->getHeight() + 4);
   
   return TRUE;
}   

void FGTextList::onWake()
{
   //load the fonts - force all in Fear to look the same
   hFont = SimResource::get(manager)->load("sf_orange214_10.pft");
   AssertFatal(hFont.operator bool(), "Unable to load sf_orange214_10.pft.");
   hFontHL = SimResource::get(manager)->load("sf_white_10.pft");
   AssertFatal(hFontHL.operator bool(), "Unable to load sf_white_10.pft");
   hFontDisabled = SimResource::get(manager)->load("sf_grey100_10b.pft");
   AssertFatal(hFontDisabled.operator bool(), "Unable to load sf_grey100_10b.pft.");
   
   setSize(Point2I(1, 0));
}   


void FGTextList::onRenderCell(GFXSurface *sfc, Point2I offset, Point2I cell, bool selected, bool mouseOver)
{
   //compiler warning
   mouseOver;
   
   Point2I textOffset = offset;
   textOffset.x += ENTRY_H_MARGIN;
   textOffset.y += textVPosDelta;
   
   bool ghosted = FALSE;
   SimGui::Control *topDialog = root->getDialogNumber(1);
   if ((! active) ||
       (topDialog && (topDialog != getTopMostParent()) && (topDialog->findControlWithTag(IDCTG_DIALOG))))
   {
      ghosted = TRUE;
   }
   
   GFXFont *font = (ghosted ? hFontDisabled : (selected ? hFontHL : hFont));
   if (! font) font = hFont;
   if (selected)
   {
      sfc->drawRect2d_f(&RectI(offset.x, offset.y,
                                 offset.x + cellSize.x - 1, offset.y + cellSize.y - 1), 254);
   }
   if (bool(font) && textList[cell.y].text)
   {
      sfc->drawText_p(font, &textOffset, textList[cell.y].text);
   }
   
}   

IMPLEMENT_PERSISTENT_TAG(FGTextList, FOURCC('F','G','t','l'));
};