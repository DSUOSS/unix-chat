#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ncurses.h>
#include <signal.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>

#define SOCKET_PATH "/tmp/chat_socket"
#define WELCOME_MSG "Welcome to UnixChat"
#define MSG_SIZE 1024

typedef unsigned int uint;

typedef struct _msg {
	char *txt;
	struct _msg *next;
} *msg;

WINDOW *chat;
WINDOW *input;
WINDOW *name;
int parent_x, parent_y;
msg lines;
uint cSocket;

// create socket and connect to file as specified in SOCKET_PATH
uint create_socket(){

	int soc, len;
	struct sockaddr_un remote;

	soc = socket(AF_UNIX, SOCK_STREAM, 0);
	if (soc == -1){
		perror("Failed to create socket");
		exit(-1);
	}

	remote.sun_family = AF_UNIX;
	strcpy(remote.sun_path, SOCKET_PATH);
	len = strlen(remote.sun_path) + sizeof(remote.sun_family);

	if (connect(soc, (struct sockaddr *)&remote, len) == -1){
		perror("Connect failed");
		exit(-2);
	}

	return soc;
}

// return length of linked list containing messages
int count_messages() {
	int count;
	msg ptr;

	count = 0;
	ptr = lines;
	do {
		count++;
		ptr = ptr->next;
	} while(ptr != NULL);

	return count;
}

void window_refresh_handler() {

	int i, start_point, c, str_len;
	char *str;
	msg tmp_ptr;

	getmaxyx(stdscr, parent_y, parent_x);

	mvwin(chat, 0, 0);
	wresize(chat, parent_y - 1, parent_x);

	mvwin(input, parent_y - 1, strlen(getlogin()) + 2);
	wresize(input, 1, parent_x - strlen(getlogin()) - 2);

	mvwin(name, parent_y - 1, 0);
	wresize(name, 1, strlen(getlogin()) + 2);

	wclear(chat);
	wclear(input);
	wclear(name);

	start_point = count_messages() - parent_y + 2;

	tmp_ptr = lines;
	for(i=0; i<start_point; i++) {
		tmp_ptr = tmp_ptr->next;
	}

	wattron(chat, A_BOLD);
	mvwprintw(chat, 0, 0, "%s", tmp_ptr->txt);
	tmp_ptr = tmp_ptr->next;
	wattroff(chat, A_BOLD);

	for(i=1; tmp_ptr != NULL; i++) {
		mvwprintw(chat, i, 0, "%s", tmp_ptr->txt);
		tmp_ptr = tmp_ptr->next;
	}

	mvwprintw(name, 0, 0, "%s: ", getlogin());

	wrefresh(chat);
	wrefresh(input);
	wrefresh(name);

}

void recieve_manager(){

	// currentStr is for keeping track of what part of the buffer has yet to be processed, temp is for holding the strings that will go into the message
	char *buffer, *currentStr, *temp; 
	msg new_msg, tmp_ptr;
	int len, buffer_sz;

	while(1){

		window_refresh_handler();
		buffer = NULL;	
		buffer_sz = 0;

		// read in characters 1024 at a time until socket has no more waiting bytes storing them in buffer
		// buffer_sz tracks the current size of the buffer, which will grow to accommodate each read
		do
		{
			buffer_sz += MSG_SIZE;
			buffer = realloc(buffer, buffer_sz*sizeof(char));
		    if(!buffer) {
				perror("Failed to allocate memory");
				exit(-1);
			}	
			memset(buffer+(buffer_sz-MSG_SIZE), 0, MSG_SIZE); // set new area to be all 0 bytes (important for later determining where strings are)

			len = read(cSocket, buffer+(buffer_sz - MSG_SIZE), MSG_SIZE); // read should start in the newly created space, which will start at the current size of the buffer minus MSG_SIZE
		}while(len == MSG_SIZE);	

		currentStr = buffer;	// starting with the first substring of the buffer
		do
		{
			// allocating space for the message string
			len = strlen(currentStr);
			temp = calloc(len+1, sizeof(char));

			strcpy(temp, currentStr);

			new_msg = malloc(sizeof(struct _msg));
			if(!new_msg) {
				perror("Failed to allocate memory");
				exit(-1);
			}
			new_msg->txt = temp;
			new_msg->next = NULL;

			// attaching the new message to the end of the linked list
			tmp_ptr = lines;
			while(tmp_ptr->next != NULL)
				tmp_ptr = tmp_ptr->next;
			tmp_ptr->next = new_msg;

			// move to the next substring
			currentStr += (len+1);
		}while(currentStr[0] != 0 && currentStr <= (buffer+buffer_sz));	// loop should end when we either run out of valid substrings, or when we go beyond the end of the buffer

		free(buffer);
	}
}

void send_manager(){
	char in[MSG_SIZE];
	char out[MSG_SIZE];

	while(1){
		wgetnstr(input, in, MSG_SIZE);
		snprintf(out, MSG_SIZE, "[%s]: %s", getlogin(), in);
		write(cSocket, out, strlen(out));
	}
}

int main(int argc, char **argv) {

	pthread_t recieveT, sendT;
	int len;
	cSocket = create_socket();

	lines = malloc(sizeof(struct _msg));
	if(!lines) {
		perror("Failed to allocate memory");
		exit(-1);
	}
	lines->txt = strdup(WELCOME_MSG);
	lines->next = NULL;

	signal(SIGWINCH, window_refresh_handler);

	initscr();
	getmaxyx(stdscr, parent_y, parent_x);

	chat = newwin(parent_y - 1, parent_x, 0, 0);
	input = newwin(1, parent_x - strlen(getlogin()) - 2, parent_y - 1, strlen(getlogin()) + 2);
	name = newwin(1, strlen(getlogin()) + 2, parent_y - 1, 0);

	wattron(name, A_BOLD);

	scrollok(chat, 1);

	window_refresh_handler();

	pthread_create(&recieveT, NULL, (void *) &recieve_manager, NULL);
	pthread_create(&sendT, NULL, (void *) &send_manager, NULL);

	pthread_join(recieveT, NULL);
	pthread_join(sendT, NULL);

	return 0;

}
