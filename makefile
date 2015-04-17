PREFIX=/usr/

all: store mysql

store: hc-sensor.c config.c pellets-store.c
	gcc -o pellets-store \
		-D_DEBUG=0 \
		-lwiringPi \
		hc-sensor.c config.c pellets-store.c

mysql: hc-sensor.c config.c pellets-store.c
	gcc -o pellets-mysql \
		-D_DEBUG=0 \
		-D_MYSQL=1 \
		-lwiringPi \
		$(shell mysql_config --cflags) \
		$(shell mysql_config --libs) \
		hc-sensor.c config.c pellets-store.c

debug: hc-sensor.c config.c pellets-store.c
	gcc -o pellets-store \
		-D_DEBUG=2 -g -ggdb \
		-D_MYSQL=1 \
		-lwiringPi \
		$(shell mysql_config --cflags) \
		$(shell mysql_config --libs) \
		hc-sensor.c config.c pellets-store.c

clean:
	- rm -f ../bin/weather-reader core

install: pellets-store

.PHONY: install uninstall clean
