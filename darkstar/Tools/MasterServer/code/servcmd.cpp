#include "windows.h"

#include <stdio.h>
#include <conio.h>
#include <iostream.h>

#include "servcmd.h"


void ServCmd::list_commands( void )
{
	cout << endl;
	cout << "---------------------------------------------" << endl;
	cout << endl;
   for( int i = 0; i < num_commands; i++ )
		cout << command_list[i] << endl;
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
      process_func( 0 );
      Sleep(5);
   }

   input[--index] = '\0';
}

void ServCmd::prompt( void )
{
	cout << endl << "command:";
}


