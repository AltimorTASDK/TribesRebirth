// This is a class for a sorted linked list of integers.

//#include <bool.h>
//#include <iostream.h>

const WORD BANNED_SERVER_KEY = 0x0000;
const WORD NORMAL_SERVER_KEY = 0x0001;
const WORD ANY_SERVER_KEY    = 0x0002;
const WORD VERIFY_SERVER_KEY = 0x0003; // should be last key in list, all greater numbers used to verify

enum
{
   GameSpyPing = 0x60,
   GameSpyPingResponse,
   GameSpyQuery1,
   GameSpyQuery1Response,
};

class ServerList
{
 private:

	struct node
	{
   	WORD		 key;      // either ids type of server, or used to verify server
		u_long    beat;    // time of last heartbeat
      serv_addr server;  // full network address (in network order)
      u_long    online;   // time server came online
      char      name[MAX_NAME_SIZE+1];  // server name
   	node     *next;
	};

	typedef node * nodeptr;

	nodeptr start;

	int server_count, index, max_servers;

   void DeleteNode( nodeptr p );
   nodeptr FindServer( serv_addr *server );

 public:

// Constructor

	ServerList()
	{
		start = NULL;
		server_count = 0;
      index = 0;
      max_servers = DEFAULT_MAXSERVERS;
	}

// Destructor

	~ServerList()
	{
   	DeleteList();
 	}
	
	int ServerCount (void) {return server_count;}

// Reset fillBuffer index
	void ResetFillIndex()  { index = 0; }

// Delete entire list
	void DeleteList( void );

// Put a node at front of list (most recent heartbeat first)
	void AddServer( serv_addr *server, u_long beat, WORD key, char *name );

// Delete the first node found with the value x, if one exists.
	void DeleteNode( serv_addr *server );

// Time out any old servers in the list
	void TimeoutList( u_long time, u_long timeout );

// Output the values in the nodes, one integer per line.
	void ListServers();

// Return server info specified by index
	bool GetServer( serv_addr *server, int index );

// Return true if there in a node in the list with the value x.
	bool IsInList( serv_addr *server );
	bool IsInList( serv_addr *server, WORD key );

// Update values for server (heartbeat)
// returns false if it's been updated REALLY recently.
	bool UpdateServer( serv_addr *server, u_long beat, WORD key );

// Return a count of the number of nodes in the list.
	int Size();

// Fill buffer with data until full, return TRUE if more data left
	bool FillBuffer( int *num_servers, int *fill_size, int buffer_size, char *buffer );

   void SetMaxServers( int max ) { max_servers = max; };

// checks to see if we have reached the 'max_server' value for this IP address
   bool MaxServersReached( serv_addr *server );
};


