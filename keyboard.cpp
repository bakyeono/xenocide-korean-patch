#include "mem_check.h"

#include "keyboard.h"
#include "system.h"
#include <curses.h>

KEYBOARD :: KEYBOARD ()
{
      sw=49;
      s=50;
      se=51;
      w=52;
      e=54;
      nw=55;
      n=56;
      ne=57;
      get='g';
      quit='Q';
      wait=53;
      escape=27;
	  backspace=8;
      readmore=' ';
      readmore2='\n';
      inc_fire=']';
      dec_fire='[';
      inventory='i';
      reload='r';
      options='!';
      fire='f';
      nearest='n';
      help='?';
      messagebuffer='M';
      char_info='@';
      look='l';
      open='o';
      close='c';
      _throw='t';
	  save='S';
	  up='<';
	  down='>';
	  rest='R';
	  attack='A';
	  exchange='e';
	  activate_armor='\'';
	  activate_weapon=';';
	  explore='E';
	  travel='X';
	  show_visible='L';
}

bool KEYBOARD :: yes_no()
{
       int character;
       while (1)
       {
         character=wait_for_key();
         if (character=='y' || character=='Y')
           return true;
         if (character=='n' || character=='N' || character==escape)
           return false;
       }
}

int KEYBOARD :: getkey()
{
  return getkeyvalue();
}

int KEYBOARD :: ungetkey(int character)
{
  return ungetch(character);
}

int KEYBOARD :: wait_for_key()
{
  int character;
  character=KEYBOARD_NONE;
  while( character==KEYBOARD_NONE)
    character=getkeyvalue();
  return character;
}

string KEYBOARD :: get_name(int posx, int posy, int size)
{
  char character;
  char temp[2];
  string imie;
  string letters="abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

  temp[1]='\0';

  show_cursor();  
  print_character(posx-1,posy,' ');
  set_cursor(posx,posy);

  while(1)
  {
	  character=getkeyvalue();
	  if (character!=KEYBOARD_NONE)
	  {
		temp[0] = character;
		if (character==backspace && imie.size()>0)
		{
			imie.resize(imie.size()-1);
			imie += " ";
			print_text(posx,posy,imie);
			imie.resize(imie.size()-1);
			print_text(posx,posy,imie);
			myrefresh();
		}
		if (character==readmore2)
			break;
		if (imie.size()<size && letters.find(temp)!=-1)
		{
			imie += temp;
			print_text(posx,posy,imie);
			myrefresh();
		}
	  }
  }

  if (imie.size()==0)
	imie = "Unnamed";

  hide_cursor();
  return imie;
}

int KEYBOARD :: get_value(int posx, int posy, int size)
{
	char character;
	char temp[2];
	string value;
	string letters="0123456789";
	
	temp[1]='\0';
	
	show_cursor();  
	print_character(posx-1,posy,' ');
	set_cursor(posx,posy);
	
	while(1)
	{
		character=getkeyvalue();
		if (character!=KEYBOARD_NONE)
		{
			temp[0] = character;
			if (character==backspace && value.size()>0)
			{
				value.resize(value.size()-1);
				value += " ";
				print_text(posx,posy,value);
				value.resize(value.size()-1);
				print_text(posx,posy,value);
				myrefresh();
			}
			if (character==readmore2)
				break;
			if (value.size()<size && letters.find(temp)!=-1)
			{
				value += temp;
				print_text(posx,posy,value);
				myrefresh();
			}
		}
	}
	if (value.size()==0)
		return -1;

	char *stopstring;	
	int l = strtol( value.c_str(), &stopstring, 10 );
		
	hide_cursor();
	return l;
}



// Pobiera direction i zwraca *dx z przedzialu (-1,1) i tak samo *dy;

int KEYBOARD :: get_direction(int &dx, int &dy)
{
      int key;
      dx=0;
      dy=0;
      key=this->getkey();
   
      if (key==this->n)
      {
         dy--;
      }
      else if (key==this->ne)
      {
         dy--;
         dx++;
      }
      else if (key==this->e)
      {
         dx++;
      }
      else if (key==this->se)
      {
         dx++;
         dy++;
      }
      else if (key==this->s)
      {
         dy++;
      }
      else if (key==this->sw)
      {
         dy++;
         dx--;
      }
      else if (key==this->w)      
      {
         dx--;
      }
      else if (key==this->nw)
      {
         dx--;
         dy--;
      }
      return key;
}

