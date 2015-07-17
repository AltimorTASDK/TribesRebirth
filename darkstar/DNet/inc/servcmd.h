
typedef bool (*PROCESS)( int );

class ServCmd
{
 private:

   PROCESS process_func;
   int input_count;

 public:

	char    input[128];
   ServCmd( PROCESS process_func_ptr );
   ~ServCmd() {};

   void list_commands( void );
   void get_input( void );
   bool check_input( void );
   void prompt( const char *prompt );
};
