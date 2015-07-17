#include "simGuiDelegate.h"
#include "SimGuiTextEdit.h"
#include "console.h"
#include "simGuiTextList.h"
#include "main.strings.h"
#include "netCSDelegate.h"

namespace SimGui
{

class StartDelegate : Delegate
{

private:
   typedef Delegate Parent;
   TextEdit *edit;
   Control *curGui;
   bool startedQuery;
   
public:
   void onNewContentControl(Control *oldGui, Control *newGui);
   DECLARE_PERSISTENT(StartDelegate);
   StartDelegate() { curGui = NULL; }
   DWORD onMessage(SimObject *sender, DWORD msg);
};

void StartDelegate::onNewContentControl(Control *, Control *newGui)
{
   curGui = newGui;

   // only run the query if we have a server select list

#ifdef NO_BUILD

   if(curGui->findControlWithTag(IDCTG_SERVER_SELECT_LIST))
   {
      CMDConsole *console = CMDConsole::getLocked();

      startedQuery = FALSE;
      Net::CSDelegate *del = (Net::CSDelegate *) manager->findObject(SimCSDelegateId);
      if (del)
      {
          del->queryServers();
          startedQuery = TRUE;
      }
   }
#endif
}

DWORD StartDelegate::onMessage(SimObject *sender, DWORD msg)
{
#ifdef NO_BUILD
   if(msg == IDCMD_REFRESH)
   {
      startedQuery = FALSE;
      Net::CSDelegate *del = (Net::CSDelegate *) manager->findObject(SimCSDelegateId);
      AssertFatal(del, "ARG!");
      del->queryServers();
      startedQuery = TRUE;
   }
   else if (msg == IDCTG_SERVER_REFRESH_LIST)
   {
      Net::CSDelegate *del = (Net::CSDelegate *) manager->findObject(SimCSDelegateId);
      AssertFatal(del, "ARG!");
      if (! startedQuery)
      {
          del->queryServers();
          startedQuery = TRUE;
      }
          
      TextList *tl = (TextList *)(curGui->findControlWithTag(IDCTG_SERVER_SELECT_LIST));
      if (! tl) return 0;

      Vector<Net::CSDelegate::ServerListEntry>::iterator sle;
      for(sle = del->serverList.begin(); sle != del->serverList.end(); sle++)
      {
          tl->addEntryBack(sle->address);
      }
      return 0;
   }
#endif
   return Parent::onMessage(sender, msg);
}

IMPLEMENT_PERSISTENT(StartDelegate);

};