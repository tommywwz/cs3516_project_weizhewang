all: TCP_client TCP_server

TCP_client: TCP_client.o
	gcc -g TCP_client.o -o TCP_client

TCP_client.o: TCP_client.c
	gcc -g -c TCP_client.c

TCP_server: TCP_server.o
	gcc -g TCP_server.o -o TCP_server

TCP_server.o: TCP_server.c
	gcc -g -c TCP_server.c

clean:
	rm -r *.o TCP_server TCP_client