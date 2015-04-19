#include "hc-sensor.h"

#define MAX_ECHO 20000			// Aprox 3.4 meters

HCSensor *hc_init( char trig_pin, char echo_pin ) {
	HCSensor *s = (HCSensor *) malloc( sizeof( HCSensor ) );
	if ( !s )
		return NULL;
	s->trig_pin = trig_pin;
	s->echo_pin = echo_pin;
	pinMode( trig_pin, OUTPUT);
	pinMode( echo_pin, INPUT);
	return s;
}
 
int hc_measure( HCSensor *s ) {
	long start, end, max;
	//Send trig_pin pulse
	digitalWrite( s->trig_pin, LOW  );
	delayMicroseconds(2);
	digitalWrite( s->trig_pin, HIGH );
	delayMicroseconds(10);
	digitalWrite( s->trig_pin, LOW  );
	
	// Measure echo_pin (max MAX_ECHO ms)
	max   = micros() + MAX_ECHO;
	while( digitalRead( s->echo_pin ) == LOW  && micros() < max );
	start = micros();
	while( digitalRead( s->echo_pin ) == HIGH && micros() < max );
	end   = micros();
	
	if ( end >= max )
		return -1;
	
	return (int) ( end - start ) / 58;
}
 
int hc_sample( HCSensor *s, int samples ) {
	int var[samples], i, j, k;
	
	// Take samples
	for ( i = 0; i < samples; i++ ) {
		// Max 10 retries
		for ( j = 0; j < 10 && (k = hc_measure( s ) ) < 0; j++ );
		var[i] = k;
	}
	
	// Bubble sort
	for ( j = 0; j < (samples - 1) ; j++ ) {
		for ( i = 0; i < (samples - j - 1) ; i++ ) {
			if ( var[i] > var[i+1] ) {
				k        = var[i];
				var[i]   = var[i+1];
				var[i+1] = k;
			}
		}
	}
	
	// Return median value
	return var[samples/2];
}
