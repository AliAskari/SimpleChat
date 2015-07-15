#include "childWin.h"


childWin::childWin(parentWin parent, double yRelative, double xRelative, int yOrigin, int xOrigin, bool scrolling){
    yParent = parent.y;
    xParent = parent.x;
    localWin = newwin(yRelative, xRelative, yOrigin, xOrigin);

    getmaxyx(localWin, yMax, xMax);

    // set window settings
    keypad(localWin, TRUE);
    if(scrolling == TRUE){
        scrollok(localWin, TRUE);
        idlok(localWin, TRUE); 
    }
}

childWin::~childWin(){
    //
    delwin(localWin);
}

void childWin::print(std::string txt, bool nextLine){
    // clearLine();
    wprintw(localWin, txt.c_str());
    wrefresh(localWin);

    if(nextLine == TRUE){
        ++y;
        if(y == yMax){
            scroll(localWin);
            y = y-1;            
        }     
        moveCursor(y, 0);
    }
}

void childWin::clearWin(){
	wclear(localWin);
    wrefresh(localWin);
}

void childWin::clearLine(){
    moveCursor(y, 0);
	wclrtoeol(localWin);
    wrefresh(localWin);
    moveCursor(y, 0);
}

void childWin::moveCursor(int y, int x){
    wmove(localWin, y, x);
    wrefresh(localWin);
    getyx(localWin, this->y, this->x);    
}

void childWin::refreshWin(){
    //
	wrefresh(localWin);
}

void childWin::drawBorders(){
  int x, y, i;
  getmaxyx(localWin, y, x);
  // 4 corners
  mvwprintw(localWin, 0, 0, "+");
  mvwprintw(localWin, y - 1, 0, "+");
  mvwprintw(localWin, 0, x - 1, "+");
  mvwprintw(localWin, y - 1, x - 1, "+");
  // sides
  for (i = 1; i < (y - 1); i++) {
    mvwprintw(localWin, i, 0, "|");
    mvwprintw(localWin, i, x - 1, "|");
  }
  // top and bottom
  for (i = 1; i < (x - 1); i++) {
    mvwprintw(localWin, 0, i, "-");
    mvwprintw(localWin, y - 1, i, "-");
  }
  wrefresh(localWin); 
}

void childWin::getStr(std::string& s){
    char aCString[1024];
    wgetstr(localWin, aCString);
    s = aCString;
}

void childWin::setTitle(const std::string title){
    moveCursor(0, 2);
    wattron(localWin, A_REVERSE);
    wprintw(localWin, title.c_str());
    wattroff(localWin, A_REVERSE);
    wrefresh(localWin);
}

void childWin::boxOn(){
    box(localWin, 0, 0);
    wrefresh(localWin);
}

void childWin::instructions(){
    moveCursor(0, 0);
    wattron(localWin, A_REVERSE);
    wprintw(localWin, "/X");
    wattroff(localWin, A_REVERSE);
    wprintw(localWin, " Exit ");
    wrefresh(localWin);

    wattron(localWin, A_REVERSE);
    wprintw(localWin, "/S");
    wattroff(localWin, A_REVERSE);
    wprintw(localWin, " Select A User ");
    wrefresh(localWin);
}