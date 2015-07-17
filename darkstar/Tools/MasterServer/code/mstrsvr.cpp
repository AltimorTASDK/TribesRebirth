#include "windows.h"
#include "winsock.h"
#include "stdio.h"
#include "mstrsvr.h"
#include "servlist.h"
#include "servlog.h"
#include "servcmd.h"

#include <iostream.h>

bool process_socket( BYTE response_type );
void echo_function( void );
int  format_query_string( char * data );
void parse_args( short argc, char * argv[] );
void read_config_file(void);
void init_socket( void );
void query_game(void);
BYTE get_total_packets( int num_servers, int server_size, int packet_bytes, int first_packet_start);

char config_file[] = "MSTRSVR.CFG";
char *config_options[NUM_CONFIG_OPTIONS] = {
   "SND_BUFFER",
   "RCV_BUFFER",
   "TIMEOUT",
   "LOG_FILE",
   "ECHO_FILE",
   "SERVER_LIMIT",
   "DEFAULT_PORT",
	"MASTER_NAME",
   "MASTER_ID",
	"MOTD_STRING",
	"PACKET_SIZE",
	"RELOAD_INTERVAL",
   "BAN_ADDRESS",
};

const char num_commands = 9;
char *command_list[num_commands] = {
	"        d - display game server list",
   "        b - display banned server list",
   "        r - reload config file",
   "        g - query game server",
   "        l - toggle log file",
   "        e - toggle log echo",
   "        x - delete game server list",
   "        ? - list commands",
	"        q - quit",
};

class LogFile log_file(echo_function);
class ServerList server_list, banned_list, verify_list;
class ServCmd command( process_socket, num_commands, command_list );

SOCKET gSocket;
struct sockaddr_in client, server;
BYTE   version_number = 0x10;

// config file variables
int     snd_buf = DEFAULT_SNDBUF;
int     rcv_buf = DEFAULT_RCVBUF;
u_long  timeout_value =DEFAULT_TIMEOUT;
bool    enable_log=true, enable_echo=false;
u_short server_port = 0;
int     max_servers = DEFAULT_MAXSERVERS;
bool    reset_prompt = true;
char    master_name[MAX_NAME_SIZE+1], motd_string[MAX_MOTD_SIZE+1];
DWORD	  reload_interval=900000, next_reload_time;
WORD    packet_size=DEFAULT_PACKET_SIZE;
WORD    master_id = 0;


void main(int argc, char **argv)
{
   bool	 running = TRUE;

   /*---------------------------------------------------*/
   /* parse command line arguments and initialize stuff */
   /*---------------------------------------------------*/

   parse_args( argc, argv );
   log_file.init_log(TRUE,TRUE,2,NULL);
   read_config_file();

   init_socket();

   command.list_commands();
   command.prompt();
   do
   {
      command.get_input();
      switch( command.input[0] )
      {

         case 'd':
         	server_list.ListServers();
         	break;

         case 'b':
         	banned_list.ListServers();
         	break;

         case 'r':
			   read_config_file();
         	break;

         case 'g':
         	query_game();
//            wait_for_response( GAMESVR_REPLY );
            break;

      	case 'q':
         case 'Q':
         	running = FALSE;
            break;

         case 'l':
         	enable_log = !enable_log;
            log_file.set_log(enable_log);
            cout << "Log file is " << (enable_log?"on.":"off.") << endl;
         	break;

         case 'e':
         	enable_echo = !enable_echo;
            log_file.set_echo(enable_echo);
            cout << "Log echo is " << (enable_echo?"on.":"off.") << endl;
         	break;

         case '?':
         case 'h':
         	command.list_commands();
            break;

         default:
         case 'x':
         	server_list.DeleteList();
         	break;
      }
      if( GetTickCount() > next_reload_time )
      {
      	read_config_file();
         next_reload_time = GetTickCount() + reload_interval;
      }
      command.prompt();
   }
   while( running );
}

void init_socket()
{
   int    ret_val;

   /*------------------------*/
	/* Initialize WinSock DLL */
   /*------------------------*/

   WSADATA stWSAData;

   ret_val = WSAStartup(0x0101, &stWSAData);
   if( ret_val != 0 )
   {
      printf("WSAStartup failed, error: %d\n", ret_val);
      exit(EXIT_FAILURE);
   }


	/*-------------------*/
   /* Initialize socket */
   /*-------------------*/

   gSocket = socket(AF_INET, SOCK_DGRAM, 0);
   if( gSocket == INVALID_SOCKET )
   {
   	printf("Unable to create a socket!\n");
      WSACleanup();
      exit(EXIT_FAILURE);
   }

	/*-----------------------------*/
   /* Set socket to non-blocking. */
   /*-----------------------------*/

	u_long ioctl_arg = 1;
   ret_val = ioctlsocket( gSocket, FIONBIO, &ioctl_arg );
   if( ret_val == SOCKET_ERROR ) //
   {
   	ret_val = WSAGetLastError();
   	printf("Unable to unblock socket, error: %d\n", ret_val);
      WSACleanup();
      exit(EXIT_FAILURE);
   }

   /*----------------------------*/
   /* bind socket to server port */
   /*----------------------------*/

	memset((char *)&server, 0, sizeof(server));
	server.sin_family      = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port        = htons(server_port);

	if( bind(gSocket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR )
   {
   	ret_val = WSAGetLastError();
   	printf("Unable to bind socket, error: %d\n", ret_val);
      WSACleanup();
      exit(EXIT_FAILURE);
   }

   /*------------------------------------------------------------*/
   /* Set the socket buffer size from command line (default=32K) */
   /*------------------------------------------------------------*/

   int optlen = sizeof(int);
   if( setsockopt( gSocket, SOL_SOCKET, SO_SNDBUF, (char *)&snd_buf, optlen ))
   {
   	ret_val = WSAGetLastError();
   	printf("Unable to set SNDBUF, error: %d\n", ret_val);
      WSACleanup();
      exit(EXIT_FAILURE);
   }
   if( setsockopt( gSocket, SOL_SOCKET, SO_RCVBUF, (char *)&rcv_buf, optlen ))
   {
   	ret_val = WSAGetLastError();
   	printf("Unable to set RCVBUF, error: %d", ret_val);
      WSACleanup();
      exit(EXIT_FAILURE);
   }

   /*-----------------------------------------*/
   /* Prepare to receive packets from clients */
   /*-----------------------------------------*/

	memset((char *) &client, 0, sizeof(client));
	client.sin_family = AF_INET;
	client.sin_addr.s_addr = htonl(INADDR_ANY);
	client.sin_port = htons(server_port);

}

bool process_socket( BYTE response_type )
{
   int     	 addr_len, ret_val;
	serv_addr game_server;
   struct packet receive_packet, send_packet;
   int    	 num_servers, fill_size, current_byte;
	int packets_to_send;
   BYTE      packet_num, packet_tot;

	addr_len = sizeof( client );
   ret_val = recvfrom(gSocket, (char *) &receive_packet, packet_size, 0,
                         (struct sockaddr *)&client, &addr_len);

   if( ret_val == SOCKET_ERROR )
   {
    	ret_val = WSAGetLastError(); // if WSAWOULDBLOCK, no message was waiting
		if( ret_val != WSAEWOULDBLOCK )
      {
      	printf("Function recvfrom failure, error: %d\n", ret_val);
		   WSACleanup();
		   exit(EXIT_FAILURE);
      }
      return false;
   }

	/* check to seee if server is banned */
   game_server.addr.s_addr = client.sin_addr.s_addr;
   game_server.port        = client.sin_port;
   if( banned_list.IsInList( &game_server ) )
   {
   	log_file.log("Ignoring message from banned server %s",inet_ntoa(client.sin_addr));
      return false;
   }

   /*---------------------------------------------*/
   /* check version number                        */
   /* for now, only support exact version matches */
   /*---------------------------------------------*/

   if( receive_packet.version != version_number )
   {
   	log_file.log("Ignoring message: Invalid version %d",(int)receive_packet.version);
      return false;
   }
   DWORD time = GetTickCount();

   switch( receive_packet.type )
   {
   	case GAMESPY_QUERY: // reply to GAMESPY_QUERY with list of GameServers

         log_file.log("Query from %s:%ld, id = %ld",inet_ntoa(client.sin_addr),(long)ntohs(client.sin_port),(long)ntohs(receive_packet.key));

			// get rid of old entries before sending off
      	server_list.TimeoutList(time, timeout_value);

         /*--------------------------------------------------------------------*/
         /* The first packet we send will have the MASTER_NAME and MOTD string */
         /* Note that the null byte is not sent.                               */
         /*--------------------------------------------------------------------*/

         current_byte = 0;
         send_packet.data[current_byte++] = (BYTE)strlen(master_name);
         strcpy(&send_packet.data[current_byte],master_name);
         current_byte += strlen(master_name);

         send_packet.data[current_byte++] = (BYTE)strlen(motd_string);
         strcpy(&send_packet.data[current_byte],motd_string);
         current_byte += strlen(motd_string);

         /*---------------------------------------------------------------*/
      	/* Since the buffer may not be large enough to handle all of the */
         /* servers, keep sending packets until all the data gets across. */
         /*---------------------------------------------------------------*/

         packet_tot = get_total_packets(server_list.Size(), SERVER_INFO_SIZE, packet_size-HEADER_SIZE-sizeof(short), current_byte);
         packets_to_send = receive_packet.packet_num;
         if( packets_to_send == REQUEST_ALL_PACKETS )
         	packets_to_send = packet_tot;
         //cout<<"   Sending "<< packets_to_send <<" packets out of "<< (int)packet_tot << endl;
			server_list.ResetFillIndex();
			for( packet_num = 1; packet_num <= packet_tot; packet_num++ )
         {
         	// FillBuffer fills the data buffer with server information
            // 2 byte are reserved in data buffer for num_servers
				server_list.FillBuffer( &num_servers, &fill_size,
              					packet_size-HEADER_SIZE-current_byte,
                           &send_packet.data[current_byte+sizeof(short)] );

            if( receive_packet.packet_num == REQUEST_ALL_PACKETS ||
                receive_packet.packet_num == packet_num )
            {
					send_packet.packet_num = (BYTE)packet_num;
               send_packet.packet_tot = (BYTE)packet_tot;
					send_packet.version    = (BYTE)version_number;
   	      	send_packet.type       = (BYTE)MSTRSVR_REPLY;
      		   send_packet.key        = receive_packet.key;
               send_packet.id         = htons(master_id);

	           	*((short *)&send_packet.data[current_byte]) = htons((short)num_servers);
               current_byte += sizeof(short);

	  				ret_val = sendto(gSocket, (char *) &send_packet,
                                HEADER_SIZE+current_byte+fill_size, 0,
  			                      (struct sockaddr *)&client, addr_len);
               if (ret_val == SOCKET_ERROR) {
                  cout<<"   Sent "<< (int)packet_num << " ret_val = " << (int)ret_val << endl;
                  cout << " error code : " << int(WSAGetLastError()) << endl;
               }
            }
            current_byte = 0;
         }
         break;

      case HEARTBT_MESSAGE:
        	server_list.TimeoutList(time, timeout_value); // get rid of old entries
  	      // if server already in list, just update new heartbeat time
  	      if( server_list.IsInList( &game_server ) )
     	   	server_list.UpdateServer( &game_server, time, NORMAL_SERVER_KEY );
			// If server not in list, add to verification list
         // and query game server for verification
        	else
         {
         	// max sure that we don't already have too many addresses from same IP
          	if(!server_list.MaxServersReached( &game_server ))
            {
					// assign verification key and add server to verify list
					WORD key=NORMAL_SERVER_KEY;
               do
               {
               	key = (WORD)GetTickCount();
               }
               while (key <= VERIFY_SERVER_KEY );

               // get rid of old entries
        			verify_list.TimeoutList(time, timeout_value);
               // don't want duplicate entries, so if already in verify
               // list for some reason, just update key
               if( verify_list.IsInList( &game_server ))
               {
               	if(verify_list.UpdateServer( &game_server, time, key))
                     return false;
               }
               else
	               verify_list.AddServer( &game_server, time, key, NULL);

            	// send off verification message
               send_packet.version = version_number;
					send_packet.type    = (BYTE)GAMESPY_QUERY;
               send_packet.key     = htons(key);
               send_packet.id      = htons(master_id);
					fill_size=format_query_string( send_packet.data);
   				ret_val = sendto(gSocket, (char *) &send_packet,
            	             HEADER_SIZE+fill_size, 0,
									(struct sockaddr *)&client, addr_len);
            }
         }
         break;

      case GAMESVR_REPLY:

      	// first, see if server is in verify list at all

         if(BYTE(receive_packet.packet_tot) == 0xf0) // check game type
         {
            if( verify_list.IsInList( &game_server ) )
            {
         	   // is in list, now check key (we could do all this in one step but we get better error checking this way
				   if( verify_list.IsInList( &game_server, ntohs(receive_packet.key) ))
               {
            	   // make sure max servers haven't been reached for this server (this will rarely happen at this stage)
	          	   if(!server_list.MaxServersReached( &game_server ))
                  {
            		   // server name should be part of message
	            	   server_list.AddServer( &game_server, time, NORMAL_SERVER_KEY, &receive_packet.data[2]);
   	               verify_list.DeleteNode( &game_server );
                  }
               }
               else
               {
                  log_file.log("Error: Wrong verification key sent by server: %s:%ld",inet_ntoa(game_server.addr),ntohs(game_server.port));
               }
            }
            else
            {
         	   log_file.log("Received unsolicited GAMESVR_REPLY");
            }
	     	}
            
	     	break;

      case GAMESVR_VERIFY:
      	break;

     	default:
        	log_file.log("Unknown message %ld recvfrom %ld\n",(long)receive_packet.type,(long)ntohs(client.sin_port));
        	break;
   }

	return false;
}



void parse_args( short argc, char * argv[] )
{
	if (argc < 2)
   	return;

   // get port
   if( (server_port = (u_short)atoi(argv[1])) == 0 )
   {
		printf("Usage: MSTRSVR [port]\n\n");
      printf("   Port can be entered on command line or in config file.\n");
      printf("   All other options are read in from 'mstrsvr.cfg'.\n");
   }
}


void read_config_file()
{
	serv_addr banned_server;
	int option, i, option_value, wildcard;
	char 	data[512],*s, *value_str;
   BYTE addr[4];
	FILE 	*configf;

	if ( (configf = fopen(config_file,"rt")) == NULL )
	{
		printf("Unable to open '%s'... Using default options.\n\n",config_file);
      if( server_port == 0 )
      	server_port = DEFAULT_PORT;
		return;
	}

	// make sure options get printed to log file
	reset_prompt = false;
   log_file.set_log(TRUE);
   log_file.set_echo(TRUE);

   banned_list.DeleteList();

   while((s = fgets( data, 127, configf )) != NULL)
   {
		if( *s == '/' || (s = strchr( data, ':' )) == NULL ) // ignore comments
      	continue;
      *s++ = 0; // s now points to option value

      // determine which option this is
      for( option = 0; option < NUM_CONFIG_OPTIONS; option++)
      	if( !strcmp(data,config_options[option]))
         	break;

      if( option == NUM_CONFIG_OPTIONS ) // some unknown option or comment
      {
   		log_file.log("Unknown option '%s' in config file",data);
	     	continue;
      }

     	while( *s == ' ' || *s == '\t') s++; // get rid of white spaces if any
      value_str = s;

      if( option == CONFIG_BAN_ADDRESS )
      {
			for(i = 0, wildcard = 0; i < 4; i++ )
         {
				while(*s != '.' && *s != '\n' && *s != 0) s++; // find delimiter
            *s++ = 0;
            if( *value_str == '*' )
            {
					wildcard = i+1;
            	addr[i] = 0xFF;
            }
            else
	         	addr[i] = (BYTE)atoi(value_str);

            value_str = s;
         }
//      	log_file.log("BANNING address %d.%d.%d.%d",(int)addr[0],(int)addr[1],(int)addr[2],(int)addr[3]);
      }
      else if (option == CONFIG_MASTER_NAME || option == CONFIG_MOTD_STRING )
      {
			while(*s != '\n' && *s != 0 && *s != '/') s++; // find end of line
         if( *(s-1) == ' ' ) --s;
	      *s++ = 0;
      }
      else
      {
	      // convert value string into integer value
      	while( *s >= '0' && *s <= '9' ) s++; // find end of numeric characters
	      *s = 0;
   	   option_value = atoi( value_str );

//      	log_file.log("%s:%d",config_options[option],option_value);
      }

		switch (option)
   	{
			case CONFIG_SND_BUFFER:
         	if( option_value >= MIN_SNDBUF && option_value <= MAX_SNDBUF )
	         	snd_buf = option_value;
            else
            	log_file.log("SND_BUFFER value of %ld out of range",(long)option_value);
			   log_file.log("SND_BUFF size set to %d.",(int)snd_buf);
      		break;

	   	case CONFIG_RCV_BUFFER:
         	if( option_value >= MIN_RCVBUF && option_value <= MAX_RCVBUF )
	         	rcv_buf = option_value;
            else
            	log_file.log("RCV_BUFFER value of %ld out of range",(long)option_value);
			   log_file.log("RCV_BUFF size set to %d.",(int)rcv_buf);
   	   	break;

	   	case CONFIG_TIMEOUT:
         	option_value *= 1000;
         	if( option_value >= MIN_TIMEOUT && option_value <= MAX_TIMEOUT )
	         	timeout_value = option_value;
            else
            	log_file.log("TIMEOUT value of %ld out of range",(long)option_value);
			   log_file.log("Timeout set to %ld seconds.",(long)(timeout_value/1000));
   	   	break;

	   	case CONFIG_LOG_FILE:
            enable_log = option_value != 0;
			   log_file.log("Log file is %s.",(enable_log?"on":"off"));
   	   	break;

	     	case CONFIG_ECHO_FILE:
         	enable_echo= option_value != 0;
			   log_file.log("Log echo is %s.",(enable_echo?"on":"off"));
     	 		break;

	   	case CONFIG_SERVER_LIMIT:
				max_servers = option_value;
			   server_list.SetMaxServers( max_servers );
				log_file.log("MaxServers from single IP set to %d.",(int)max_servers);
   	   	break;

	   	case CONFIG_DEFAULT_PORT:
         	if( option_value >= MIN_PORT && option_value <= MAX_PORT )
            	server_port = option_value;
            else
            	log_file.log("PORT value of %ld out of range",(long)option_value);
			   log_file.log("Master Server running on port %d.",(int)server_port);
   	   	break;

	   	case CONFIG_BAN_ADDRESS:
            banned_server.addr.s_addr = *((unsigned long *)addr);
            banned_server.port        = wildcard;
				banned_list.AddServer( &banned_server, GetTickCount(), BANNED_SERVER_KEY, NULL );
   	   	break;

         case CONFIG_MASTER_NAME:
            if( strlen(value_str) > MAX_NAME_SIZE )
            {
            	log_file.log("MASTER_NAME %s too long, truncating to %ld bytes",value_str,(long)MAX_NAME_SIZE);
               value_str[MAX_NAME_SIZE] = 0;
            }
				strcpy(master_name, value_str);
         	break;

         case CONFIG_MASTER_ID:
         	if( option_value >= 0 && option_value <= 0xFFFF )
            	master_id = (WORD)option_value;
            else
            	log_file.log("MASTER ID of %ld out of range",(long)option_value);
			   log_file.log("Master Server id %ld.",(long)master_id);
				break;

         case CONFIG_MOTD_STRING:
            if( strlen(value_str) > MAX_MOTD_SIZE )
            {
            	log_file.log("MOTD_STRING %s too long, truncating to %ld bytes",value_str,(long)MAX_MOTD_SIZE);
               value_str[MAX_MOTD_SIZE] = 0;
            }
         	strcpy(motd_string, value_str);
         	break;

         case CONFIG_PACKET_SIZE:
         	packet_size = (WORD)option_value;
            if(packet_size > MAX_PACKET_SIZE)
            {
            	log_file.log("PACKET_SIZE %ld out of range, set to %ld",(long)option_value,(long)MAX_PACKET_SIZE);
            	packet_size = MAX_PACKET_SIZE;
            }
            else if(packet_size < MIN_PACKET_SIZE)
            {
            	log_file.log("PACKET_SIZE %ld out of range, set to %ld",(long)option_value,(long)MIN_PACKET_SIZE);
            	packet_size = MIN_PACKET_SIZE;
            }
         	break;

         case CONFIG_RELOAD_INT:
         	reload_interval = (DWORD)option_value * 60000 /*TICKS_PER_MINUTE*/;
            next_reload_time = GetTickCount() + reload_interval;
         	break;

	      default:
   	   	printf("ERROR: Unknown config option %d %s\n",option, data);
      		break;
	   }
   }
   fclose(configf);

   log_file.set_log(enable_log);
   log_file.set_echo(enable_echo);
	reset_prompt = true;
}

int format_query_string( char * data )
{
	int num_commands=0;
   char *s = data, str[256];

   data += sizeof(short);

	strcpy(data, "s_name");
   data += strlen(data)+1;
  	num_commands++;
#if 0
   strcpy(data, "g_name");
  	data += strlen(data)+1;
   num_commands++;

  	strcpy(data, "g_version");
   data += strlen(data)+1;
  	num_commands++;

   strcpy(data, "g_id");
  	data += strlen(data)+1;
   num_commands++;
#endif

	*(short *)s =num_commands; // number of commands

   return data - s;
}

void query_game_server( serv_addr *game_server )
{
   int    addr_len, ret_val;
   struct packet message;

	client.sin_addr = game_server->addr;
	client.sin_port = game_server->port;
	addr_len = sizeof( client );
	message.type = htons( GAMESPY_QUERY );
	int fill_size = format_query_string( message.data);
   ret_val = sendto(gSocket, (char *) &message,
                HEADER_SIZE+fill_size, 0,
				    (struct sockaddr *)&client, addr_len);
}

void query_game()
{
	serv_addr game_server;
   int id;

	server_list.ListServers();
   cout << "   server id:";
   command.get_input();
   id = command.input[0] - '0' - 1;
   if( server_list.GetServer( &game_server, id ))
   {
   	cout<<endl<<"Querying game server "<<inet_ntoa(game_server.addr)<<":"<<ntohs(game_server.port)<<"..."<<endl;
//		printf("Querying game server %s:%ld...",inet_ntoa(game_server.addr),(long)ntohs(game_server.port));
		query_game_server( &game_server );
   }
   else
	   cout << "Invalid game server id " << id << "!" << endl;

}

void echo_function( void )
{
	if( reset_prompt )
		command.prompt();
}

// Figures out how many packets it will take to send server data.
BYTE get_total_packets( int num_servers, int bytes_per_server,
                        int bytes_per_packet, int first_packet_header_size)
{
   // first packet can't hold as many because of extra header stuff
	int num_servers_in_first_packet = (bytes_per_packet - first_packet_header_size) / bytes_per_server;
   num_servers -= num_servers_in_first_packet;
   if( num_servers <= 0 )
   	return 1;

   // more than one packet necessary, find out how many additional packets   
   int num_servers_per_packet = bytes_per_packet / bytes_per_server;
   int additional_packets = num_servers / num_servers_per_packet + 1;
   if( additional_packets > 255 )
   {
   	additional_packets = 255;
      log_file.log("ERROR: Too many packets!");
   }
   return additional_packets + 1;
}

