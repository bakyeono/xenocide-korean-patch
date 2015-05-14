#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <string>

using namespace std;

class KEYBOARD {
public:
  int sw;
  int s;
  int se;
  int w;
  int e;
  int nw;
  int n;
  int ne;
  int quit;
  int wait;
  int get;
  int escape;
  int backspace;
  int readmore;
  int readmore2;
  int inventory;
  int fire;
  int help;
  int messagebuffer;
  int inc_fire;
  int dec_fire;
  int options;
  int reload;
  int nearest;
  int char_info;
  int look;
  int open;
  int close;
  int _throw;
  int save;
  int down;
  int up;
  int rest;
  int attack;
  int exchange;
  int activate_armor;
  int activate_weapon;
  int explore;
  int travel;
  int show_visible;
  
  KEYBOARD();
  string get_name(int posx, int posy, int size);
  int get_value(int posx, int posy, int size);
  int getkey();
  int wait_for_key();
  int ungetkey(int character);
  int get_direction(int &dx, int &dy);
  bool yes_no();
};

#endif


