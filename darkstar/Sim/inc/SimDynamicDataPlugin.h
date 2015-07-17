//----------------------------------------------------------------------------
// Description:   Generic dynamic data plugin 
//
// $Workfile$     SimDynamicDataPlugin.h
// $Revision$     1.0
// $Author  $     Robert Mobbs (robertm@dynamix.com)
// $Modtime $     04/11/98
//
// Copyright (c) 1998 Dynamix Incorporated.  All rights reserved
//----------------------------------------------------------------------------

#ifndef _H_SIMDYNAMICDATAPLUGIN
#define _H_SIMDYNAMICDATAPLUGIN

#include <assert.h>
#include <tvector.h>
#include <tstring.h>
#include <simconsoleplugin.h>

// Before CBuilder3, Borland for some reason used
// typeinfo instead of type_info
#ifdef __BORLANDC__
   #if (__BORLANDC__ < 0x0530)
      typedef typeinfo type_info;
   #endif
   #pragma warn -pia
#endif


#define DATANAME_MAX_LENGTH 44

#define BASETYPE(x) (x & 0x0000FFFF)

class SimDynamicDataPlugin : public SimConsolePlugin
{
   public:
      enum
      {
         DATAERROR_NOERROR,
         DATAERROR_NOTFOUND,
         DATAERROR_INDEXOOB,
         DATAERROR_SYNTAX,
         DATAERROR_NOTIMPL,
         DATAERROR_NOCURRENT,
         DATAERROR_EXISTS,
         DATAERROR_NOSUBSCRIPT,
         DATAERROR_INVALIDTYPE,
         DATAERROR_UNKNOWN,
      };

      struct Element
      {
         char   szName[DATANAME_MAX_LENGTH];
         int    offset;
         int    type;
         int    elemsize;
         int    rows;      // needed for 2-d arrays
         int    cols;      // needed for 2-d and 1-d arrays
      };

      typedef Vector<Element *> vec_elem;

      class Data
      {
         public:
            char  szName[DATANAME_MAX_LENGTH];
            int   iSize, iStrSize;
            void *lpvData, *lpvBase;
            bool  isStatic;

            vec_elem elements;

             Data(const char *, void *, int, bool isStatic = false);
            ~Data();
             vec_elem::iterator findElem(const char *);
             void               addElem (const char *, int, int, int, int, int);
      };

      typedef Vector<Data *> vec_data;
      
   protected:
      enum CallbackID
      {
         EditDataBlock = 0,
         RegisterDataBlock,
         RegisterStaticDataBlock,
         SetData
      };

      enum
      {
         // We'll use the first word to specify types ...
         DATATYPE_BOOL,
         DATATYPE_CHAR,
         DATATYPE_SHORT,
         DATATYPE_INT,
         DATATYPE_LONG,
         DATATYPE_FLOAT,
         DATATYPE_DOUBLE,
         DATATYPE_TSMATERIALPROP,
         DATATYPE_COUNT,

         // And the second word to specify qualifiers
         DATAQUALIFIER_UNSIGNED = 0x00010000,
         DATAQUALIFIER_ARRAY    = 0x00020000,
         DATAQUALIFIER_PTR      = 0x00040000,
         DATAQUALIFIER_2DARRAY  = 0x00080000,
         DATAQUALIFIER_PTRPTR   = 0x00100000,
      };

      Data            *pDataCurrent;
      ResourceManager *resManager;
      vec_data         data;

      int findStaticBlock(const char* lpszName);

      // registerBlock creates a new instance of a Data object
      int registerBlock(const char *, void *, int);
      int registerStaticBlock(const char *, void *, int);

      // Needed b/c the map class won't compile under Borland
      vec_data::iterator findData(const char *);
      int                getType(const char *);
      int                getBaseType(const char *);
      int                subscript(const char *, int &, int &);
      virtual int        registerData(const char      *, 
                                      const type_info &, 
                                      void            *, 
                                      int             iRows = 1, 
                                      int             iCols = 1);
      virtual int        setData(Element *, const char *argv[], int, int, int);
      virtual int        registerDataBlock(const char *, const char *);
      virtual int        registerStaticDataBlock(const char *, const char *);
      virtual int        verifySubscripts(Element *, int, int &, int &);

   public:
       SimDynamicDataPlugin();
      ~SimDynamicDataPlugin();

      // Necessary plugin overrides
      virtual void init();
      const char *consoleCallback(CMDConsole *, int, int, const char *argv[]);
};

#endif // _H_SIMDYNAMICDATAPLUGIN

