PROGS =	server client

all: ${PROGS}

server: server.o
	${CC} ${CFLAGS} -o $@ server.o

client: client.o
	${CC} ${CFLAGS} -o $@ client.o

