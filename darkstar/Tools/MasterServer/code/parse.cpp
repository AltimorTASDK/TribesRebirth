//------------------------------------------------------------------
//
//	parse.cpp
//
//
//------------------------------------------------------------------

#include "windows.h"
#include "stdio.h"
#include "stdarg.h"

struct IPRecord {
	int	ip1;
	int	ip2;
	int	ip3;
	int	ip4;
	int	port;
	IPRecord *next;
};

#define HASH_TABLE_SIZE 509

FILE	*inFile;
FILE	*outFile;
char	outFileName[256];
bool	verbose = true;
int	interval = 5;
unsigned long runningServers = 0;
unsigned long serverTotal = 0;
unsigned long durationTotal = 0;
unsigned long queryCount = 0;
unsigned long serverCount = 0;
IPRecord queryTable[HASH_TABLE_SIZE];
IPRecord serverTable[HASH_TABLE_SIZE];
IPRecord runningTable[HASH_TABLE_SIZE];
unsigned long collisions = 0;

void parsePrintf (char *controlString, ...)
{
	if (!verbose)
		return;

	va_list ptr;

	va_start (ptr, controlString);
	vprintf (controlString, ptr);
	va_end (ptr);
}

void skipInput (char *buff, int count)
{
	parsePrintf ("%s ", buff);
	for (int i = 0; i < count; i++)
		{
			fscanf (inFile, "%s", buff);
			parsePrintf ("%s ", buff);
		}
	parsePrintf ("\n");
}

void skipToEOL (char *buff)
{
	parsePrintf ("%s", buff);
	int i = 0;
	fscanf (inFile, "%c", &buff[0]);
	while (buff[i] != '\n')
		{
			i++;
			fscanf (inFile, "%c", &buff[i]);
		}
	buff[i+1] = '\0';
	parsePrintf ("%s", buff);
}

void decodeID (char *buff, int *id)
{
	fscanf (inFile, "%s", buff);
	fscanf (inFile, "%s", buff);
	fscanf (inFile, "%s", buff);
	*id = atoi (buff);
}

void parseArgs (int argc, char **argv)
{
	strcpy (outFileName, "data.txt");
	if (argc < 2)
		{
			printf ("\nUsage: PARSE logfile [options]\n");
			printf (" -v	Turn verbose output mode OFF (on by default).\n");
			printf (" -oY	Set output file to Y (\"%s\" is default).\n", outFileName);
			printf (" -iX	Set interval to X minutes (%d minutes by default, 60 minutes or less).", interval);
			exit (EXIT_FAILURE);
	   }

	for (int count = 2; count < argc; count++)
		{
			char *opt = argv[ count ];
			if( opt[0] == '-' )
				{
					switch( opt[1] )
						{
							case 'v':
								verbose = false;
								break;

							case 'i':
								interval = atoi (&opt[2]);
								if (interval < 0)
									interval = 0;
//								if (interval > 60)
//									interval = 60;
								parsePrintf ("Reporting interval set to %d minutes.\n", interval);
								break;
						}
				}
	      else
		      {
		      	printf("Error: Invalid argument %s.\n",opt);
			      exit(EXIT_FAILURE);
		      }
		}

	inFile = fopen (argv[1], "r");
	if (!inFile)
		{
			printf ("Couldn't open file %s.\n", argv[1]);
			exit (EXIT_FAILURE);
		}

	outFile = fopen (outFileName, "w");
	if (!outFile)
		{
			printf ("Couldn't open file %s.\n", argv[1]);
			exit (EXIT_FAILURE);
		}

	fprintf (outFile, "File \"%s\"\n", argv[1]);
	fprintf (outFile, "%d minute intervals.\n", interval);
}

void incCount (IPRecord *table, unsigned long *count, int ip1, int ip2, int ip3, int ip4, int port)
{
	IPRecord *ptr;

	int hash = (ip1 + ip2 + ip3 + ip4) % HASH_TABLE_SIZE;

	if (hash >= HASH_TABLE_SIZE || hash < 0)
		{
			printf ("Bad hash value : %d\n", hash);
			exit (EXIT_FAILURE);
		}

	ptr = &(table[hash]);
	while (ptr)
		{
			if (ptr->ip1 == -1 && ptr->ip2 == -1 && ptr->ip3 == -1 && ptr->ip4 == -1 && ptr->port == -1)
				{
					ptr->ip1 = ip1;
					ptr->ip2 = ip2;
					ptr->ip3 = ip3;
					ptr->ip4 = ip4;
					ptr->port = port;
					ptr->next = NULL;
					*count += 1;
					return;
				}
			else
				if (ptr->ip1 == ip1 && ptr->ip2 == ip2 && ptr->ip3 == ip3 && ptr->ip4 == ip4 && ptr->port == port)
					return;
				else
					if (!ptr->next)
						{
							IPRecord *newRecord = new IPRecord;

							ptr->next = newRecord;
							newRecord->ip1 = ip1;
							newRecord->ip2 = ip2;
							newRecord->ip3 = ip3;
							newRecord->ip4 = ip4;
							newRecord->port = port;
							newRecord->next = NULL;
							*count += 1;
							return;
						}

			ptr = ptr->next;
		}
}

void removeElement (IPRecord *table, unsigned long *count, int ip1, int ip2, int ip3, int ip4, int port)
{
	IPRecord *ptr;
	IPRecord *prev = NULL;

	int hash = (ip1 + ip2 + ip3 + ip4) % HASH_TABLE_SIZE;

	if (hash >= HASH_TABLE_SIZE || hash < 0)
		{
			printf ("Bad hash value : %d\n", hash);
			exit (EXIT_FAILURE);
		}

	ptr = &(table[hash]);
	while (ptr)
		{
			if (ptr->ip1 == ip1 && ptr->ip2 == ip2 && ptr->ip3 == ip3 && ptr->ip4 == ip4 && ptr->port == port)
				{
					*count -= 1;
					if (prev)
						{
							prev->next = ptr->next;
							delete ptr;
						}
					else
						{
							if (!ptr->next)
								{
									ptr->ip1 = -1;
									ptr->ip2 = -1;
									ptr->ip3 = -1;
									ptr->ip4 = -1;
									ptr->port = -1;
								}
							else
								{
									ptr->ip1 = ptr->next->ip1;
									ptr->ip2 = ptr->next->ip2;
									ptr->ip3 = ptr->next->ip3;
									ptr->ip4 = ptr->next->ip4;
									ptr->port = ptr->next->port;
									ptr->next = ptr->next->next;
								}
						}

					return;
				}

			prev = ptr;
			ptr = ptr->next;
		}
}

void deleteList (IPRecord *head)
{
	if (head->next)
		deleteList (head->next);

	delete head;
}

void initTable (IPRecord *table, unsigned long *count)
{
	for (int i = 0; i < HASH_TABLE_SIZE; i++)
		{
			table[i].ip1 = -1;
			table[i].ip2 = -1;
			table[i].ip3 = -1;
			table[i].ip4 = -1;
			table[i].port = -1;
			table[i].next = NULL;
		}

	*count = 0;
}

void clearCount (IPRecord *table, unsigned long *count)
{
	for (int i = 0; i < HASH_TABLE_SIZE; i++)
		{
			if (table[i].next)
				deleteList (table[i].next);
		}

	initTable (table, count);
}

void newInterval (void)
{
	parsePrintf (" - New interval (%d minutes, %d queries, %d new servers)\n", interval, queryCount, serverCount);
	fprintf (outFile, "%d, %d, %d\n", queryCount, serverCount, runningServers);
	clearCount (queryTable, &queryCount);
	clearCount (serverTable, &serverCount);
}

void main(int argc, char **argv)
{
	parseArgs (argc, argv);

	initTable (queryTable, &queryCount);
	initTable (serverTable, &serverCount);
	initTable (runningTable, &runningServers);

	int	lastHour = -1;
	int	lastMinute = -1;
	int	hour;
	int	minute;
	int	second;
	char	buff[256];
	while (fscanf (inFile, "%s", buff) != EOF)
		{
			buff[2] = '\0';
			hour = atoi (&buff[0]);
			buff[5] = '\0';
			minute = atoi (&buff[3]);
			buff[8] = '\0';
			second = atoi (&buff[6]);
			
			if (lastHour == -1 && lastMinute == -1)
				{
					lastHour = hour;
					lastMinute = minute;
					fprintf (outFile, "Start time = %02d:%02d:%02d\n", hour, minute, second);
				}

			if (interval)
				{
					if (lastHour > hour)
						lastHour -= 24;

					if (lastHour != hour)
						{
							lastMinute -= abs(hour - lastHour) * 60;
							lastHour = hour;
						}
							
					while (minute - lastMinute >= interval)
						{
							newInterval ();
							lastMinute += interval;
//							if (lastMinute > 59)
//								{
//									lastMinute -= 60;
//									lastHour += 1;
//								}
						}
				}

			parsePrintf ("At %02d:%02d:%02d - ", hour, minute, second);
			fscanf (inFile, "%s", buff);
			if (!stricmp ("log", buff))
				skipInput (buff, 3);
			else
				if (!stricmp ("SND_BUFF", buff) || !stricmp ("RCV_BUFF", buff) || !stricmp ("Timeout", buff))
					skipInput (buff, 4);
				else
					if (!stricmp ("Maxservers", buff))
						skipInput (buff, 6);
					else
						if (!stricmp ("Master", buff))
							{
								parsePrintf ("%s ", buff);
								fscanf (inFile, "%s", buff);
								parsePrintf ("%s ", buff);
								fscanf (inFile, "%s", buff);
								
								if (!stricmp ("running", buff))
									skipInput (buff, 3);
								else
									if (!stricmp ("id", buff))
										skipInput (buff, 1);
							}
						else
							if (!stricmp ("Server", buff))
								{
									int ip1;
									int ip2;
									int ip3;
									int ip4;
									int port;
									parsePrintf ("%s ", buff);
									fscanf (inFile, "%d.%d.%d.%d:%d ", &ip1, &ip2, &ip3, &ip4, &port);
									parsePrintf ("IP:%d.%d.%d.%d:%d ", ip1, ip2, ip3, ip4, port);
									fscanf (inFile, "%s", buff);
									if (!stricmp ("is", buff))
										skipInput (buff, 2);
									else
										if (!stricmp ("offline:", buff))
											{
												int duration;
												fscanf (inFile, "%s", buff);
												
												parsePrintf ("%s ", buff);
												fscanf (inFile, "%s", buff);
												parsePrintf ("%s ", buff);
												fscanf (inFile, "%d", &duration);
												parsePrintf ("%d.\n", duration);
												serverTotal++;
												durationTotal += duration;
												removeElement (runningTable, &runningServers, ip1, ip2, ip3, ip4, port);
											}
										else
											{
												parsePrintf ("%s\n", buff);
												incCount (serverTable, &serverCount, ip1, ip2, ip3, ip4, port);
												if (!stricmp ("online", buff))
													incCount (runningTable, &runningServers, ip1, ip2, ip3, ip4, port);
											}
								}
							else
								if (!stricmp ("Query", buff))
									{
										int ip1;
										int ip2;
										int ip3;
										int ip4;
										int port;
										int id;
										parsePrintf ("%s ", buff);
										fscanf (inFile, "%s", buff);
										parsePrintf ("%s ", buff);
										fscanf (inFile, "%d.%d.%d.%d:%d, ", &ip1, &ip2, &ip3, &ip4, &port);
										parsePrintf ("IP:%d.%d.%d.%d:%d", ip1, ip2, ip3, ip4, port);
										decodeID (buff, &id);
										parsePrintf (", id = %d\n", id);
//										incCount (queryTable, &queryCount, ip1, ip2, ip3, ip4, port);
										incCount (queryTable, &queryCount, ip1, ip2, ip3, ip4, -1);
									}
								else
									if (!stricmp ("Ignoring", buff) || !stricmp ("Received", buff) || !stricmp ("Error:", buff))
										skipToEOL (buff);
									else
										if (!stricmp ("Unknown", buff))
											{
												skipToEOL (buff);
												skipToEOL (buff);
											}
										else
											{
												printf ("\nUnknown data line.\n");
												exit (EXIT_FAILURE);
											}
		}

	newInterval ();

	printf ("\nEnd of file.\n");
	printf ("Average server lifespan : %d minutes.\n", durationTotal / serverTotal);

	if (inFile)
		fclose (inFile);

	if (outFile)
		fclose (outFile);
}
