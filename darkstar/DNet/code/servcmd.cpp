#include "servcmd.h"

#include <stdio.h>
#include <conio.h>
#include <iostream.h>


ServCmd::ServCmd( PROCESS process_func_ptr )
{
	process_func = process_func_ptr;
   input_count = 0;
}


void ServCmd::list_commands( void )
{
	cout << endl;
	cout << "---------------------------------------------" << endl;
	cout << endl;
	cout << "     Command -  Result" << endl;
	cout << "        l - list transports" << endl;
	cout << "        a - activate transport" << endl;
	cout << "        c - connect transport" << endl;
	cout << "        d - disconnect transport" << endl;
   cout << "        s - send message" << endl;
   cout << "        i - initiate/stop bttw test" << endl;
   cout << "        x - turn on/off log file" << endl;
   cout << "        ? - list commands" << endl;
	cout << "        q - quit" << endl;
	cout << endl;
	cout << endl;
	cout << "---------------------------------------------" << endl;

}	// Display_Menu

//-------------------------------------------------------------------

void ServCmd::get_input( void )
{
   char c=0;
   int  index=0;

   while( c != '\r' )
   {
   	if( kbhit() )
      {
			c = getche();
			input[index++] = c;
     	}
      if( process_func )
	      process_func( 0 );
   }

   input[--index] = '\0';
}

bool ServCmd::check_input( void )
{
   char c=0;
	while( kbhit() && c != '\r')
   {
   	c = getche();
   	input[input_count++] = c;
   }

   if( c == '\r' )
   {
   	input[input_count-1] = 0;
      input_count = 0;
      cout << endl;
      return true;
   }
   else
   	return false;
}

void ServCmd::prompt( const char *prompt )
{
	if( prompt )
   	cout << endl << prompt;
   else
		cout << endl << "command:";
}


