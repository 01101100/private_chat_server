all: server
server: server.o
	cc server.o -oserver
server.o: server.c
	cc -c server.c
