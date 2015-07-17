
// defines and defaults for config file variables
#define DEFAULT_SNDBUF      8192
#define MIN_SNDBUF	   	 256
#define MAX_SNDBUF          32768
#define DEFAULT_RCVBUF      32768
#define MIN_RCVBUF		    256
#define MAX_RCVBUF          32768
#define DEFAULT_TIMEOUT     300000  // 300 seconds, five minutes
#define MIN_TIMEOUT         5000
#define MAX_TIMEOUT		    300000
#define DEFAULT_PORT        2222
#define MIN_PORT            2000
#define MAX_PORT            32768
#define MAX_NAME_SIZE       31
#define MAX_MOTD_SIZE       255
#define DEFAULT_PACKET_SIZE 512
#define MIN_PACKET_SIZE     128
#define MAX_PACKET_SIZE     2048
#define DEFAULT_MAXSERVERS  15        // max servers from one IP address

enum {
   CONFIG_SND_BUFFER,
   CONFIG_RCV_BUFFER,
   CONFIG_TIMEOUT,
   CONFIG_LOG_FILE,
   CONFIG_ECHO_FILE,
   CONFIG_SERVER_LIMIT,
   CONFIG_DEFAULT_PORT,
   CONFIG_MASTER_NAME,
   CONFIG_MASTER_ID,
   CONFIG_MOTD_STRING,
   CONFIG_PACKET_SIZE,
   CONFIG_RELOAD_INT,
   CONFIG_BAN_ADDRESS,
   NUM_CONFIG_OPTIONS
};

// message types between mstrsvr, testgame and testspy
#define GAMESPY_QUERY   0x03
#define GAMESVR_REPLY   0x04
#define HEARTBT_MESSAGE 0x05
#define MSTRSVR_REPLY   0x06
#define GAMESVR_VERIFY  0x07
#define NEW_QUERY   0x09

#define REQUEST_ALL_PACKETS   0xFF   // request all packets from masterserver
#define HEADER_SIZE      8           // size of master server packet header in bytes
#define SERVER_INFO_SIZE 7           // amount of info in bytes that is sent for each server

// **** NOTE **** if this structure size is changed, change HEADER_SIZE to match
struct packet {
	BYTE version;
   BYTE type;         // message type
   BYTE packet_num;   // packet number out of total; starts at 1
   BYTE packet_tot;   // total packets of info
   WORD key;          // for verification and id purposes
   WORD id;           // master server id (read from config file)
   char data[MAX_PACKET_SIZE - HEADER_SIZE];
};


struct server_address {
   BYTE           size;   // 6 bytes for the rest of the address
	u_short        port;   // port (network order )
   struct in_addr addr;   // valid address (network order) or INVALID_GAME_SERVER
};

typedef struct server_address serv_addr;



