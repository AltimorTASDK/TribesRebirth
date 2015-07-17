#include "simGuiDelegate.h"
#include "FearGuiTestEdit.h"
#include "console.h"
#include "fear.strings.h"
#include "simGuiTextList.h"


namespace SimGui
{
//----------------------------------------------------------------------------

const char* MissionNameTemplate = "missions\\*.mis";


//----------------------------------------------------------------------------

class MissionSelectDelegate : Delegate
{

private:
   typedef Delegate Parent;
   FearGui::TestEdit *edit;
public:
   void onNewContentControl(Control *oldGui, Control *newGui);
   DECLARE_PERSISTENT(MissionSelectDelegate);
   MissionSelectDelegate() { edit = NULL; }
   DWORD onMessage(SimObject *sender, DWORD msg);
};

LONG RegQueryI(HKEY key, char *skey, LPDWORD type, LPBYTE buf, LPDWORD size)
{
   char *rest = strchr(skey, '\\');
   if(!rest)
      return RegQueryValueEx(key, skey, NULL, type, buf, size);

   rest[0] = 0;
   rest++;
   HKEY newKey;
   LONG ret = RegOpenKeyEx(key, skey, NULL, KEY_QUERY_VALUE, &newKey);

   if(ret != ERROR_SUCCESS)
      return ret;
   
   ret = RegQueryI(newKey, rest, type, buf, size);    
   RegCloseKey(newKey);
   return ret;
}


LONG RegQuery(HKEY key, const char *skey, LPDWORD type, LPBYTE buf, LPDWORD size)
{
   char sbuf[255];
   strcpy(sbuf, skey);
   return RegQueryI(key, sbuf, type, buf, size);
}

void MissionSelectDelegate::onNewContentControl(Control *, Control *newGui)
{
   CMDConsole* console = CMDConsole::getLocked();
   char buf[256];
   edit = (FearGui::TestEdit *)(newGui->findControlWithTag(IDCTG_SESSION));
   if(edit)
   {
      const char *var = console->getVariable("Server::Session");
      if(!var[0])
      {
         strcpy(buf, "FearSession");
         DWORD type;
         DWORD len = 255;
         RegQuery(
            HKEY_LOCAL_MACHINE,
            "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\WinLogon\\DefaultDomainName",
            &type,
            (unsigned char *)buf,
            &len);
      }
      else
         strcpy(buf, var);

      edit->setText(buf);
   }

   TextList *tl = (TextList *)(newGui->findControlWithTag(IDCTG_MISSION_SELECT_LIST));
   if (! tl) return;

   WIN32_FIND_DATA data;
   HANDLE handle = FindFirstFile(MissionNameTemplate, &data);

   if (handle != INVALID_HANDLE_VALUE)
   {
      do
      {
         data.cFileName[strlen(data.cFileName) - 4] = '\0';
         tl->addEntryBack(data.cFileName);
      }
      while (FindNextFile(handle,&data));
      FindClose(handle);
   }
}

DWORD MissionSelectDelegate::onMessage(SimObject *sender, DWORD msg)
{
   char buf[256];
   if(edit)
   {
     	CMDConsole* console = CMDConsole::getLocked();
      edit->getText(buf);
     	console->setVariable("Client::Session", buf);
     	console->setVariable("Server::Session", buf);
   }
   if(msg == IDCTG_MISSION_SELECT_LIST)
   {
     	CMDConsole* console = CMDConsole::getLocked();
      TextList *tl = (TextList *)(sender);
      sprintf(buf, "SERVER %s;startup", tl->getSelectedText());
      console->evaluate(buf, false);
      return 0;
   }
   return Parent::onMessage(sender, msg);
}

IMPLEMENT_PERSISTENT(MissionSelectDelegate);

};