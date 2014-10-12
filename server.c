#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <sys/fcntl.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef unsigned int uint;

typedef struct{
	uint sockets[100];
	uint count;
}sock_container;

void add_client(sock_container* c, uint s){
	c->sockets[c->count] = s;
	c->count++;
}

void remove_client(sock_container* c, uint s){
	uint i, loc = 0;
	for (i = 0; i < c->count; ++i){
		if (s == c->sockets[i]){
			loc = i;
			break;
		}
	}
	--c->count;
	for (i = loc; i < c->count; ++i){
		c->sockets[i] = c->sockets[i+1];
	}
}

uint build_fd_set(sock_container* c, uint server, fd_set* s){
	uint i, highest, sock;
	FD_ZERO(s);
	FD_SET(server, s);
	highest = server;
	for (i=0; i < c->count; ++i){
		sock = c->sockets[i];
		FD_SET(sock, s);
		if (sock > highest)
			highest = sock;
	}
	return highest;
}

uint init_socket(){
	uint s;
	struct sockaddr_un local;
	uint len;

	s= socket(AF_UNIX, SOCK_STREAM, 0);
	if (!s){
		perror("Failed to create socket");
		exit(-1);
	}

	local.sun_family = AF_UNIX;
	strcpy(local.sun_path, "/tmp/chat_socket");
	unlink(local.sun_path);
	len = strlen(local.sun_path) + sizeof(local.sun_family);

	if(bind(s, (struct sockaddr *)&local, len) == -1){
		perror("Failed to bind");
		exit(-2);
	}
	if(listen(s, 100) == -1){
		perror("Failed to listen");
		exit(-3);
	}

	fcntl(s, F_SETFL, O_NONBLOCK);
	return s;
}

int main(int argc, char** argv){
	uint server, client, i, j, len, highest;
	fd_set fds;
	sock_container clients = {};
	char buffer[1024];

	server = init_socket();

	while(1){
		//places clients and server in fds
		//this is required every loop because select modifys fds
		highest = build_fd_set(&clients, server, &fds); 

		select(highest + 1, &fds, NULL, NULL, NULL);

		//accept any new clients
		if (FD_ISSET(server, &fds)){
			client= accept(server, NULL, NULL);
			add_client(&clients, client);
		}

		//read data from clients
		for(i = 0; i<clients.count; ++i){
			uint socket = clients.sockets[i];
			if(FD_ISSET(socket, &fds)){
				bzero(buffer, sizeof(buffer));
				len = read(socket, &buffer, 1024);
				if (len>0){
					//write data to all clients
					for (j = 0; j<clients.count; ++j){
						write(clients.sockets[j], buffer, len);
					}
				}
				//client closed connection
				else{
					close(socket);
					remove_client(&clients, socket);
				}
			}
		}
	}
	return 0;
}
