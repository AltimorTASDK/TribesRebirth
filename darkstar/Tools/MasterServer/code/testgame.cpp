#include "windows.h"
#include "winsock.h"
#include "stdio.h"
#include "mstrsvr.h"
#include "servinfo.h"

#define HEARTBEAT 5000 // milliseconds between heartbeat sends

// function prototypes
struct in_addr *atoaddr(char *address);
char *game_info( KEY key, int num );
void send_info( char *data, int data_size, WORD key );

// global variables
SOCKET sock;
struct sockaddr_in client, server;
struct packet send_packet, receive_packet;
BYTE   version_number = 0x10;


class  ServerInfo server_info(send_packet.data,(int)(MAX_PACKET_SIZE-HEADER_SIZE),send_info,game_info);

void main(int argc, char **argv)
{
   int    addr_len, ret_val, event;
   u_long heartbeat = HEARTBEAT, ioctl_arg, last_time = 0;

   /*---------------------------*/
   /* Check for valid arguments */
   /*---------------------------*/

	if (argc < 3)
	{
		printf("Usage: testgame address port [rate]\n");
		printf(" address - ip address of server.\n");
		printf(" port - server's port.\n");
		printf(" rate - seconds between sending heartbeat [default=%d].\n",HEARTBEAT/1000);
		exit(EXIT_FAILURE);
   }

   /*---------------------------*/
   /* Extract command line data */
   /*---------------------------*/

   u_short port = (u_short)atoi(argv[2]);
   if( argc == 4 )
   {
   	heartbeat = atoi(argv[3])*1000;
      if( heartbeat <= 2000 ) // minimum of 2 seconds
      	heartbeat = 2000;
   }
	struct in_addr *server_addr = atoaddr(argv[1]);
	if (server_addr == NULL)
   {
   	printf("Invalid server address!\n");
    	exit(EXIT_FAILURE);
	}

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
   printf("WSAStartup successful!\n");


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
   printf("Socket initialized!\n");

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
   printf("Socket set to non-blocking!\n");

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
   printf("Socket is bound!\n");

   /*------------------------------------------*/
   /* Prepare to sent packets to master server */
   /*------------------------------------------*/

	memset((char *) &server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr =  server_addr->s_addr;
	server.sin_port = htons(port);

   /*-------------------------------------------------------------*/
   /* This loop does two things.  It sends out a heartbeat to the */
   /* MasterServer at regular intervals, and it polls the socket  */
   /* for incoming messages from Gamespy or other service         */
   /* requesting information.                                     */
   /*-------------------------------------------------------------*/

   while( 1 )
   {
      Sleep(1000);
      event = 1;
      if( GetTickCount() > (last_time + heartbeat))
      {
			last_time = GetTickCount();
	     	event = 0;
      }
		switch( event )
      {
      	case 0: // send out heartbeat
				addr_len = sizeof( server );
		      send_packet.type    = (BYTE)HEARTBT_MESSAGE;
            send_packet.version = (BYTE)version_number;
		      ret_val = sendto(sock, (char *) &send_packet, HEADER_SIZE, 0,
      	                   (struct sockaddr *)&server, addr_len);
         	printf(".");

		      if( ret_val == SOCKET_ERROR )
		      {
   				ret_val = WSAGetLastError();
		         printf("Function sendto failure, error: %d\n", ret_val);
      		   WSACleanup();
		         exit(EXIT_FAILURE);
      		}
            break;

         case 1: // check for incoming messages
				addr_len = sizeof( client );
		      ret_val = recvfrom(sock, (char *) &receive_packet, MAX_PACKET_SIZE, 0,
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
      		}
            else // process incoming message
            {
				   /*---------------------------------------------*/
   				/* check version number                        */
	   			/* for now, only support exact version matches */
		   		/*---------------------------------------------*/

	   			if( receive_packet.version != version_number )
		   		{
			   		printf("Ignoring message: Invalid version %d",(int)receive_packet.version);
   				}
               else
               {
				      switch( receive_packet.type )
				      {
				      	case GAMESPY_QUERY: // reply to GAMESPY_QUERY
         	         	printf("\nKeys requested... ");
            	         if( server_info.HandleRequest( receive_packet.data, ntohs(receive_packet.key) ))
               	      {
                  	   	printf("\nHandleRequest error, exiting...\n");
                     	   exit(EXIT_FAILURE);
   	                  }
	                     printf("\n");
      	            	break;

         	         default:
           					printf("Unknown message %ld recvfrom %ld\n",(long)receive_packet.type,(long)ntohs(client.sin_port));
               	 	break;
                  }
               }
            }
         	break;
      }
   }
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

void send_info( char *data, int data_size, WORD key )
{
   int    addr_len, ret_val, packet_len;

	send_packet.type = (BYTE)GAMESVR_REPLY;
   send_packet.version = version_number;
   send_packet.key  = htons(key);
   packet_len = HEADER_SIZE + data_size;
	addr_len = sizeof( client );
   ret_val = sendto(sock, (char *) &send_packet, packet_len, 0,
           	       (struct sockaddr *)&client, addr_len);

   if( ret_val == SOCKET_ERROR )
	{
      printf("Function recvfrom failure, error: %d\n", WSAGetLastError());
		WSACleanup();
   	exit(EXIT_FAILURE);
   }
}

#define NUM_PLAYERS 4
char *player_name[NUM_PLAYERS] = {"Bob", "Carol", "Ted", "Alice" };
char *player_team[NUM_PLAYERS] = {"Alpha", "Bravo", "Alpha", "Bravo"};
char  player_id[NUM_PLAYERS] = {7,22,86,99};
char  player_ping[NUM_PLAYERS] = {1,2,2,1};
char  info[10];

char *game_info( KEY key, int num )
{
   char *s=NULL;

	switch( key )
   {
     	case SERVER_NAME:
        	s = "Romeo";
        	break;

     	case GAME_NAME:
        	s = "EarthSiege III";
        	break;

     	case GAME_VERSION:
        	s = "1.01";
        	break;

     	case GAME_ID:
        	s = "Bravo 6";
        	break;

     	case NUM_PLAYERS:
        	s = "3";
        	break;

     	case MAX_PLAYERS:
        	s = "4";
        	break;

     	case PLAYER_NAME:
        	s = player_name[num];
        	break;

     	case PLAYER_ID:
        	s = itoa(player_id[num], info, 10);
        	break;

     	case PLAYER_PING:
        	s = itoa(player_ping[num], info, 10);
        	break;

     	case PLAYER_TEAM:
        	s = player_team[num];
        	break;

      case INVALID_KEY:
      default:
         printf("\nError:game_info invalid key %ld.\n",(long)key);
         s = "Invalid";
         break;
	}

   return s;
}

