all: pellets-store

pellets-store: pellets-store.c
	gcc pellets-store.c -lwiringPi -o pellets-store

clean:
	- rm -f ../bin/weather-reader core

install: pellets-store

.PHONY: install uninstall clean
