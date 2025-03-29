.DEFAULT_GOAL :=gcc

gcc: src/main.c
	gcc src/*.c  -o dns
install: dns
	cp dns /usr/local/bin
	cp dns.service /lib/systemd/system

uninstall:
	rm /usr/local/bin/dns
	rm /lib/systemd/system/dns.service
