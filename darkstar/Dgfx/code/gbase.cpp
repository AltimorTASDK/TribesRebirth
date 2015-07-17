//================================================================
//   
// $Workfile:   base.cpp  $
// $Version$
// $Revision:   1.16  $
//   
// DESCRIPTION:
//   
// (c) Copyright Dynamix Inc.   All rights reserved.
//================================================================

#include "base.h"

#define SPRINTF wsprintf

#include <stdio.h>

Phoenix_Assert Assert;
const BOOL AFAILED = 0;

//================================================================
// NAME
//   Phoenix_Assert::Phoenix_Assert
//   
// DESCRIPTION
//   Phoenix_Assert Constructor
//   initializes the Phoenix_Assert class and stores the current working directory
//   
// ARGUMENTS 
//   none
//   
// RETURNS 
//   none
//   
// NOTES 
//   
//================================================================
Phoenix_Assert::Phoenix_Assert():
   fnAssertCB(NULL),
   flags(NULL),
   firstAssert(YES),
   seperator("========================================================================\r\n"),
   countMessage(0),
   countWarning(0),
   countISV(0),
   exceptionsEnabled(FALSE)

{
   seperatorLength = strlen(seperator);

   //get the path to the current working directory and place the assert
   //log file there
   if (GetCurrentDirectory(AMAX_LENGTH-14, logFile))
      strcat(logFile, "\\dgfx.log");
   else
      *logFile = 0;
}  


//================================================================
// NAME
//   Phoenix_Assert::~Phoenix_Assert
//   
// DESCRIPTION
//   Phoenix_Assert destructor
//   
// ARGUMENTS 
//   none
//   
// RETURNS 
//   none
//   
// NOTES 
//   
//================================================================
Phoenix_Assert::~Phoenix_Assert()
{
   atExit();
}   

//================================================================
// NAME
//   Phoenix_Assert::atExit
//   
// DESCRIPTION
//   Preform exit assert report. 
//   If any asserts have been issued display a message showing the 
//   count of each assert type and the full path of the log file.
//   
// ARGUMENTS 
//   none
//   
// RETURNS 
//   none
//   
// NOTES 
//   
//================================================================
void Phoenix_Assert::atExit()
{
#ifdef DEBUG
   if (countWarning || countISV)
   {
      char buff[256];
      wsprintf(buff, "%d\tMessage%s\n%d\tWarning%s\n%d\tFatal\nLogged in %s", 
      countMessage, (countMessage != 1) ? "s":"", 
      countWarning, (countWarning != 1) ? "s":"", countISV, logFile);
      MessageBox( NULL,
         buff, "Assertions Logged",
         MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TASKMODAL);
   } 
#endif   
}


//================================================================
// NAME
//   Phoenix_Assert::setFlags
//   
// DESCRIPTION
//   Set the assert flags
//   
// ARGUMENTS 
//   in_aflags - Bitfield of Phoenix_AssertFlag values
//   
// RETURNS 
//   none
//   
// NOTES 
//   
//================================================================
void Phoenix_Assert::setFlags(Phoenix_AssertFlag in_aflags)
{
    flags = in_aflags;
}


//================================================================
// NAME
//   Phoenix_Assert::installCallback
//   
// DESCRIPTION
//   Install a callback to be executed when a Fatal/ISV assert is
//   issued.
//   
// ARGUMENTS 
//   in_fnAssertCB - pointer to an assert callback.  NULL is valid.
//   
// RETURNS 
//   none
//   
// NOTES 
//   
//================================================================
void Phoenix_Assert::installCallback(void (*in_fnAssertCB)(const char *in_line1, const char *in_line2))
{
    fnAssertCB = in_fnAssertCB;    
}           


//================================================================
// NAME
//   Phoenix_Assert::log
//   
// DESCRIPTION
//   Output an assert text message to the assert log file.
//   
// ARGUMENTS 
//   in_line1 - String specifying assert type, file and line
//              NULL if assertMessage
//   in_line2 - Assert message string
//   
// RETURNS 
//   none
//   updated assert log file
//   
// NOTES 
//   
//================================================================
void Phoenix_Assert::log(const char *in_line1, const char *in_line2)
{
    if (flags & ASSERT_NO_LOG) return;

 //Custom Win32s Code for writing a log file
    HANDLE hFile;
    hFile = CreateFile(logFile, GENERIC_WRITE|GENERIC_READ, 0,
                       NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    
    if (hFile != INVALID_HANDLE_VALUE)
    {
        DWORD bytesWritten;
        SetFilePointer(hFile, 0, NULL, FILE_END);   //seek to end of file
        if (firstAssert)
        {
            WriteFile(hFile, seperator, seperatorLength, &bytesWritten, NULL);
            firstAssert = FALSE;
        }
        if (in_line1) WriteFile(hFile, in_line1, strlen(in_line1), &bytesWritten, NULL);
        WriteFile(hFile, in_line2, strlen(in_line2), &bytesWritten, NULL);
        WriteFile(hFile, "\r\n", 2, &bytesWritten, NULL);
        FlushFileBuffers(hFile);

        CloseHandle(hFile);
    }
}


//================================================================
// NAME
//   avar  (assert variable argument)
//   
// DESCRIPTION
//   Helper function to create variable argument strings for the 
//   assert functions.
//   
// ARGUMENTS 
//   in_msg - printf stype format string
//   ...    - variable number of arguments for string formatting
//   
// RETURNS 
//   const char* - formatted string
//   
// NOTES 
//   Assumes the formatted string will be less than 254 characters.
//   
// EXAMPLE  
//   AssertMessage(avar("this is a %s", foo ? "TEST" : "COOL")); 
//   
//================================================================
const char* __cdecl avar(const char *in_msg, ...)
{
   static char buffer[255];
   va_list pArgs;
   va_start(pArgs, in_msg);
   vsprintf(buffer, in_msg, pArgs);
   return( buffer );
}


//================================================================
// NAME
//   process
//   
// DESCRIPTION
//   Process an assertion.
//   
// ARGUMENTS 
//   in_type - what type of assert is it
//   in_pFile- pointer to __FILE__ where the assert occured
//   in_line - line number (__LINE__) where the assert occured
//   in_msg  - user defined message associated with the assert
//   
// RETURNS 
//   none
//   
// NOTES 
//   
//================================================================
void Phoenix_Assert::process(Phoenix_AssertType in_type, const char *in_pFile, UInt32 in_line, const char *in_msg)
{
   char sFileLine[255];
   switch (in_type)
   {
      case ASSERT_MSG:
         if (flags & (ASSERT_NO_MESSAGE|ASSERT_NO_LOG)) return;   
         countMessage++;
         log(NULL, in_msg);
         break;

      case ASSERT_WRN:
         if (flags & (ASSERT_NO_WARNING|ASSERT_NO_LOG)) return;   
         countWarning++;
         SPRINTF(sFileLine, "WARNING: %s @ %d\r\n  ", in_pFile, in_line);
         log(sFileLine, in_msg);
         break;

      case ASSERT_EXC:
      case ASSERT_ISV:
      default:
         countISV++;
         {
            if( !exceptionsEnabled )
            {
               SPRINTF(sFileLine, "FATAL:   %s @ %d\n\nRetry to Break", in_pFile, in_line);
               if (!(flags & ASSERT_NO_DIALOG))
               {
                  if( IDRETRY == MessageBox( NULL,
                     in_msg,
                     sFileLine,
                     MB_RETRYCANCEL | MB_ICONHAND | 
                     MB_SETFOREGROUND | MB_TASKMODAL) )

                     {
                     DebugBreak();
					 // so we can tell where the assert happened with fastcall
					 return;
					 }
               }
            }
            SPRINTF(sFileLine, "FATAL:   %s @ %d\r\n", in_pFile, in_line);
            if (fnAssertCB)  (*fnAssertCB)(sFileLine, in_msg);
            log(sFileLine, in_msg);

            //Program exit code
#ifndef  NO_ASSERT_EXCEPTIONS
            if( exceptionsEnabled )
               throw AssertException( in_pFile, in_line, in_msg );
            else
#endif            
            {
               atExit();
               ExitProcess(1);
            }
         }
         break;
   }
}

