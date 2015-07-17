
typedef bool (*PROCESS)( BYTE );

class ServCmd
{
 private:

   PROCESS process_func;
   int     num_commands;
   char   **command_list;

 public:

	char    input[128];
   ServCmd( PROCESS process_func_ptr, int num, char **list )
	{
		process_func = process_func_ptr;
	   num_commands = num;
   	command_list = list;
	};
   ~ServCmd() {};

   void list_commands( void );
   void get_input( void );
   void prompt( void );
};
