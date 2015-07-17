#include "banlist.h"
#include "stdlib.h"
#include "stdio.h"
#include "filstrm.h"

BanList gBanList;

void BanList::addBan(const char *TA, time_t banTime)
{
   time_t curTime = time(NULL);
   if(banTime != 0 && banTime < curTime)
      return;

   Vector<BanInfo>::iterator i;
   for(i = list.begin();i != list.end();i++)
   {
      if(!stricmp(i->transportAddress, TA))
      {
         i->bannedUntil = banTime;
         return;
      }
   }
   BanInfo b;
   strcpy(b.transportAddress, TA);
   if(!strnicmp(b.transportAddress, "ip:", 3))
   {
      char *c = strchr(b.transportAddress+3, ':');
      if(c)
      {
         *(c+1) = '*';
         *(c+2) = 0;
      }
   }
   b.bannedUntil = banTime;
   list.push_back(b);
}

void BanList::addBanRelative(const char *TA, int numSeconds)
{
   time_t curTime = time(NULL);
   time_t banTime = numSeconds != -1 ? curTime + numSeconds : 0;
   addBan(TA, banTime);
}

void BanList::removeBan(const char *TA)
{
   Vector<BanInfo>::iterator i;
   for(i = list.begin();i != list.end();i++)
   {
      if(!stricmp(i->transportAddress, TA))
      {
         list.erase(i);
         return;
      }
   }
}

bool BanList::isBanned(const char *TA)
{
   time_t curTime = time(NULL);
   Vector<BanInfo>::iterator i;
   for(i = list.begin();i != list.end();)
   {
      if(i->bannedUntil != 0 && i->bannedUntil < curTime)
      {
         list.erase(i);
         continue;
      }
      else if(isTAEq(i->transportAddress, TA))
         return true;
      i++;
   }
   return false;
}

bool BanList::isTAEq(const char *bannedTA, const char *TA)
{
   char a, b;
   for(;;)
   {
      a = *bannedTA++;
      b = *TA++;
      if(a == '*' || (!a && b == ':')) // ignore port
         return true;
      if(tolower(a) != tolower(b))
         return false;
      if(!a)
         return true;
   }
}

void BanList::exportToFile(const char *fileName)
{
   FileWStream stream(fileName);
   char buf[1024];
   Vector<BanInfo>::iterator i;
   for(i = list.begin(); i != list.end(); i++)
   {
      sprintf(buf, "BanList::addAbsolute(\"%s\", %d);\r\n", i->transportAddress, i->bannedUntil);
      stream.write(strlen(buf), buf);
   }
}