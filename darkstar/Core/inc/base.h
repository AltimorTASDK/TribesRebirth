//================================================================
//   
// $Workfile:   base.h  $
// $Version$
// $Revision:   1.16  $
//   
// DESCRIPTION:
//   
// (c) Copyright Dynamix Inc.   All rights reserved.
//================================================================

#ifndef _BASE_H_
#define _BASE_H_

#include <types.h>
#include <stdarg.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

typedef DWORD Phoenix_AssertFlag;
const Phoenix_AssertFlag ASSERT_NO_LOG     = 1<<0;
const Phoenix_AssertFlag ASSERT_NO_DIALOG  = 1<<1;
const Phoenix_AssertFlag ASSERT_NO_MESSAGE = 1<<3;
const Phoenix_AssertFlag ASSERT_NO_WARNING = 1<<2;

typedef DWORD Phoenix_AssertType;
const Phoenix_AssertType ASSERT_MSG = 1;
const Phoenix_AssertType ASSERT_WRN = 2;
const Phoenix_AssertType ASSERT_ISV = 3;
const Phoenix_AssertType ASSERT_EXC = 4;

const UInt32 AMAX_LENGTH = 255;

typedef void (*AssertCB)(const char *in_line1, const char *in_line2);

//------------------------------ Phoenix_Assert PROTOTYPES

class DLLAPI Phoenix_Assert
{
private:
   AssertCB    fnAssertCB;
   Phoenix_AssertFlag  flags;
   Bool        firstAssert;
   char        logFile[AMAX_LENGTH];
   char        *seperator;
   UInt32      seperatorLength;
   UInt32      countMessage;
   UInt32      countWarning;
   UInt32      countISV;
#ifndef  NO_ASSERT_EXCEPTIONS
   Bool        exceptionsEnabled;
#endif

   void log(const char *in_line1, const char *in_line2);
   void atExit();

public:
   Bool        inDebugger;

   Phoenix_Assert();   
   ~Phoenix_Assert();
   
   void process(Phoenix_AssertType in_type, const char *in_pFile, UInt32 in_line, const char *in_msg);

   void installCallback(AssertCB in_acb);
   void setFlags(Phoenix_AssertFlag in_aflags);

#ifndef  NO_ASSERT_EXCEPTIONS
   void enableExceptions() { exceptionsEnabled = TRUE; }
   void disableExceptions() { exceptionsEnabled = FALSE; }
#endif
};   
extern DLLAPI Phoenix_Assert Assert;
extern DLLAPI const BOOL AFAILED;    //constant equal to 0 (keeps borland from complaining...

#ifndef  NO_ASSERT_EXCEPTIONS
#define AssertEnableExceptions()    Assert.enableExceptions();
#define AssertDisableExceptions()    Assert.disableExceptions();
#endif

//ISV in shipped version (always define)
const char* __cdecl avar(const char *in_msg, ...);
#define AssertISV(tf, msg)  { if ((BOOL)(tf) == AFAILED) Assert.process(ASSERT_ISV, __FILE__, __LINE__, msg); }
#define AssertInstallCallback(in_fn)   Assert.installCallback(in_fn)
#define AssertSetFlags(in_flags)       Assert.setFlags(in_flags)

//------------------------------ DEBUG
#ifdef DEBUG
   //#define AssertMessage Assert.fileLine(ASSERT_MSG, __FILE__, __LINE__); Assert.preprocess
   #define AssertMessage(tf, msg){ if ((BOOL)(tf) == AFAILED) Assert.process(ASSERT_MSG, __FILE__, __LINE__, msg); }
   #define AssertWarn(tf, msg)   { if ((BOOL)(tf) == AFAILED) Assert.process(ASSERT_WRN, __FILE__, __LINE__, msg); }
#ifndef  NO_ASSERT_EXCEPTIONS
   #define AssertFatal(tf, msg)  { if ((BOOL)(tf) == AFAILED) { if(Assert.inDebugger) DebugBreak(); else Assert.process(ASSERT_EXC, __FILE__, __LINE__, msg); }}
#else
   #define AssertFatal(tf, msg)  { if ((BOOL)(tf) == AFAILED) { if(Assert.inDebugger) DebugBreak(); else Assert.process(ASSERT_ISV, __FILE__, __LINE__, msg); }}
#endif

//------------------------------ OPTIMIZED
#else
   #define AssertMessage(tf, msg){}
   #define AssertWarn(tf, msg)   {}
   #define AssertFatal(tf, msg)  {}
#endif   //end DEBUG

#ifndef  NO_ASSERT_EXCEPTIONS
class AssertException
{
public:
   char     fFile[200];
   UInt32   fLine;
   char     fMessage[200];
   AssertException( const char *file, 
                    UInt32 line,
                    const char * msg ) 
      { 
      strncpy( fFile, file, 200 ); 
      fFile[199] = 0;
      fLine = line;
      strncpy( fMessage, msg, 200 ); 
      fMessage[199] = 0;
      }

   void die()
      {
      Assert.process( ASSERT_ISV, fFile, fLine, fMessage );
      }
};
#endif



//------------------------------------------------------------------------------
inline char* strnew( const char *string )
{
   if ( !string ) return NULL;
   return strcpy( new char[ strlen(string)+1 ], string );
}


#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_BASE_H_
