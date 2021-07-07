all: TCP_client_gui

TCP_client_gui: TCP_client_gui.o
	gcc `pkg-config --cflags gtk+-3.0` -pthread -g TCP_client_gui.o -o TCP_client_gui `pkg-config --libs gtk+-3.0`

TCP_client_gui.o: TCP_client_gui.c
	gcc `pkg-config --cflags gtk+-3.0` -g -c TCP_client_gui.c `pkg-config --libs gtk+-3.0`

clean:
	rm -r *.o TCP_client_gui