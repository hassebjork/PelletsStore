#ifndef _HC_SENSOR_h
#define _HC_SENSOR_h

#include <stdlib.h>
#include <wiringPi.h>
 
typedef struct {
	char  trig_pin;				// Trigger pin
	char  echo_pin;				// Echo pin
} HCSensor;


HCSensor *hc_init( char trig_pin, char echo_pin );
int hc_measure( HCSensor *s );
int hc_sample(  HCSensor *s, int samples );

#endif