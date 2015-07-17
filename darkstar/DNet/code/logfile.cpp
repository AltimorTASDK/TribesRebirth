#include "resManager.h"
#include "windows.h"
#include "winbase.h"
#include "logfile.h"

#include <stdio.h>
#include <dos.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#define LOGGING_ENABLED (0)

LogFile::LogFile(bool active, bool echo, int file_id, char * header )
{
   #if LOGGING_ENABLED
	FILE *fp;

	log_echo = echo;
   log_active = active;

	// create log filename
   strcpy(log_filename,"log");
	itoa((int)file_id, &log_filename[3], 10);
   strcat(log_filename,".txt");

   if( log_active )
   {
      AssertISV(ResourceManager::sm_pManager == NULL ||
                ResourceManager::sm_pManager->isValidWriteFileName(log_filename) == true,
                avar("Attempted write to file: %s.\n"
                     "File is not in a writable directory.", log_filename));

		if ((fp=fopen(log_filename,"wt")) == 0)
		{
			printf("FATAL ERROR: failed to open %s",log_filename);
		}
		fclose(fp);
   }

	InitializeCriticalSection(&logSection);
   log(header);
   #endif
}

LogFile::~LogFile()
{
	log_active = false;
   #if LOGGING_ENABLED
	DeleteCriticalSection(&logSection);
   #endif
}

void LogFile::log( char *message, ...)
{
   #if LOGGING_ENABLED
	FILE *fp;
	size_t length;
	char *ptr, assert_message[512];
	char time_str[32];
	va_list arg_ptr;
	struct time time;

	va_start(arg_ptr, message);

	/* Build message string */
	ptr = assert_message;
	if(message)
		vsprintf(ptr, message, arg_ptr);
   va_end(arg_ptr);

	length = strlen( ptr )+1;
	ptr[length-1] = '\n';
	ptr[length] = 0;

	gettime(&time);
	sprintf(time_str,"%02ld:%02ld:%02ld ",(long)time.ti_hour,(long)time.ti_min,(long)time.ti_sec);

   if( log_active )
   {
      AssertISV(ResourceManager::sm_pManager == NULL ||
                ResourceManager::sm_pManager->isValidWriteFileName(log_filename) == true,
                avar("Attempted write to file: %s.\n"
                     "File is not in a writable directory.", log_filename));

		EnterCriticalSection(&logSection);
		if ((fp=fopen(log_filename,"at")) == 0)
		{
			printf(0,"FATAL ERROR: failed to open %s",log_filename);
		}

	   fwrite(time_str,strlen(time_str), 1, fp);
		fwrite(assert_message, length, 1, fp );
		fflush(fp);
		fclose(fp);
		LeaveCriticalSection(&logSection);
   }

   if( log_echo )
   {
   	printf( time_str );
      printf( assert_message );
   }
   #endif
}

void LogFile::log( enum LogType type, char *message, ...)
{
   #if LOGGING_ENABLED
	FILE *fp;
	size_t length;
	char *ptr, assert_message[512];
	char time_str[32];
	va_list arg_ptr;
	struct time time;

	va_start(arg_ptr, message);

	/* Build message string */
	ptr = assert_message;
	if(message)
		vsprintf(ptr, message, arg_ptr);
   va_end(arg_ptr);

	length = strlen( ptr )+1;
	ptr[length-1] = '\n';
	ptr[length] = 0;

	gettime(&time);
	sprintf(time_str,"%02ld:%02ld:%02ld ",(long)time.ti_hour,(long)time.ti_min,(long)time.ti_sec);

   if( log_active || type == LOG_ERROR)
   {
      AssertISV(ResourceManager::sm_pManager == NULL ||
                ResourceManager::sm_pManager->isValidWriteFileName(log_filename) == true,
                avar("Attempted write to file: %s.\n"
                     "File is not in a writable directory.", log_filename));

		EnterCriticalSection(&logSection);
		if ((fp=fopen(log_filename,"at")) == 0)
		{
			printf(0,"FATAL ERROR: failed to open %s",log_filename);
		}

	   fwrite(time_str,strlen(time_str), 1, fp);
		fwrite(assert_message, length, 1, fp );
		fflush(fp);
		fclose(fp);
		LeaveCriticalSection(&logSection);
   }

   if( log_echo  || type == LOG_ERROR)
   {
   	printf( time_str );
      printf( assert_message );
   }
   #endif
}


