#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ncurses.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>

#define SOCKET_PATH "/tmp/chat_socket"

typedef unsigned int uint;

WINDOW *chat;
WINDOW *input;
int parent_x, parent_y;

uint cSocket;

uint create_socket(){
	int s, len;
	struct sockaddr_un remote;

	s = socket(AF_UNIX, SOCK_STREAM, 0);
	if (s == -1){
		perror("Failed to create socket");
		exit(-1);
	}

	remote.sun_family = AF_UNIX;
	strcpy(remote.sun_path, SOCKET_PATH);
	len = strlen(remote.sun_path) + sizeof(remote.sun_family);

	if (connect(s, (struct sockaddr *)&remote, len) == -1){
		perror("Connect failed");
		exit(-2);
	}

	return s;
}

void window_refresh_handler() {

	getmaxyx(stdscr, parent_y, parent_x);

	mvwin(chat, 0, 0);
	wresize(chat, parent_y - 1, parent_x);
	mvwin(input, parent_y - 1, 0);
	wresize(input, 1, parent_x);

	wclear(input);
	mvwprintw(input, 0, 0, "[%s]: ", getlogin());

	wrefresh(chat);
	wrefresh(input);

}

void recieve_manager(){
	char *buffer;
	char *lines[100];
	int i, lineCount, len;

	for (;;){
		buffer = malloc(100);
		len = read(cSocket, buffer, 100);
		if (!len)
			break;
		buffer[len] = '\0';

		lines[lineCount] = buffer;
		lineCount++;

		for(i=0;i<lineCount;i++) {
			mvwprintw(chat, i, 0, "%s", lines[i]);
		}
		window_refresh_handler();
	}
}

void send_manager(){
	char line[100];
	for(;;){
		wgetnstr(input, line, 100);
		write(cSocket, line, strlen(line));
	}
}


int main(int argc, char *argv[]) {
	pthread_t recieveT, sendT;
	cSocket = create_socket();

	signal(SIGWINCH, window_refresh_handler);

	initscr();
	getmaxyx(stdscr, parent_y, parent_x);

	chat = newwin(parent_y - 1, parent_x, 0, 0);
	input = newwin(1, parent_x, parent_y - 1, 0);

	window_refresh_handler();

	pthread_create(&recieveT, NULL, (void *) &recieve_manager, NULL);
	pthread_create(&sendT, NULL, (void *) &send_manager, NULL);

	pthread_join(recieveT, NULL);
	pthread_join(sendT, NULL);

	return 0;

}
