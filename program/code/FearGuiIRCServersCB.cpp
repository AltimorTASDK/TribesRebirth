//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "console.h"
#include "simResource.h"
#include "simGuiScrollCtrl.h"
#include "fearGuiScrollCtrl.h"
#include "simGuiArrayCtrl.h"
#include "g_bitmap.h"
#include "g_surfac.h"
#include "m_qsort.h"
#include "fearGlobals.h"
#include "fear.strings.h"
#include "FearGuiShellPal.h"
#include "FearGuiIRCServersCB.h"

namespace FearGui
{
 
bool FGIRCServersPopUp::onAdd()
{
   //can't do the parent::onAdd(), since it also sets up sc, and ac
   if(!Parent::onAdd())
      return false;

   //create the popup menu
   FearGuiScrollCtrl *fgSC = new FearGuiScrollCtrl();
   AssertFatal(fgSC, "could not allocate memory");
   fgSC->setBorderThickness(6);
   fgSC->forceVScrollBar = SimGui::ScrollCtrl::ScrollBarDynamic;
   fgSC->forceHScrollBar = SimGui::ScrollCtrl::ScrollBarAlwaysOff;
   fgSC->pbaTag = IDPBA_SCROLL_SHELL;
   fgSC->willFirstRespond = false;
   fgSC->extent.set(240, 200);
   sc = fgSC;
   manager->addObject(sc);
   addObject(sc);
   SimGui::Control *ctrl = (SimGui::Control*)(sc->getScrollContentCtrl());
   if (ctrl)
   {
      ctrl->mbBoarder = TRUE;
      ctrl->boarderColor = GREEN_78;
      ctrl->mbOpaque = TRUE;
      ctrl->fillColor = GREEN_78;
   }
   
   ac = new IRCServersVarCtrl();
   AssertFatal(ac, "could not allocate memory");
   ac->extent.set(240, 200);
   manager->addObject(ac);
   sc->addObject(ac);
   ac->message = IDSTR_STRING_DEFAULT; // we need the text list to
                                       // give the pop up ctrl some
                                       // message indicating that
                                       // mouseUp or keyUp has occured
   //setBoarder();                                       
   return true;
}  

//------------------------------------------------------------------------------
Point2I FGIRCServersComboBox::getSelected(void)
{
   if ((! popUpCtrl) || (! popUpCtrl->getArrayCtrl()))
   {
      return Point2I(-1, -1);
   }
   else return popUpCtrl->getArrayCtrl()->getSelectedCell();
}

void FGIRCServersComboBox::setPopUpMessage(Int32 msg)
{
   if (popUpCtrl && popUpCtrl->getArrayCtrl())
   {
      popUpCtrl->getArrayCtrl()->setMessage(msg);
   }
}

bool FGIRCServersComboBox::onAdd()
{
   if(!Parent::onAdd())
      return false;

   popUpCtrl = new FGIRCServersPopUp(this);
   AssertFatal(popUpCtrl, "could not allocate memory");
   manager->addObject(popUpCtrl);
   deleteNotify(popUpCtrl);
   
   //set the title bmp
   mTitleBMP = SimResource::get(manager)->load("POP_ChatServer_DF.BMP");
   mTitleGhostBMP = SimResource::get(manager)->load("POP_ChatServer_NA.BMP");
   
   //set the text
   IRCServersVarCtrl *fvCtrl = (IRCServersVarCtrl*)(popUpCtrl->getArrayCtrl());
   const char *prefServer = Console->getVariable("Pref::IRCServer");
   const char *prefPort = Console->getVariable("Pref::IRCPort");
   if (prefServer[0] && prefPort[0])
   {
      fvCtrl->setSelected(prefServer);
   }
   else
   {
      fvCtrl->setSelected(0);
   }
   updateFromArrayCtrl();                                    
   return true;
}  
 
void FGIRCServersComboBox::openPopUpCtrl()
{
   if (popUpCtrl)
   {
      // set up the data for the pop up ctrl
      IRCServersVarCtrl *mlCtrl = (IRCServersVarCtrl*)getArrayCtrl();
      AssertFatal(mlCtrl, "pop up control is missing a text list");
      mlCtrl->onWake();
      Point2I globalPos = localToGlobalCoord(Point2I(0,0));
      popUpCtrl->setPos(globalPos.x - 1, globalPos.y + extent.y - 1);
      popUpCtrl->setDim(extent.x + 2, 200, 200);
      root->pushDialogControl(popUpCtrl);
      root->tabNext();
      popUpActive = true;
      root->mouseLock(popUpCtrl);
   }
}   

void FGIRCServersComboBox::updateFromArrayCtrl()
{
   AssertFatal(popUpCtrl && popUpCtrl->getArrayCtrl(), "pop up control is missing a text list");
   IRCServersVarCtrl *fvCtrl = (IRCServersVarCtrl*)(popUpCtrl->getArrayCtrl());
   Point2I selected = fvCtrl->getSelectedCell();
   if (selected.y >= 0)
   {
      setText(fvCtrl->getCellText(NULL, selected, selected, selected));
   }
   else
   {
      setText("");
   }
}

void FGIRCServersComboBox::addIRCServer(char *description)
{
   IRCServersVarCtrl *fvCtrl = (IRCServersVarCtrl*)(popUpCtrl->getArrayCtrl());
   if (description && description[0])
   {
      fvCtrl->addIRCServer(description);
      updateFromArrayCtrl();
      setUpdate();
   }
}

IRCServersVarCtrl::IRCServerRep* FGIRCServersComboBox::getSelectedServer(void)
{
   IRCServersVarCtrl *fvCtrl = (IRCServersVarCtrl*)(popUpCtrl->getArrayCtrl());
   if (fvCtrl) return fvCtrl->getSelectedServer();
   else return NULL;
}

void FGIRCServersComboBox::removeSelected(void)
{
   IRCServersVarCtrl *fvCtrl = (IRCServersVarCtrl*)(popUpCtrl->getArrayCtrl());
   if (fvCtrl)
   {
      fvCtrl->removeSelected();
      updateFromArrayCtrl();
      setUpdate();
   }
}

void FGIRCServersComboBox::setSelectedServerText(char *text)
{
   IRCServersVarCtrl *fvCtrl = (IRCServersVarCtrl*)(popUpCtrl->getArrayCtrl());
   if (fvCtrl) fvCtrl->setSelectedServerText(text);
}

void FGIRCServersComboBox::setSelectedPortText(char *text)
{
   IRCServersVarCtrl *fvCtrl = (IRCServersVarCtrl*)(popUpCtrl->getArrayCtrl());
   if (fvCtrl) fvCtrl->setSelectedPortText(text);
}

void FGIRCServersComboBox::setSelectedRoomText(char *text)
{
   IRCServersVarCtrl *fvCtrl = (IRCServersVarCtrl*)(popUpCtrl->getArrayCtrl());
   if (fvCtrl) fvCtrl->setSelectedRoomText(text);
}

void FGIRCServersComboBox::writeScriptFile(void)
{
   IRCServersVarCtrl *fvCtrl = (IRCServersVarCtrl*)(popUpCtrl->getArrayCtrl());
   if (fvCtrl) fvCtrl->writeScriptFile();
}


IMPLEMENT_PERSISTENT_TAG(FGIRCServersComboBox, FOURCC('F','G','i','b'));
 
//------------------------------------------------------------------------------

static int __cdecl serverAscend(const void *a,const void *b)
{
   IRCServersVarCtrl::IRCServerRep **entry_A = (IRCServersVarCtrl::IRCServerRep **)(a);
   IRCServersVarCtrl::IRCServerRep **entry_B = (IRCServersVarCtrl::IRCServerRep **)(b);
   return (stricmp((*entry_A)->description, (*entry_B)->description));
} 

IRCServersVarCtrl::~IRCServersVarCtrl(void)
{
   cg.ircServerList = NULL;
}

void IRCServersVarCtrl::addIRCServer(int argc, const char *argv[])
{
   //verify args
   if (argc < 4) return;
	IRCServerRep newServer;
   
   //address
   strncpy(newServer.address, argv[1], 255);
   newServer.address[255] = '\0';
   
   //port
   newServer.port = atoi(argv[2]);
   
   //description
   strncpy(newServer.description, argv[3], 255);
   newServer.description[255] = '\0';
   
   if (argc == 5)
   {
      //defaultRoom
      strncpy(newServer.defaultRoom, argv[4], 255);
   }
   else
   {
      newServer.defaultRoom[0] = '\0';
   }
   newServer.defaultRoom[255] = '\0';
   
   //add it to the list and set the size
   entries.push_back(newServer);
   entryPtrs.push_back(&entries.last());
   setSize(Point2I(1, entries.size()));
}

void IRCServersVarCtrl::addIRCServer(char *description)
{
   //verify args
	IRCServerRep newServer;
   
   //address
   newServer.address[0] = '\0';
   
   //port
   newServer.port = 6667;
   
   //description
   strncpy(newServer.description, description, 255);
   newServer.description[255] = '\0';
   
   //default room
   newServer.defaultRoom[0] = '\0';
   
   //add it to the list and set the size
   entries.push_back(newServer);
   entryPtrs.push_back(&entries.last());
   setSize(Point2I(1, entries.size()));
   
   //make it the selected sell
   setSelected(entries.size() - 1);
}

void IRCServersVarCtrl::removeSelected(void)
{
   if (selectedCell.y >= 0 && selectedCell.y < entryPtrs.size())
   {
      //first remove from the entries vector
      for (int i = 0; i < entries.size(); i++)
      {
         if ((DWORD)(&entries[i]) == (DWORD)(entryPtrs[selectedCell.y]))
         {
            entries.erase(i);
            break;
         }
      }
      entryPtrs.erase(selectedCell.y);
   }
   
   if (entryPtrs.size() > 0)
   {
      setSelected(0);
   }
   else
   {
      cellSelected(Point2I(-1, -1));
   }
   
   //update the array size
   setSize(Point2I(1, entries.size()));
}

bool IRCServersVarCtrl::onAdd()
{
   if(!Parent::onAdd())
      return false;
   
   //set the global   
   cg.ircServerList = this;
   
   setSize(Point2I(1, 0));
   
   //the list of servers is stored in a console script
   Console->evaluate("exec(\"IRCServerList.cs\");", false);
   
   //sort the list
   if (entryPtrs.size())
   {
      m_qsort((void *)&entryPtrs[0], entryPtrs.size(),
                     sizeof(IRCServersVarCtrl::IRCServerRep *),
                     serverAscend);
   }
   
   return true;
}

void IRCServersVarCtrl::setSelected(const char *address)
{
   for (int i = 0; i < entryPtrs.size(); i++)
   {
      if (! stricmp(address, entryPtrs[i]->address))
      {
         cellSelected(Point2I(0, i));
         break;
      }
   }
}

void IRCServersVarCtrl::setSelected(int index)
{
   if (index >= 0 && index < entryPtrs.size())
   {
      cellSelected(Point2I(0, index));
   }
}

bool IRCServersVarCtrl::cellSelected(Point2I cell)
{
   if (cell.y >= 0 && cell.y < entryPtrs.size())
   {
      char buf[16];
      Console->setVariable("Pref::IRCServer", entryPtrs[cell.y]->address);
      sprintf(buf, "%d", entryPtrs[cell.y]->port);
      Console->setVariable("Pref::IRCPort", buf);
      
      Parent::cellSelected(cell);
      return TRUE;
   }
   else
   {
      selectedCell.set(-1, -1);
      return FALSE;
   }
}

IRCServersVarCtrl::IRCServerRep* IRCServersVarCtrl::getSelectedServer(void)
{
   if (selectedCell.y >= 0 && selectedCell.y < entryPtrs.size())
   {
      return entryPtrs[selectedCell.y];
   }
   else return NULL;
}

void IRCServersVarCtrl::setSelectedServerText(char *text)
{
   if (selectedCell.y >= 0 && selectedCell.y < entryPtrs.size())
   {
      strncpy(entryPtrs[selectedCell.y]->address, text, 255);
      entryPtrs[selectedCell.y]->address[255] = '\0';
   }
}

void IRCServersVarCtrl::setSelectedPortText(char *text)
{
   if (selectedCell.y >= 0 && selectedCell.y < entryPtrs.size())
   {
      if ((! text) || text[0] == '\0') entryPtrs[selectedCell.y]->port = 0;
      else entryPtrs[selectedCell.y]->port = atoi(text);
   }
}

void IRCServersVarCtrl::setSelectedRoomText(char *text)
{
   if (selectedCell.y >= 0 && selectedCell.y < entryPtrs.size())
   {
      strncpy(entryPtrs[selectedCell.y]->defaultRoom, text, 255);
      entryPtrs[selectedCell.y]->defaultRoom[255] = '\0';
   }
}

const char* IRCServersVarCtrl::getCellText(GFXSurface *, const Point2I &cell, const Point2I &, const Point2I &)
{
   return entryPtrs[cell.y]->description;
}

void IRCServersVarCtrl::writeScriptFile(void)
{
   FileWStream fileOut("IRCServerList.cs");
   char buf[512];
   
   //write out the header
   sprintf(buf, "//-----------------------------------\n");
   fileOut.write(strlen(buf), buf);
   sprintf(buf, "//\n");
   fileOut.write(strlen(buf), buf);
   sprintf(buf, "// Database of IRC servers\n");
   fileOut.write(strlen(buf), buf);
   sprintf(buf, "//\n");
   fileOut.write(strlen(buf), buf);
   sprintf(buf, "//-----------------------------------\n\n");
   fileOut.write(strlen(buf), buf);
   
   //now dump out each addIRCServer command
   for (int i = 0; i < entries.size(); i++)
   {
      sprintf(buf, "addIRCServer(\"%s\", \"%d\", \"%s\", \"%s\");\n",
                  entries[i].address, entries[i].port, entries[i].description, entries[i].defaultRoom);
      fileOut.write(strlen(buf), buf);
   }
}

IMPLEMENT_PERSISTENT_TAG(IRCServersVarCtrl, FOURCC('F','G','i','q'));

};