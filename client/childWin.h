#ifndef CHILDWIN_CHILDWIN_H
#define CHILDWIN_CHILDWIN_H

#include <iostream>
#include <cmath>
#include <ncurses.h>
#include "parentWin.h"


class childWin{
public:
    int xParent;
    int yParent;
    int xMax;
    int yMax;
    int x;
    int y;

    WINDOW *localWin;

    childWin(parentWin parent, double yRelative, double xRelative, int yOrigin, int xOrigin, bool scrolling);
    ~childWin();
    void print(std::string txt, bool nextLine);
    void moveCursor(int y, int x);
    void clearWin();
    void refreshWin();
    void clearLine();
    void drawBorders();

    void getStr(std::string& s);
    void setTitle(const std::string title);
    void boxOn();
    void instructions();
};


#endif // CHILDWIN_CHILDWIN_H
