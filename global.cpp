//#define DRAW_ALL_MONSTERS
//#define DRAW_BRESENHAM

#include "mem_check.h"

#include "global.h"
#include "fov.h"
#include "map.h"
#include "monster.h"
#include "keyboard.h"
#include "options.h"
#include "parser.h"
#include <string>
#include <math.h>
#include "level.h"
#include "mt19937int.h"
#include "places.h"
#include "tutorial.h"
#include "system.h"
#include "sounds.h"

#include <algorithm>
#include <cctype>
#include <vector>

#include "directions.h"

#ifndef M_PI
#define M_PI 3.1415926536
#endif

extern OPTIONS options;

// global screen
extern struct Screen_copy screen_copy; // z system.cpp
extern unsigned long genrand();

extern LEVEL level;
MYSCREEN screen;
DEFINITIONS definitions;
DESCRIPTIONS descriptions;
PLACES places;
KEYBOARD keyboard;
TUTORIAL tutorial;
SOUNDS sounds;
vector <int> square_root;

extern KEYBOARD keyboard;

void init_square_root(int size)
{
	size_t old_size = square_root.size();
	if (size<=0 || static_cast <size_t>(size)<old_size)
		return;
	square_root.resize(size+1);
	// count square root
	for (size_t a=old_size;a<static_cast <size_t>(size+1);++a)
		square_root[a]=(int )ceil(sqrt((double) a));
}

int distance(const int& x1,const int& y1,const int& x2,const int& y2)
{

	const int diff_x = x2-x1;
	const int diff_y = y2-y1;
	const int dist = diff_x*diff_x + diff_y*diff_y;

	if (dist>=static_cast <int> (square_root.size()))
		init_square_root(dist);
	if (square_root.size()==0)
		init_square_root(1);

	return square_root[dist];
}


void MYSCREEN :: init()
{
 init_graph();
 clear_all();
}

void MYSCREEN :: relase()
{
 end_graph();
}

void MYSCREEN :: draw_smoke()
{
   ptr_list::iterator m,_m;
   GAS *temp;
   for(m=level.gases_on_map.begin(),_m=level.gases_on_map.end(); m!=_m; m++)
   {
       temp=(GAS *)*m;
       if (level.map.seen_by_player(temp->pX(),temp->pY()))
       {
         temp->display();
       }
   }
}

void MYSCREEN :: draw_items()
{
   ptr_list::iterator m,_m;
   ITEM *temp;
   for(m=level.items_on_map.begin(),_m=level.items_on_map.end(); m!=_m; m++)
   {
       temp=(ITEM *)*m;
       if  ( level.map.seen_by_player(temp->pX(),temp->pY()) && !level.map.isShield(temp->pX(),temp->pY()))
       {
		 if (temp->IsRobotShell() && temp->owner==NULL) // if shell doesn't have owner, make it black
		 {
			 int color = temp->color;
			 temp->color = 8;
			 temp->display();
			 temp->color = color;
			 level.map.setLastTile(temp->pX(),temp->pY(),temp->tile);
		 }
		 else if (!temp->property_controller() && !temp->invisible) // if not controller and not invisible
		 {
			temp->display();
			level.map.setLastTile(temp->pX(),temp->pY(),temp->tile);
		 }
       }
   }
}


void MYSCREEN :: draw_monsters()
{
   ptr_list::iterator m,_m;
   MONSTER *temp;

   for(m=level.monsters.begin(),_m=level.monsters.end(); m!=_m; m++)
   {
       temp=(MONSTER *)*m;

#ifdef DRAW_ALL_MONSTERS
       temp->display();
#endif
       
       if (level.map.seen_by_player(temp->pX(),temp->pY()))
       {
		   if (temp==level.player->enemy) // if this is the player's target
		   {
			   temp->color+=16;
			   temp->display(); // draw monster
			   temp->color-=16;
		   }
		   else if (level.player->is_friendly(temp) && temp!=level.player)
		   {
			   temp->color+=32;
			   temp->display(); // draw monster
			   temp->color-=32;
		   }
		   else
			   temp->display(); // draw monster
		   
	   }
   }
}

void MYSCREEN :: draw_all()
{
  screen.clear_no_console();
  level.map.display();
  screen.draw_items();
  screen.draw_smoke();
  screen.draw_monsters();
}

// draw screen without FXs
void MYSCREEN :: draw()
{
	draw_all();
	myrefresh();
}

// draw screen with blue FX

void MYSCREEN :: draw2()
{
  draw_all();
  myrefresh();
}

void MYSCREEN :: clear_all()
{
  myclear();
}

void MYSCREEN :: clear_no_console() // clear bottom of the screen
{
   set_color(0);
   char *pusty="                                                                                ";
   print_text(0,40,pusty);
   print_text(0,46,pusty);
   print_text(0,47,pusty);
   print_text(0,48,pusty);
   print_text(0,49,pusty);
}


void MYSCREEN :: CONSOLE :: add_and_zero(string text, int color)
{
   clean();
   add(text,color);
   show();
   zero();
}

void MYSCREEN :: CONSOLE :: add(string text, int color, bool stop_repeating)
{
	if (text.size()==0)
		return;
	// don't add messages after player's death, but show the one of death
	if (dont_show_more)
		return;

	if (stop_repeating && level.player!=NULL)
		level.player->stop_repeating();

	if (level.player!=NULL && level.player->is_dead)
	{
		dont_show_more = true;
	}
	// if the same as the last one, and not damage
	if (buffer.size()>3 && text==last_text && text.find("damage")==-1)
		return;
	else
		last_text=text;
	
   string male=string("abcdefghijklmnopqrstuvwxyz");
   string::iterator it;
   // change first letter to big one.
   it = text.begin();
   it++; // druga litera

   if (text.size()>1)
     if (male.find(*text.c_str())!=-1) // start with small
       transform(text.begin(), it, text.begin(), toupper);
   
   char col[4];
   col[0]=' ';
   col[1]='@';
   col[2]=(char) color;
   col[3]='\0';
   buffer+=string(col);
   buffer+=text;

   // add to buffer
   string to_add;
   col[0]=(char) color;
   col[1]='\0';
   
   for (int a=0;a<text.size();a++)
   {	   
		if (text[a]=='\n' || to_add.size()>=79)
		{
		   message_buffer.push_back(string(col) + to_add);
		   to_add="";
		}
		else
		{
	      to_add+=text[a];
		}
   }
   if (to_add.size()>0)
   {
	   message_buffer.push_back(string(col) + to_add);
   }
   
   while(message_buffer.size()>1000)
	   message_buffer.pop_front();
}

void MYSCREEN :: CONSOLE :: show_message_buffer(bool show_only_last)
{
		  last_text="";
		  list < string >::iterator m,_m;
		  bool first, last;
		  int key;
		  string do_wypisania;
		  
		  _m = message_buffer.end();
		  while (1)
		  {
			  screen.clear_all();		  
			  set_color(2);
			  print_text(0,0,  "--------------------------------------------------------------------------------");
			  print_text(0,49, "--------------------------------------------------------------------------------");
			  set_color(10);
			  
			  m=_m;
			  int count=0;
			  
			  while (m!=message_buffer.begin() && count<48)
			  {
				  count++;
				  m--;
			  }
			  if (m==message_buffer.begin())
			  {
				  first=true;
				  print_text(32,0," MESSAGE BUFFER ");
			  }
			  else
			  {
				  first=false;
				  print_text(36,0," ^ ^ ^ ^ ");
			  }
			  
			  count=0;
			  while(m!=message_buffer.end() && count<48)
			  {
				  do_wypisania = *m;
				  set_color(do_wypisania[0]);
				  do_wypisania=do_wypisania.substr(1,do_wypisania.size()-1);			  
				  print_text(0,count+1,do_wypisania);
				  
				  m++;
				  count++;
			  }
			  set_color(10);
			  
			  if (m==message_buffer.end())
			  {
				  last=true;
				  print_text(33,49," END OF BUFFER ");
			  }
			  else
			  {
				  last=false;
				  print_text(36,49," v v v v ");
			  }

			  if (show_only_last)
			  {
				  myrefresh();
				  return;
			  }
			  else
				  myrefresh();
			  
			  key = keyboard.getkey();
			  if (key == keyboard.n && first!=true)
			  {
				  _m--;
			  }
			  else if (key == keyboard.s && last!=true)
			  {
				  _m++;
			  }
			  else if (key == keyboard.escape)
			  {
				  break;
			  }
		  }
		  
		  screen.clear_all();
		  screen.draw2();		  
}

void MYSCREEN :: CONSOLE :: clean()
{
   char *pusty="                                                                                ";
   set_color(7);
   print_text(0,41,pusty);
   print_text(0,42,pusty);
   print_text(0,43,pusty);
   print_text(0,44,pusty);
   print_text(0,45,pusty);
}

void MYSCREEN :: CONSOLE :: zero()
{
   buffer="";
}


void MYSCREEN :: CONSOLE :: show()
{
   char next_color;
   string word;
   string tmp;
   int ptr, _ptr;
   string::iterator it;
   string end=" \n\r\t@";
   int key;
   int limit;
   int color_before_MORE;

   posx=1;
   posy=41;
   tmp=buffer;
   next_color=true;

   while(tmp.length()>0)
   {
      ptr=0;
      if (posy<45)
          limit=80;
      else
          limit=74;
      // cut the word
      if (next_color==false)
      {
         _ptr=tmp.find_first_of(end);
         if (_ptr!=-1)
           word=tmp.substr(ptr,_ptr+1);
         else
           word=tmp;
      }
      else
      {
		  color_before_MORE=*tmp.c_str();
		  if (color_before_MORE>=32)
			  color_before_MORE = 0;
		  set_color(color_before_MORE);
         tmp=tmp.substr(1,(unsigned int) -1);
         next_color=false;
         continue;
      }
      if (word=="@")
        next_color=true;
      else
      {
        if (posy>45)
          posx=100;    // pokazanie more

        if (posx+word.length()<limit)
            print_text(posx,posy,word);
        else
        {
          if (posy<45)
               posy++;
          else
          {
              set_color(14);
              print_text(72,45,"(MORE)");

              myrefresh();
              while (1)
              {
               key=keyboard.wait_for_key();
               if (key==keyboard.readmore || key==keyboard.readmore2)
                  break;
              }
              clean();
              set_color(color_before_MORE);
              posy=41;
          }
          posx=0;
          print_text(posx,posy,word);
        }
      it=word.end();
        it--;
        if (*it=='\n')
        {
            posx=0;
            posy++;
        }
        else
        posx+=word.length();

      }
      tmp=tmp.substr(word.length(),(unsigned int) -1);
   }
   myrefresh();
}

unsigned int random(unsigned int range)
{
  unsigned int random_value;
  if (range==0)
    return 0;
  random_value= (unsigned int) (((float)genrand_int32() / (float)0xFFFFFFFF)*(range));
  return random_value;
}

bool coin_toss()
{
	return random(10000)<5000;	
}

bool lower_random(int lower_value, int range)
{
   if (range==0)
     return false;
   if (random(range)<lower_value)
     return true;
   return false;
}

string IntToStr(long number)
{
  string tmp;
  int decpt, sign;
  if (number==0)
	  return "0";
  fcvt (number,0, &decpt, &sign);
  if (sign==0)
    tmp="";
  else
    tmp="-";
  tmp+=string(fcvt (number,0, &decpt, &sign));
  return tmp;
}

double RadToDeg(double angle)
{
  angle=(angle*180)/M_PI;
  return angle;
}

double DegToRad(double angle)
{
  angle=(angle*M_PI)/180;
  return angle;
}

int round_up(double variable)
{
   double down, rest;
   down=floor(variable);
   rest=variable-down;
   rest*=100;
   if (rest>50)
     return (int) ceil(variable);
   else
     return (int) floor(variable);
}

int generate_bresenham_line(int x1, int y1, int x2, int y2, struct POSITION *position,int max_range)
{
        int i,deltax,deltay,numpixels;
        int d, dinc1, dinc2;
        int x, xinc1, xinc2;
        int y, yinc1, yinc2;

//  { Calculate deltax and deltay for initialisation }
  deltax = abs(x2 - x1);
  deltay = abs(y2 - y1);
  d=0;
  dinc1=0;
  dinc2=0;

//   { Initialize all vars based on which is the independent variable }
  if (deltax==0 && deltay==0)
  {
      xinc1 = 0;
      xinc2 = 0;
      yinc1 = 0;
      yinc2 = 0;
      numpixels=1;
  }
  else
  {
   if (deltax >= deltay)
   {
//      { x is independent variable }
      numpixels = deltax + 1;
      d = (deltay << 1) - deltax;
      dinc1 = deltay<<1;
      dinc2 = (deltay - deltax)<<1;
      xinc1 = 1;
      xinc2 = 1;
      yinc1 = 0;
      yinc2 = 1;
   }
   else
   {
//      { y is independent variable }
      numpixels = deltay + 1;
      d = (deltax << 1) - deltay;
      dinc1 = deltax << 1;
      dinc2 = (deltax - deltay) << 1;
      xinc1 = 0;
      xinc2 = 1;
      yinc1 = 1;
      yinc2 = 1;
   }
  }
//  { Make sure x and y move in the right directions }
  if (x1 > x2)
  {
      xinc1 = - xinc1;
      xinc2 = - xinc2;  
  }
  if (y1 > y2)
  {

      yinc1 = - yinc1;
      yinc2 = - yinc2;
  }

//  { Start drawing at <x1, y1> }
  x = x1;
  y = y1;

//  { Draw the pixels }
//  for (i = 0 ; i<numpixels; i++)
  for (i = 0 ; i<max_range; i++) 
  {
      // wpisanie tego w tablice (x,y)
      position->x=x;
      position->y=y;

#ifdef DRAW_BRESENHAM
	  if (level.map.onMap(x,y))
	  {
		  set_color(15);
		  print_character(x,y,'*');
		  myrefresh();
	  }
#endif

      position++;

      if (d < 0)
      {
          d += dinc1;
          x += xinc1;
          y += yinc1;
      }
      else
      {
          d += dinc2;
          x += xinc2;
          y += yinc2;
      }
  }
  return numpixels;
}

int MYSCREEN :: draw_box(int color,int left,int top, int right,int down)
{
   int x,y;
   set_color(color);
   for (x=left;x<=right;x++)
    for (y=top;y<=down;y++)
    {
       if (x==left || x==right || y==top || y==down)
          print_character(x,y,'.');
       else
          print_character(x,y,' ');
    }
	return 0;
}

void MYSCREEN :: draw_targeting_line(int x1,int y1,int x2, int y2)
{
     int cells;
     unsigned long time_counter;
     int step;
     
     POSITION position[100]; // longest line - 100
     cells=generate_bresenham_line(x1,y1,x2,y2, (POSITION *) position,100);
     
	 set_color(9);
     for (int a=1;a<cells;a++)
     {        
		 print_character(position[a].x, position[a].y,'*');
     }
	 print_character(x2, y2,'+');
     myrefresh();
}

bool MYSCREEN :: draw_stunning_explosion(int *data)
{
	int x,y;
	bool player_saw_explosion;

	player_saw_explosion = false;
	set_color(15);
    for (x=0;x<MAPWIDTH;x++)
     for (y=0;y<MAPHEIGHT;y++)
     {		 
       if (data[x+y*MAPWIDTH]>0)
       {
		   if (level.map.seen_by_player(x,y))
		   {
				print_character(x,y,screen_copy.copy[x][y]);
				player_saw_explosion=true;
		   }
	   }
	 }
	 if (player_saw_explosion)
	 {
		myrefresh();
		delay(200);
	 }
	 return player_saw_explosion;
}

bool MYSCREEN :: draw_emp_explosion(int *data, int max)
{
	bool player_saw_explosion;
	int x,y;
	set_color(11);
	for (x=0;x<MAPWIDTH;x++)
		for (y=0;y<MAPHEIGHT;y++)
		{		 
			if (data[x+y*MAPWIDTH]>0)
			{
				if (level.map.seen_by_player(x,y))
				{
					print_character(x,y,screen_copy.copy[x][y]);
					player_saw_explosion=true;
				}
			}
		}
		if (player_saw_explosion)
		{
			myrefresh();
			delay(200);
		}
	return player_saw_explosion;
}

bool MYSCREEN :: draw_explosion(int *data, int max)
{
	int x,y,a;
	struct Screen_copy scr_copy;
	bool drawn;
	bool changed;
	int value;
	int w1,w2,w3;
	int character;
	int color;
	char colors[8]= { 8,8,4,12,14,15,15,15 };

	value=max;
	drawn=false;
	while(--value !=0)
	{
		for (x=0;x<MAPWIDTH;x++)
			for (y=0;y<MAPHEIGHT;y++)
			{
				if (level.map.seen_by_player(x,y))
				{
					if (data[x+y*MAPWIDTH]==value)
			  {
				  color=colors[(value*7)/max];
				  set_color(color);
				  print_character(x,y,'*');
				  drawn=true;
			  }
				}

			}
			if (drawn==true)
			{
				myrefresh();
				delay(50);
				drawn=false;
			}
	}
	return drawn;
}


void bad_virtual_call(string method)
{
  set_color(12);
  print_text(0,0,"Method " + method + " is only virtual for this class right now.");
  myrefresh();
  getchar();  
}

#ifndef max
int max(int value1, int value2)
{
   return ( (value1 > value2) ? value1 : value2);
}
#endif
#ifndef min
int min(int value1, int value2)
{
   return ( (value1 < value2) ? value1 : value2);
}
#endif

string DESCRIPTIONS::get_description(string to_find)
{
	map < string, string >::iterator iter;
	iter = descript_texts.find(to_find);
	if (iter != descript_texts.end())
		return descript_texts[to_find];
	else
		return string("");	
}

string DESCRIPTIONS::get_attack(string to_find)
{
	map < string, string >::iterator iter;
	iter = attacks.find(to_find);
	if (iter != attacks.end())
		return attacks[to_find];
	else
		return string("");	
}

bool DESCRIPTIONS::add_description(string object,string to_add)
{
	descript_texts[object] = to_add;
	return true;
}

bool DESCRIPTIONS::add_attack(string object,string to_add)
{
	attacks[object] = attacks[object] + to_add;
	return true;
}

bool DESCRIPTIONS::zero_attack(string object)
{
	map < string, string >::iterator iter;
	iter = attacks.find(object);
	if (iter != attacks.end())
	{
		attacks.erase(iter);
		return true;
	}
	return false;
}

bool DESCRIPTIONS::show_description(string to_find)
{
	string temp;
	if (to_find=="")
		return false;

    struct Screen_copy scr_copy;
    store_screen(&scr_copy);
	myclear();
	int size = to_find.size();
    set_color(2);
    print_text(0,0, "--------------------------------------------------------------------------------");
    print_text(0,2, "--------------------------------------------------------------------------------");
    print_text(0,49,"--------------------------------------------------------------------------------");
    set_color(10);
	print_text(39-size/2,1,to_find);
	set_color(7);
	temp = get_description (to_find);
	if (temp=="")
		temp = "There is no description for it.";

	temp+=" ";

	char character;
	string word;

	int x=2;
	int y=4;

	for (int a=0;a<temp.size();a++)
	{
		character = temp[a];
		word += character;
		if (character==' ')
		{
			if (x+word.size()>=78)
			{
				x=2;
				y++;
			}
			print_text(x,y,word);
			x+=word.size();
			word="";
		}
	}
	myrefresh();	
	keyboard.wait_for_key();
    restore_screen(&scr_copy);
	myrefresh();	
	return true;
}
