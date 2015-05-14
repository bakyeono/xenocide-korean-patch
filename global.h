#ifndef GLOBAL_H
#define GLOBAL_H


#pragma warning (disable: 4786)
#pragma warning (disable: 4788)
// disable warning (compare bool with int)
#pragma warning (disable: 4805)
#pragma warning (disable: 4503)

#include "fov.h"
#include "map.h"
#include "monster.h"
#include <string>
#include <map>
#include "attrib.h"

unsigned int random(unsigned int range);
bool coin_toss();
string IntToStr(long number);
bool lower_random(int lower_value, int range);
int generate_bresenham_line(int x1, int y1, int x2, int y2, struct POSITION *position, int max_range);
double RadToDeg(double angle);
double DegToRad(double angle);
int round_up(double variable);

#ifndef max
int max(int value1, int value2);
#endif

#ifndef min
int min(int value1, int value2);
#endif

int distance(const int& x1,const int& y1,const int& x2,const int& y2);

class MYSCREEN {
private:
       void draw_items();
       void draw_smoke();
public:
       void draw_monsters(); // to publiczne, aby monsters zobaczyly gracza po jego of movement (potrzebne?)
       void draw_all();
       void init();
       void relase();
       void clear_all();
       void clear_no_console();
       void draw();
       void draw2();
       void draw_targeting_line(int x1,int y1,int x2, int y2);
       int draw_box(int color,int left,int top, int right,int down);
       bool draw_explosion(int *data, int max);
       bool draw_stunning_explosion(int *data);
	   bool draw_emp_explosion(int *data, int max);


       class CONSOLE {
	   friend class TOSAVE;
       private:
              string buffer;
			  string last_text;
              int posx;
              int posy;
			  bool dont_show_more;
			  list < string > message_buffer;
       public:
			  CONSOLE() { dont_show_more=false; };
              void add(string text, int color, bool stop_repeating=true);
              void add_and_zero(string text, int color);
			  void show_message_buffer(bool show_only_last);
              
              void zero();
              void clean();
              void show();  // pokazanie jej z efektem
       };

       CONSOLE console;

};

// mapa opisow

class DESCRIPTIONS {
private:
	map < string, string> descript_texts;
	map < string, string> attacks;
public:
	string get_description(string to_find);
	bool add_description(string object,string to_add);
	bool show_description(string to_find);

	string get_attack(string to_find);
	bool add_attack(string object,string to_add);	
	bool zero_attack(string object);	
};


template <class _T1>
void shuffle_list(list < _T1 > &l)
{
	typename list < _T1 >::iterator m,_m;
	m=l.begin();
	
	for (size_t a=0;a<l.size();a++,m++)
	{
		_T1 t;
		_m=l.begin();
		
		int random_value = random(l.size());
		for (int b=0;b<random_value;b++,_m++);
		t = *m;
		*m = *_m;
		*_m = t;
	}	
}

#endif


