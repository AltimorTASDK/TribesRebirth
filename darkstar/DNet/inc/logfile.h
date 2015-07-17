
// This class is for logging information to a text file

enum LogType
{
   LOG_ERROR,
   LOG_INFO,
};

class LogFile
{
   private:

   	bool log_active, log_echo;
   	char log_filename[25];
   	CRITICAL_SECTION logSection;

   public:

	   LogFile(bool active, bool echo, int file_id, char * header );
      ~LogFile();
      void log( enum LogType type, char *message, ...);
      void log( char *message, ...);
      void activate( bool activate ) { log_active = activate; };

};

