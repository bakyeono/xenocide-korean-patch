#include "mem_check.h"

// Ten file zawiera procedury zalezne od systemu operacyjnego
// oraz COMPILERa

#define CPLUSPLUS

#include <string>
#include <ctype.h>
#include "system.h"
#include <time.h>
#include <sys/timeb.h>
#include "options.h"
#include "mt19937int.h"


#ifdef __GNUC__
#define COMPILER_GCC
#endif

#ifdef _MSC_VER
#define COMPILER_MSVC
#endif

#ifdef __MINGW32_VERSION
#define COMPILER_MINGW
#endif

#ifdef COMPILER_MSVC
#include <direct.h>
#endif

#ifdef COMPILER_GCC
#include <unistd.h>
#include <sys/stat.h>
#endif

#if defined COMPILER_MSVC || defined COMPILER_MINGW
#define COMPILER_WINDOWS 
#endif

#ifdef COMPILER_WINDOWS
#include <windows.h>
#undef MOUSE_MOVED
#endif

#include <curses.h>

extern void make_screenshot(string filename,bool put_date);

extern class OPTIONS options;
extern string IntToStr(long number);
extern unsigned int random(unsigned int range);
int OLD_LINES, OLD_COLS;
char player_name_8[9];

// Inicjalizuje funkcje grafiki

int color_copy;
struct Screen_copy screen_copy;
struct Screen_copy last_refresh;

bool global_dont_wait_for_key;

void init_system()
{
  // init rand()
  strcpy(player_name_8,"Player");
  delay(1); // inicjalizacja zegara
  init_genrand(get_ticks_count());
}

void create_directory(string name)
{
#ifdef COMPILER_GCC
       #ifdef __MINGW32_VERSION
       mkdir(name.c_str());
       #else
       mkdir(name.c_str(), S_IWUSR);
       #endif
#endif
#ifdef COMPILER_MSVC
	_mkdir(name.c_str());
#endif
}

void delete_file(string name)
{
#ifdef COMPILER_GCC
    unlink(name.c_str());
#endif
#ifdef COMPILER_MSVC
	_unlink(name.c_str());
#endif
}

unsigned long get_ticks_count() // proba zwrocenia w milisekundach
{
  struct timeb t;
  ftime(&t);
  return (t.time*1000 + t.millitm);
//  return  uclock()/(UCLOCKS_PER_SEC/1000);
}

void delay(long how_long)
{
   unsigned long start, end;
   start=get_ticks_count();
   
   while (1)
   {
     end=get_ticks_count();
     if (start+how_long<=end)
       break;
   }
}

void store_screen(Screen_copy *s_copy)
{
   *s_copy=screen_copy;
}


char get_char_from_screen(int x,int y)
{
   if (x<0 || x>80 || y<0 || y>50)
     return 0;
   return screen_copy.copy[x][y];
}

char get_color_from_screen(int x,int y)
{
   if (x<0 || x>80 || y<0 || y>50)
     return 0;
   return screen_copy.color[x][y];
}

char get_char_from_stored_screen(int x,int y, Screen_copy *s_copy)
{
   if (x<0 || x>80 || y<0 || y>50)
     return 0;
   return s_copy->copy[x][y];
}

char get_color_from_stored_screen(int x,int y, Screen_copy *s_copy)
{
   if (x<0 || x>80 || y<0 || y>50)
     return 0;
   return s_copy->color[x][y];
}



int set_80_25(void)
{
#ifdef __PDCURSES__
  resize_term(OLD_LINES,OLD_COLS);
#else
  resizeterm(OLD_LINES,OLD_COLS);
#endif
  return 0;
}
                  
int set_80_50(void)
{
  OLD_LINES=LINES;
  OLD_COLS=COLS;
#ifdef __PDCURSES__
  resize_term(50,80);
#else
  resizeterm(50,80);
#endif
  
  if (LINES<50 || COLS<80)
  {
	  print_text(1,1,"Warning! Xenocide requires screen at least 80 columns x 50 lines.");
	  string size = string("Your screen size is ") + IntToStr(COLS) + " columns x " + IntToStr(LINES) + " lines.";
	  print_text(1,2,size);
	  print_text(1,3,"The game wasn't able to change this size to proper one.");
	  print_text(1,4,"The screen will be probably screwed... :/");
	  print_text(1,6,"Do you want to continue game with such screen? (y/n)");
	  refresh();
	  while(1)
	  {
		  int key=getkeyvalue();
		  if (key=='y' || key=='Y')
			  break;
		  if (key=='n' || key=='N')
		  {
			  end_graph();
			  exit(0);
		  }
	  }
  }
  
  return 0;
}

void turn_on_wait_for_key()
{
	nodelay( stdscr, FALSE ); // WLACZENIE CZEKANIA W GETCH();
	global_dont_wait_for_key = false;
}

void turn_off_wait_for_key()
{
	nodelay( stdscr, TRUE ); // WYLACZENIE CZEKANIA W GETCH();
	global_dont_wait_for_key = true;
}


int init_graph(void)
{
  int front;
  initscr();
  turn_on_wait_for_key();
  
  keypad( stdscr, TRUE ); // Dodanie specjalnych klawiszy
  cbreak();
  noecho();

  if (has_colors())
   start_color();
  for (front=0;front<48;front++)
  {
  switch(front)
  {
          case 0:
          case 8:
          init_pair(front,COLOR_BLACK,COLOR_BLACK);
          break;
          case 1:
          case 9:
          init_pair(front,COLOR_BLUE,COLOR_BLACK);
          break;
          case 2:
          case 10:
          init_pair(front,COLOR_GREEN,COLOR_BLACK);
          break;
          case 3:
          case 11:
          init_pair(front,COLOR_CYAN,COLOR_BLACK);
          break;
          case 4:
          case 12:
          init_pair(front,COLOR_RED,COLOR_BLACK);
          break;
          case 5:
          case 13:
          init_pair(front,COLOR_MAGENTA,COLOR_BLACK);
          break;
          case 6:
          case 14:
          init_pair(front,COLOR_YELLOW,COLOR_BLACK);
          break;
          case 7:
          case 15:
          init_pair(front,COLOR_WHITE,COLOR_BLACK);
          break;
          
          case 16:
          case 24:
          init_pair(front,COLOR_BLACK,COLOR_RED);
          break;
          case 17:
          case 25:
          init_pair(front,COLOR_BLUE,COLOR_RED);
          break;
          case 18:
          case 26:
          init_pair(front,COLOR_GREEN,COLOR_RED);
          break;
          case 19:
          case 27:
          init_pair(front,COLOR_CYAN,COLOR_RED);
          break;
          case 20:
          case 28:
          init_pair(front,COLOR_RED,COLOR_RED);
          break;
          case 21:
          case 29:
          init_pair(front,COLOR_MAGENTA,COLOR_RED);
          break;
          case 22:
          case 30:
          init_pair(front,COLOR_YELLOW,COLOR_RED);
          break;
          case 23:
          case 31:
          init_pair(front,COLOR_WHITE,COLOR_RED);
          break;			  

          case 32:
          case 40:
			  init_pair(front,COLOR_BLACK,COLOR_BLUE);
			  break;
          case 33:
          case 41:
			  init_pair(front,COLOR_BLUE,COLOR_BLUE);
			  break;
          case 34:
          case 42:
			  init_pair(front,COLOR_GREEN,COLOR_BLUE);
			  break;
          case 35:
          case 43:
			  init_pair(front,COLOR_CYAN,COLOR_BLUE);
			  break;
          case 36:
          case 44:
			  init_pair(front,COLOR_BLUE,COLOR_BLUE);
			  break;
          case 37:
          case 45:
			  init_pair(front,COLOR_MAGENTA,COLOR_BLUE);
			  break;
          case 38:
          case 46:
			  init_pair(front,COLOR_YELLOW,COLOR_BLUE);
			  break;
          case 39:
          case 47:
			  init_pair(front,COLOR_WHITE,COLOR_BLUE);
			  break;			  
			  
    }
  }
  set_80_50();
  hide_cursor();

  // hide mouse cursor on full screen
#ifdef COMPILER_WINDOWS 
  SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), 0);
#endif
  return 0;
}

// Ustawia atrybuty tekstu, ktorym bedzie wszystko wypisywac

void set_color(int front)
{
       if (front<8)
        attrset(COLOR_PAIR(front));
       else
        attrset(COLOR_PAIR(front) | A_BOLD);
       color_copy=front;
}

void print_text_to_buffer(int x, int y, char *ptr)
{
  int index;
  char *tmp;
  tmp=ptr;
  for (index=x;;index++)
  {
    if (*tmp=='\0')
     break;
    if (index>=0 && index<80 && y>=0 && y<50)
    {
      screen_copy.copy [index][y] = *tmp;
      screen_copy.color[index][y] = color_copy;
    }
    tmp++;
  }
}


// Wypisuje w x,y text

void print_text(int x,int y,string text)
{
  string t2;
  for (size_t a=0;a<text.size();a++)
  {
	  if (text[a]=='%')
		  t2+="%%";
	  else
		  t2+=text[a];
  }
  if (x>=0 && y>=0)
  {
    mvprintw(y,x,(char *) t2.c_str());
    print_text_to_buffer(x,y,(char *)text.c_str());
  }
}

// Wypisuje w x,y liczbe

void print_number(int x,int y,long number)
{
  char tmp[50];
  if (x>=0 && y>=0)
  {
    mvprintw(y,x,"%ld",number);
    sprintf(tmp,"%ld",number);
    print_text_to_buffer(x,y,tmp);
  }
}

// Wypisuje liczbe przesunieta w to_right

void print_number_right_align(int x,int y,long number,int places)
{
  char tmp[50];
  int dec, sign;
  int ndig;
  if (number!=0)
  {
	  ndig=10;
	  ecvt(number, ndig, &dec, &sign);
  }
  else
	  dec = 1;

  x=x+places-dec;
  if (x>=0 && y>=0)
  {
    mvprintw(y,x,"%ld",number);
    sprintf(tmp,"%ld",number);
    print_text_to_buffer(x,y,tmp);
  }
}


// Wypisuje w x,y character

void print_character(int x,int y,unsigned char character)
{
  char tmp[50];
  if (x>=0 && y>=0)
  {
    mvprintw(y,x,"%c",character);
    sprintf(tmp,"%c",character);
    print_text_to_buffer(x,y,tmp);
  }
}

// Zamyka funkcje grafiki

void end_graph(void)
{
  set_80_25();
  show_cursor();
  endwin();
}

// Kopiuje bufor grafiki na ekran

void myrefresh(void)
{
  refresh();
  last_refresh=screen_copy;
}

void clear_copy(void)
{
  for (int x=0;x<80;x++)
   for (int y=0;y<50;y++)
   {
     screen_copy.copy [x][y]=' ';
     screen_copy.color[x][y]=7;
   }
}

void myclear(void)
{
  clear();
  clear_copy();
}

// Pobiera character z stdin bez czekania. Zwraca character pobrany.

int getkeyvalue(void)
{
 int character;
 character=getch();
 Screen_copy scr_copy;
 if (character==KEY_F(11))
 {
	 make_screenshot(player_name_8,true);
	 store_screen(&scr_copy);
	 set_color(15);
	 print_text(0,0,string("[Screenshot saved to ") + player_name_8 + ".htm]");
	 myrefresh();
	 delay(500);
	 restore_screen(&scr_copy);
	 myrefresh();
 }
 return character;
}

void set_cursor(int x,int y)
{
	mvcur(0,0,y,x);
}

void hide_cursor(void)
{
	curs_set(0);
	leaveok(stdscr, true);	
}

void show_cursor(void)
{
 curs_set(1);
 leaveok(stdscr, false);
}


void restore_screen(Screen_copy *kopia)
{
  int character, color;
  for (int x=0;x<80;x++)
   for (int y=0;y<50;y++)
   {
     character=kopia->copy [x][y];
     color=kopia->color[x][y];
     
     set_color(color);
     print_character(x,y,character);
   }
}


