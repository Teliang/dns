.DEFAULT_GOAL :=gcc

CFLAGS = -Ilib

gcc: src/main.c
	gcc $(CFLAGS) src/*.c lib/*.h -o dns
debug: src/main.c
	gcc $(CFLAGS) -g src/*.c lib/*.h -o dns
install: dns
	cp dns /usr/local/bin
	cp dns.service /lib/systemd/system

uninstall:
	rm /usr/local/bin/dns
	rm /lib/systemd/system/dns.service
