all : client server

client : common.h common.c client.c
	gcc client.c common.c -lc -o client.o

server : common.h common.c server.c
	gcc server.c common.c -lc -o server.o

clean :
	rm client.o server.o
