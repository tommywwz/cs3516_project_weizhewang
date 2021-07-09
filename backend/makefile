all: TCP_client TCP_server

TCP_client: TCP_client.o
	gcc -pthread -g TCP_client.o -o TCP_client

TCP_client.o: TCP_client.c
	gcc -g -c TCP_client.c

TCP_server: TCP_server.o hash.o
	gcc -pthread -g TCP_server.o hash.o -o TCP_server

TCP_server.o: TCP_server.c hash.h
	gcc -g -c TCP_server.c

hash.o: hash.c hash.h
	gcc -g -c hash.c

clean:
	rm -r *.o TCP_server TCP_client