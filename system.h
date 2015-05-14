#ifndef SYSTEM_H
#define SYSTEM_H


#ifdef XCURSES
    #define KEYBOARD_NONE -1
#else
    #define KEYBOARD_NONE 0
#endif

#pragma warning (disable: 4786)
#pragma warning (disable: 4788)
// disable warning (compare bool with int)
#pragma warning (disable: 4805)

// Prototypy funkcji
#include <time.h>
#include <string>

using namespace std;

//// COPYOF EKRANU
struct Screen_copy {
 char copy[80][50];
 char color[80][50];
 bool operator != (Screen_copy &right)
 {
	 if (memcmp(this->copy,&right.copy,80*50)!=0 || memcmp(this->color,&right.color,80*50)!=0)
		return true;
	 return false;
 }
};


void init_system();
void create_directory(string name);
int init_graph(void);
void set_color(int front);
void print_text(int x,int y,string text);
void print_number(int x,int y,long number);
void print_number_right_align(int x,int y,long number,int places);
void print_character(int x,int y,unsigned char character);
void end_graph(void);
void myrefresh(void);
void myrefresh(void);
void myrefresh(void);
void myclear(void);
int getkeyvalue(void);
void hide_cursor(void);
void show_cursor(void);
void set_cursor(int x,int y);

char get_char_from_screen(int x,int y);
char get_color_from_screen(int x,int y);
void restore_screen(struct Screen_copy *kopia);
void store_screen(struct Screen_copy *s_copy);
char get_char_from_stored_screen(int x,int y,struct Screen_copy *s_copy);
char get_color_from_stored_screen(int x,int y,struct Screen_copy *s_copy);
unsigned long get_ticks_count();
void delay(long how_long);

void delete_file(string name);

#endif

