all: client server

client:
	gcc -O3 client.c -lpthread -lncurses -o client

server:
	gcc -O3 server.c -o server
