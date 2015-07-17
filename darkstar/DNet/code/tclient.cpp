#include "windows.h"
#include "dnet.h"
#include "clientsess.h"
#include "stdio.h"
#include "signal.h"
#include "logfile.h"
#include "servcmd.h"

#include <iostream.h>


extern class LogFile log_file;

HINSTANCE hInst;
ClientSession *client_session, *server_session;
DNet::Transport *transport[50] = {NULL}, *server_transport;
DNet::VC *vc = NULL;
class ServCmd command( NULL );
char tname[64];
bool log_active = true;

bool handle_input( void );

#define BTTW_TEST_SIZE 256
BYTE *bttw_data;
bool  bttw_test = false;
int   bttw_size=0;
DWORD bttw_stat_time, bttw_stat_size, bttw_stat_thru;
BYTE  bttw_increment = 0;
WORD  bttw_interval = 2000;
char  bttw_init_string[]="bttw_test";
char  bttw_address_string[256];
void  initialize_bttw_data(int bttw_size, const char *addr_str);
void  start_bttw_test();
void  stop_bttw_test();
void  bttw_test_process();
bool  compare_bttw_data( BYTE *data, int datalen );


void _USERENTRY _EXPFUNC cleanup(int sig)
{
   log_file.log("Cleaning up...\n");
   delete client_session;
   delete server_session;
   exit(0);
}

void main(int argc, char **argv)
{
	char address[64],address1[64];
	MSG msg;      // message
   bool running = TRUE;
   hInst = GetModuleHandle(argv[0]);

   signal(SIGINT, cleanup);
   signal(SIGBREAK, cleanup);

   log_file.log("command line %s",argv[1]);


   if( argc >= 2 )
   {
   	strcpy(address,argv[1]);
      log_file.log("number to call %s",address);
      if( argc == 3 )
      	strcpy(address1,argv[2]);
      else
      	address1[0]=0;
   }
   else
   	address[0]=0;

   client_session = new ClientSession(hInst); // creates list of transports available
   server_session = new ClientSession(hInst); // for testing LOOP transport

	command.list_commands();
	while(running)
   {
   	if(command.check_input())
      	running = handle_input();
   	if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
	   {
          TranslateMessage(&msg);
          DispatchMessage(&msg);
      }
   	client_session->process();
      server_session->process();
      bttw_test_process();
   }

   delete client_session;
   delete server_session;
}

void ClientSession::onReceive(DNet::VC *conn, const char *transportAddress, BYTE *data, int datalen)
{
   log_file.log("ClientSession(%lx)::onReceive - got packet of %d bytes from %s",(long)this, datalen, transportAddress);
   bool test_flag = false;

	if( bttw_test )
   {
     if(compare_bttw_data( data, datalen ))
     {
        test_flag = true;
        cout << "BTTW packet " << (int)data[1] << " received: " << bttw_stat_thru << " bytes/sec" << endl;
     }
     else
        cout << "onReceive data failed BTTW test!" << endl;
   }
   else
   {
	   if(!strncmp( data, bttw_init_string, strlen(bttw_init_string) ))
  		{
	  		int size = *((WORD *)&data[strlen(bttw_init_string)]);
         bttw_interval = *((WORD *)&data[strlen(bttw_init_string)+2]);
   		cout << "BTTW test request for " << size << " bytes, " << bttw_interval << " msecs, sending data!" << endl;
      	initialize_bttw_data( size, transportAddress );
	  	}
   }

   if(!test_flag)
  	{
	  	cout << "Message: '" << data << "' received from " << transportAddress << endl;
	}


/*   if(data[0] == 100)
      sess->disconnect(conn, NULL);*/
}

void ClientSession::onConnection(DNet::VC *conn, DNet::Session::ConnectionNotifyType ct, BYTE *data, int datalen)
{
	switch( ct )
   {
   	case DNet::Session::ConnectionRequested:
	      log_file.log("ClientSession::onConnection - ConnectionRequested\n");
	      cout << "ClientSession::onConnection - ConnectionRequested" << endl;
         conn->accept(NULL,0); // accept the connection
         break;
   	case DNet::Session::ConnectionAccepted:
	      log_file.log("ClientSession::onConnection - ConnectionAccepted\n");
	      cout << "ClientSession::onConnection - ConnectionAccepted" << endl;
         break;
   	case DNet::Session::Connected:
	      log_file.log("ClientSession::onConnection - Connected\n");
	      cout << "ClientSession::onConnection - Connected" << endl;
         break;
   	case DNet::Session::Disconnected:
      	vc = NULL;
	      cout << "ClientSession::onConnection - Disconnected" << endl;
	      log_file.log("ClientSession::onConnection - Disconnected\n");
         break;
   	case DNet::Session::ConnectionRejected:
      	vc = NULL;
	      cout << "ClientSession::onConnection - ConnectionRejected" << endl;
	      log_file.log("ClientSession::onConnection - ConnectionRejected\n");
         break;
   	case DNet::Session::TimedOut:
      	vc = NULL;
 	      cout << "ClientSession::onConnection - TimedOut" << endl;
	      log_file.log("ClientSession::onConnection - TimedOut\n");
         break;
      default:
	      cout << "ClientSession::onConnection - Unknown connection type " << ct << endl;
      	log_file.log("ClientSession::onConnection - Unknown connection type %lx",(long)ct);
   }
}

void ClientSession::onNotify(DNet::VC *, DWORD notifyKey, bool received)
{
   log_file.log("ClientSession::onNotify %d, %d\n", notifyKey, received);
   cout << "ClientSession::onNotify " << notifyKey << " " << received << endl;
}

#define INPUT_GET_TRANSPORT      1
#define INPUT_GET_ACTIVATE_PORT  2
#define INPUT_GET_CONN_STR       3
#define INPUT_GET_SEND_ADDR      4
#define INPUT_GET_MESSAGE        5
#define INPUT_GET_BTTW_SIZE      6
#define INPUT_GET_BTTW_ADDR      7
#define INPUT_GET_BTTW_INTERVAL  8
#define INPUT_GET_BROADCAST_PORT 9

bool handle_input()
{
	static int transport_id;
	int i,port_num, num;
   static char input_state = 0;
   static char send_msg[128];
   char broadcast_addr[128];
   static int test_size;
   int cmd;
   Error ret;

	if( input_state )
   	cmd = input_state;
   else
   	cmd = command.input[0];

	switch( cmd )
  	{
  		case 'l':
      	for( i = 0; i < client_session->numTransports(); i++ )
 	      	cout << "   " << client_session->getTransportName(i) << endl;
  			break;

      case 'a':
			command.prompt("Transport name to activate:");
         input_state = INPUT_GET_TRANSPORT;
  			break;

      case 'c':
      	cout << "Connect string format [transport name:address]" << endl;
         cout << "[IP:123.45.67.89:2222]" << endl;
         cout << "[IPX:00000000:00600896c336:2222" << endl;
         cout << "[MODEMn:123-4567]" << endl;
         cout << "[COMn:]" << endl;
         cout << "[LOOPBACK:]" << endl;
         command.prompt("Enter connect string:");
      	input_state = INPUT_GET_CONN_STR;
  		   break;

     	case 'd':
     		break;

      case 's':

      	command.prompt("Message to send:");
         input_state = INPUT_GET_MESSAGE;
      	break;

      case 'i':
			if( bttw_test )
         	stop_bttw_test();
         else
         {
	      	command.prompt("Packet size:");
   	      input_state = INPUT_GET_BTTW_SIZE;
         }
         break;

      case 'x':
      	log_active = !log_active;
         log_file.activate( log_active );
        	cout << "log is " << (log_active?"on":"off") << endl;
         break;


   	case 'q':
  	   case 'Q':
     		return false;
        	break;

      case '?':
  	   case 'h':
     		command.list_commands();
        	break;

      case INPUT_GET_TRANSPORT:
         if( !strcmp(command.input,"IP") || !strcmp(command.input,"IPX"))
         {
      		strcpy(tname, command.input);
				command.prompt("Port # to activate:");
	         input_state = INPUT_GET_ACTIVATE_PORT;
         }
         else
         {
   	      input_state = 0;
				if(!strcmp(command.input,"LOOPBACK"))
	            transport[transport_id] = server_session->activate(command.input,0);
            else
	            transport[transport_id] = client_session->activate(command.input,0);
            if( transport[transport_id] == NULL )
            	cout << "Unable to activate transport!" << endl;
            else
            	cout << "Transport " << command.input << " activated!" << endl;
            transport_id++;
         }
      	break;

      case INPUT_GET_ACTIVATE_PORT:
      case INPUT_GET_BROADCAST_PORT:
      	port_num = atoi(command.input);
         if( port_num >= 0 && port_num > 32768 )
         {
         	cout << "Port " << port_num << " is out of range." << endl;
         }
         else
         {
         	if( cmd == INPUT_GET_ACTIVATE_PORT )
            {
	         	transport[transport_id] = client_session->activate(tname,port_num);
   	         if( transport[transport_id] == NULL )
      	      	cout << "Unable to activate transport!" << endl;
         	   else
           			cout << "Transport " << tname << " activated" << endl;
               transport_id++;
            }
            else
            {
               sprintf(broadcast_addr,"IP:broadcast:%s",command.input);
		       	cout << "Broadcasting message: '" << send_msg << "' to " << broadcast_addr << endl;
		      	client_session->send(broadcast_addr,send_msg,strlen(send_msg)+1);
               sprintf(broadcast_addr,"IPX:broadcast:%s",command.input);
		       	cout << "Broadcasting message: '" << send_msg << "' to " << broadcast_addr << endl;
	   	   	client_session->send(broadcast_addr,send_msg,strlen(send_msg)+1);
            }
         }
         input_state = 0;
      	break;

		case INPUT_GET_CONN_STR:
      	vc = client_session->connect(command.input, NULL, 0);
         if( vc == NULL )
           	cout << "Unable to connect transport!" << endl;
         else
         	cout << "Transport " << command.input << " connected" << endl;
         input_state = 0;
        	break;

      case INPUT_GET_SEND_ADDR:
      	i = atoi(command.input);
         if( i == 0 ) // broadcast
         {
				command.prompt("Port # to broadcast to:");
         	input_state = INPUT_GET_BROADCAST_PORT;
         }
         else
         if( i < 1 || i > client_session->numConnections() )
         {
         	cout << "Connection number " << i << " invalid!" << endl;
         	input_state = 0;
         }
         else
         {
	       	cout << "Sending message: '" << send_msg << "'" << "to " << client_session->getConnectionName(i) << endl;
   	     	ret = client_session->send(client_session->getConnectionName(i),
               send_msg, strlen(send_msg) + 1);
            if( ret != DNet::NoError )
            {
            	log_file.log("Send Error %ld",(long)ret);
               cout << "Send error " << ret << endl;
            }
         	input_state = 0;
         }
         break;


      case INPUT_GET_MESSAGE:
      	strcpy(send_msg, command.input);
         num = client_session->numConnections();
         cout << 0 << " - " << "IP/IPX broadcast" << endl;
	      for( i = 0; i < num; i++ )
   	     	cout << (i+1) << " - " << client_session->getConnectionName(i+1) << endl;
      	command.prompt("Enter address for send:");
         input_state = INPUT_GET_SEND_ADDR;
      	break;

		case INPUT_GET_BTTW_INTERVAL:
      	bttw_interval = (WORD)atoi(command.input);

        	num = client_session->numConnections();
      	for( i = 0; i < num; i++ )
  	      	cout << (i+1) << " - " << client_session->getConnectionName(i+1) << endl;
     	   command.prompt("Enter connection for test:");
         input_state = INPUT_GET_BTTW_ADDR;
      	break;

      case INPUT_GET_BTTW_SIZE:
      	test_size = atoi(command.input);
         if( test_size <= 0 || test_size > DNet::MaxPacketSize )
         {
				cout << "Invalid test size "<< test_size << endl;
	         input_state = 0;
         }
         else
         {
      	   command.prompt("Enter send interval (msecs):");
            input_state = INPUT_GET_BTTW_INTERVAL;
	      }
         break;

      case INPUT_GET_BTTW_ADDR:
      	i = atoi(command.input);
         if( i < 1 || i > client_session->numConnections() )
         {
         	cout << "Connection number " << i << " invalid!" << endl;
         }
         else
         {
				strcpy(send_msg, bttw_init_string);
            *((WORD *)(&send_msg[strlen(bttw_init_string)])) = (WORD)test_size;
            *((WORD *)(&send_msg[strlen(bttw_init_string)+2])) = (WORD)bttw_interval;
   	     	ret = client_session->send(client_session->getConnectionName(i),
            	send_msg, strlen(bttw_init_string) + 4);
            if( ret != DNet::NoError )
            {
            	log_file.log("Send Error %ld",(long)ret);
               cout << "Send error " << ret << endl;
            }
            else
            {
            	cout << "BTTW test message sent, initialization successful!" << endl;
		      	initialize_bttw_data(test_size, client_session->getConnectionName(i));
            }
         }
      	input_state = 0;
      	break;


      default:
  	   	cout << "Unrecognized command '" << command.input[0] << "'." << endl;
     		break;
   }

   return true;
}

/*-------------------------------------------------------------*/
/* bttw test is designed to put a lot of data through the pipe */
/* also some primitive data collection going on                */
/*-------------------------------------------------------------*/

void initialize_bttw_data( int data_size, const char *addr_str)
{
	BYTE value;
   int  i;

   bttw_size = data_size;
   bttw_test = true;
   bttw_stat_time = GetTickCount();
   bttw_stat_size = 0;

   bttw_data = new BYTE[bttw_size];

   bttw_data[0] = 0;
   bttw_data[1] = bttw_increment++;
   for( i = 2, value = 0; i < bttw_size; i++, value++ )
   	bttw_data[i] = i;

   strcpy(bttw_address_string, addr_str);
   client_session->send( bttw_address_string, bttw_data, bttw_size );
}


bool compare_bttw_data( BYTE *data, int data_size )
{
	int i;
	if( data_size != bttw_size )
   {
	   log_file.log("BTTW_TEST FAILURE: size mismatch - host size %ld, data size %ld",(long)bttw_size, (long)data_size);
   	return false;
   }

	for( i = 2; i < bttw_size; i++ )
   	if( data[i] != bttw_data[i] )
      {
			log_file.log("BTTW_TEST FAILURE: data mismatch at byte %ld - host value %ld, data value %ld",(long)i, (long)bttw_data[i], (long)data[i]);
      	return false;
      }

	bttw_stat_size += bttw_size;
   bttw_stat_thru = (bttw_stat_size * 1000 ) / (GetTickCount()-bttw_stat_time);
   return true;
}

void bttw_test_process()
{
	static DWORD last_send_time = 0;

	if(bttw_test && GetTickCount() > (last_send_time + (DWORD)bttw_interval))
   {
      bttw_data[0] = 0;
   	bttw_data[1] = bttw_increment++;
      client_session->send(bttw_address_string,bttw_data, bttw_size );
      last_send_time = GetTickCount();
   }
}

void stop_bttw_test()
{
	delete bttw_data;
   bttw_test = false;
}

