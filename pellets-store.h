#ifndef _PELLETS_STORE_h_
#define _PELLETS_STORE_h_

#include <mysql.h>
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
	
	int  pelletsEchoPin;
	int  pelletsTriggerPin;
	int  pelletsSamples;
} ConfigSettings;

ConfigSettings 	*config;
MYSQL 		*db;
HCSensor 	*pellets_sensor;

int main( int argc, char *argv[]);
void setup( void );
int configRead();
void setupMysql();

#endif