all: server fancyclient

server: server.c
	gcc -Wall server.c -o server

fancyclient: fancyclient.c
	gcc -Wall fancyclient.c -o fancyclient

clean:
	rm server fancyclient
