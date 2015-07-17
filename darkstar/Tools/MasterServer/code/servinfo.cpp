
#include "windows.h"
#include "stdio.h"
#include "servinfo.h"

#define MAX_KEYS 100

//
SEND_DATA SendData;
KEY_TO_STRING KeyToString;

// Constructor
ServerInfo::ServerInfo( char *data, int data_size, SEND_DATA send_func_ptr, KEY_TO_STRING info_func_ptr )
{
  	// needs packet, packetsize and send routine
   reply_data   = data;
   reply_length = data_size;
   SendData     = send_func_ptr;
   KeyToString  = info_func_ptr;
}

/*---------------------------------------------------------------------------*/
/* HandleRequest parses through request data and separates the requests into */
/* server keys and player keys.  The server keys it sends off first in a     */
/* separate packet and then sends the player keys in one or more packets.    */
/* Since it separates the packets into server and player packets, the        */
/* incoming keys should be in order (server keys first, then player), or the */
/* client wouldn't get back the data in the order that it was sent.          */
/*---------------------------------------------------------------------------*/

bool ServerInfo::HandleRequest( char * query_data, WORD packet_key )
{
	KEY   key;
   int   i, data_size;
   int   player_key_index = 0; // number of player keys received
   int   server_key_index = 0; // number of server keys received
   bool  server_query,player_query;
   KEY   player_key_list[MAX_KEYS]; // stores player keys
   KEY   server_key_list[MAX_KEYS]; // stores server keys
   short num_commands, num_players;

	// initialize
   for( i = 0; i < MAX_KEYS; i++ ) { player_key_list[i] = INVALID_KEY; }
   for( i = 0; i < MAX_KEYS; i++ ) { player_key_list[i] = INVALID_KEY; }

   num_commands = *((short *)query_data); // number of command strings sent
   query_data += sizeof(short);

	server_query   = FALSE;
   player_query   = FALSE;
   while( num_commands )
   {
printf("%s ",query_data);
		key = StringToKey(query_data);

      /*-----------------------------------------------------------------*/
      /* We assume that server keys come first, so once we start getting */
      /* to the player keys, all further server keys are invalid.        */
      /*-----------------------------------------------------------------*/

      if( server_key( key ) ) // server info request
      {
      	server_query = TRUE;
      	if( player_query ) // player data already started, so this is treated as invalid player key
            player_key_list[player_key_index++] = INVALID_KEY;
         else
         	server_key_list[server_key_index++] = key;
      }
      else // player request
      {
      	player_query = TRUE;
			player_key_list[player_key_index++] = key; // save up player keys
      }

      query_data  += strlen(query_data) + 1;
      num_commands--;
   }

   if( server_query) // send server packets
   {
      data_size = PackData( server_key_list, server_key_index, 0 );
      SendData( reply_data, data_size, packet_key );
   }

   if ( player_query ) // send player packets
   {
   	num_players  = atoi(KeyToString( NUM_PLAYERS, 0 ));
		for( i = 0; i < num_players; i++ )
      {
      	data_size = PackData( player_key_list, player_key_index, i );
	      SendData( reply_data, data_size, packet_key );
      }
   }

   return FALSE; // succesfull
}

int ServerInfo::PackData( KEY key[], int size, int num )
{
	int i, len, reply_index;
   char *str;

   *(short *)&reply_data[0] = size; // number of strings in this packet
	reply_index = sizeof(short);

   for( i = 0; i < size; i++ )
   {
   	str = KeyToString( key[i], num);
		len = strlen( str ) + 1;

      // should check for buffer overflow
      if( reply_index + len > reply_length )
      {
      	printf("Warning: PackData buffer too small on key %ld.\n",(long)key[i]);
      }
		else
      {
		   strcpy(&reply_data[reply_index], str);
   		reply_index += len;
      }
   }
   return reply_index;
}


KEY ServerInfo::StringToKey( char * s)
{
	int i;

   for( i = 0; i < NUM_SERVER_REQUESTS; i++ )
   {
   	if(!strcmp(s,server_strings[i]))
      	return (KEY)i;
   }

   return INVALID_KEY;
}
