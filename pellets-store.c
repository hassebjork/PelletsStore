#include "pellets-store.h"

#define CONFIG_FILE_NAME "/etc/pellets-store.conf"

#ifdef _MYSQL
static const char * CREATE_TABLE_MYSQL[] =  {
	"CREATE TABLE IF NOT EXISTS ps_pellets( id INT NOT NULL AUTO_INCREMENT, distance SMALLINT, time TIMESTAMP, PRIMARY KEY (id) )",
	0
};
#endif

int main( int argc, char *argv[]) {
	int       pellets_depth;
	
	setup();
	
	if ( pellets_sensor ) {
		pellets_depth = hc_sample( pellets_sensor, config->pelletsSamples );
#ifdef _MYSQL
		mysqlPelletsStore( pellets_depth );
#endif
#ifdef _UDP
		udpPelletsStore( pellets_depth );
#endif
		printf ( "%d cm\n", pellets_depth );
	}
	
	return 0;
}

void setup( void ) {
	// Create config object
	config = malloc( sizeof( ConfigSettings ) );
	if ( !config ) {
		fprintf( stderr, "ERROR: Could not allocate memory for configurations\n" );
		exit( 1 );
	}
	
	configRead();
#ifdef _MYSQL
	mysqlSetup();
#endif
	wiringPiSetup();
	
	pellets_sensor = hc_init( config->pelletsTriggerPin, config->pelletsEchoPin );
	if ( !pellets_sensor )
		fprintf( stderr, "ERROR: Could not allocate HC-Sensor\n" );
}

int configRead() {
	char rdBuf[READ_BUFSIZE];
	FILE *fp;
	
	/* Default Values */
	config->mysqlServer[0]    = '\0';
	config->mysqlUser[0]      = '\0';
	config->mysqlPass[0]      = '\0';
	config->mysqlDatabase[0]  = '\0';
	config->pelletsEchoPin    = 0;
	config->pelletsTriggerPin = 1;
	config->pelletsSamples    = 7;
	config->udpServer[0]      = '\0';
	config->udpPort     = 0;
	
	if ( ( fp = fopen( CONFIG_FILE_NAME, "r" ) ) == NULL ) {
		fprintf( stderr, "ERROR: Could not read configuration file %s\n", CONFIG_FILE_NAME );
		exit( 2 );
	} else {
		fprintf( stderr, "Reading configurations: %s\n", CONFIG_FILE_NAME );
	}
	
	/* Read each line in the file and process the tags on that line. */
	while ( fgets( rdBuf, READ_BUFSIZE, fp ) != NULL ) {
		if ( ( rdBuf[0] != ';' ) && ( rdBuf[0] != '[' ) ) {
			if ( configStringVar( rdBuf, "mysqlServer", config->mysqlServer ) ) {}
			else if ( configStringVar( rdBuf, "mysqlUser", config->mysqlUser ) ) {}
			else if ( configStringVar( rdBuf, "mysqlPass", config->mysqlPass ) ) {}
			else if ( configStringVar( rdBuf, "mysqlDatabase", config->mysqlDatabase ) ) {}
			else if ( configIntVar( rdBuf, "pelletsEchoPin", &config->pelletsEchoPin ) ) {}
			else if ( configIntVar( rdBuf, "pelletsTriggerPin", &config->pelletsTriggerPin ) ) {}
			else if ( configIntVar( rdBuf, "pelletsSamples", &config->pelletsSamples ) ) {}
			else if ( configStringVar( rdBuf, "udpServer", config->udpServer ) ) {}
			else if ( configIntVar( rdBuf, "udpPort", &config->udpPort ) ) {}
		}
	}
	fclose( fp );
	
	config->mysql  = ( config->mysqlServer[0] != 0 && config->mysqlUser[0] != 0 && config->mysqlPass[0] != 0 && config->mysqlDatabase[0] != 0 );
	config->server = ( config->udpServer[0] != 0 && config->udpPort != 0 );
	
	return 0;
}

#ifdef _MYSQL
void mysqlSetup() {
	int i;
	my_bool myb = 1;

	if ( !config->mysql )
		return;
	
	db = mysql_init( db );
	if ( !db ) {
		fprintf( stderr, "ERROR in mysqlInit: Initiating MySQL database!\n%s\n", mysql_error( db ) );
		config->mysql = 0;
	
	// Connect to database
	} else {
		mysql_options( db, MYSQL_OPT_RECONNECT, &myb);
		if ( mysql_real_connect( db, config->mysqlServer, config->mysqlUser, config->mysqlPass, config->mysqlDatabase, 0, NULL, 0 ) == NULL ) {
			fprintf( stderr, "ERROR in mysqlInit: Can not connect to MySQL database!\n%s\n", mysql_error( db ) );
			mysql_close( db );
			config->mysql = 0;
		
		// Create tables
		} else {
			fprintf( stderr, "Using MySQL database: mysql://%s/%s\n", config->mysqlServer, config->mysqlDatabase );
			for ( i=0; CREATE_TABLE_MYSQL[i] != 0; i++ ) {
				if ( mysql_query( db, CREATE_TABLE_MYSQL[i] ) ) {
					fprintf( stderr, "ERROR in mysqlQuery: \t%s.\n%s\n", mysql_error( db ), CREATE_TABLE_MYSQL[i] );
				}
			}
		}
	}
}

void mysqlPelletsStore( int distance ) {
	char query[256] = "";
	if ( distance > 0 && config->mysql ) {
		sprintf( query, "INSERT INTO `ps_pellets` (`distance`) VALUES (%d)", distance ); 
		if ( mysql_query( db, query ) ) {
			fprintf( stderr, "ERROR in mysqlQuery: \t%s.\n%s\n", mysql_error( db ), query );
		}
	}
}
#endif

#ifdef _UDP
void udpPelletsStore( int distance ) {
	fprintf( stderr, "Using UDP client: %s:%d\n", config->udpServer, config->udpPort );
	char data[256] = "";
	if ( distance > 0 && config->server ) {
		sprintf( data, "PELL DIST%d", distance ); 
		if ( client_udp( data, config->udpServer, config->udpPort ) == 1 )
			config->server = 0;
	}
}
#endif
