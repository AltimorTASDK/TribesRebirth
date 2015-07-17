//------------------------------------------------------------------
//
//	TESTSPY.cpp
//
//	This is a quick and dirty example of how to get info from
//	Tribes servers using the non-compressed query.
//
//
//------------------------------------------------------------------

#include "windows.h"
#include "winsock.h"
#include "stdio.h"
#include "mstrsvr.h"
#include "servlist.h"
#include "servlog.h"
#include "servinfo.h"
#include "servcmd.h"

#include <iostream.h>

#define RESPONSE_TIMEOUT 5000

int  format_query_string( char * data );
void decode_query_reply( char * data, char *addr, long port );
bool process_socket( BYTE response_type );
bool process_gamespy_socket (int sequence);
void query_master( void );
void query_games( SOCKET sock, int packetType );
void init_socket( void );
void parse_args( short argc, char * argv[] );
struct in_addr *atoaddr(char *address);
bool wait_for_response( BYTE response_type );
bool wait_for_gamespy_response (int sequence);
void display_server_totals (void);
void display_player_totals (void);

char send_buffer[10];

const char num_commands = 7;
char  *command_list[num_commands] = {
   "        m - get list from MasterServer",
	"        s - ping all game servers (minimal info)",
	"        a - query all game servers (lots of info)",
	"        d - display game server list",
	"        t - display totals",
   "        ? - list commands",
	"        q - quit",
};

class LogFile log_file(NULL);
class ServCmd command( process_socket, num_commands, command_list );
class ServerList server_list;

struct sockaddr_in client, server;
SOCKET sock;
u_short server_port;
struct in_addr *server_addr;
bool enable_log = TRUE, enable_echo = TRUE;
struct packet receive_packet, send_packet;
short  query_id=21;
BYTE   version_number = 0x10;
char   master_name[MAX_NAME_SIZE+1], motd_string[MAX_MOTD_SIZE+1];

int	totalPlayers = -1;
int 	lastPlayerCount;

void main(int argc, char **argv)
{
   bool	 running = TRUE;

   /*---------------------------------------------------*/
   /* parse command line arguments and initialize stuff */
   /*---------------------------------------------------*/

   parse_args( argc, argv );
   log_file.init_log(enable_log,enable_echo,(int)server_port,"TESTSPY\n");
   init_socket();

   command.list_commands();
   command.prompt();
   do
   {
      command.get_input();
      switch( command.input[0] )
      {
         case 'M':
         case 'm':
         	query_master();
            wait_for_response( MSTRSVR_REPLY );
				display_server_totals ();
         	break;

         case 'A':
         case 'a':
         	query_games(sock, GameSpyQuery1);
				display_server_totals ();
				display_player_totals ();
            break;

         case 'S':
         case 's':
         	query_games(sock, GameSpyPing);
				display_server_totals ();
				display_player_totals ();
            break;

         case 'D':
         case 'd':
         	server_list.ListServers();
         	break;

      	case 'q':
         case 'Q':
         	running = FALSE;
            break;

			case 't':
			case 'T':
				display_server_totals ();
				display_player_totals ();
				break;

         case '?':
         case 'h':
         case 'H':
         	command.list_commands();
            break;

         default:
         	cout << "Unrecognized command '" << command.input[0] << "'." << endl;
         	break;
      }
      command.prompt();
   }
   while( running );


}

bool process_socket( BYTE response_type )
{
	serv_addr game_server;
   char   *serv_buf;
   int    addr_len, ret_val, num_servers, i;
   int    str_size,str_start,data_start;

	addr_len = sizeof( server );
   ret_val = recvfrom(sock, (char *) &receive_packet, MAX_PACKET_SIZE, 0,
                      (struct sockaddr *)&server, &addr_len);

   if( ret_val == SOCKET_ERROR )
   {
    	ret_val = WSAGetLastError(); // if WSAWOULDBLOCK, no message was waiting
		if( ret_val != WSAEWOULDBLOCK )
      {
      	printf("Function recvfrom failure, error: %d\n", ret_val);
		   WSACleanup();
		   exit(EXIT_FAILURE);
      }
   }
   else
   {
	   if( receive_packet.version != version_number )
   	{
   		log_file.log("Ignoring message: Invalid version %d",(int)receive_packet.version);
	      return false;
   	}

      switch( receive_packet.type )
      {
      	case MSTRSVR_REPLY:
            if( ntohs(receive_packet.key) == query_id ) // if reply id is different this is an old packet, ignore it
            {
               // if this is the first packet in the request, it will have additional info
               // master_name_size, master_name, motd_size, motd_string
               data_start = 0;
               printf("Received packet %ld of %ld",(long)receive_packet.packet_num,(long)receive_packet.packet_tot);
               if( receive_packet.packet_num == 1 )
               {
               	str_start= 0;
               	str_size = receive_packet.data[str_start];
               	if( str_size <= MAX_NAME_SIZE )
                  {
                  	memcpy(master_name,&receive_packet.data[str_start+1],str_size);
                     master_name[str_size] = 0;
                  }

                  str_start= str_size + 1;
                  str_size = receive_packet.data[str_start];
                  if( str_size <= MAX_MOTD_SIZE )
                  {
                  	memcpy(motd_string,&receive_packet.data[str_start+1],str_size);
                     motd_string[str_size] = 0;
                  }
                  data_start = str_start + str_size + 1;
                  printf("\nMasterserver #%ld, %s says %s",(long)ntohs(receive_packet.id),master_name, motd_string);
               }

	            // build list of game servers
   	      	num_servers = ntohs( *((short *)(&receive_packet.data[data_start])) );
      	      serv_buf    = &receive_packet.data[data_start+2];
               printf("\n");
         	   for( i = 0; i < num_servers; i++ )
            	{
      				game_server.addr.s_net   = serv_buf[1];
      				game_server.addr.s_host  = serv_buf[2];
      				game_server.addr.s_lh    = serv_buf[3];
      				game_server.addr.s_impno = serv_buf[4];
						game_server.port = ntohs( *(short *)&serv_buf[5] );
	            	server_list.AddServer( &game_server, GetTickCount(), NORMAL_SERVER_KEY, NULL );
   	            serv_buf += SERVER_INFO_SIZE;
      	      }

	            if( response_type == receive_packet.type && receive_packet.packet_num == receive_packet.packet_tot)
   	            return TRUE;
            }
            else
               printf("Old packet received.\n");
         	break;

         case GAMESVR_REPLY:
         	decode_query_reply(receive_packet.data,inet_ntoa(server.sin_addr),(long)ntohs(server.sin_port));
            if( response_type == receive_packet.type )
               return TRUE;
         	break;

         default:
         	printf("Unknown message %ld recvfrom %ld\n",(long)receive_packet.type,(long)ntohs(server.sin_port));
         	break;
      }
   }
   return false;
}

void query_master ()
{
   int    addr_len;

	server_list.DeleteList();

   /*-------------------------------------------------------------------------*/
   /* Testspy queries the MasterServer for a list of addresses of GameServers */
   /*-------------------------------------------------------------------------*/

   cout << endl << "\nQuerying masterServer " << inet_ntoa(*server_addr) << ":" << server_port << "..." << endl;

   /*------------------------------------*/
   /* Prepare to query the master server */
   /*------------------------------------*/

	memset((char *) &server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr =  server_addr->s_addr;
	server.sin_port = htons(server_port);
   addr_len = sizeof( server );
   send_packet.version = version_number;
   send_packet.type    = GAMESPY_QUERY;
   send_packet.key     = htons(++query_id);
	send_packet.packet_num = REQUEST_ALL_PACKETS;
   sendto(sock, (char *) &send_packet, HEADER_SIZE, 0, (struct sockaddr *)&server, addr_len);
}


// Responses to NEW_QUERYs will be straight text (i.e. no packing)
void query_game_server( SOCKET sock, serv_addr *game_server, int sequence, int packetType )
{
	struct sockaddr_in gserver;
   int    addr_len;

	gserver.sin_family = AF_INET;
	gserver.sin_addr = game_server->addr;
	gserver.sin_port = game_server->port;
	addr_len = sizeof( gserver );
	send_buffer[0] = packetType;
	send_buffer[1] = sequence / 256;
	send_buffer[2] = sequence % 256;
   sendto(sock, (char *) &send_buffer, 3, 0, (struct sockaddr *)&gserver, addr_len);
}


void query_games(SOCKET sock, int packetType)
{
	serv_addr game_server;
   int       i;
	bool	timed_out;

	printf ("\n");
	totalPlayers = 0;

  	// query GameServers
   for( i = 0; i < server_list.Size(); i++ )
   {
   	server_list.GetServer( &game_server, i );
		lastPlayerCount = 0;
      query_game_server (sock, &game_server, i, packetType);
      timed_out = wait_for_gamespy_response (i);
		totalPlayers += lastPlayerCount;
		if (timed_out)
			printf (".");
	}
	printf ("\n");
}


void init_socket()
{
   int    ret_val;
	u_long ioctl_arg;

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

   sock = socket(AF_INET, SOCK_DGRAM, 0);
   if( sock == INVALID_SOCKET )
   {
   	printf("Unable to create a socket!\n");
      WSACleanup();
      exit(EXIT_FAILURE);
   }

	/*-----------------------------*/
   /* Set socket to non-blocking. */
   /*-----------------------------*/

	ioctl_arg = 1;
   ret_val = ioctlsocket( sock, FIONBIO, &ioctl_arg );
   if( ret_val == SOCKET_ERROR ) //
   {
   	ret_val = WSAGetLastError();
   	printf("Unable to unblock socket, error: %d\n", ret_val);
      WSACleanup();
      exit(EXIT_FAILURE);
   }

   /*----------------------------*/
   /* bind socket to client port */
   /*----------------------------*/

	memset((char *)&client, 0, sizeof(client));
	client.sin_family      = AF_INET;
	client.sin_addr.s_addr = htonl(INADDR_ANY);
	client.sin_port        = htons(0);

	if( bind(sock, (struct sockaddr *)&client, sizeof(client)) == SOCKET_ERROR )
   {
   	ret_val = WSAGetLastError();
   	printf("Unable to bind socket, error: %d\n", ret_val);
      WSACleanup();
      exit(EXIT_FAILURE);
   }

}

void parse_args( short argc, char * argv[] )
{
	int  count;
	char * opt;

   /*---------------------------*/
   /* Check for valid arguments */
   /*---------------------------*/

	if (argc < 3)
	{
		printf("Usage: TESTSPY address port [options]\n\n");
      printf("-l     turn off message Log\n");
      printf("-e     turn off message Echo\n");
		printf("-t		output totals to TOTAL.TXT\n");
		exit(EXIT_FAILURE);
   }

   /*---------------------------*/
   /* Extract command line data */
   /*---------------------------*/

	server_addr = atoaddr(argv[1]);
	if (server_addr == NULL)
   {
   	printf("Error: Invalid address!\n");
    	exit(EXIT_FAILURE);
	}

   if( (server_port = (u_short)atoi(argv[2])) == 0 )
   {
   	printf("Error: Invalid port!\n");
      exit(EXIT_FAILURE);
   }


	for( count = 3; count < argc; count++ )
	{
		opt = argv[ count ];
		if( opt[0] == '-' )
		{
			switch( opt[1] )
			{
				case 'l':
					enable_log = FALSE;
					break;
				case 'e':
					enable_echo = FALSE;
					break;
				case 't':
					{
						FILE *fp;

					   init_socket();
		         	query_master();
		            wait_for_response( MSTRSVR_REPLY );
		         	query_games(sock, GameSpyPing);
						display_server_totals ();
						display_player_totals ();

						if ((fp=fopen("total.txt","w")) == 0)
							{
								printf("ERROR: failed to open log file total.txt.\n");
							   exit(EXIT_FAILURE);
							}

						char buff[25];
						sprintf (buff, "%d\n", server_list.ServerCount());
					   fwrite(buff, strlen (buff), 1, fp);
						sprintf (buff, "%d\n", totalPlayers);
					   fwrite(buff, strlen (buff), 1, fp);
						fclose(fp);
						exit (0);
					}
					break;

			}
		}
      else
      {
      	printf("Error: Invalid argument %s!\n",opt);
	      exit(EXIT_FAILURE);
      }
	}
}


void display_server_totals (void)
{
	printf ("\tServers responding: %d\n", server_list.ServerCount());
}

void display_player_totals (void)
{
	printf ("\t     Total players: %d\n", totalPlayers);
}


/* Converts ascii text to in_addr struct.  NULL is returned if the address
   can not be found. */
struct in_addr *atoaddr(char *address)
{
  struct hostent *host;
  static struct in_addr saddr;

  /* First try it as aaa.bbb.ccc.ddd. */
  saddr.s_addr = inet_addr(address);
  if (saddr.s_addr != INADDR_NONE) {
    return &saddr;
  }
  host = gethostbyname(address);
  if (host != NULL) {
    return (struct in_addr *) *host->h_addr_list;
  }
  return NULL;
}

int format_query_string( char * data )
{
	int num_commands=0;
   char *s = data, str[256];

   data += sizeof(short);

   FILE * fp = fopen("keys.txt","rt");
   if( fp )
   {
		while( fgets(str,256,fp))
      {
			if( str[0] != '\n' )
         {
	      	strcpy(data, str);
   	      data += strlen(data)-1;
      	   if( *data == '\n' )
	      	   *data = 0; // get rid of newline char
				data++;
   	      num_commands++;
         }
      }
   	fclose( fp );
   }
   else
   {
		strcpy(data, "s_name");
	   data += strlen(data)+1;
   	num_commands++;
	   strcpy(data, "g_name");
   	data += strlen(data)+1;
	   num_commands++;
   	strcpy(data, "g_version");
	   data += strlen(data)+1;
   	num_commands++;
	   strcpy(data, "g_id");
   	data += strlen(data)+1;
	   num_commands++;
   	strcpy(data, "num_players");
	   data += strlen(data)+1;
	   num_commands++;
   	strcpy(data, "max_players");
	   data += strlen(data)+1;
   	num_commands++;
	   strcpy(data, "p_name");
   	data += strlen(data)+1;
	   num_commands++;
   	strcpy(data, "p_id");
	   data += strlen(data)+1;
   	num_commands++;
      strcpy(data, "p_team");
   	data += strlen(data)+1;
	   num_commands++;
   }

	*(short *)s =num_commands; // number of commands
   return (int)(data-s);
}

void decode_query_reply( char * data, char *addr, long port )
{
	short num_strings = *(short *)data;
   short i,index=2;

   printf("%s:%ld\t",addr,port);
   for( i = 0; i < num_strings; i++ )
   {
      printf("%s ",&data[index]);
      index += strlen(&data[index])+1;
   }

   printf("\n");
}

bool wait_for_response( BYTE response_type )
{
	u_long tick = GetTickCount();
   bool no_response = TRUE;
	bool timed_out = false;

   while( !timed_out && no_response )
   {
   	no_response = !process_socket( response_type );
		timed_out = !(GetTickCount() < (tick + RESPONSE_TIMEOUT));
   }

	return timed_out;
}

int decode_and_print (char *buff, char *dest, char *format)
{
	int count = *buff;

	if (count)
		{
			buff++;
			memcpy(dest, buff, count);
			dest[count] = 0;
			if (format)
				printf (format, dest);
		}

	return count + 1;
}

int decode_bool (char *buff, bool *dest, char *yes, char *no)
{
	*dest = *buff;
	if (*dest)
		printf (yes);
	else
		printf (no);
	return 1;
}

int decode_byte (char *buff, char *dest, char *format)
{
	*dest = *buff;
	if (format)
		printf (format, *dest);
	return 1;
}


// Some ugly, messy code that decodes the GameSpyResponse and GameSpyPingResponse data...
bool process_gamespy_data (char packetType, char *buff)
{
	unsigned char count;
	printf ("\n");

	char gameName[256];
	buff += decode_and_print (buff, gameName, NULL);
	if (stricmp ("tribes", gameName))
		return 1;

	char version[256];
	buff += decode_and_print (buff, version, NULL);

	char host[33];
	buff += decode_and_print (buff, host, "%s ");

	printf ("(%s ", gameName);
	printf ("%s)\n\t", version);
	
	bool dedicated;
	buff += decode_bool (buff, &dedicated, "(dedicated) ", "(non-dedicated) ");

	bool password;
	buff += decode_bool (buff, &password, "(password) ", "(no password) ");

	char playerCount;
	buff += decode_byte (buff, &playerCount, "%d/");
	lastPlayerCount = playerCount;

	char maxPlayers;
	buff += decode_byte (buff, &maxPlayers, "%d players ");

	if (packetType == GameSpyPingResponse)
		{
			printf ("\n");
			return false;
		}

	short *cpuSpeed = (short *)(buff);
	buff += sizeof (short);
	printf ("%dcpu ", *cpuSpeed);

	char mod[33];
	buff += decode_and_print (buff, mod, "%s ");

	char missionType[9];
	buff += decode_and_print (buff, missionType, "%s ");

	char missionName[17];
	buff += decode_and_print (buff, missionName, "%s ");

	char serverInfo[257];
	count = *buff;
	buff++;
	if (count)
		{
			memcpy(serverInfo, buff, count);
			buff += count;
			serverInfo[count] = 0;
		}

	char numTeams;
	buff += decode_byte (buff, &numTeams, "%d teams\n");

	char teamScoreHeading[65];
	count = *buff;
	buff++;
	memcpy(teamScoreHeading, buff, count);
	buff += count;
	teamScoreHeading[count] = 0;
	char *beg = &teamScoreHeading[0];
	int length = strlen (teamScoreHeading);
	printf ("\n\t");
	for (int j = 0; j < length; j++)
		if (teamScoreHeading[j] == '\t')
			{
				teamScoreHeading[j] = '\0';
				printf ("%-15.15s ", beg);
				j += 1;
				beg = &teamScoreHeading[j+1];
			}
	printf ("%-15.15s\n", beg);

	char clientScoreHeading[65];
	count = *buff;
	buff++;
	memcpy(clientScoreHeading, buff, count);
	buff += count;
	clientScoreHeading[count] = 0;

	char teamNames[8][33];
	char score[255];
	char *scoreString;
   for(int j = 0; j < numTeams && j < 8; j++)
   {
		count = *buff;
		buff++;
		memcpy(teamNames[j], buff, count);
		buff += count;
		teamNames[j][count] = 0;
		count = *buff;
		buff++;
		memcpy(score, buff, count);
		buff += count;
		score[count] = 0;
		scoreString = score;

		for (int h = 0; h < count; h++)
			if (score[h] == '%')
				{
					switch (score[h+1])
						{
							case 't':
								printf ("\t%-15.15s ", teamNames[j]);
								scoreString = &score[h+2];
								break;
								
						};
					h++;
				}

		while (*scoreString && (*scoreString == ' ' || *scoreString == '\t'))
			scoreString++;
		printf ("%-15.15s\n", scoreString);
   }

	length = strlen (clientScoreHeading);
	beg = clientScoreHeading;
	printf ("\n\t");
	for (int j = 0; j < length; j++)
		if (clientScoreHeading[j] == '\t')
			{
				clientScoreHeading[j] = '\0';
				printf ("%-11.11s ", beg);
				j += 1;
				beg = &clientScoreHeading[j+1];
			}
	printf ("%-11.11s\n\t", beg);

	char name[19];
	unsigned char ping;
	char pl;
	char team;
   for(int j = 0; j < lastPlayerCount; j++)
   {
		ping = *buff;
		buff++;
		pl = *buff;
		buff++;
		team = *buff;
		buff++;
		count = *buff;
		buff++;
		memcpy(name, buff, count);
		buff += count;
		name[count] = 0;
		count = *buff;
		buff++;
		memcpy(score, buff, count);
		buff += count;
		score[count] = 0;

		for (int h = 0; h < count; h++)
			if (score[h] == '%')
				{
					int len;
					switch (score[h+1])
						{
							case 'n':
								printf ("%-11.11s ", name);
								break;

							case 't':
								if (team == -1)
									printf ("%-11.11s ", "(observer)");
								else
									if (team < numTeams && team >= 0)
										printf ("%-11.11s ", teamNames[team]);
									else
										printf ("%-11.11s ", " ");
								break;

							case 'p':
								char pingStr[10];
								sprintf (pingStr, "%d", ping << 2);
								printf ("%-11.11s ", pingStr);
								break;
								
							case 'l':
								char lossStr[10];
								sprintf (lossStr, "%d", pl);
								printf ("%-11.11s ", lossStr);
								break;
								
						};
					h++;
				}
			else
				if (score[h] != ' ' && score[h] != '\t')
					{
						int k = 0;
						while (score[h] != ' ' && score[h] != '\t')
							{
								scoreString[k] = score[h];
								h++;
								k++;
							}

						scoreString[k] = 0;
						printf ("%-11.11s ", scoreString);
					}

		printf ("\n\t");
   }

	printf ("\n");
	return false;
}

bool process_gamespy_socket (int sequence)
{
	char buff[1500];
	serv_addr game_server;
   char   *serv_buf;
   int    addr_len, ret_val, num_servers;
   int    str_size,str_start,data_start;

	addr_len = sizeof( server );
   ret_val = recvfrom(sock, (char *) &buff, 1400, 0, (struct sockaddr *)&server, &addr_len);

   if( ret_val == SOCKET_ERROR )
   {
    	ret_val = WSAGetLastError(); // if WSAWOULDBLOCK, no message was waiting
		if( ret_val != WSAEWOULDBLOCK )
      {
      	printf("Function recvfrom failure, error: %d\n", ret_val);
		   WSACleanup();
		   exit(EXIT_FAILURE);
      }
   }
   else
   {
		char buff1 = sequence / 256;
		char buff2 = sequence % 256;
		if ((buff[0] == GameSpyQuery1Response || buff[0] == GameSpyPingResponse) && buff[1] == buff1 && buff[2] == buff2)
			process_gamespy_data (buff[0], &buff[4]);
   }
   return false;
}

bool wait_for_gamespy_response( int i )
{
	u_long tick = GetTickCount();
   bool no_response = TRUE;
	bool timed_out = false;

   while( !timed_out && no_response )
   {
   	no_response = !process_gamespy_socket (i);
		timed_out = GetTickCount() >= (tick + 400);
   }

	return timed_out;
}
