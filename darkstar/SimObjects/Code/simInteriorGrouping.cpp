//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
#include <resManager.h>
#include <simResource.h>
#include <console.h>
#include <simEv.h>

#include "simInterior.h"
#include "simInteriorGrouping.h"

class ResourceTypeInteriorGroupingScript: public ResourceType
{
  public:   
   ResourceTypeInteriorGroupingScript(const char *ext = ".emf")
    : ResourceType( ResourceType::typeof(ext) )  { }
   void* construct(StreamIO *stream,int);
   void  destruct(void *p);
};

static ResourceTypeInteriorGroupingScript  _resemf(".emf");

void* ResourceTypeInteriorGroupingScript::construct(StreamIO *stream, int size)
{
   InteriorGroupingScript* ret = new InteriorGroupingScript;
   ret->size   = size;
   ret->data   = new char[size];
   stream->read(size, ret->data);
   return (void*)ret;
}

void ResourceTypeInteriorGroupingScript::destruct(void *p)
{
   delete (InteriorGroupingScript*) p;
}


const char* const InteriorGroupingScript::sm_pIGScriptBeginTag = "[EMF]";
const char* const InteriorGroupingScript::sm_pIGScriptEndTag   = "[END]";

bool InteriorGroupingScript::getNextLine(char* out_pLine)
{
   AssertFatal(valid, "Invalid script?  Must call isValid first");
   if (isEnd() == true)
      return false;

   int dataPos = 0;
   while (data[curPos] != '\n') {
      out_pLine[dataPos++] = data[curPos];
      curPos++;
   }
   curPos++;

   // Convert the \r to a \0
   //
   out_pLine[dataPos-1] = '\0';
   
   return true;
}

bool InteriorGroupingScript::isEnd()
{
   return (strncmp(&data[curPos], sm_pIGScriptEndTag,
                   strlen(sm_pIGScriptEndTag)) == 0);
}

bool InteriorGroupingScript::isValid()
{
   char trash[512];
   valid = (strncmp(data, sm_pIGScriptBeginTag,
                    strlen(sm_pIGScriptBeginTag)) == 0);
   if (valid)
      getNextLine(trash);

   return valid;
}


//------------------------------------------------------------------------------

bool SimInteriorGrouping::processArguments(int argc, const char** argv)
{
   CMDConsole* console = CMDConsole::getLocked();
   if (argc != 1) {
      console->printf("SimInteriorGrouping: <igFile>.emf");
      return false;
   }

   if (!manager) {
      console->printf("SimInteriorGrouping: no manager");
      return false;
   }
   
   ResourceManager* resManager = SimResource::get(manager);
   if(!resManager) {
      console->printf("SimInteriorGrouping: could not get ResourceManager");
      return false;
   }

   Resource<InteriorGroupingScript> igScript;
   igScript = resManager->load(argv[0]);
   if(!bool(igScript)) {
      console->printf("SimInteriorGrouping: could not load IGScript: %s", argv[0]);
      return false;
   }

   if (parseIGScript(igScript) == false) {
      console->printf("SimInteriorGrouping: could not parse IGScript: %s", argv[0]);
      return false;
   }

   return true;
}


bool SimInteriorGrouping::parseIGScript(Resource<InteriorGroupingScript>& io_igScript)
{
   CMDConsole* console = CMDConsole::getLocked();
   if (io_igScript->isValid() == false)
      return false;

   char pLine[512];
   
   // First line in the script is always the name of the base interior shape
   //
   io_igScript->getNextLine(pLine);
   
   SimInterior* pInterior = new SimInterior;
   
   // Register with the manager, and add to our group...
   //
   manager->registerObject(pInterior);
   addObject(pInterior);
   pInterior->assignName(pLine);
   
   // Make the interior load it's shape...
   //
   const char* argv[1];
   argv[0] = pLine;
   if (pInterior->processArguments(1, argv) == false) {
      console->printf("SimInteriorGrouping: SimInterior failed processArguments");
      return false;
   }
   // Create any sub groups, or entities that remain...
   //
   while (io_igScript->getNextLine(pLine) == true) {
      if (strncmp("GROUP", pLine, strlen("GROUP")) == 0) {
         char groupClass[256];
         sscanf(pLine, "GROUP %s", groupClass);
         io_igScript->getNextLine(pLine);
         if (parseGroupName(groupClass, pLine) == NULL) {
            console->printf("SimInteriorGrouping: parse error processing group");
            return false;
         }
      } else if (strncmp("OBJECT", pLine, strlen("OBJECT")) == 0) {
         if (parseObject(io_igScript) == false) {
            console->printf("SimInteriorGrouping: parse error processing object");
            return false;
         }
      } else {
//         console->printf("SimInteriorGrouping: parse error, did not expect: %s", pLine);
//         return false;
      }
   }

   return true;
}


SimGroup* SimInteriorGrouping::parseGroupName(const char* in_pGroupClass,
                                    char*       io_pGroupSpec,
                                    const bool  in_createGroup)
{
   Vector<char*> pGroupNames;
   char* strtokArg = io_pGroupSpec;
   char* strtokRet;
   
   while ((strtokRet = strtok(strtokArg, ":\n")) != NULL) {
      strtokArg = NULL;
      pGroupNames.push_back(strtokRet);
   }
   
   SimGroup* pCurrGroup = this;
   Vector<char*>::iterator itr = pGroupNames.begin();
   for ( ; itr != (pGroupNames.end() - 1); itr++) {
      SimObject* pObject = pCurrGroup->findObject(*itr);
      pCurrGroup = dynamic_cast<SimGroup*>(pObject);
      if (pCurrGroup == NULL) {
         return NULL;
      }
   }

   if (in_createGroup == true) {
      Persistent::Base* pBaseObj = (Persistent::Base*)Persistent::create(in_pGroupClass);
      SimGroup* pNewGroup = dynamic_cast<SimGroup*>(pBaseObj);
      if (pNewGroup == NULL) {
         CMDConsole* console = CMDConsole::getLocked();
         console->printf("parseGroupName: persistent create failed for class: %s",
                         in_pGroupClass);
         return NULL;
      }
      manager->registerObject(pNewGroup);
      pCurrGroup->addObject(pNewGroup);
      pCurrGroup->assignName(pNewGroup, pGroupNames.last());
      
      return pNewGroup;
   } else {
      SimObject* pObject = pCurrGroup->findObject(pGroupNames.last());
      SimGroup* pRet = dynamic_cast<SimGroup*>(pObject);
      
      return pRet;
   }
}

bool SimInteriorGrouping::parseObject(Resource<InteriorGroupingScript>& io_igScript)
{
   CMDConsole* console = CMDConsole::getLocked();
   
   char lineBuff[512];
   char nameBuff[128];
   
   // First line is the group
   io_igScript->getNextLine(lineBuff);
   SimGroup* pGroup = parseGroupName("SimGroup", lineBuff, false);
   if (pGroup == NULL) {
      return false;
   }
   
   // Next line is the object name, which we'll set aside for the moment...
   io_igScript->getNextLine(nameBuff);
   
   // Class name, create and verify the object...
   io_igScript->getNextLine(lineBuff);
   SimObject* pSimObj = static_cast<SimObject*>(Persistent::create(lineBuff));
   if (pSimObj == NULL) {
      console->printf("Failed persistent create of class %s", lineBuff);
      return false;
   }
   
   // Add it to the group and name it...
   manager->registerObject(pSimObj);
   pGroup->addObject(pSimObj);
   pGroup->assignName(pSimObj, nameBuff);
   
   // Get the args string (max 32 args)
   int         argc = 0;
   const char* argv[32];
   io_igScript->getNextLine(lineBuff);
   
   char* strtokArg = lineBuff;
   char* strtokRet;
   while ((strtokRet = strtok(strtokArg, " \t")) != NULL) {
      argv[argc++] = strtokRet;
      strtokArg    = NULL;
   }
   
   if (pSimObj->processArguments(argc, argv) == false) {
      // Some sort of goof-up
      return false;
   }
   
   // And the position and rotation.  Note that object must respond to the
   //  SimObjectTransformEvent for this to be useful...
   //
   Point3F position;
   EulerF  rotation;
   io_igScript->getNextLine(lineBuff);
   sscanf(lineBuff, "%f %f %f", &position.x, &position.y, &position.z);
   io_igScript->getNextLine(lineBuff);
   sscanf(lineBuff, "%f %f %f", &rotation.x, &rotation.y, &rotation.z);
   
   // fire off the event...
   SimObjectTransformEvent transEvent;
   transEvent.tmat.set(rotation, position);
   pSimObj->processEvent(&transEvent);
   
   return true;
}
