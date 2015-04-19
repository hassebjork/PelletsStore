#ifndef _PELLETS_STORE_h_
#define _PELLETS_STORE_h_

#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "hc-sensor.h"

typedef struct {
	char  mysql;
	char  mysqlServer[MAX_TAG_SIZE];
	char  mysqlUser[MAX_TAG_SIZE];
	char  mysqlPass[MAX_TAG_SIZE];
	char  mysqlDatabase[MAX_TAG_SIZE];
	
	char  server;
	char  udpServer[15];
	int   udpPort;
	
	int   pelletsEchoPin;
	int   pelletsTriggerPin;
	int   pelletsSamples;
	int   pelletsFull;
	int   pelletsEmpty;
} ConfigSettings;

ConfigSettings 	*config;
HCSensor 	*pellets_sensor;

int main( int argc, char *argv[]);
void setup( void );
int configRead();

#ifdef _MYSQL
#include <mysql.h>
MYSQL 		*db;
void mysqlSetup();
void mysqlPelletsStore( int distance );
#endif

#ifdef _UDP
#include "client-udp.h"
void udpPelletsStore( int distance );
#endif

#endif

