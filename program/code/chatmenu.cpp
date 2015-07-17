#include "sim.h"
#include "simInput.h"
#include "fearGlobals.h"
#include "fearGuiTSCommander.h"
#include "chatMenu.h"
#include "console.h"

IMPLEMENT_PERSISTENT(ChatMenu);

ChatMenu::ChatMenu()
{
   curMenu = chatTree = NULL;
   curMode = Inactive;
   heading[0] = 0;
   lastKey = 0;
}

void ChatMenu::deltree(Node *node)
{
   if(!node)
      return;

   deltree(node->firstChild);
   deltree(node->nextSibling);
   int i;
   for(i = 0; i < node->argc; i++)
      delete[] (char*)node->argv[i];
   delete node;
}

ChatMenu::~ChatMenu()
{
   deltree(chatTree);
}

void ChatMenu::setMode(int mode)
{
   if(mode < Inactive || mode > OneShot)
      return;
   if (mode == Inactive)
   {      
      //nasty hack - if this is the commander menu, cancel setting a waypoint
      if ((! stricmp(getName(), "CommandChatMenu")) && (cg.commandTS))
      {
         cg.commandTS->cancelWaypointMode();
      }
   }
   curMode = mode;
   curMenu = chatTree;
}

bool ChatMenu::processArguments(int argc, const char **argv)
{
   if(argc == 1)
   {
      strcpy(heading, argv[0]);
      return true;
   }
   return false;
}

bool ChatMenu::processEvent(const SimEvent *event)
{
   if(event->type != SimInputEventType)
      return false;
   const SimInputEvent *ev = (const SimInputEvent*)event;

   if(ev->deviceType != SI_KEYBOARD || ev->deviceInst != 0 || ev->objType != SI_KEY)
      return false;

   if(ev->action == SI_BREAK && lastKey && lastKey == ev->objInst)
   {
      lastKey = 0;
      return true;
   }
   else if(ev->action == SI_BREAK)
      return false;
 
   if(curMode == Inactive)
      return false;

   if(ev->objInst == DIK_ESCAPE)
   {
      lastKey = DIK_ESCAPE;
      bool cancelWaypoint = FALSE;
      //nasty hack - if this is the commander menu, cancel setting a waypoint
      if ((! stricmp(getName(), "CommandChatMenu")) && (cg.commandTS))
      {
         cancelWaypoint = cg.commandTS->inWaypointMode();
         cg.commandTS->cancelWaypointMode();
      }
      if(curMode == OneShot)
      {
         curMenu = chatTree;
         setMode(Inactive);
         return true;
      }
      else if (curMenu != chatTree)
      {
         curMenu = chatTree;
         return true;
      }
      else if (cancelWaypoint)
      {
         return TRUE;
      }
      else
      {
         return false;
      }
   }
   if(ev->modifier != 0 || ev->ascii == 0)
      return false;

   bool ret = processKey(ev->ascii);
   if(ret)
      lastKey = ev->objInst;
   return ret;
}

bool ChatMenu::onAdd()
{
   if(!Parent::onAdd())
      return false;
   addToSet(SimInputConsumerSetId);
   return true;
}

bool ChatMenu::processQuery(SimQuery *query)
{
   if (query->type == SimInputPriorityQueryType)
   {
      SimInputPriorityQuery *q = (SimInputPriorityQuery*)query;
         q->priority = SI_PRIORITY_NORMAL + 1; // one up from the action handler
      return (true);
   }
   return false;
}

bool ChatMenu::processKey(char key)
{
   bool ret = false;
   Node *walk = curMenu;
   while(walk)
   {
      if(walk->key == key)
      {
         if(walk->argc)
            Console->execute(walk->argc, walk->argv);
         curMenu = walk->firstChild;
         ret = true;
         break;
      }
      walk = walk->nextSibling;
   }
   if(!curMenu)
   {
      curMenu = chatTree;
      if(curMode == OneShot)
      {
         setMode(Inactive);
      }
   }
   return ret;
}

bool ChatMenu::addMenuString(const char *menuString, int argc, const char **argv)
{
   Node **walk = &chatTree;
   Node *parent = NULL;
   for(;;)
   {
      char c = *menuString++;
      if(!c)
      {
         curMenu = chatTree;
         return false;
      }
      const char *str = menuString;
      int mlen = 0;

      while(*menuString && *menuString != '\\')
      {
         mlen++;
         menuString++;
      }
      if(!mlen)
      {
         curMenu = chatTree;
         return false;
      }
      bool leaf = true;
      if(*menuString == '\\')
      {
         menuString++;
         leaf = false;
      }     
      Node *temp = *walk;
      while(temp)
      {
         if(temp->key == c)
         {
            parent = temp;
            if(leaf)
            {
               curMenu = chatTree;
               return false;
            }
            walk = &temp->firstChild;
            break;
         }
         temp = temp->nextSibling;
      }
      // if temp was not there, make a menu entry for it.
      if(!temp)
      {
         temp = new Node;
         temp->key = c;
         strncpy(temp->heading, str, min(mlen, int(MaxHeadingLen)));
         temp->heading[min(mlen, (int)MaxHeadingLen)] = 0;
         temp->parent = parent;
         parent = temp;
         if(leaf)
         {
            if(argc > MaxArgs)
               argc = MaxArgs;
            int i;
            for(i = 0; i < argc; i++) {
					char* copy = new char[strlen(argv[i])+1];
					strcpy(copy,argv[i]);
					temp->argv[i] = copy;
            }
            temp->argc = argc;
         }
         else
            temp->argc = 0;
         temp->firstChild = NULL;
         while(*walk)
            walk = &((*walk)->nextSibling);
         *walk = temp;
         walk = &temp->firstChild;
         temp->nextSibling = NULL;
         if(leaf)
         {
            curMenu = chatTree;
            return true;
         }
      }
   }
}