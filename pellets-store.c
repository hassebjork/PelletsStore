#include "pellets-store.h"

#define CONFIG_FILE_NAME "/etc/pellets-store.conf"

static const char * CREATE_TABLE_MYSQL[] =  {
#if _DEBUG > 2
 	"DROP TABLE IF EXISTS ps_pellets",
#endif
	"CREATE TABLE IF NOT EXISTS ps_pellets( id INT NOT NULL AUTO_INCREMENT, distance SMALLINT, time TIMESTAMP, PRIMARY KEY (id) )",
	0
};

int main( int argc, char *argv[]) {
	int       pellets_depth;
	
	setup();
	
	if ( pellets_sensor ) {
		pellets_depth = hc_sample( pellets_sensor, config->pelletsSamples );
		printf ( "%d cm\n", pellets_depth );
		if ( pellets_depth > 0 && config->mysql ) {
			char query[256] = "";
			sprintf( query, "INSERT INTO `ps_pellets` (`distance`) VALUES (%d)", pellets_depth ); 
			if ( mysql_query( db, query ) ) {
				fprintf( stderr, "ERROR in mysqlQuery: \t%s.\n%s\n", mysql_error( db ), query );
			}
		}
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
	setupMysql();
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
		}
	}
	fclose( fp );
	
	config->mysql = ( config->mysqlServer[0] != 0 && config->mysqlUser[0] != 0 && config->mysqlPass[0] != 0 && config->mysqlDatabase[0] != 0 );
	
	return 0;
}

void setupMysql() {
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
