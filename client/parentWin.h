#ifndef PARENTWIN_PARENTWIN_H
#define PARENTWIN_PARENTWIN_H

#include <iostream>
#include <ncurses.h>

class parentWin{

public:
    int x;
    int y;

    parentWin();
    ~parentWin();

	void printInstructions();
};


#endif // PARENTWIN_PARENTWIN_H
