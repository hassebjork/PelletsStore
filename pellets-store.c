#include "pellets-store.h"

#define CONFIG_FILE_NAME "/etc/pellets-store.conf"

#ifdef _MYSQL
static const char * CREATE_TABLE_MYSQL[] =  {
	"CREATE TABLE IF NOT EXISTS `ps_pellets`( `id` INT NOT NULL AUTO_INCREMENT, `level` INT, `samples` INT UNSIGNED NOT NULL DEFAULT 1, `time` TIMESTAMP, PRIMARY KEY (`id`) ) DEFAULT CHARSET=utf8",
	0
};
#endif

int main( int argc, char *argv[]) {
	int pellets_level;
	
	setup();
	
	if ( pellets_sensor 
		&& ( pellets_level = hc_sample( pellets_sensor, config->pelletsSamples ) ) >= 0 ) {
// 		printf ( "%d cm\n", pellets_level );
		
		// Convert to percent
		pellets_level = 100 - ( pellets_level - config->pelletsFull ) * 100 / config->pelletsEmpty;
		if ( pellets_level < -15 || pellets_level > 115 ) {
			fprintf( stderr, "ERROR: Pellets level out of bounds %d%%\n", pellets_level );
		} else {
		
			// Store and display values
#ifdef _MYSQL
			if ( config->mysql )
				mysqlPelletsStore( pellets_level );
#endif
#ifdef _UDP
			udpPelletsStore( pellets_level );
#endif
			printf ( "%d %%\n", pellets_level );
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
	config->pelletsFull       = 0;
	config->pelletsEmpty      = 100;
	config->udpServer[0]      = '\0';
	config->udpPort           = 0;
	
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
			else if ( configIntVar( rdBuf, "pelletsFull", &config->pelletsFull ) ) {}
			else if ( configIntVar( rdBuf, "pelletsEmpty", &config->pelletsEmpty ) ) {}
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

void mysqlPelletsStore( int level ) {
	char         query[256] = "SELECT `id`, `level`, `samples` FROM `ps_pellets` ORDER BY `time` DESC LIMIT 1";
	MYSQL_RES   *result;
	
	// Update samples if value remains the same
	if ( !mysql_query( db, query ) ) {
		result = mysql_store_result( db );
		if ( result ) {
			MYSQL_ROW row;
			if ( ( row = mysql_fetch_row( result ) ) ) {
				int old_lvl = atoi( row[1] );
				if ( level == old_lvl )
					sprintf( query, "UPDATE `ps_pellets` SET `samples`=%d WHERE `id`=%d", (atoi( row[2] )+1), atoi( row[0] ) );
			}
		} else {
			fprintf( stderr, "ERROR in mysqlPelletsStore: Storing result!\n%s\n", mysql_error( db ) );
		}
	} else {
		fprintf( stderr, "ERROR in mysqlPelletsStore: Selecting\n%s\n%s\n", mysql_error( db ), query );
	}
	
	if ( query[0] == 'S' )
		sprintf( query, "INSERT INTO `ps_pellets` (`level`) VALUES (%d)", level );
	if ( mysql_query( db, query ) ) {
		fprintf( stderr, "ERROR in mysqlQuery: \t%s.\n%s\n", mysql_error( db ), query );
	}
}
#endif

#ifdef _UDP
void udpPelletsStore( int level ) {
	fprintf( stderr, "Using UDP client: %s:%d\n", config->udpServer, config->udpPort );
	char data[256] = "";
	if ( config->server && level > -128  && level < 128 ) {
// 		sprintf( data, "PELL LVL%d", level ); 
		sprintf( data, "PELL LVL%c", ( level + 128 ) );
		printf( "'%s' %d\n", data, strlen( data ) );
		if ( client_udp( data, config->udpServer, config->udpPort ) == 1 )
			config->server = 0;
	}
}
#endif
