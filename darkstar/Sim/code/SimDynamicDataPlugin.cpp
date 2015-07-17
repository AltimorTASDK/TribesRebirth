//
// $Workfile$     SimDynamicDataPlugin.cpp
// $Revision$     1.0
// $Author  $     Robert Mobbs (robertm@dynamix.com)
// $Modtime $     04/11/98
//
// Copyright (c) 1998 Dynamix Incorporated.  All rights reserved
//----------------------------------------------------------------------------

#include <assert.h>
#include <ts_material.h>
#include <simtagdictionary.h>
#include <simdynamicdataplugin.h>
#include "simResource.h"

static char const* const s_baseStaticBlockTypes[] =
{
   "TSDEFAULTMATERIALPROPS"
};

enum BaseBlockTypes {
   TSDEFAULTMATERIALS,
   BASE_BLOCK_COUNT
};

static const char *szTypes[] =
{
   "BOOL",
   "CHAR",
   "SHORT",
   "INT",
   "LONG",
   "FLOAT",
   "DOUBLE",
   "TS::DEFAULTMATERIALPROPS::MATERIALPROPS"
};

SimDynamicDataPlugin::SimDynamicDataPlugin()
{
   pDataCurrent = NULL;
}

SimDynamicDataPlugin::~SimDynamicDataPlugin()
{
   vec_data::iterator iter;

   for (iter = data.begin(); iter < data.end(); iter ++)
   {
      delete (*iter);
   }
}

// init is called when the plugin is added to the game
void SimDynamicDataPlugin::init()
{
   // Add our console commands
   console->addCommand(EditDataBlock,           "editDataBlock",           this, 3);
   console->addCommand(RegisterDataBlock,       "registerDataBlock",       this, 3);
   console->addCommand(RegisterStaticDataBlock, "registerStaticDataBlock", this, 3);
   console->addCommand(SetData,                 "setData",                 this, 3);
}

// consoleCallback handles the processing of locally-defined commands
const char *SimDynamicDataPlugin::consoleCallback(CMDConsole *, 
                                          int         iCmd,
                                          int         argc,
                                          const char *argv[])
{
   char szDup[BUFSIZ];

   switch (iCmd)
   {
      // RegisterDataBlock is used to create a new instance of a 
      // known data block type
      case RegisterDataBlock:
         if (argc == 3)
         {
            if (strlen(argv[1]) >= DATANAME_MAX_LENGTH)
            {
               console->printf("registerDataBlock: name too long");
            }
            else
            {
               int ret = registerDataBlock(argv[1], argv[2]);

               switch (ret)
               {
                  case DATAERROR_NOTIMPL:
                     console->printf("registerDataBlock: not implemented");
                     break;

                  case DATAERROR_NOTFOUND:
                     console->printf("registerDataBlock: data type not found");
                     break;

                  case DATAERROR_UNKNOWN:
                     console->printf("registerDataBlock: unknown error");
                     break;

                  case DATAERROR_EXISTS:
                     console->printf("registerDataBlock: named data block exists");
                     break;
               }
            }
         }
         else
         {
            console->printf("registerDataBlock <name> <type>");
         }
         break;

      // RegisterDataBlock is used to create a new instance of a 
      // known data block type
      case RegisterStaticDataBlock:
         if (argc == 3)
         {
            if (strlen(argv[1]) >= DATANAME_MAX_LENGTH)
            {
               console->printf("registerDataBlock: name too long");
            }
            else
            {
               int ret = registerStaticDataBlock(argv[1], argv[2]);

               switch (ret)
               {
                  case DATAERROR_NOTIMPL:
                     console->printf("registerStaticDataBlock: not implemented");
                     break;

                  case DATAERROR_NOTFOUND:
                     console->printf("registerStaticDataBlock: data type not found");
                     break;

                  case DATAERROR_UNKNOWN:
                     console->printf("registerStaticDataBlock: unknown error");
                     break;

                  case DATAERROR_EXISTS:
                     console->printf("registerStaticDataBlock: named data block exists");
                     break;
               }
            }
         }
         else
         {
            console->printf("registerStaticDataBlock <name> <type>");
         }
         break;

      // EditDataBlock will either bring the requested data block into
      // scope, or create the requested data block if it was not found
      case EditDataBlock :
         if (argc == 3)
         {
            if (strlen(argv[1]) >= DATANAME_MAX_LENGTH)
            {
               console->printf("editDataBlock: name too long");
            }
            else
            {
               vec_data::iterator iter;
               strcpy(szDup, argv[1]);

               if ((iter = findData(strupr(szDup))) != data.end())
               {
                  pDataCurrent = (Data *)(*iter);
               }

               else
               {
                  int ret;

                  ret = registerDataBlock(argv[1], argv[2]);

                  switch (ret)
                  {
                     case DATAERROR_NOTIMPL:
                        console->printf("editDataBlock: not implemented");
                        break;

                     case DATAERROR_NOTFOUND:
                        console->printf("editDataBlock: data type not found");
                        break;

                     case DATAERROR_UNKNOWN:
                        console->printf("editDataBlock: unknown error");
                        break;
                  }
               }
            }
         }
         else
         {
            console->printf("editDataBlock <name> <type>");
         }
         break;

      // SetData is used to set the values of elements within an in-scope
      // data block
      case SetData:
      {
         Element            *element;
         vec_elem::iterator  iter;
         int                 iRow = 0, iCol = 0, ret;

         if (argc > 2)
         {
            if (pDataCurrent == NULL)
            {
               ret = DATAERROR_NOCURRENT;
            }

            else
            {
               // See if this is a subscripted element
               ret = subscript(argv[1], iRow, iCol);

               if (ret == DATAERROR_NOERROR)
               {
                  strcpy(szDup, argv[1]);
                  if ((iter  = pDataCurrent->findElem(strupr(szDup))) 
                            != pDataCurrent->elements.end())
                  {
                     element = (Element *)(*iter);

                     ret = verifySubscripts(element, argc - 2, iRow, iCol);

                     if (ret == DATAERROR_NOERROR)
                     {
                        ret = setData(element, &argv[2], argc - 2, iRow, iCol);
                     }
                  }
                  else
                  {
                     ret = DATAERROR_NOTFOUND;
                  }
               }
            }

            switch (ret)
            {
               case DATAERROR_NOCURRENT:
                  console->printf("setData: no current record");
                  break;

               case DATAERROR_NOTFOUND:
                  console->printf("setData: data field: %s not found", argv[1]);
                  break;

               case DATAERROR_INDEXOOB:
                  console->printf("setData: index out of bounds");
                  break;

               case DATAERROR_NOSUBSCRIPT:
                  console->printf("setData: cannot access multidimensional array without subscript");
                  break;

               case DATAERROR_SYNTAX:
                  console->printf("setData: syntax error (badly formed)");
                  break;
            }
         }
         else
         {
            console->printf("setData <name> <value>");
         }
      }
      break;
   }

   return 0;
}

int SimDynamicDataPlugin::registerBlock(const char *lpszName,
                                        void       *lpvBlock,
                                        int         iBlockSize)
{
   char szDup[BUFSIZ];

   strcpy(szDup, lpszName);
   char *lpszUpper = strupr(szDup);
   
   // Get the resource manager
   resManager = SimResource::get(manager);

   // Make sure we got something
   assert(resManager);

   // Make sure the requested name doesn't already exist
   if (findData(lpszUpper) == data.end())
   {
      Data *pData = new Data(lpszUpper, lpvBlock, iBlockSize);

      // Add it to the resource manager
      if (resManager->add(ResourceType::typeof("dynamic.dat"),
          lpszUpper, pData->lpvData, true))
      {
         // Set this as the current Data object
         pDataCurrent = pData;

         // Add it to the local datamap
         data.push_back(pData);

         return (DATAERROR_NOERROR);
      }

      return (DATAERROR_UNKNOWN);
   }

   return (DATAERROR_EXISTS);
}

int SimDynamicDataPlugin::registerStaticBlock(const char *lpszName,
                                              void       *lpvBlock,
                                              int         iBlockSize)
{
   char szDup[BUFSIZ];
   strcpy(szDup, lpszName);
   char *lpszUpper = strupr(szDup);
   
   // Get the resource manager
   resManager = SimResource::get(manager);

   // Make sure we got something
   assert(resManager);

   // Make sure the requested name doesn't already exist
   if (findData(lpszUpper) == data.end())
   {
      Data *pData = new Data(lpszUpper, lpvBlock, iBlockSize, true);

      // Add it to the resource manager
      if (resManager->add(ResourceType::typeof("dynamic.sdt"),
          lpszUpper, pData->lpvData, true))
      {
         // Set this as the current Data object
         pDataCurrent = pData;

         // Add it to the local datamap
         data.push_back(pData);

         return (DATAERROR_NOERROR);
      }

      return (DATAERROR_UNKNOWN);
   }

   return (DATAERROR_EXISTS);
}

int SimDynamicDataPlugin::registerData(const char      *lpszName, 
                                       const type_info &infType, 
                                       void            *lpvOffset, 
                                       int              iRows, 
                                       int              iCols)
{
   int   iSize, iRelativeOffset;
   char  szDup[BUFSIZ];

   strcpy(szDup, lpszName);
   char *lpszUpper = strupr(szDup);

   assert(pDataCurrent);

   // Make sure this data field does not exist in the current data block
   if (pDataCurrent->findElem(lpszUpper) == pDataCurrent->elements.end())
   {
      char szTmp[BUFSIZ];
      Persistent::AbstractClass::convertClassName(infType.name(), szTmp);

      int type = getType(strupr(szTmp));
   
      switch (BASETYPE(type))
      {
         case DATATYPE_CHAR :
            iSize = type & DATAQUALIFIER_PTR ? sizeof(int) : sizeof(char);
            break;

         case DATATYPE_BOOL :
            iSize = sizeof(bool);
            break;

         case DATATYPE_SHORT :
            iSize = sizeof(short);
            break;

         case DATATYPE_INT :
            iSize = sizeof(int);
            break;

         case DATATYPE_LONG :
            iSize = sizeof(long);
            break;

         case DATATYPE_FLOAT :   
            iSize = sizeof(float);
            break;

         case DATATYPE_DOUBLE :
            iSize = sizeof(double);
            break;

         case DATATYPE_TSMATERIALPROP :
            iSize = sizeof(TS::DefaultMaterialProps::MaterialProps);
            break;

         default :
            return DATAERROR_INVALIDTYPE;
      }
         
      // Get the relative offset position (offset of item within structure
      // minus offset of beginning of structure)
      iRelativeOffset = (char *)lpvOffset - (char *)pDataCurrent->lpvBase;

      // Add the new data element to our list of elements
      pDataCurrent->addElem(strupr(szDup), 
                            iRelativeOffset, type, iSize, iRows, iCols);

      return (DATAERROR_NOERROR);
   }
   return (DATAERROR_NOTFOUND);
}

int SimDynamicDataPlugin::setData(Element    *element,
                                  const char *lpszValues[],
                                  int         iValues,
                                  int         iRow,
                                  int         iCol)
{
   int iVal;
   char *lpszData = (char *)pDataCurrent->lpvData + element->offset + 
                    (((element->cols * iRow) + iCol) * element->elemsize);

   // Have to use datatype to correctly convert and pass data
   switch (BASETYPE(element->type))
   {
      case DATATYPE_CHAR :
      {
         if (element->type & DATAQUALIFIER_PTR)
         {
            // Put the offset of the string into the data block
            *lpszData = pDataCurrent->iSize + pDataCurrent->iStrSize;

            // Allocate enough memory for the new string (plus NUL)
            pDataCurrent->lpvData = realloc(pDataCurrent->lpvData, 
                                            pDataCurrent->iSize      + 
                                            pDataCurrent->iStrSize   +
                                            strlen(lpszValues[iRow]) + 1);

                  // Copy the string in
            strcpy((char *)pDataCurrent->lpvData +
                           pDataCurrent->iSize   +
                           pDataCurrent->iStrSize, lpszValues[iRow]);

            // Adjust the size of the string table
            pDataCurrent->iStrSize += strlen(lpszValues[iRow]) + 1;

            resManager = SimResource::get(manager);

            // Make sure we have something
            assert(resManager);

            // Never hurts to double-check
            assert(resManager->find(pDataCurrent->szName));

            // Update pointer in resource manager
            resManager->find(pDataCurrent->szName)->
                             resource = pDataCurrent->lpvData;
         }
         else
         {
            if (element->type & DATAQUALIFIER_UNSIGNED)
            {
               for (int iIndex = 0; iIndex < iValues; iIndex ++)
               {
                  // Fixed-length character strings (character arrays)
                  if (strlen(lpszValues[iIndex]) > 1 &&
                     (element->rows > 1 || element->cols > 1))
                  {
                     strcpy(&lpszData[0], (char *)lpszValues[iIndex]);
                  }

                  else
                  {
                     *((unsigned char *)lpszData) =
                           (unsigned char)(*(lpszValues[iIndex]));
                     lpszData += sizeof(char);
                  }
               }
            }
            else
            {
               for (int iIndex = 0; iIndex < iValues; iIndex ++)
               {
                  // Fixed-length character strings (character arrays)
                  if (strlen(lpszValues[iIndex]) > 1 &&
                     (element->rows > 1 || element->cols > 1))
                  {
                     strcpy(&lpszData[0], (char *)lpszValues[iIndex]);
                  }

                  else
                  {
                     *((char *)lpszData) = *(lpszValues[iIndex]);
                     lpszData += sizeof(char);
                  }
               }
            }
         }
      }
      break;

      case DATATYPE_SHORT :
      {
         if (element->type & DATAQUALIFIER_UNSIGNED)
         {
            for (int iIndex = 0; iIndex < iValues; iIndex ++)
            {
               iVal = atoi(lpszValues[iIndex]);

               *((unsigned short *)lpszData) = iVal;
               lpszData += sizeof(short);
            }
         }
         else
         {
            for (int iIndex = 0; iIndex < iValues; iIndex ++)
            {
               iVal = atoi(lpszValues[iIndex]);

               *((short *)lpszData) = iVal;
               lpszData += sizeof(short);
            }
         }
      }
      break;
 
      case DATATYPE_BOOL :
      {
         for (int iIndex = 0; iIndex < iValues; iIndex ++)
         {
            *((bool *)lpszData) = stricmp(lpszValues[iIndex], "false") ? 
                                  stricmp(lpszValues[iIndex], "true" ) ?
                                  (bool)atoi(lpszValues[iIndex]) : TRUE : FALSE;

            lpszData += sizeof(bool);
         }
      }
      break;

      case DATATYPE_INT :
      {
         if (element->type & DATAQUALIFIER_UNSIGNED)
         {
            for (int iIndex = 0; iIndex < iValues; iIndex ++)
            {
               if (stricmp(lpszValues[iIndex], "false") == 0)
               {
                  *((int *)lpszData) = (int)FALSE;
               }
               else if (stricmp(lpszValues[iIndex], "true") == 0)
               {
                  *((int *)lpszData) = (int)TRUE;
               }
               else
               {
                  iVal = atoi(lpszValues[iIndex]);
                  *((unsigned int *)lpszData) = iVal;
               }

               lpszData += sizeof(int);
            }
         }
         else
         {
            for (int iIndex = 0; iIndex < iValues; iIndex ++)
            {
               if (stricmp(lpszValues[iIndex], "false") == 0)
               {
                  *((int *)lpszData) = (int)FALSE;
               }
               else if (stricmp(lpszValues[iIndex], "true") == 0)
               {
                  *((int *)lpszData) = (int)TRUE;
               }
               else
               {
                  iVal = atoi(lpszValues[iIndex]);
                  *((int *)lpszData) = iVal;
               }

               lpszData += sizeof(int);
            }
         }
      }
      break;

      case DATATYPE_LONG :
      {
         if (element->type & DATAQUALIFIER_UNSIGNED)
         {
            for (int iIndex = 0; iIndex < iValues; iIndex ++)
            {
               iVal = atol(lpszValues[iIndex]);
               *((unsigned long *)lpszData) = iVal;

               lpszData += sizeof(long);
            }
         }
         else
         {
            for (int iIndex = 0; iIndex < iValues; iIndex ++)
            {
               iVal = atol(lpszValues[iIndex]);
               *((long *)lpszData) = iVal;

               lpszData += sizeof(long);
            }
         }
      }
      break;

      case DATATYPE_FLOAT :
      {
         for (int iIndex = 0; iIndex < iValues; iIndex ++)
         {
            *((float *)lpszData) = atof(lpszValues[iIndex]);

            lpszData += sizeof(float);
         }
      }
      break;

      case DATATYPE_DOUBLE :
      { 
         for (int iIndex = 0; iIndex < iValues; iIndex ++)
         {
            *((double *)lpszData) = (double)atof(lpszValues[iIndex]);

            lpszData += sizeof(double);
         }
      }
      break;

      case DATATYPE_TSMATERIALPROP:
      {
         int     iIndex = 0;
         TS::DefaultMaterialProps::MaterialProps prop;

         prop.friction   = atof(lpszValues[iIndex ++]);
         prop.elasticity = atof(lpszValues[iIndex ++]);

         *((TS::DefaultMaterialProps::MaterialProps *)lpszData) = prop;
      }
      break;
   }
   return (DATAERROR_NOERROR);
}

int SimDynamicDataPlugin::subscript(const char *lpszName, int &iRow, int &iCol)
{
   char               *lpszOpen1, *lpszOpen2, *lpszClose;

   // See if they are trying to access an array
   lpszOpen1 = strchr((char *)lpszName, '[');

   // If we don't find a subscript, it may not be an error,
   // so set sentinel values and return okay
   if (lpszOpen1 == NULL)
   {
      iRow = -1;
      iCol = -1;
      return (DATAERROR_NOERROR);
   }

   *lpszOpen1 = '\0';
   lpszOpen1 ++;
      
   // Opening brackets must have matching closing brackets
   lpszClose = strchr(lpszOpen1, ']');

   if (lpszClose == NULL)
   {
      return (DATAERROR_SYNTAX);
   }

   lpszOpen2 = strchr(lpszOpen1, '[');

   // If there is a second opening bracket (multi-dimensional array),
   // make sure it occurs after the first closing bracket, and make
   // sure there is a second closing bracket
   if (lpszOpen2 && (lpszOpen2 < lpszClose || strchr(lpszOpen2, ']') == NULL))
   {
      return (DATAERROR_SYNTAX);
   }

   int  iCnt;
   char szInt[4];
         
   // Convert the passed-in index to an integer
   for (iCnt = 0; *lpszOpen1 != ']' && iCnt < 4; iCnt ++)
   {
      szInt[iCnt] = *(lpszOpen1 ++);
   }

   szInt[iCnt] = '\0';

   // If there was anything between the two brackets ...
   if (iCnt == 0)
   {
      return (DATAERROR_SYNTAX);
   }
            
   // Make sure this is a valid index
   if (iCnt >= 4 || (iRow = atoi(szInt)) < 0)
   {
      iRow = 0;
      return (DATAERROR_INDEXOOB);
   }

   // Now get the column (if specified)
   if (lpszOpen2)
   {
      lpszOpen2 ++;

      for (iCnt = 0; *lpszOpen2 != ']' && iCnt < 4; iCnt ++)
      {
         szInt[iCnt] = *(lpszOpen2 ++);
      }
      
      szInt[iCnt] = '\0';

      if (iCnt == 0)
      {
         iRow = 0;
         return (DATAERROR_SYNTAX);
      }

      if (iCnt >= 4 || (iCol = atoi(szInt)) < 0)
      {
         iRow = 0;
         iCol = 0;
         return (DATAERROR_INDEXOOB);
      }
   }

   return (DATAERROR_NOERROR);
}

int SimDynamicDataPlugin::verifySubscripts(Element *element, 
                                           int      iValues, 
                                           int      &iRow, 
                                           int      &iCol)
{
   if (element->cols > 1)
   {
      if (iValues > 1)
      {
         if (iRow == -1 || iValues < element->cols)
         {
            return DATAERROR_SYNTAX;
         }
         else if (iValues > element->cols || iRow >= element->rows)
         {
            return DATAERROR_INDEXOOB;
         }
      }
      else
      {
         if (iRow == -1 || iCol == -1)
         {
            return DATAERROR_SYNTAX;
         }
      }
   }
   else if (element->rows > 1)
   {
      if (iValues > 1)
      {
         if ((iValues < element->rows || iRow != -1) && ((element->type & 0xFFFF) < DATATYPE_COUNT))
         {
            return DATAERROR_SYNTAX;
         }
         else if (iValues > element->rows)
         {
            return DATAERROR_INDEXOOB;
         }
      }
      else
      {
         if (iRow == -1 && !(element->type & DATATYPE_CHAR))
         {
            return DATAERROR_SYNTAX;
         }
         else if (iRow >= element->rows)
         {
            return DATAERROR_INDEXOOB;
         }
      }
   }
   else if (iRow != -1 || iCol != -1)
   {
      return DATAERROR_SYNTAX;
   }
 
   iRow = (iRow == -1) ? 0 : iRow;
   iCol = (iCol == -1) ? 0 : iCol;

   return (DATAERROR_NOERROR);
}

int SimDynamicDataPlugin::registerDataBlock(const char *, 
                                            const char *)
{
   // Base class knows of no specialized data structures

   return (DATAERROR_NOTIMPL);
}

int SimDynamicDataPlugin::registerStaticDataBlock(const char *lpszName,
                                                  const char *lpszBlock)
{
   int ret = DATAERROR_NOERROR;
   char szDup[BUFSIZ];
   strcpy(szDup, lpszBlock);
   int id  = findStaticBlock(strupr(szDup));

   switch (id)
   {
     case TSDEFAULTMATERIALS: {
      TS::DefaultMaterialProps* pDefaults             = TS::DefaultMaterialProps::getDefaultProps();
      TS::DefaultMaterialProps::MaterialProps* pProps = pDefaults->m_typeProps;

      registerStaticBlock(lpszName, (void*)pProps, sizeof(TS::DefaultMaterialProps::MaterialProps));
      registerData("Default",     typeid(pProps[0]),  &pProps[0]);
      registerData("Concrete",    typeid(pProps[1]),  &pProps[1]);
      registerData("Carpet",      typeid(pProps[2]),  &pProps[2]);
      registerData("Metal",       typeid(pProps[3]),  &pProps[3]);
      registerData("Glass",       typeid(pProps[4]),  &pProps[4]);
      registerData("Plastic",     typeid(pProps[5]),  &pProps[5]);
      registerData("Wood",        typeid(pProps[6]),  &pProps[6]);
      registerData("Marble",      typeid(pProps[7]),  &pProps[7]);
      registerData("Snow",        typeid(pProps[8]),  &pProps[8]);
      registerData("Ice",         typeid(pProps[9]),  &pProps[9]);
      registerData("Sand",        typeid(pProps[10]), &pProps[10]);
      registerData("Mud",         typeid(pProps[11]), &pProps[11]);
      registerData("Stone",       typeid(pProps[12]), &pProps[12]);
      registerData("SoftEarth",   typeid(pProps[13]), &pProps[13]);
      registerData("PackedEarth", typeid(pProps[14]), &pProps[14]);
      }
      break;

     default:
      ret = DATAERROR_NOTFOUND;
      break;
   }
   return ret;
}

SimDynamicDataPlugin::Data::Data(const char *lpszName, void *lpvBase, int iSize, bool isStatic)
{
   // Make a local copy of the data name
   strcpy(szName, lpszName);

   this->iSize    = iSize;
   this->lpvBase  = lpvBase;
   this->isStatic = isStatic;

   iStrSize = 0;

   if (isStatic == false) {
      this->lpvData  = (void *)malloc(iSize);
   } else {
      this->lpvData  = lpvBase;
   }

   if (isStatic == false) {
      // Clear the memory
      memset(lpvData, 0, iSize);
   } else {
      // Assume there is something valid in a static block, do not clear
      //
   }
}

SimDynamicDataPlugin::Data::~Data()
{
   vec_elem::iterator iter;

   // Delete all of this data objects element structures
   for(iter = elements.begin(); iter < elements.end(); iter ++)
   {
      delete (*iter);
   }

   // NOTE: resource manager frees lpvData buffer
}

SimDynamicDataPlugin::vec_elem::iterator 
SimDynamicDataPlugin::Data::findElem(const char *lpszName)
{
   vec_elem::iterator iter;

   for (iter = elements.begin(); iter < elements.end(); iter ++)
   {
      if (strcmp(((Element *)(*iter))->szName, lpszName) == 0)
      {
         return (iter);
      }
   }

   return (elements.end());
}

// This had to be separated into a function b/c both Borland and MSVC
// segfault trying to interpret the templated vector code
void SimDynamicDataPlugin::Data::addElem(const char *lpszName, 
                                         int         iRelativeOffset, 
                                         int         type, 
                                         int         iSize, 
                                         int         iRows,
                                         int         iCols)
{
   Element *element = new Element;

   strcpy(element->szName, lpszName);

   element->offset   = iRelativeOffset; 
   element->type     = type; 
   element->elemsize = iSize; 
   element->rows     = iRows; 
   element->cols     = iCols;

   elements.push_back(element);
}

SimDynamicDataPlugin::vec_data::iterator 
SimDynamicDataPlugin::findData(const char *lpszName)
{
   vec_data::iterator iter;

   for (iter = data.begin(); iter < data.end(); iter ++)
   {
      if (strcmp(((Data *)(*iter))->szName, lpszName) == 0)
      {
         return (iter);
      }
   }

   return (data.end());
}

int SimDynamicDataPlugin::getType(const char *lpszType)
{
   int   iTmp = 0, iType = 0;
   char *lpszTmp;
   char  szDup[BUFSIZ];
   strcpy(szDup, lpszType);
   char *lpszDup = strupr(szDup);

   while (lpszDup && *lpszDup != '\0')
   {
      // Find the next separating token
      for (lpszTmp = lpszDup; lpszTmp && *lpszTmp != '\0'; lpszTmp ++)
      {
         if (*lpszTmp == ' ' || *lpszTmp == '[')
         {
            *(lpszTmp ++) = '\0';
            break;
         }
      }

      if (*lpszDup != '\0')
      {
         // See if we can find the base type
         if (BASETYPE(iType) == 0 && (iTmp = getBaseType(lpszDup)))
         {
            iType |= iTmp;
         }

         // Otherwise, see if it's a qualifier
         else if (strcmp(lpszDup, "UNSIGNED") == 0)
         {
            iType |= DATAQUALIFIER_UNSIGNED;
         }

         // Otherwise, see if it's a ptr
         else if (strchr(lpszDup, '*'))
         {
            iType |= DATAQUALIFIER_PTR;
         }

         // See if it's an array
         else if (strchr(lpszDup, ']'))
         {
            if (iType & DATAQUALIFIER_ARRAY)
            {
               iType ^= DATAQUALIFIER_ARRAY;
               iType |= DATAQUALIFIER_2DARRAY;
            }
            else
            {
               iType |= DATAQUALIFIER_ARRAY;
            }
         }
      }

      lpszDup = lpszTmp;
   }

#if defined(_MSC_VER) || (defined(__BORLANDC__) && (__BORLANDC__ < 0x0530))
   // MS and Borland (before CBuilder 3) see arrays as pointers, so we have 
   // to do some fixups to make sure what we see is what we get

   // Take care of any pointers (except char *) by converting
   // them to arrays
   if ((iType & DATAQUALIFIER_PTR) && (BASETYPE(iType) != DATATYPE_CHAR))
   {
      // If already an array, make it a 2D array
      if (iType & DATAQUALIFIER_ARRAY)
      {
         iType ^= DATAQUALIFIER_ARRAY;
         iType |= DATAQUALIFIER_2DARRAY;
      }
      else
      {
         iType |= DATAQUALIFIER_ARRAY;
      }

      iType ^= DATAQUALIFIER_PTR;
   }

   // Take care of pointer pointers by converting char * * to 
   // array-of-char *, and others to 2D arrays
   if (iType & DATAQUALIFIER_PTRPTR)
   {
      iType ^= DATAQUALIFIER_PTRPTR;
      iType |= BASETYPE(iType) == DATATYPE_CHAR ?
               DATAQUALIFIER_PTR | DATAQUALIFIER_ARRAY : DATAQUALIFIER_2DARRAY;
   }
#endif

   // We may have done some wacky stuff, so a sanity check is good:
   // - pointers other than char * are not allowed
   // - pointers to pointers are not allowed
   // - can't have a type that is both an array and a 2D array
   if (((iType & DATAQUALIFIER_PTR)   && (BASETYPE(iType) != DATATYPE_CHAR)) ||
        (iType & DATAQUALIFIER_PTRPTR) ||
       ((iType & DATAQUALIFIER_ARRAY) && (iType & DATAQUALIFIER_2DARRAY)))
   {
      iType = DATATYPE_COUNT;
   }

   return (iType);
}

int SimDynamicDataPlugin::getBaseType(const char *lpszBlah)
{
   int iIndex;

   for (iIndex = 0; iIndex < DATATYPE_COUNT; iIndex ++)
   {
      if (strcmp(szTypes[iIndex], lpszBlah) == 0)
      {
         return (iIndex);
      }
   }
   
   return (0);
}

int
SimDynamicDataPlugin::findStaticBlock(const char* lpszName)
{
   int   iIndex;
   char  szDup[BUFSIZ];
   strcpy(szDup, lpszName);
   char *lpszDup = strupr(szDup);

   // Strip out classname and structname qualifiers
   if (strstr(lpszDup, "STRUCT") || strstr(lpszDup, "CLASS"))
   {
      lpszDup = strchr(lpszDup, ' ');
      lpszDup ++;
   }

   // Currently can only create simple types -- no arrays or pointers
   // to dynamic structures
   for (iIndex = 0; iIndex < BASE_BLOCK_COUNT; iIndex ++)
   {
      if (strcmp(s_baseStaticBlockTypes[iIndex], lpszName) == 0)
      {
         break;
      }
   }

   return (iIndex == BASE_BLOCK_COUNT) ? -1 : iIndex;
}

