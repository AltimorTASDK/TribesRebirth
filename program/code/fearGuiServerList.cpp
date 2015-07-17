#include "simResource.h"
#include "PlayerManager.h"
#include "FearCSDelegate.h"
#include "g_surfac.h"
#include "g_bitmap.h"
#include "fear.strings.h"
#include "help.strings.h"
#include "SimGuiSimpleText.h"
#include "fearGlobals.h"
#include "FearGuiShellPal.h"
#include "FearGuiFilters.h"
#include "m_qsort.h"
#include "FearGuiArrayCtrl.h"
#include "FearGuiServerList.h"
#include "FearGuiServerInfo.h"
#include "console.h"
#include "g_font.h"
#include "version.h"

int (__cdecl *g_secondarySortPtr)(const void *, const void *);

namespace SimGui {

extern Control *findControl(const char *name);
};

namespace FearGui
{

static void strncpyt(char *dst, const char *src, int n)
{
   while(n--)
   {
      char c = *dst++ = *src++;
      if(!c)
         return;
   }
   *dst = 0;
}

static int __cdecl favoriteAscend(const void *a,const void *b);
static int __cdecl favoriteDescend(const void *a,const void *b);
static int __cdecl statusAscend(const void *a,const void *b);
static int __cdecl statusDescend(const void *a,const void *b);
static int __cdecl serverNameAscend(const void *a,const void *b);
static int __cdecl serverNameDescend(const void *a,const void *b);
static int __cdecl pingAscend(const void *a,const void *b);
static int __cdecl pingDescend(const void *a,const void *b);
static int __cdecl playersAscend(const void *a,const void *b);
static int __cdecl playersDescend(const void *a,const void *b);
static int __cdecl addressAscend(const void *a,const void *b);
static int __cdecl addressDescend(const void *a,const void *b);
static int __cdecl cpuAscend(const void* a, const void* b);
static int __cdecl cpuDescend(const void* a, const void* b);
static int __cdecl missionAscend(const void* a, const void* b);
static int __cdecl missionDescend(const void* a, const void* b);
static int __cdecl versionAscend(const void* a, const void* b);
static int __cdecl versionDescend(const void* a, const void* b);
static int __cdecl versionAscend(const void* a, const void* b);
static int __cdecl versionDescend(const void* a, const void* b);
static int __cdecl mtypeAscend(const void* a, const void* b);
static int __cdecl mtypeDescend(const void* a, const void* b);
static int __cdecl modAscend(const void* a, const void* b);
static int __cdecl modDescend(const void* a, const void* b);



static bool mbConsoleFunctionsAdded = FALSE;
static const char *FGServerListResort(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 2)
   {
      Console->printf("%s(control);", argv[0]);
      return "false";
   }
   SimGui::Control *ctrl = SimGui::findControl(argv[1]);
   ServerListCtrl *list = NULL;
   if (ctrl) list = dynamic_cast<ServerListCtrl *>(ctrl);

   if (! list)
   {
      Console->printf("%s - invalid control %s.", argv[0], argv[1]);
      return "false";
   }
   list->refreshList();
   return "TRUE";
}

static int __cdecl favoriteAscend(const void *a,const void *b)
{
   FearCSDelegate::ServerInfo **entry_A = (FearCSDelegate::ServerInfo **)(a);
   FearCSDelegate::ServerInfo **entry_B = (FearCSDelegate::ServerInfo **)(b);
   if ((*entry_A)->neverPing  == (*entry_B)->neverPing)
   {
      if ((*entry_A)->favorite  == (*entry_B)->favorite)
      {
         if ((*entry_A)->hasBuddy  == (*entry_B)->hasBuddy) {
            if (g_secondarySortPtr != NULL &&
                g_secondarySortPtr != favoriteAscend &&
                g_secondarySortPtr != favoriteDescend) {
               // All things equal...
               return g_secondarySortPtr(a, b);
            } else {
               return stricmp((*entry_A)->name, (*entry_B)->name);
            }
         }
         else {
            return ((*entry_A)->hasBuddy ? -1 : 1);
         }
      }
      else {
         return ((*entry_A)->favorite ? -1 : 1);
      }
   }
   else { 
      return ((*entry_A)->neverPing ? 1 : -1);
   }
} 

static int __cdecl favoriteDescend(const void *a,const void *b)
{
   return -1 * favoriteAscend(a, b);
} 

static int __cdecl statusAscend(const void *a,const void *b)
{
   FearCSDelegate::ServerInfo **entry_A = (FearCSDelegate::ServerInfo **)(a);
   FearCSDelegate::ServerInfo **entry_B = (FearCSDelegate::ServerInfo **)(b);
   if ((*entry_A)->neverPing  == (*entry_B)->neverPing)
   {
      if ((*entry_A)->status == (*entry_B)->status)
      {
         //dedicated servers first
         if ((*entry_A)->dedicated == (*entry_B)->dedicated)
         {
            //password next
            if ((*entry_A)->password == (*entry_B)->password)
            {
               if (g_secondarySortPtr != NULL &&
                   g_secondarySortPtr != statusAscend &&
                   g_secondarySortPtr != statusDescend)
                  return g_secondarySortPtr(a, b);
               else
                  return stricmp((*entry_A)->name, (*entry_B)->name);
            }
            else return ((*entry_A)->password ? 1 : -1);
         }
         else return ((*entry_A)->dedicated ? -1 : 1);
      }
      else return ((*entry_A)->status - (*entry_B)->status);
   }
   else return ((*entry_A)->neverPing ? 1 : -1);
} 

static int __cdecl statusDescend(const void *a,const void *b)
{
   return -1 * statusAscend(a, b);
} 

static int __cdecl serverNameAscend(const void *a,const void *b)
{
   FearCSDelegate::ServerInfo **entry_A = (FearCSDelegate::ServerInfo **)(a);
   FearCSDelegate::ServerInfo **entry_B = (FearCSDelegate::ServerInfo **)(b);
   if ((*entry_A)->neverPing  == (*entry_B)->neverPing)
   {
      int retVal = stricmp((*entry_A)->name, (*entry_B)->name);

      if (retVal == 0 &&
          g_secondarySortPtr != NULL &&
          g_secondarySortPtr != serverNameAscend &&
          g_secondarySortPtr != serverNameDescend)
         return g_secondarySortPtr(a, b);
      else
         return retVal;
   }
   else return ((*entry_A)->neverPing ? 1 : -1);
} 

static int __cdecl serverNameDescend(const void *a,const void *b)
{
   return -1 * serverNameAscend(a, b);
} 

static int __cdecl pingAscend(const void *a,const void *b)
{
   FearCSDelegate::ServerInfo **entry_A = (FearCSDelegate::ServerInfo **)(a);
   FearCSDelegate::ServerInfo **entry_B = (FearCSDelegate::ServerInfo **)(b);
   if ((*entry_A)->neverPing  == (*entry_B)->neverPing)
   {
      if ((*entry_A)->pingTime == (*entry_B)->pingTime)
      {
         if (g_secondarySortPtr != NULL &&
             g_secondarySortPtr != pingAscend &&
             g_secondarySortPtr != pingDescend)
            return g_secondarySortPtr(a, b);
         else
            return stricmp((*entry_A)->name, (*entry_B)->name);
      }
      else return ((*entry_A)->pingTime - (*entry_B)->pingTime);
   }
   else return ((*entry_A)->neverPing ? 1 : -1);
} 

static int __cdecl pingDescend(const void *a,const void *b)
{
   return -1 * pingAscend(a, b);
} 

static int __cdecl playersAscend(const void *a,const void *b)
{
   FearCSDelegate::ServerInfo **entry_A = (FearCSDelegate::ServerInfo **)(a);
   FearCSDelegate::ServerInfo **entry_B = (FearCSDelegate::ServerInfo **)(b);
   if ((*entry_A)->neverPing  == (*entry_B)->neverPing)
   {
      if ((*entry_A)->numPlayers == (*entry_B)->numPlayers)
      {
         if (g_secondarySortPtr != NULL &&
             g_secondarySortPtr != playersAscend &&
             g_secondarySortPtr != playersDescend)
            return g_secondarySortPtr(a, b);
         else
            return stricmp((*entry_A)->name, (*entry_B)->name);
      }
      else return ((*entry_A)->numPlayers - (*entry_B)->numPlayers);
   }
   else return ((*entry_A)->neverPing ? 1 : -1);
} 

static int __cdecl playersDescend(const void *a,const void *b)
{
   return -1 * playersAscend(a, b);
} 

static int __cdecl addressAscend(const void *a,const void *b)
{
   FearCSDelegate::ServerInfo **entry_A = (FearCSDelegate::ServerInfo **)(a);
   FearCSDelegate::ServerInfo **entry_B = (FearCSDelegate::ServerInfo **)(b);
   if ((*entry_A)->neverPing  == (*entry_B)->neverPing)
   {
      int result = stricmp((*entry_A)->transportAddress, (*entry_B)->transportAddress);
      if (! result)
      {
         if (g_secondarySortPtr != NULL &&
             g_secondarySortPtr != addressAscend &&
             g_secondarySortPtr != addressDescend)
            return g_secondarySortPtr(a, b);
         else
            return stricmp((*entry_A)->name, (*entry_B)->name);
      }
      else return result;
   }
   else return ((*entry_A)->neverPing ? 1 : -1);
} 

static int __cdecl addressDescend(const void *a,const void *b)
{
   return -1 * addressAscend(a, b);
} 

static int __cdecl cpuAscend(const void* a, const void* b)
{
   FearCSDelegate::ServerInfo **entry_A = (FearCSDelegate::ServerInfo **)(a);
   FearCSDelegate::ServerInfo **entry_B = (FearCSDelegate::ServerInfo **)(b);
   if ((*entry_A)->neverPing  == (*entry_B)->neverPing)
   {
      if ((*entry_A)->cpuSpeed == (*entry_B)->cpuSpeed)
      {
         if (g_secondarySortPtr != NULL &&
             g_secondarySortPtr != cpuAscend &&
             g_secondarySortPtr != cpuDescend)
            return g_secondarySortPtr(a, b);
         else
            return stricmp((*entry_A)->name, (*entry_B)->name);
      }
      else return ((*entry_A)->cpuSpeed - (*entry_B)->cpuSpeed);
   }
   else return ((*entry_A)->neverPing ? 1 : -1);
}

static int __cdecl cpuDescend(const void* a, const void* b)
{
   return -1 * cpuAscend(a, b);
}

static int __cdecl missionAscend(const void* a, const void* b)
{
   FearCSDelegate::ServerInfo **entry_A = (FearCSDelegate::ServerInfo **)(a);
   FearCSDelegate::ServerInfo **entry_B = (FearCSDelegate::ServerInfo **)(b);
   if ((*entry_A)->neverPing  == (*entry_B)->neverPing)
   {
      int result = stricmp((*entry_A)->missionName, (*entry_B)->missionName);
      if (! result)
      {
         if (g_secondarySortPtr != NULL &&
             g_secondarySortPtr != missionAscend &&
             g_secondarySortPtr != missionDescend)
            return g_secondarySortPtr(a, b);
         else
            return stricmp((*entry_A)->name, (*entry_B)->name);
      }
      else return result;
   }
   else return ((*entry_A)->neverPing ? 1 : -1);
}

static int __cdecl missionDescend(const void* a, const void* b)
{
   return -1 * missionAscend(a, b);
}

static int __cdecl versionAscend(const void* a, const void* b)
{
   FearCSDelegate::ServerInfo **entry_A = (FearCSDelegate::ServerInfo **)(a);
   FearCSDelegate::ServerInfo **entry_B = (FearCSDelegate::ServerInfo **)(b);
   if ((*entry_A)->neverPing  == (*entry_B)->neverPing)
   {
      int result = stricmp((*entry_A)->version, (*entry_B)->version);
      if (! result)
      {
         if (g_secondarySortPtr != NULL &&
             g_secondarySortPtr != versionAscend &&
             g_secondarySortPtr != versionDescend)
            return g_secondarySortPtr(a, b);
         else
            return stricmp((*entry_A)->name, (*entry_B)->name);
      }
      else return result;
   }
   else return ((*entry_A)->neverPing ? 1 : -1);
}

static int __cdecl versionDescend(const void* a, const void* b)
{
   return -1 * versionAscend(a, b);
}

static int __cdecl mtypeAscend(const void* a, const void* b)
{
   FearCSDelegate::ServerInfo **entry_A = (FearCSDelegate::ServerInfo **)(a);
   FearCSDelegate::ServerInfo **entry_B = (FearCSDelegate::ServerInfo **)(b);
   if ((*entry_A)->neverPing  == (*entry_B)->neverPing)
   {
      int result = stricmp((*entry_A)->missionType, (*entry_B)->missionType);
      if (! result)
      {
         if (g_secondarySortPtr != NULL &&
             g_secondarySortPtr != mtypeAscend &&
             g_secondarySortPtr != mtypeDescend)
            return g_secondarySortPtr(a, b);
         else
            return stricmp((*entry_A)->name, (*entry_B)->name);
      }
      else return result;
   }
   else return ((*entry_A)->neverPing ? 1 : -1);
}

static int __cdecl mtypeDescend(const void* a, const void* b)
{
   return -1 * mtypeAscend(a, b);
}

static int __cdecl modAscend(const void* a, const void* b)
{
   FearCSDelegate::ServerInfo **entry_A = (FearCSDelegate::ServerInfo **)(a);
   FearCSDelegate::ServerInfo **entry_B = (FearCSDelegate::ServerInfo **)(b);
   if ((*entry_A)->neverPing  == (*entry_B)->neverPing)
   {
      int result = stricmp((*entry_A)->modName, (*entry_B)->modName);
      if (! result)
      {
         if (g_secondarySortPtr != NULL &&
             g_secondarySortPtr != modAscend &&
             g_secondarySortPtr != modDescend)
            return g_secondarySortPtr(a, b);
         else
            return stricmp((*entry_A)->name, (*entry_B)->name);
      }
      else return result;
   }
   else return ((*entry_A)->neverPing ? 1 : -1);
}

static int __cdecl modDescend(const void* a, const void* b)
{
   return -1 * modAscend(a, b);
}

enum {
   
   SIFavorite = 0,
   SIConnection,
   SIStatus,
   SIName,
   SIPing,
   SIMissionType,
   SIMission,
   SIPlayersAvail,
   SICPU,
   SIAddress,
   SIVersion,
   SIMod,
   SICount
};

static FGArrayCtrl::ColumnInfo gServerInfo[SICount] =
{
   { IDSTR_SI_FAVORITES,       10, 300,   0,   0,    TRUE,    52, favoriteAscend,   favoriteDescend   },
   { IDSTR_SI_CONNECTION,      10, 300,   1,   1,    TRUE,    52, pingAscend,       pingDescend       },
   { IDSTR_SI_STATUS,          10, 300,   2,   2,    TRUE,    54, statusAscend,     statusDescend     },
   { IDSTR_SI_NAME,            10, 300,   3,   3,    TRUE,   105, serverNameAscend, serverNameDescend },
   { IDSTR_SI_PING,            10, 300,   4,   4,    TRUE,    43, pingAscend,       pingDescend       },
   { IDSTR_SI_MTYPE,           10, 300,   5,   5,    TRUE,    90, mtypeAscend,      mtypeDescend      },
   { IDSTR_SI_MISSION,         10, 300,   6,   6,    TRUE,    90, missionAscend,    missionDescend    },
   { IDSTR_054,                10, 300,   7,   7,    TRUE,    59, playersAscend,    playersDescend    },
   { IDSTR_SI_CPU,             10, 300,   8,   8,    TRUE,    33, cpuAscend,        cpuDescend        },
   { IDSTR_SI_ADDRESS,         10, 300,   9,   9,    TRUE,   102, addressAscend,    addressDescend    },
   { IDSTR_SRVR_INFO_VERSION,  10, 300,  10,  10,    TRUE,   100, versionAscend,    versionDescend    },
   { IDSTR_SI_MOD,             10, 300,  11,  11,    TRUE,    90, modAscend,        modDescend        },
};
static FGArrayCtrl::ColumnInfo *gInfoPtrs[SICount];

//there is a mirror of this var in FearGuiColumns.cpp
static char *columnDisplayVariables = "pref::ServerColumnDisplay";

static const char *sortColumnVariable = "pref::JGSortColumn";
static const char *sortAscendVariable = "pref::JGSortAscend";
static const char *sortColumnVariableSecond = "pref::JGSortColumnSecond";
static const char *sortAscendVariableSecond = "pref::JGSortAscendSecond";
static const char *curFilterVariable  = "pref::UseFilter";

int ServerListCtrl::prefColumnToSort         = -1;
bool ServerListCtrl::prefSortAscending       = TRUE;
int ServerListCtrl::prefColumnToSortSecond   = -1;
bool ServerListCtrl::prefSortAscendingSecond = TRUE;

int ServerListCtrl::mConnectionHi = 200;
int ServerListCtrl::mConnectionLo = 350;

   
IMPLEMENT_PERSISTENT_TAG(ServerListCtrl, FOURCC('F','G','s','L'));

ServerListCtrl::ServerListCtrl()
{
}


void ServerListCtrl::readDisplayTable(void)
{
   int i;
   for (i = 0; i < SICount; i++)
   {
      char varBuf[80];
      sprintf(varBuf, "%s%d", columnDisplayVariables, i); 
      const char *var = Console->getVariable(varBuf);
      if (var && var[0] != '\0')
      {
         char buf[64], *temp, *temp2;
         strcpy(buf, var);
         temp = &buf[0];
         
         //set the position;
         temp2 = strchr(temp, ',');
         if (temp2)
         {
            *temp2 = '\0';
            gServerInfo[i].position = atoi(buf);
            temp = temp2 + 1;
         }
         
         //set the active flag
         temp2 = strchr(temp, ',');
         if (temp2)
         {
            *temp2 = '\0';
            if (! stricmp(temp, " ON")) gServerInfo[i].active = TRUE;
            else gServerInfo[i].active = FALSE;
            temp = temp2 + 1;
         }
         
         //set the width
         gServerInfo[i].width = atoi(temp);
      }
      
      //set the pointer table
      gInfoPtrs[gServerInfo[i].position] = &gServerInfo[i];
   }
   
   //now validate the gInfoPtrs table
   bool valid = TRUE;
   for (i = 0; i < SICount; i++)
   {
      bool found = FALSE;
      for (int j = 0; j < SICount; j++)
      {
         if (! gInfoPtrs[j]) break;
         else if (gInfoPtrs[j]->position == i)
         {
            found = TRUE;
            break;
         }
      }
      
      if (! found)
      {
         valid = FALSE;
         break;
      }
   }
   
   //if their not valid, use the default table
   if (! valid)
   {
      Console->printf("Server Info table invalid - resetting");
      for (int i = 0; i < SICount; i++)
      {
         gServerInfo[i].position = i;
         gInfoPtrs[i] = &gServerInfo[i];
      }
      
      writeDisplayTable();
   }
}

void ServerListCtrl::writeScriptFile(void)
{
   FileWStream fileOut("config\\GameServerList.cs");
   char buf[512];
   
   //write out the header
   sprintf(buf, "//-----------------------------------\n");
   fileOut.write(strlen(buf), buf);
   sprintf(buf, "//\n");
   fileOut.write(strlen(buf), buf);
   sprintf(buf, "// Database of GAME servers\n");
   fileOut.write(strlen(buf), buf);
   sprintf(buf, "//\n");
   fileOut.write(strlen(buf), buf);
   sprintf(buf, "//-----------------------------------\n\n");
   fileOut.write(strlen(buf), buf);
   
   //now dump out each addIRCServer command
   for (int i = 0; i < servers.size(); i++)
   {
      sprintf(buf, "addGameServer(\"%s\", \"%s\", \"%s\", \"%d\", \"%s\", \"%s\", \"%s\", \"%s\", \"%d\", \"%s\");\n",
                  servers[i].transportAddress, servers[i].name, servers[i].version,
                  servers[i].pingTime, servers[i].favorite ? "TRUE" : "FALSE",
                  servers[i].modName, servers[i].missionName, servers[i].missionType,
                  servers[i].cpuSpeed, servers[i].neverPing ? "TRUE" : "FALSE");
      fileOut.write(strlen(buf), buf);
   }
}

void ServerListCtrl::serverTimeout(const char *address)
{
   if ((! address) || (! address[0])) return;
   
   //see if the server is in the list
   for (int i = 0; i < servers.size(); i++)
   {
      if (! strcmp(servers[i].transportAddress, address))
      {
         servers[i].status = FearCSDelegate::SERVER_TIMEOUT;
         
         //do not erase it if the server was individually pinged or if the master server timedout
         if ((! stricmp(address, pingAddress)) || mbMasterTimedOut)
         {
            //clear this in rebuildFinished() instead
            //pingAddress[0] = '\0';
         }
         //see if we should remove it from the list
         else if ((! servers[i].favorite) && (! servers[i].neverPing)) 
         {
            if (selectedCell.y >= 0 && serverPtrs[selectedCell.y] == &servers[i])
            {
               selectedCell.set(-1, -1);
            }
            servers.erase(i);
            refresh = TRUE;
         }
         break;
      }
   }
   setUpdate();
}


bool ServerListCtrl::isNeverPing(const char *address)
{
   for (int i = 0; i < servers.size(); i++)
   {
      if (! stricmp(servers[i].transportAddress, address))
      {
         return servers[i].neverPing;
      }
   }
   return FALSE;
}

void ServerListCtrl::setNeverPing(const char *address, bool value)
{
   for (int i = 0; i < servers.size(); i++)
   {
      if (! stricmp(servers[i].transportAddress, address))
      {
         servers[i].neverPing = value;
         break;
      }
   }
}

void ServerListCtrl::rebuildNeverPingList(void)
{
   for (int i = 0; i < servers.size(); i++)
   {
      if (servers[i].neverPing)
      {
         delegate->setNeverPing(servers[i].transportAddress);
      }
   }
}

bool ServerListCtrl::isFavorite(const char *address)
{
   for (int i = 0; i < servers.size(); i++)
   {
      if (! stricmp(servers[i].transportAddress, address))
      {
         return servers[i].favorite;
      }
   }
   return FALSE;
}

void ServerListCtrl::setFavorite(const char *address, bool value)
{
   for (int i = 0; i < servers.size(); i++)
   {
      if (! stricmp(servers[i].transportAddress, address))
      {
         servers[i].favorite = value;
         break;
      }
   }
}

void ServerListCtrl::addGameServer(int argc, const char *argv[])
{
   //sanity checks
   if (argc < 6) return;
   const char *addr = argv[1];
   const char *name = argv[2];
   const char *ver  = argv[3];
   const char *ping = argv[4];
   const char *favStr = argv[5];
   
   const char *serverType = "N/A";
   const char *missionName = "N/A";
   const char *missionType = "N/A";
   int cpuSpeed = -1;
   
   bool favorite;
   bool neverPing;
   
   if (! addr[0]) return;
   if (! name[0]) return;
   if (! ver[0]) return;
   if (! ping[0]) return;
   if (! favStr[0]) return;
   
   if ((! favStr[0]) || (! stricmp(favStr, "FALSE")))
   {
      favorite = FALSE;
   }
   else
   {
      favorite = TRUE;
   }
   
   if (argc < 7)
   {
      neverPing = FALSE;
   }
   else if (argc == 7)
   {
      const char *nevPing = argv[6];
      if ((! nevPing[0]) || (! stricmp(nevPing, "FALSE")))
      {
         neverPing = FALSE;
      }
      else neverPing = TRUE;
   }
   
   else if (argc == 11)
   {
      serverType = argv[6];
      missionName = argv[7];
      missionType = argv[8];
      cpuSpeed = atoi(argv[9]);
      const char *nevPing = argv[10];
      if ((! nevPing[0]) || (! stricmp(nevPing, "FALSE")))
      {
         neverPing = FALSE;
      }
      else neverPing = TRUE;
   }
   
   //preserve the selected cell
   char buf[256];
   buf[0] = '\0';
   if (selectedCell.y >= 0)
   {
      strcpy(buf, serverPtrs[selectedCell.y]->transportAddress);
   }
   serverPtrs.clear();
   selectedCell.set(-1, -1);
   
   FearCSDelegate::ServerInfo *newServer = NULL;
   //first see if the server is already in the list
   for (int i = 0; i < servers.size(); i++)
   {
      if (! strcmp(servers[i].transportAddress, addr))
      {
         newServer = &servers[i];
         break;
      }
   }
   
   //if the server was not already in the list, add a new server entry
   if (! newServer)
   {
      servers.increment();
      new(&servers.last()) FearCSDelegate::ServerInfo;
      newServer = &servers.last();
   }
   
   //now fill in the server detail
   strcpy(newServer->game, "Tribes");
   strcpy(newServer->name, name);
   strcpy(newServer->version, ver);
   strcpy(newServer->missionName, missionName);
   strcpy(newServer->modName, serverType);
   strcpy(newServer->hostInfo, "N/A");
   strcpy(newServer->missionType, missionType);
   newServer->numPlayers = 0;
   newServer->maxPlayers = 0;
   newServer->pingTime = atoi(ping);
   strcpy(newServer->transportAddress, addr);
   
   if (! stricmp(newServer->version, "N/A"))
   {
      newServer->correctVersion = TRUE;
   }
   else
   {
      //only compare the major version number
      char buf[16], *bufPtr;
      strcpy(buf, newServer->version);
      bufPtr = strchr(buf, '.');
      if (bufPtr) *bufPtr = '\0';
      
      char buf2[16];
      strcpy(buf2, FearVersion);
      bufPtr = strchr(buf2, '.');
      if (bufPtr) *bufPtr = '\0';
      
      newServer->correctVersion = !strcmp(buf, buf2);
   }
   
   newServer->dedicated = 0;
   newServer->password = 0;
   newServer->scoreLimit = -1;
   newServer->timeLimit = -1;
   newServer->cpuSpeed = cpuSpeed;
   newServer->status = FearCSDelegate::SERVER_UNKNOWN;
   newServer->favorite = favorite;
   newServer->neverPing = neverPing;
   newServer->hasBuddy = FALSE;
   
   //set the size
   setSize(Point2I(1, servers.size()));
   
   //do it *after* the servers *resizeable - and moveable* vector is complete
   for (int j = 0; j < size.y; j++)
   {
      serverPtrs.push_back(&servers[j]);
   }
   
   //resort
   if (columnToSort >= 0)
   {
      if (columnToSortSecond >= 0 && columnToSortSecond != columnToSort) {
         if (sortAscendingSecond && gInfoPtrs[columnToSortSecond]->sortAscend) {
            g_secondarySortPtr = gInfoPtrs[columnToSortSecond]->sortAscend;
         } else if (!sortAscendingSecond && gInfoPtrs[columnToSortSecond]->sortDescend) {
            g_secondarySortPtr = gInfoPtrs[columnToSortSecond]->sortDescend;
         } else {
            g_secondarySortPtr = NULL;
         }
      } else {
         g_secondarySortPtr = NULL;
      }

      //sort the column
      if (sortAscending && gInfoPtrs[columnToSort]->sortAscend)
      {
         m_qsort((void *)&serverPtrs[0], serverPtrs.size(),
                        sizeof(FearCSDelegate::ServerInfo *),
                        gInfoPtrs[columnToSort]->sortAscend);
      }
      else if ((! sortAscending) && gInfoPtrs[columnToSort]->sortDescend)
      {
         m_qsort((void *)&serverPtrs[0], serverPtrs.size(),
                        sizeof(FearCSDelegate::ServerInfo *),
                        gInfoPtrs[columnToSort]->sortDescend);
      }
   }
   
   //reselecte the selected server
   if (buf[0])
   {
      for (int k = 0; k < size.y; k++)
      {
         if (! strcmp(serverPtrs[k]->transportAddress, buf))
         {
            selectedCell.set(0, k);
            break;
         }
      }
   }
   setUpdate();
}

void ServerListCtrl::removeCurrentServer(void)
{
   if (selectedCell.y >= 0 && selectedCell.y < serverPtrs.size())
   {
      for (int i = 0; i < servers.size(); i++)
      {
         if (&servers[i] == serverPtrs[selectedCell.y])
         {
            delegate->removeServer(servers[i].transportAddress);            
            servers.erase(i);
            selectedCell.set(-1, -1);
            refresh = TRUE;
            break;
         }
      }
   }
}

void ServerListCtrl::onKeyDown(const SimGui::Event &event)
{
   switch(event.diKeyCode)
   {
      case DIK_DELETE:
         removeCurrentServer();
         return;
         
      case DIK_INSERT:
         Console->executef(3, "GuiPushDialog", "MainWindow", "gui\\NewServer.gui");
         break;
   }
}

void ServerListCtrl::updatePingList(const char *address)
{
   //now set the status
   for (int i = 0; i < servers.size(); i++)
   {
      if (! strcmp(servers[i].transportAddress, address))
      {
         servers[i].status = FearCSDelegate::SERVER_PENDING;
         break;
      }
   }
   refresh = TRUE;
   setUpdate();
}

void ServerListCtrl::gotPingInfo(const char *address, const char *name, UInt16 version, int ping)
{
   //get the major version number
   char buf[256];
   strcpy(buf, FearVersion);
   char *tempPtr = strchr(buf, '.');
   AssertFatal(tempPtr, "Fear version is not of the form x.y");
   *tempPtr = '\0';
   UInt16 FearPacketVersion = atoi(buf);
   
   char localVersion[16];
   sprintf(localVersion, "%d", int(version));
   char pingStr[8];
   sprintf(pingStr, "%d", ping);
   const char *argv[6] = { "addGameServer", &address[0], &name[0], &localVersion[0], &pingStr[0], "FALSE", }; 
   
   //see if the server is already in the list
   bool found = FALSE;
   int i;
   for (i = 0; i < servers.size(); i++)
   {
      if (! strcmp(servers[i].transportAddress, address))
      {
         found = TRUE;
         break;
      }
   }

   //if it wasn't, add it to the list
   if (! found) addGameServer(6, argv);
   
   //now set the status
   for (i = 0; i < servers.size(); i++)
   {
      if (! strcmp(servers[i].transportAddress, address))
      {
         strcpy(servers[i].name, name);
         servers[i].pingTime = ping;
         
         //see if the major version has changed
         char buf[32];
         strcpy(buf, servers[i].version);
         char *tempPtr = strchr(buf, '.');
         if (! tempPtr) sprintf(servers[i].version, "%s.0", localVersion);
         else
         {
            //if the major version has changed, update
            *tempPtr = '\0';
            if (stricmp(buf, localVersion))
            {
               sprintf(servers[i].version, "%s.0", localVersion);
            }
         }
         
         //now validate the local version
         if (atoi(localVersion) == int(FearPacketVersion))
         {
            //set pinged, wait for a game response
            servers[i].status = FearCSDelegate::SERVER_PINGED;
            servers[i].correctVersion = TRUE;
         }
         else
         {
            //we won't even do a game info request, so set the status info
            servers[i].status = FearCSDelegate::SERVER_QUERIED;
            servers[i].correctVersion = FALSE;
            servers[i].password = FALSE;
            servers[i].dedicated = FALSE;
         }
            
         break;
      }
   }
   //refresh = TRUE;
   setUpdate();
}

void ServerListCtrl::gotGameInfo(FearCSDelegate::ServerInfo *info)
{
   //now set the status
   for (int i = 0; i < servers.size(); i++)
   {
      if (! strcmp(servers[i].transportAddress, info->transportAddress))
      {
         /*
         //set the status
         servers[i].status = FearCSDelegate::SERVER_QUERIED;
         
         //see if it has a buddy
         servers[i].hasBuddy = findBuddy(&servers[i]);
         
         //get the number of players
         servers[i].numPlayers = info->numPlayers;
            
         //get the maximum number of players
         servers[i].maxPlayers = info->maxPlayers;
         
         //get the mission name
         strncpyt(servers[i].missionName, info->missionName, 31);
         
         //dedicated?
         servers[i].dedicated = info->dedicated;
         
         //password?
         servers[i].password = info->password;
         
         //cpuSpeed
         servers[i].cpuSpeed = info->cpuSpeed;
         
         //mod
         strncpyt(servers[i].modName, info->modName, 31);
         
         //mission type
         strncpyt(servers[i].missionType, info->missionType, 31);
         */
         
         bool favorite = servers[i].favorite;
         bool neverPing = servers[i].neverPing;
         int pingTime = servers[i].pingTime;
         servers[i] = *info;
         servers[i].status = FearCSDelegate::SERVER_QUERIED;
         servers[i].hasBuddy = findBuddy(&servers[i]);
         servers[i].favorite = favorite;
         servers[i].pingTime = pingTime;
         servers[i].neverPing = neverPing;
         
         //update the server info list if it exists
         char buf[256];
         SimGui::SimpleText *ctrl;
         SimGui::Control *dlgCtrl = NULL;
         if (root && root->getTopDialog()) dlgCtrl = root->getTopDialog();
         FearGui::ServerInfoCtrl *siCtrl;
         if (dlgCtrl)
         {
            //server address
            ctrl = (SimGui::SimpleText*)root->getTopDialog()->findControlWithTag(IDCTG_SRVR_INFO_ADDRESS);
            if (ctrl && ctrl->getText() && (! stricmp(ctrl->getText(), info->transportAddress)))
            {
               siCtrl = dynamic_cast<FearGui::ServerInfoCtrl*>(dlgCtrl->findControlWithTag(IDCTG_SRVR_INFO_LIST));
               if (siCtrl) siCtrl->setServerInfo(info);
            
               //server Ping
               ctrl = (SimGui::SimpleText*)dlgCtrl->findControlWithTag(IDCTG_SRVR_INFO_PING);
               if (ctrl) {
                  sprintf(buf, "%d", info->pingTime);
                  ctrl->setText(buf);
               }
               
               //server mission
               ctrl = (SimGui::SimpleText*)dlgCtrl->findControlWithTag(IDCTG_SRVR_INFO_MISSION);
               if (ctrl) ctrl->setText(info->missionName);
            
               //server password
               ctrl = (SimGui::SimpleText*)dlgCtrl->findControlWithTag(IDCTG_SRVR_INFO_PASSWORD);
               if (ctrl) ctrl->setText((info->password ? "YES" : "NO"));
            }
         }
         
         break;
      }
   }
   //refresh = TRUE;
   setUpdate();
}

void ServerListCtrl::onMouseDown(const SimGui::Event &event)
{
   if(!active)
   {
      Parent::onMouseDown(event);
      return;
   }
   Point2I pt = globalToLocalCoord(event.ptMouse);
   
   //if the point is not within the column header
   if (!(pt.y + position.y >= 0 && pt.y + position.y < headerDim.y))
   {
      pt.x -= headerDim.x; pt.y -= headerDim.y;
      Point2I cell((pt.x < 0 ? -1 : pt.x / cellSize.x), (pt.y < 0 ? -1 : pt.y / cellSize.y));
      if(cell.x >= 0 && cell.x < size.x && cell.y >= 0 && cell.y < size.y)
      {
         //see if its in the favorites column
         int widthSoFar = 0;
         for (int i = 0; i < gServerInfo[SIFavorite].position; i++)
         {
            //add the widths of the previous positions
            for (int j = 0; j < SICount; j++)
            {
               if (gServerInfo[j].position == i)
               {
                  widthSoFar += gServerInfo[j].width;
                  break; 
               }
            }
         }
         
         //now see if the point clicked was within the favorites column
         if (pt.x >= widthSoFar && pt.x <= widthSoFar + gServerInfo[SIFavorite].width)
         {
            if (serverPtrs[cell.y]->favorite)
            {
               serverPtrs[cell.y]->favorite = FALSE;
               serverPtrs[cell.y]->neverPing = TRUE;
            }
            else if (serverPtrs[cell.y]->neverPing)
            {
               serverPtrs[cell.y]->favorite = FALSE;
               serverPtrs[cell.y]->neverPing = FALSE;
            }
            else
            {
               serverPtrs[cell.y]->favorite = TRUE;
               serverPtrs[cell.y]->neverPing = FALSE;
            }
         }
         else
         {
            //see if we double clicked on the same cell
            Point2I prevSelected = selectedCell;
            Parent::onMouseDown(event);
            if (selectedCell.y >= 0)
            {
               if (! serverPtrs[selectedCell.y]->correctVersion) return;
               else Console->setVariable("Server::Address", serverPtrs[selectedCell.y]->transportAddress);
               
               if (event.mouseDownCount == 2 && selectedCell.y == prevSelected.y && cg.csDelegate)
               {
                  cg.csDelegate->pushPingInfoRequest(serverPtrs[selectedCell.y]->transportAddress);
                  serverPtrs[selectedCell.y]->status = FearCSDelegate::SERVER_PENDING;
                  strcpy(pingAddress, serverPtrs[selectedCell.y]->transportAddress); 
               }
            }
         }
      }
      else
      {
         Parent::onMouseDown(event);
      }
   }
   else
   {
      Parent::onMouseDown(event);
   }
}

FearCSDelegate::ServerInfo* ServerListCtrl::getServerSelected(bool &infoAvail)
{
   if (selectedCell.y >= 0 && selectedCell.y < serverPtrs.size())
   {
      if (serverPtrs[selectedCell.y]->status == FearCSDelegate::SERVER_QUERIED) infoAvail = TRUE; 
      else infoAvail = FALSE;
      return serverPtrs[selectedCell.y];
   }
	return NULL;
}
   
void ServerListCtrl::onMouseUp(const SimGui::Event &event)
{
   writeDisplayTable();
   Parent::onMouseUp(event);
}

void ServerListCtrl::writeDisplayTable(void)
{
   for (int i = 0; i < SICount; i++)
   {
      char varBuf[80];
      sprintf(varBuf, "%s%d", columnDisplayVariables, i); 
      char buf[64];
      sprintf(buf, "%d, %s, %d", gServerInfo[i].position,
                                 (gServerInfo[i].active ? "ON" : "FALSE"),
                                 gServerInfo[i].width); 
      Console->setVariable(varBuf, buf);
   }
}

bool ServerListCtrl::cellSelectable(const Point2I &cell)
{
   if(cell.y < 0 || cell.y >= servers.size()) return FALSE;
   if (! serverPtrs[cell.y]->correctVersion) return FALSE;
   return TRUE;
}

bool ServerListCtrl::cellSelected(Point2I cell)
{
   //used to unselect all rows, but don't want the Parent::() to resize and change offsets
   if ((cell.y < 0) || (cell.y >= servers.size()))
      return false;
      
   if (cell.y >= 0)
   {
      if (! serverPtrs[cell.y]->correctVersion) return FALSE;
   }

   //if(cell.y >= 0)
   //{
   //   if (! serverPtrs[cell.y]->correctVersion) return false;
   //   else Console->setVariable("Server::Address", serverPtrs[cell.y]->transportAddress);
   //   
   //   if (cg.csDelegate)
   //   {
   //      cg.csDelegate->pushPingInfoRequest(serverPtrs[cell.y]->transportAddress);
   //      serverPtrs[cell.y]->status = FearCSDelegate::SERVER_PENDING;
   //      strcpy(pingAddress, serverPtrs[cell.y]->transportAddress); 
   //   }
   //}
   
   //return Parent::cellSelected(cell);
   selectedCell = cell;
   return TRUE;
}

Int32 ServerListCtrl::getMouseCursorTag(void)
{
   Point2I cursorPos = root->getCursorPos();
   Point2I pt = globalToLocalCoord(cursorPos);
   
   //if the point is within the column header
   int inColumn = -1;
   if (pt.y + position.y >= 0 && pt.y + position.y < headerDim.y)
   {
      //see if the point is on a column boundary
      int widthSoFar = 0;
      for (int i = 0; i < numColumns; i++)
      {
         if (columnInfo[i]->active)
         {
            widthSoFar += columnInfo[i]->width;
            if ((pt.x >= widthSoFar - 2) && (pt.x <= widthSoFar + 2))
            {
               inColumn = -1;
               break;
            }
            else if (pt.x < widthSoFar)
            {
               inColumn = i;
               break;
            }
         }
      }
      if (inColumn < 0) return IDBMP_CURSOR_HADJUST;
   }
   
   return IDBMP_CURSOR_HAND;
}

Int32 ServerListCtrl::getHelpTag(float elapsedTime)
{
   if (elapsedTime < 500.0) return 0;
   
   Point2I cursorPos = root->getCursorPos();
   Point2I pt = globalToLocalCoord(cursorPos);
   
   //if the point is within the column header
   int inColumn = -1;
   if (pt.y + position.y >= 0 && pt.y + position.y < headerDim.y)
   {
      //see if the point is on a column boundary
      int widthSoFar = 0;
      for (int i = 0; i < numColumns; i++)
      {
         if (columnInfo[i]->active)
         {
            widthSoFar += columnInfo[i]->width;
            if ((pt.x >= widthSoFar - 2) && (pt.x <= widthSoFar + 2))
            {
               inColumn = -1;
               break;
            }
            else if (pt.x < widthSoFar)
            {
               inColumn = i;
               break;
            }
         }
      }
   }
   
   if (inColumn < 0) return 0;
   
   //see which column the mouse is in
   switch (columnInfo[inColumn]->origPosition)
   {
      case SIFavorite:
         return IDHELP_SERVER_FAVORITE;
         
      case SIConnection:
         return IDHELP_SERVER_CONNECT;
         
      case SIStatus:
         return IDHELP_SERVER_STATUS;
         
      case SIName:
         return IDHELP_SERVER_NAME;
         
      case SIMission:
         return IDHELP_SERVER_MISSION;
         
      case SIPing:
         return IDHELP_SERVER_PING;
         
      case SIPlayersAvail:
         return IDHELP_SERVER_PLAYERS;
         
      case SICPU:
         return IDHELP_SERVER_CPU;
         
      case SIAddress:
         return IDHELP_SERVER_ADDRESS;
         
      case SIVersion:
         return IDHELP_SERVER_VERSION;
         
      case SIMissionType:
         return IDHELP_SERVER_MISSION_TYPE;
         
      case SIMod:
         return IDHELP_SERVER_TYPE;

      default:
         //return "no help"
         return 0;
   }
}

bool ServerListCtrl::onAdd()
{
   if(!Parent::onAdd())
      return false;

   columnsDualSortable = true;

   //add the console function
   if (! mbConsoleFunctionsAdded)
   {
      mbConsoleFunctionsAdded = TRUE;
      Console->addCommand(0, "Server::ResortList", FGServerListResort);
   }
   
   //status bitmaps
   statusPingRed = SimResource::get(manager)->load("I_PingRed.bmp");
   AssertFatal(statusPingRed, "Unable to load statusPingRed bitmap");
   statusPingRed->attribute |= BMA_TRANSPARENT;
   
   statusPingYellow = SimResource::get(manager)->load("I_PingYellow.bmp");
   AssertFatal(statusPingYellow, "Unable to load statusPingYellow bitmap");
   statusPingYellow->attribute |= BMA_TRANSPARENT;
   
   statusPingGreen = SimResource::get(manager)->load("I_PingGreen.bmp");
   AssertFatal(statusPingGreen, "Unable to load statusPingGreen bitmap");
   statusPingGreen->attribute |= BMA_TRANSPARENT;
   
   statusLocked = SimResource::get(manager)->load("I_Locked.bmp");
   AssertFatal(statusLocked, "Unable to load statusLocked bitmap");
   statusLocked->attribute |= BMA_TRANSPARENT;
   
   statusDedicated = SimResource::get(manager)->load("I_Dedicated.bmp");
   AssertFatal(statusDedicated, "Unable to load statusDedicated bitmap");
   statusDedicated->attribute |= BMA_TRANSPARENT;
   
   statusUnknown = SimResource::get(manager)->load("I_Unknown.bmp");
   AssertFatal(statusUnknown, "Unable to load statusUnknown bitmap");
   statusUnknown->attribute |= BMA_TRANSPARENT;
   
   statusPinging = SimResource::get(manager)->load("I_Pinging.bmp");
   AssertFatal(statusPinging, "Unable to load I_Pinging bitmap");
   statusPinging->attribute |= BMA_TRANSPARENT;
   
   statusQuerying = SimResource::get(manager)->load("I_Querying.bmp");
   AssertFatal(statusQuerying, "Unable to load I_Querying bitmap");
   statusQuerying->attribute |= BMA_TRANSPARENT;
   
   statusTimeout = SimResource::get(manager)->load("I_Timeout.bmp");
   AssertFatal(statusTimeout, "Unable to load statusTimeout bitmap");
   statusTimeout->attribute |= BMA_TRANSPARENT;
   
   statusFavorite = SimResource::get(manager)->load("I_Favorite.bmp");
   AssertFatal(statusFavorite, "Unable to load statusFavorite bitmap");
   statusFavorite->attribute |= BMA_TRANSPARENT;
   
   statusBuddy = SimResource::get(manager)->load("irc_icon_spec.bmp");
   AssertFatal(statusBuddy, "Unable to load statusBuddy bitmap");
   statusBuddy->attribute |= BMA_TRANSPARENT;
   
   //set the local vars
   refresh = TRUE;
   columnToResize = -1;
	numColumns = SICount;
	columnInfo = gInfoPtrs;
   
   //pref vars
	CMDConsole::getLocked()->addVariable(0, sortAscendVariable, CMDConsole::Bool, &prefSortAscending);
	CMDConsole::getLocked()->addVariable(0, sortColumnVariable, CMDConsole::Int,  &prefColumnToSort);
	CMDConsole::getLocked()->addVariable(0, sortAscendVariableSecond, CMDConsole::Bool, &prefSortAscendingSecond);
	CMDConsole::getLocked()->addVariable(0, sortColumnVariableSecond, CMDConsole::Int,  &prefColumnToSortSecond);
   
	CMDConsole::getLocked()->addVariable(0, "pref::ConnectionGoodPing", CMDConsole::Int, &mConnectionHi);
	CMDConsole::getLocked()->addVariable(0, "pref::ConnectionPoorPing", CMDConsole::Int, &mConnectionLo);
   
   //get the widths from the pref vars
   readDisplayTable();
   
   //set the cell dimensions
   int width = 0;
   for (int i = 0; i < SICount; i++)
   {
      if (gInfoPtrs[i]->active)
      {
         width += gInfoPtrs[i]->width;
      }
   }
   cellSize.set(width, hFont->getHeight() + 6);
   
   //set the global
   cg.gameServerList = this;
   
   readScriptFile = TRUE;
   pingAddress[0] = '\0';
	mbMasterTimedOut = TRUE;
   mbQueryMoreServers = FALSE;
   mbRefreshFromMaster = FALSE;
   Console->printf("ServerListCtrl::onAdd() mbQueryMoreServers: %s", mbQueryMoreServers ? "TRUE" : "FALSE");
   
   return true;
}

void ServerListCtrl::onWake()
{
   //set the delegate
   delegate = (FearCSDelegate *) manager->findObject(SimCSDelegateId);
   AssertFatal(delegate, "Unable to locate the FearCSDelegate!");
   
   //get the widths from the pref vars
   readDisplayTable();
   
   //set the cell dimensions
   int width = 0;
   for (int i = 0; i < SICount; i++)
   {
      if (gInfoPtrs[i]->active)
      {
         width += gInfoPtrs[i]->width;
      }
   }
   //cellSize.set(width, hFont->getHeight() + 6);
   cellSize.set(max(long(width), parent->extent.x), cellSize.y);
   
   setSize(Point2I( 1, delegate->mServerList.size()));
   refresh = TRUE;
   mPrevFilter = -1;
   
   //execute the script of stored vars
   if (readScriptFile)
   {
      Console->evaluate("exec(\"GameServerList.cs\");", false);
      readScriptFile = FALSE;
   }
}

void ServerListCtrl::updateCurFilter(void)
{
   mCurFilter = atoi(Console->getVariable(curFilterVariable));
   if (mCurFilter != mPrevFilter)
   {
      mPrevFilter = mCurFilter;
      
      //make sure the list gets updated
      refresh = TRUE;
      
      //clear the array
      entries.clear();
      
      if (mCurFilter < 0) return;
      
      //get the variable
      char buf[256];
      sprintf(buf, "%s%d", gFilterVariable, max(0, mCurFilter));
      const char *var = Console->getVariable(buf);
      
      //if we found a variable
      if (var)
      {
         strncpy(buf, var, 255);
         buf[255] = '\0';
         
         //scan past the filter name
         char *temp = strchr(buf, ':');
         if (! temp) return;
         temp++;
         
         //loop through and read in the conditions
         char *end;
         while (temp)
         {
            FilterCtrl::FilterInfo info;
            
            //find the variable
            end = strchr(temp, ',');
            if (! end) break;
            *end = '\0';
            info.variable = atoi(temp);
            temp = end + 1;
            
            //find the condition
            end = strchr(temp, ',');
            if (! end) break;
            *end = '\0';
            info.condition = atoi(temp);
            temp = end + 1;
            
            //find the value
            end = strchr(temp, ':');
            if (end) *end = '\0';
            if (*temp == '\0') break;
            strncpy(info.value, temp, 255);
            info.value[255] = '\0';
            if (end) temp = end + 1;
            else temp = NULL;
            
            //push the filter condition
            entries.push_back(info);
         }
      }
   }
}

bool ServerListCtrl::findBuddy(FearCSDelegate::ServerInfo *info, const char *buddy)
{
   if ((! buddy) || (! buddy[0])) return FALSE;
   
   //convert the buddy to upper case
   char buddyUpper[250];
   strcpy(buddyUpper, buddy);
   strupr(buddyUpper);
   
   //now go through each line of the info and search for the buddy string
   for (int i = 0; i < info->mInfoList.size(); i++)
   {
      char infoUpper[1024]; 
      strcpy(infoUpper, &(info->mInfoList[i].buf[0]));
      strupr(infoUpper);
      
      //first, make sure it's a player info entry
      if (infoUpper[0] != '1' || infoUpper[1] != '0') continue;
      
      if (strstr(infoUpper, buddyUpper))
      {
         return TRUE;
      }
   }
   
   //not found
   return FALSE;
}

bool ServerListCtrl::findBuddy(FearCSDelegate::ServerInfo *info)
{
   const char *buddy = Console->getVariable("$pref::buddyList");
   if ((! buddy) || (! buddy[0])) return FALSE;
   
   char buddyList[256], *bud, *budEnd;
   strcpy(buddyList, buddy);
   
   bud = &buddyList[0];
   bool lastToken = FALSE;
   while (*bud)
   {
      //get the next token - first skip over white space
      while (*bud == ' ') bud++;
      
      //see if the token is quoted
      if (*bud == '\"')
      {
         bud++;
         budEnd = strchr(bud, '\"');
         if (! budEnd) continue; //unmatched quote
         else
         {
            *budEnd = '\0';
         }
      }
      
      //otherwise the string is space delineated
      else
      {
         budEnd = bud;
         while (*budEnd != ' ' && *budEnd != '\0') budEnd++;
         
         if (*budEnd == '\0') lastToken = TRUE;
         *budEnd = '\0';
      }
      
      //make sure we have a token
      if (*bud != '\0')
      {
         //see if the bud is in the player list
         if (findBuddy(info, bud))
         {
            return TRUE;
         }
         
         //get the next token
         if (lastToken) *bud = '\0';
         else
         {
            bud = budEnd + 1;
         }
      }
   }
   //not found
   return FALSE;
}

bool ServerListCtrl::filterServer(FearCSDelegate::ServerInfo *info)
{
   //check each filter comparison
   Vector<FilterCtrl::FilterInfo>::iterator i;
   for (i = entries.begin(); i != entries.end(); i++)
   {
      //numerical comparisson
      if (gFilterVars[i->variable].type == 0)
      {
         int serverValue;
         int compareValue;
         
         //set the serverValue
         switch (i->variable)
         {
            //ping
            case 0:
               serverValue = info->pingTime;
               break;
            
            //connected players
            case 1:
               serverValue = info->numPlayers;
               break;
               
            //invalid filter
            default:
               continue;
         }
         
         //set the compareValue
         compareValue = atoi(i->value);
         
         //now do the comparison
         switch (i->condition)
         {
            //less than
            case 0:
               if (serverValue >= compareValue) return FALSE;
               break;
               
            //greater than
            case 1:
               if (serverValue <= compareValue) return FALSE;
               break;
               
            //equal to   
            case 2:
               if (serverValue != compareValue) return FALSE;
               break;
               
            //not equal to   
            case 3:
               if (serverValue == compareValue) return FALSE;
               break;
               
            //invalid filter
            default:
               continue;
         }
      }
      
      //text comparisson
      else if (gFilterVars[i->variable].type == 1)
      {
         char serverText[256];
         char compareText[256];
         
         //set the server text
         switch (i->variable)
         {
            //mission name
            case 2:
               strncpy(serverText, info->game, 255);
               break;
            
            //server name
            case 3:
               strncpy(serverText, info->name, 255);
               break;
            
            //mission type
            case 4:
               strncpy(serverText, info->missionType, 255);
               break;
               
            //server type
            case 5:
               strncpy(serverText, info->modName, 255);
               break;
            
            //invalid filter
            default:
               continue;
         }
         
         //set the compareText
         strncpy(compareText, i->value, 255);
         
         //set both strings to upper case
         serverText[255] = '\0';
         compareText[255] = '\0';
         char *temp;
         
         temp = &serverText[0];
         for (UInt32 j = 0; j < strlen(serverText); j++)
         {
            *temp++ = toupper(*temp);
         }
            
         temp = &compareText[0];
         for (UInt32 k = 0; k < strlen(compareText); k++)
         {
            *temp++ = toupper(*temp);
         }
            
         //now do the comparison
         switch (i->condition)
         {
            //contains
            case 0:
               if (! strstr(serverText, compareText)) return FALSE;
               break;
            
            //does not contain
            case 1:
               if (strstr(serverText, compareText)) return FALSE;
               break;
            
            //invalid filter
            default:
               continue;
            
         }
      }
      
      //bool comparisson
      else
      {
         bool value = (i->condition == 0);
         switch(i->variable)
         {
            //is a favorite
            case 6:
               if (info->favorite != value) return FALSE;
               break;
            
            //is dedicated
            case 7:
               if (info->dedicated != value) return FALSE;
               break;
         }
      }
   }
   
   //passed all the filters - return successful
   return TRUE; 
}

void ServerListCtrl::refreshList(void)
{
   serverPtrs.clear();
   refresh = TRUE;
}
   
void ServerListCtrl::rebuildList(void)
{
   for (int i = 0; i < servers.size(); i++)
   {
      servers[i].pingTime = 9999;
      servers[i].status = FearCSDelegate::SERVER_UNKNOWN;
   }
   pingAddress[0] = '\0';
   mbMasterTimedOut = FALSE;
   mbQueryMoreServers = TRUE;
   refresh = TRUE;
   mbRefreshFromMaster = TRUE;
}
   
bool ServerListCtrl::refreshVisible(void)
{
   if ((! cg.csDelegate) || (serverPtrs.size() == 0)) return FALSE;
   
   for (int i = 0; i < serverPtrs.size(); i++)
   {
      cg.csDelegate->pushPingInfoRequest(serverPtrs[i]->transportAddress);
      serverPtrs[i]->status = FearCSDelegate::SERVER_PENDING;
   }
   pingAddress[0] = '\0';
   mbMasterTimedOut = TRUE;
   mbQueryMoreServers = FALSE;
   refresh = TRUE;
   return TRUE;
}
   
void ServerListCtrl::rebuildFinished(bool bMasterTimedOut)
{
   //don't modify the list if we're only pinging a single server
   if (pingAddress[0] != '\0')
   {
      pingAddress[0] = '\0';
      return;
   }
   
   mbMasterTimedOut = bMasterTimedOut;
   
   //remove anything not a favorite, that timed out
   int numServers = servers.size();
   
   //first go through and set the bool
   int i;
   for (i = 0; i < numServers; i++)
   {
      servers[i].timedOut = FALSE;
   }
   
   //preserve the selected server
   char selectedServer[256];
   if (selectedCell.y >= 0 && selectedCell.y < numServers)
   {
      strcpy(selectedServer, serverPtrs[selectedCell.y]->transportAddress);
   }
   else selectedServer[0] = '\0';
   
   for (i = 0; i < numServers; i++)
   {
      if (servers[i].status == FearCSDelegate::SERVER_UNKNOWN || servers[i].status == FearCSDelegate::SERVER_PENDING)
      {
         //see if we should remove it from the list
         if ((servers[i].favorite || mbMasterTimedOut) && (mbQueryMoreServers))
         {
            //now that the rebuild is finished, ping your favorites
            if (cg.csDelegate) cg.csDelegate->pushPingInfoRequest(servers[i].transportAddress);
            servers[i].status = FearCSDelegate::SERVER_PENDING;
         }
         else if (servers[i].favorite || servers[i].neverPing || mbMasterTimedOut || (! mbRefreshFromMaster))
         {
            if (servers[i].status == FearCSDelegate::SERVER_PENDING)
            {
               servers[i].status = FearCSDelegate::SERVER_UNKNOWN;
            }
         }
         else
         {
            //mark the server for deletion
            servers[i].timedOut = TRUE;
         }
         refresh = TRUE;
      }
   }
   
   //now delete all the timedOut servers
   bool rebuild = FALSE;
   int serverNum = 0;
   while (serverNum < servers.size())
   {
      if (servers[serverNum].timedOut)
      {
         servers.erase(serverNum);
         serverNum = 0;
         rebuild = TRUE;
      }
      else serverNum++;
   }
   
   //now rebuild the list of serverPtrs, and reselect the selected
   if (rebuild)
   {
      selectedCell.set(-1, -1);
      serverPtrs.clear();
      for (int i = 0; i < servers.size(); i++)
      {
         serverPtrs.push_back(&servers[i]);
         if (selectedServer[0] && (! stricmp(serverPtrs[i]->transportAddress, selectedServer)))
         {
            selectedCell.set(0, i);
         }
      }
   }
   
   //write the script file, now that we're finished
   writeScriptFile();
   mbQueryMoreServers = FALSE;
}

void ServerListCtrl::rebuildCancel(void)
{
   mbQueryMoreServers = FALSE;
   Console->printf("rebuildCancel() mbQueryMoreServers: %s", mbQueryMoreServers ? "TRUE" : "FALSE");
   mbMasterTimedOut = TRUE;
   for (int i = 0; i < servers.size(); i++)
   {
      if (servers[i].status == FearCSDelegate::SERVER_PENDING)
      {
         servers[i].status = FearCSDelegate::SERVER_UNKNOWN;
      }
   }
   setUpdate();
}

void ServerListCtrl::onPreRender()
{
   //keep the pref vars up to date
   prefColumnToSort  = columnToSort;
   prefSortAscending = sortAscending;
   prefColumnToSortSecond  = columnToSortSecond;
   prefSortAscendingSecond = sortAscendingSecond;
   
   //keep up to date with the current filter
   updateCurFilter();
   
   if (refresh)
   {
      //preserve the selected cell
      char buf[256];
      buf[0] = '\0';
      if (selectedCell.y >= 0)
      {
         strcpy(buf, serverPtrs[selectedCell.y]->transportAddress);
      }
      serverPtrs.clear();
      selectedCell.set(-1, -1);
   
      int i;
      for (i = 0; i < delegate->mServerList.size(); i++)
      {
         FearCSDelegate::ServerInfo *info = &delegate->mServerList[i];
         bool favorite = FALSE;
         bool neverPing = FALSE;
         int pingTime;
         
         FearCSDelegate::ServerInfo *newServer = NULL;
         //first see if the server is already in the list
         for (int j = 0; j < servers.size(); j++)
         {
            if (! strcmp(servers[j].transportAddress, info->transportAddress))
            {
               newServer = &servers[j];
               favorite = newServer->favorite;
               neverPing = newServer->neverPing;
               if (newServer->pingTime == 9999) pingTime = info->pingTime;
               else pingTime = newServer->pingTime;
               break;
            }
         }
         
         //if the server was not already in the list, add a new server entry
         if (! newServer)
         {
            servers.increment();
            new(&servers.last()) FearCSDelegate::ServerInfo;
            newServer = &servers.last();
            pingTime = info->pingTime;
         }
         *newServer = *info;
         newServer->status = FearCSDelegate::SERVER_QUERIED;
         newServer->favorite = favorite;
         newServer->pingTime = pingTime;
         newServer->neverPing = neverPing;
      } 
      
      //now filter the list...
      for (i = 0; i < servers.size(); i++)
      {
         servers[i].hasBuddy = findBuddy(&servers[i]);
         if (filterServer(&servers[i]))
         {
            serverPtrs.push_back(&servers[i]);
         }
      }
      
      //set the size
      setSize(Point2I(1, serverPtrs.size()));
      
      //resort
      if (columnToSort >= 0)
      {
         if (columnToSortSecond >= 0 && columnToSortSecond != columnToSort) {
            if (sortAscendingSecond && gInfoPtrs[columnToSort]->sortAscend) {
               g_secondarySortPtr = gInfoPtrs[columnToSortSecond]->sortAscend;
            } else if (!sortAscendingSecond && gInfoPtrs[columnToSort]->sortDescend) {
               g_secondarySortPtr = gInfoPtrs[columnToSortSecond]->sortDescend;
            } else {
               g_secondarySortPtr = NULL;
            }
         } else {
            g_secondarySortPtr = NULL;
         }

         //sort the column
         if (sortAscending && gInfoPtrs[columnToSort]->sortAscend)
         {
            m_qsort((void *)&serverPtrs[0], serverPtrs.size(),
                           sizeof(FearCSDelegate::ServerInfo *),
                           gInfoPtrs[columnToSort]->sortAscend);
         }
         else if ((! sortAscending) && gInfoPtrs[columnToSort]->sortDescend)
         {
            m_qsort((void *)&serverPtrs[0], serverPtrs.size(),
                           sizeof(FearCSDelegate::ServerInfo *),
                           gInfoPtrs[columnToSort]->sortDescend);
         }
      }
      
      //reselecte the selected server
      if (buf[0])
      {
         for (int k = 0; k < size.y; k++)
         {
            if (! strcmp(serverPtrs[k]->transportAddress, buf))
            {
               selectedCell.set(0, k);
               break;
            }
         }
      }
      refresh = FALSE;
      setUpdate();
      
      //now tally the number of servers/players (just for interest sake)
      int numPlayers = 0;
      for ( i = 0; i < serverPtrs.size(); i++)
      {
         numPlayers += serverPtrs[i]->numPlayers;
      }
      Console->printf("Currently showing %d servers, and %d players.", serverPtrs.size(), numPlayers);
   }
   Parent::onPreRender();
}

static char buffer[256];
char* ServerListCtrl::getCellText(GFXSurface *sfc, const Point2I &cell, const Point2I &cellOffset, const Point2I &cellExtent)
{
   bool ghosted = FALSE;
   if (root)
   {
      SimGui::Control *topDialog = root->getDialogNumber(1);
      if (topDialog && (topDialog != getTopMostParent()) && (topDialog->findControlWithTag(IDCTG_DIALOG)))
      {
         ghosted = TRUE;
      }
   }
   
   switch (cell.x)
   {
      case SIFavorite:
      {
         if (ghosted) break;
         GFXBitmap *bmp;
         if (serverPtrs[cell.y]->neverPing) bmp = statusTimeout;
         else if (serverPtrs[cell.y]->favorite) bmp = statusFavorite; 
         else bmp = NULL;
         
         //SERVER favorite/never ping
         Point2I bmpOffset;
         bmpOffset.x = cellOffset.x + 5;
         bmpOffset.y = cellOffset.y + (cellExtent.y - statusFavorite->getHeight()) / 2 + 1;
         if (bmp) sfc->drawBitmap2d(bmp, &bmpOffset);
         if (serverPtrs[cell.y]->neverPing) return NULL;
         bmpOffset.x += statusFavorite->getWidth() + 3;
         bmp = statusBuddy;
         if (serverPtrs[cell.y]->hasBuddy)
         {
            sfc->drawBitmap2d(bmp, &bmpOffset);
         }
         return NULL;
      }   
      
      case SIConnection:
      {
         if (ghosted) break;
         //SERVER STATUS
         //ping
         Point2I bmpOffset;
         GFXBitmap *bmp;
         if (int(serverPtrs[cell.y]->pingTime) < mConnectionHi) bmp = statusPingGreen;
         else if (int(serverPtrs[cell.y]->pingTime) < mConnectionLo) bmp = statusPingYellow;
         else bmp = statusPingRed;
         bmpOffset.x = cellOffset.x + 5;
         bmpOffset.y = cellOffset.y + (cellExtent.y - bmp->getHeight()) / 2 + 1;
         sfc->drawBitmap2d(bmp, &bmpOffset);
         //bmpOffset.x += bmp->getWidth() + 3;
         return NULL;
      }   
      
      case SIStatus:
      {
         if (ghosted) break;
         //SERVER STATUS
         if (serverPtrs[cell.y]->status == FearCSDelegate::SERVER_QUERIED)
         {
            //password
            Point2I bmpOffset = cellOffset;
            bmpOffset.x = cellOffset.x + 5;
            GFXBitmap *bmp;
            bmp = statusLocked;
            if (serverPtrs[cell.y]->password)
            {
               bmpOffset.y = cellOffset.y + (cellExtent.y - bmp->getHeight()) / 2 + 1;
               sfc->drawBitmap2d(bmp, &bmpOffset);
            } 
            bmpOffset.x += bmp->getWidth() + 3;
            
            //dedicated
            bmp = statusDedicated;
            if (serverPtrs[cell.y]->dedicated)
            {
               bmpOffset.y = cellOffset.y + (cellExtent.y - bmp->getHeight()) / 2 + 1;
               sfc->drawBitmap2d(bmp, &bmpOffset);
            } 
            //bmpOffset.x += bmp->getWidth() + 3;
         }
         else
         {
            //password
            Point2I bmpOffset = cellOffset;
            bmpOffset.x = cellOffset.x + 5;
            GFXBitmap *bmp;
            if (serverPtrs[cell.y]->status == FearCSDelegate::SERVER_UNKNOWN)
            {
               bmp = statusUnknown;
            }
            else if (serverPtrs[cell.y]->status == FearCSDelegate::SERVER_TIMEOUT)
            {
               bmp = statusTimeout;
            }
            else if (serverPtrs[cell.y]->status == FearCSDelegate::SERVER_PENDING)
            {
               bmp = statusPinging;
            }
            else if (serverPtrs[cell.y]->status == FearCSDelegate::SERVER_PINGED)
            {
               bmp = statusQuerying;
            }
            bmpOffset.y = cellOffset.y + (cellExtent.y - bmp->getHeight()) / 2 + 1;
            sfc->drawBitmap2d(bmp, &bmpOffset);
         }
         return NULL;
      }   
      
      case SIName:
         //SERVER NAME
         return serverPtrs[cell.y]->name;  
         
      case SIMission:
         //MISSION NAME
         {
            char *misName = serverPtrs[cell.y]->missionName;
            if (misName[0]) return misName;
            else return "N/A";
         }
         
      case SIPing:
         //PING
         sprintf(buffer, "%d", serverPtrs[cell.y]->pingTime);
         return &buffer[0];
         
      case SIPlayersAvail:
         //Players
         {
            int numPlayers = serverPtrs[cell.y]->numPlayers;
            if (numPlayers >= 0)
            {
               sprintf(buffer, "%d / %d", serverPtrs[cell.y]->numPlayers, serverPtrs[cell.y]->maxPlayers);
               return &buffer[0];
            }
            else return "N/A";
         }
         
      case SICPU:
         //CPU clock speed
         if (serverPtrs[cell.y]->cpuSpeed > 0)
         {
            sprintf(buffer, "%d", serverPtrs[cell.y]->cpuSpeed);
            return &buffer[0];
         }
         else return "N/A";

      case SIAddress:
         //Address
         return serverPtrs[cell.y]->transportAddress;
         
      case SIVersion:
         //Version
         return serverPtrs[cell.y]->version;
      case SIMod:
         return serverPtrs[cell.y]->modName;
      case SIMissionType:
         return serverPtrs[cell.y]->missionType;
   }
   return NULL;
}

};
