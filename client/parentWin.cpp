#include "parentWin.h"


parentWin::parentWin(){
    initscr();
    raw();
    getmaxyx(stdscr, y, x);
}

parentWin::~parentWin(){
	endwin();
}

void parentWin::printInstructions(){
	int yMax, xMax;
	getmaxyx(stdscr, yMax, xMax);
	move(yMax, 0);
	printw("/Ssadf dskjfhd slkf");
}