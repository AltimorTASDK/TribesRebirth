
// This class is for logging information to a text file

typedef void (*ECHO_FUNC)( void );

class LogFile
{
   private:

   	bool log_active, log_echo;
   	char log_filename[25];
      ECHO_FUNC echo_func;

   public:

	   LogFile(ECHO_FUNC efunc) { log_active = 0; echo_func = efunc;}
      void log( char *message, ...);
		void init_log( bool active, bool echo, int file_id, char * header );
      void set_log(bool active) { log_active = active; };
      void set_echo(bool echo) { log_echo = echo; };

};

