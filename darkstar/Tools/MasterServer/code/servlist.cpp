#include "windows.h"
#include <winsock>
#include "mstrsvr.h"
#include "servlist.h"
#include "servlog.h"
#include "stdio.h"

extern class LogFile log_file;

void ServerList::AddServer( serv_addr *server, u_long beat, WORD key, char *name )
{
 	nodeptr n;

   // make sure we aren't adding duplicate node
   if( IsInList( server ) )
   {
   	printf("Error: Duplicate server\n");
      return;
   }

   if( key == BANNED_SERVER_KEY )
	  	log_file.log("Server %s:%ld banned",inet_ntoa(server->addr),(long)ntohs(server->port));
   else if ( key == NORMAL_SERVER_KEY )
	  	log_file.log("Server %s:%ld online",inet_ntoa(server->addr),(long)ntohs(server->port));
   else
   	log_file.log("Server %s:%ld is being verified.",inet_ntoa(server->addr),(long)ntohs(server->port));

	n = new node;
 	n->beat = n->online = beat;
   n->key  = key;
   if( name != NULL )
   	strcpy(n->name, name);
   else
   	strcpy(n->name, "none");
   n->server.addr = server->addr;
   n->server.port = server->port;
 	server_count++;


 	if (start == NULL)
 	{
  		start = n;
  		n->next = NULL;
 	}
 	else
 	{
   	n->next = start;
      start = n;
   }
}


void ServerList::DeleteNode( nodeptr p )
{
	nodeptr prev, cur;

   cur  = start;
   prev = NULL;

   while( p != cur )
   {
   	prev = cur;
      cur = cur->next;
   }

	if ( cur == start )
  		start = start->next;
	else if( prev != NULL )
  		prev->next = cur->next;

   if( cur->key == NORMAL_SERVER_KEY )
	   log_file.log("Server %s:%ld offline: minutes online %ld",
          inet_ntoa(cur->server.addr),(long)ntohs(cur->server.port),
          (long)((GetTickCount() - cur->online)/(unsigned long)60000));
	delete cur;
	server_count--;
}

void ServerList::DeleteNode( serv_addr * server )
{
	nodeptr prev, cur;

	cur = start;

	while ( ( cur != NULL ) &&
         ( (server->port != cur->server.port) || (server->addr.s_addr != cur->server.addr.s_addr)))
   {
		prev = cur;
  		cur = cur->next;
 	}

 	if (cur != NULL) // the entry was found
 	{
  		if ( cur == start )
   		start = start->next;
  		else if( prev != NULL )
   		prev->next = cur->next;

//   	log_file.log("Server %s:%ld offline",inet_ntoa(cur->server.addr),(long)ntohs(cur->server.port));
		delete cur;
  		server_count--;
	}
}

void ServerList::TimeoutList( u_long time, u_long timeout )
{
	bool deleted = TRUE;
	nodeptr cur;

   while( deleted == TRUE )
   {
   	deleted = FALSE;
      cur = start;
      while( cur != NULL && deleted == FALSE )
      {
	      if(time - cur->beat > timeout )
   	   {
         	DeleteNode( cur );
      		deleted = TRUE;
	      }
         else
	      	cur = cur->next;
      }
   }
}

void ServerList::ListServers()
{
	int index = 1;
	nodeptr p = start;

  	printf("\n");
	printf("id    name                           address          port\n");
   printf("----------------------------------------------------------\n");

	while (p != NULL)
	{
		printf("%-5ld %-30s %-16s %ld\n",(long)index,p->name,inet_ntoa(p->server.addr),(long)ntohs(p->server.port));
  		p = p->next;
      index++;
 	}
}

bool ServerList::IsInList( serv_addr *s, WORD key )
{
	nodeptr p = start;

 	while (p != NULL)
   {
   	if( p->key == key || key == ANY_SERVER_KEY )
      {
	   	if( p->key == BANNED_SERVER_KEY )
   	   {
      	   // address can have wildcard so we have to check each byte
         	// the wildcard is indicated in the port field
	         if( ( s->addr.s_net  == p->server.addr.s_net  || p->server.port == 1 ) &&
   	          ( s->addr.s_host == p->server.addr.s_host || p->server.port == 2 ) &&
      	       ( s->addr.s_lh   == p->server.addr.s_lh   || p->server.port == 3 ) &&
         	    ( s->addr.s_impno== p->server.addr.s_impno|| p->server.port == 4 ))
            	return TRUE;
	      }
   	   else
      	{
				if( (s->addr.s_addr == p->server.addr.s_addr) &&
            	 (s->port == p->server.port) )
	   	   	return TRUE;
   	   }
      }
	  	p = p->next;
   }

 	return FALSE;
}

bool ServerList::IsInList( serv_addr *s )
{
	return IsInList( s, ANY_SERVER_KEY );
}

bool ServerList::MaxServersReached( serv_addr *server )
{
   int num_servers = 0;
	nodeptr p = start;

 	while (p != NULL)
   {
      if( server->addr.s_addr == p->server.addr.s_addr )
      {
      	num_servers++;
         if( num_servers >= max_servers )
         {
         	log_file.log("Too many servers from %s:%d",inet_ntoa(server->addr),(long)ntohs(server->port));
            return true;
         }
      }
	  	p = p->next;
   }

 	return FALSE;
}


nodeptr ServerList::FindServer( serv_addr *server )
{
	nodeptr p = start;

 	while (p != NULL)
   {
   	if((server->port == p->server.port) && (server->addr.s_addr == p->server.addr.s_addr))
      	return p;
	  	p = p->next;
   }

 	return NULL;
}

bool ServerList::UpdateServer( serv_addr *server, u_long beat, WORD key )
{
	nodeptr node = FindServer( server );

   if( node )
   {
   	if(beat - node->beat < 200)
   	   return true;
   	node->beat = beat;
      node->key  = key;
   }
   else
   	printf("Error: Unable to find server!\n");
   return false;
}

bool ServerList::GetServer( serv_addr *server, int count )
{
	nodeptr p;
   int     i;

   for( p = start, i = 0; p != NULL; p = p->next, i++ )
   {
   	if( i == count )
      {
      	server->addr = p->server.addr;
         server->port = p->server.port;
         return 1;
      }
   }
   return 0;
}

void ServerList::DeleteList()
{
	nodeptr p = start, n;

	while (p != NULL)
	{
  		n = p;
  		p = p->next;
  		delete n;
	}
   server_count = 0;
   start = 0;
}

int ServerList::Size()
{
	return server_count;
}

bool ServerList::FillBuffer( int *num_servers, int *fill_size,
                             int buffer_size, char *buffer )
{
	nodeptr p = start;
   int     counter;
   int     max_entries = buffer_size/sizeof(serv_addr);

	// check for empty list
   if( p == NULL )
   {
   	*num_servers = 0;
      index = 0;
      *fill_size = 0;
      return FALSE;
   }

   // advance in list to server # specified in index
	counter = 0;
   while( p != NULL && counter < index)
   {
   	p = p->next;
      counter++;
   }

   if( p == NULL ) // if no entry, then error occured
   	printf("FillBuffer: invalid count %ld\n",(long)index);

   // now start filling in data
   counter = 0;

   // NOTE: if data sent is changed, change SERVER_INFO_SIZE as well
   while( p != NULL && counter < max_entries)
   {
      buffer[0] = 6;
      buffer[1] = p->server.addr.s_net;
      buffer[2] = p->server.addr.s_host;
      buffer[3] = p->server.addr.s_lh;
      buffer[4] = p->server.addr.s_impno;
      buffer[5] = p->server.port >> 8;
      buffer[6] = p->server.port & 0xFF;
      buffer += SERVER_INFO_SIZE;  // #bytes sent per server

   	p = p->next;
      counter++;
      index++;
   }

   *fill_size   = counter*SERVER_INFO_SIZE;
   *num_servers = counter;
   if( p == NULL ) // list is done
   {
   	index = 0;
      return FALSE;
   }
   else
   {
   	return TRUE;
   }
}


