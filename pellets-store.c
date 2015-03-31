#include <stdio.h>
#include <wiringPi.h>
 
#define TRIG 1
#define ECHO 0
#define MAX_ECHO 20000
#define SAMPLES 5

void setup() {
	wiringPiSetup();
	pinMode(TRIG, OUTPUT);
	pinMode(ECHO, INPUT);
}
 
int getPing() {
	long start, end, max;
	//Send trig pulse
	digitalWrite( TRIG, LOW  );
	delayMicroseconds(2);
	digitalWrite( TRIG, HIGH );
	delayMicroseconds(10);
	digitalWrite( TRIG, LOW  );
	
	// Measure echo (max MAX_ECHO ms)
	max   = micros() + MAX_ECHO;
	while( digitalRead( ECHO ) == LOW && micros() < max );
	start = micros();
	while(digitalRead( ECHO ) == HIGH && micros() < max );
	end   = micros();
	
	if ( end >= max )
		return -1;
	
	return (int) ( end - start ) / 58;
}

int main(void) {
	setup();
	
	int var[SAMPLES], i, j, swap;
	
	// Sample
	for ( i = 0; i < SAMPLES; i++ ) {
		for ( j = 0; j < 10 && (swap = getPing() ) == -1; j++ );
		var[i] = swap;
// 		usleep( 200000 );
	}
	
	// Bubble sort
	for ( j = 0; j < (SAMPLES - 1) ; j++ ) {
		for ( i = 0; i < (SAMPLES - j - 1) ; i++ ) {
			if ( var[i] > var[i+1] ) {
				swap     = var[i];
				var[i]   = var[i+1];
				var[i+1] = swap;
			}
		}
	}
	
	// Show median value
	printf ( "%d cm\n",var[SAMPLES/2] );
	return 0;
}