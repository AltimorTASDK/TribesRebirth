#include "servlog.h"

#include <stdio.h>
#include <dos.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

void LogFile::init_log( bool active, bool echo, int file_id, char * header )
{
	FILE *fp;

	log_echo = echo;
   log_active = active;

	// create log filename
   strcpy(log_filename,"log");
	itoa((int)file_id, &log_filename[3], 10);
   strcat(log_filename,".txt");

   if( log_active )
   {
		if ((fp=fopen(log_filename,"wt")) == 0)
		{
			printf("FATAL ERROR: failed to open %s",log_filename);
         log_active = 0;
		}
		fclose(fp);
   }

   if(header)
	   log(header);
}

void LogFile::log( char *message, ...)
{
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

	length = strlen( ptr )+1;
	ptr[length-1] = '\n';
	ptr[length] = 0;

	gettime(&time);
	sprintf(time_str,"%02ld:%02ld:%02ld ",(long)time.ti_hour,(long)time.ti_min,(long)time.ti_sec);

   if( log_active )
   {
		if ((fp=fopen(log_filename,"at")) == 0)
		{
			printf(0,"ERROR: failed to open log file '%s'.",log_filename);
         return;
		}

	   fwrite(time_str,strlen(time_str), 1, fp);
		fwrite(assert_message, length, 1, fp );
		fflush(fp);
		fclose(fp);
   }

   if( log_echo )
   {
   	printf( time_str );
      printf( assert_message );
      if( echo_func )
      	echo_func();
   }
}

