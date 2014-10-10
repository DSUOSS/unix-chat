#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ncurses.h>
#include <signal.h>

WINDOW *chat;
WINDOW *input;
int parent_x, parent_y;

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

int main(int argc, char *argv[]) {

	char *lines[100];

	signal(SIGWINCH, window_refresh_handler);

	initscr();
	getmaxyx(stdscr, parent_y, parent_x);

	chat = newwin(parent_y - 1, parent_x, 0, 0);
	input = newwin(1, parent_x, parent_y - 1, 0);

	window_refresh_handler();

	int i = 0;
	for(;;i++) {

		window_refresh_handler();

		lines[i] = malloc(100);
		wgetnstr(input, lines[i], 100);

		mvwprintw(chat, i, 0, "<%s> %s", getlogin(), lines[i]);

	}

	return 0;

}