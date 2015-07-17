// This class if for responding to queries for info on the game server and players

enum ServerRequests {

// This section is the server request commands, please add new ones to END of section
	SERVER_NAME,
	GAME_NAME,
	GAME_VERSION,
	GAME_ID,
	NUM_PLAYERS,
	MAX_PLAYERS,

// These section is the player request commands, please add new ones to END of section
   PLAYER_NAME,
	PLAYER_ID,
	PLAYER_PING,
	PLAYER_TEAM,

//
	NUM_SERVER_REQUESTS,
   INVALID_KEY
};

typedef enum ServerRequests KEY;


char *server_strings[NUM_SERVER_REQUESTS] = {
// server request strings
	"s_name",
   "g_name",
   "g_version",
   "g_id",
   "num_players",
   "max_players",

// player request strings
   "p_name",
   "p_id",
   "p_ping",
   "p_team",
};

typedef void  (*SEND_DATA)( char *, int, WORD );
typedef char *(*KEY_TO_STRING)( KEY, int );

class ServerInfo
{
 private:

 	char *reply_data;
   int   reply_length;

 	// take a null terminated command string and convert to integer key
 	KEY StringToKey( char * s );

   bool server_key( KEY key ) { return (key < PLAYER_NAME); };

 public:

// Constructor
	ServerInfo( char *, int, SEND_DATA, KEY_TO_STRING );

// Destructor
	~ServerInfo() {}

// Parse an incoming request, build reply, and send. Returns TRUE if error occured
	bool HandleRequest( char * data, WORD packet_key );

// takes a list of keys and converts to list of strings in the reply_data buffer
// returns data size in bytes
   int PackData( KEY list[], int size, int num );

};


