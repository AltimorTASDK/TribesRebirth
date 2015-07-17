#include "windows.h"
#include "winbase.h"
#include "console.h"
#include "resManager.h"
#include "fearglobals.h"
#include "fileName.h"
#include "banList.h"

static ResourceObject *firstMatch = NULL;

static const char *c_findFirst(CMDConsole *, int, int argc, const char **argv)
{
   // File::findFirst(expr)
   if(argc != 2)
      return "";
   const char *fn;
   firstMatch = cg.resManager->findMatch(argv[1], &fn, NULL);
   if(firstMatch)
      return fn;
   else
      return "";
}

static const char *c_findNext(CMDConsole *, int, int argc, const char **argv)
{
   // File::findNext(expr)
   if(argc != 2)
      return "";
   const char *fn;
   firstMatch = cg.resManager->findMatch(argv[1], &fn, firstMatch);
   if(firstMatch)
      return fn;
   else
      return "";
}

static const char *c_copy(CMDConsole *, int, int argc, const char **argv)
{
   // File::copy(source, dest)
   if (argc != 3)
   {
      Console->printf("Invalid use of File::copy(sourceFN, destFN).");
   }
   else
   {
      if(!ResourceManager::sm_pManager->isValidWriteFileName(argv[2]))
         return "False";

      if (CopyFile(argv[1], argv[2], FALSE))
      {
         return "TRUE";
      }
   }
   return "FALSE";
}

static const char *c_delete(CMDConsole *, int, int argc, const char **argv)
{
   // File::delete(fileName)
   if (argc != 2)
   {
      Console->printf("Invalid use of File::delete(fileName).");
   }
   else
   {
      if(!ResourceManager::sm_pManager->isValidWriteFileName(argv[1]))
         return "False";
      if (DeleteFile(argv[1]))
      {
         return "TRUE";
      }
   }
   return "FALSE";
}

static const char *itoa(int value)
{
   static char buf[20];
   sprintf(buf, "%d", value);
   return buf;
}

static const char *c_findSubStr(CMDConsole *, int, int argc, const char **argv)
{
   if (argc != 3)
      return itoa(-1);
      
   char *stringBase = strnew(argv[1]);
   char *subString = strnew(argv[2]);
   char *strPtr;
   
   //make both strings uppercase
   strPtr = stringBase;
   while (*strPtr != '\0')
   {
      *strPtr = toupper(*strPtr);
      strPtr++;
   }   
   strPtr = subString;
   while (*strPtr != '\0')
   {
      *strPtr = toupper(*strPtr);
      strPtr++;
   }   
   
   //search for the substring
   int strIndex = -1;
   strPtr = strstr(stringBase, subString);
   if (strPtr) strIndex = int(strPtr) - int(stringBase);
   
   //delete the temp strings
   delete [] stringBase;
   delete [] subString;
   
   return itoa(strIndex);
}

static const char *c_getSubStr(CMDConsole *, int, int argc, const char **argv)
{
   if (argc != 4) return "";
   
   static char subString[256];
   
   int start = atoi(argv[2]);
   if (start < 0) return "";
   int len = atoi(argv[3]);
   if (len <= 0) return "";
   
   if (start > int(strlen(argv[1]))) return "";
   
   if (len > 255) len = 255;
   strncpy(subString, &argv[1][start], len);
   subString[len] = '\0';
   
   return subString;
}

static const char *c_strcmp(CMDConsole *, int, int argc, const char **argv)
{
   if (argc != 3) return "";
   int result = strcmp(argv[1], argv[2]);
   return itoa(result);
}

static const char *c_strncmp(CMDConsole *, int, int argc, const char **argv)
{
   if (argc != 4) return "";
   int result = strncmp(argv[1], argv[2], atoi(argv[3]));
   return itoa(result);
}

static const char *c_stricmp(CMDConsole *, int, int argc, const char **argv)
{
   if (argc != 3) return "";
   int result = stricmp(argv[1], argv[2]);
   return itoa(result);
}

static const char *c_strEmpty(CMDConsole *, int, int argc, const char **argv)
{
   if (argc != 2) return "";
   const char *tempPtr = argv[1];
   while (*tempPtr == ' ') tempPtr++;
   if (*tempPtr == '\0') return "true";
   else return "false";
}

static char retBuffer[1024];

// just replaces all spaces with the '_' char
static const char *c_strConvertSpaces(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 2 )
      return("");
   
   const char * src = argv[1];
   char * dest = retBuffer;
   while(*src)
   {
      if(*src == ' ')
      {
         *dest++ = '_';
         *src++;
      }
      else
         *dest++ = *src++;
   }
   *dest = 0;
   return(retBuffer);
}
   
static const char *c_getExt(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 2)
      return "";
   FileName fn(argv[1]);
   strcpy(retBuffer, fn.getExt().c_str());
   return retBuffer;
}

static const char *c_getBase(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 2)
      return "";
   FileName fn(argv[1]);
   strcpy(retBuffer, fn.getBase().c_str());
   return retBuffer;
}

static const char *c_getTitle(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 2)
      return "";
   FileName fn(argv[1]);
   strcpy(retBuffer, fn.getTitle().c_str());
   return retBuffer;
}

static const char *c_getPath(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 2)
      return "";
   FileName fn(argv[1]);
   strcpy(retBuffer, fn.getPath().c_str());
   return retBuffer;
}

static const char *c_addBanA(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 3)
      return "false";
   gBanList.addBan(argv[1], atoi(argv[2]));
   return "true";
}

static const char *c_addBan(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 3)
      return "false";
   gBanList.addBanRelative(argv[1], atoi(argv[2]));
   return "true";
}

static const char *c_removeBan(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 2)
      return "false";
   gBanList.removeBan(argv[1]);
   return "true";
}

static const char *c_exportBan(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 2)
      return "false";
   gBanList.exportToFile(argv[1]);
   return "true";
}

void addFilePluginCommands()
{
   Console->addCommand(0, "File::findFirst", c_findFirst);
   Console->addCommand(0, "File::findNext", c_findNext);
   Console->addCommand(0, "File::copy", c_copy);
   Console->addCommand(0, "File::delete", c_delete);
   Console->addCommand(0, "String::findSubStr", c_findSubStr);
   Console->addCommand(0, "String::getSubStr", c_getSubStr);
   Console->addCommand(0, "String::Compare", c_strcmp);
   Console->addCommand(0, "String::NCompare", c_strncmp);
   Console->addCommand(0, "String::ICompare", c_stricmp);
   Console->addCommand(0, "String::empty", c_strEmpty);
   Console->addCommand(0, "String::convertSpaces", c_strConvertSpaces);
   Console->addCommand(0, "BanList::add", c_addBan);
   Console->addCommand(0, "BanList::addAbsolute", c_addBanA);
   Console->addCommand(0, "BanList::remove", c_removeBan);
   Console->addCommand(0, "BanList::export", c_exportBan);
   Console->addCommand(0, "File::getExt", c_getExt);
   Console->addCommand(0, "File::getBase", c_getBase);
   Console->addCommand(0, "File::getTitle", c_getTitle);
   Console->addCommand(0, "File::getPath", c_getPath);
}