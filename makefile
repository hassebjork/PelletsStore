PREFIX=/usr/
MYSQL_CFLAGS=$(shell mysql_config --cflags)
MYSQL_LIBS=$(shell mysql_config --libs)

all: pellets-store

pellets-store: hc-sensor.c config.c pellets-store.c
	gcc -o pellets-store\
		-D_DEBUG=0 \
		-lwiringPi \
		$(MYSQL_CFLAGS) \
		$(MYSQL_LIBS) \
		hc-sensor.c config.c pellets-store.c

debug: hc-sensor.c config.c pellets-store.c
	gcc -o pellets-store\
		-D_DEBUG=2 -g -ggdb \
		-lwiringPi \
		$(MYSQL_CFLAGS) \
		$(MYSQL_LIBS) \
		hc-sensor.c config.c pellets-store.c

clean:
	- rm -f ../bin/weather-reader core

install: pellets-store

.PHONY: install uninstall clean
