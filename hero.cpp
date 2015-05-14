// #define PLAYER_IS_IMMORTAL
// #define SHOW_TURN_NUMBER

#include "mem_check.h"

#include "monster.h"
#include "map.h"
#include "keyboard.h"
#include "actions.h"
#include "system.h"
#include "global.h"
#include "directions.h"
#include "options.h"
#include "attrib.h"
#include "level.h"
#include "places.h"
#include "parser.h"
#include "sounds.h"

#include <list>
using namespace std;

extern void make_screenshot(string filename,bool put_date);

#include <stdlib.h> // dla exit(0)

// z pliku global.cpp
extern GAME game;
extern KEYBOARD keyboard;
extern LEVEL level;
extern MYSCREEN screen;
extern DEFINITIONS definitions;
extern PLACES places;
extern class OPTIONS options;
extern DESCRIPTIONS descriptions;
extern SOUNDS sounds;

extern void turn_on_wait_for_key();
extern void turn_off_wait_for_key();

HERO :: HERO()
{
	ClassName = "HERO";
	UniqueNumber = PLAYER_UNIQUE_NUMBER;

  tile='@';
  color=14;

  size=100;
  weight=80000;

  stop_repeating();

  rename("Player");
   // player x and y position
  ChangePosition(-1,-1);

  hit_points.val=23;
  hit_points.max=23;

  strength.val=12;
  dexterity.val=8;
  endurance.val=10;
  intelligence.val=10;
  speed.val=30;

  unarmed.rename("맨손");
  unarmed.DMG=5;
  unarmed.HIT=3;
  unarmed.DEF=1;

  no_armor.rename("맨몸");
  no_armor.ARM=1;
  no_armor.owner=this;

  last_activated=NULL;
  enemy=NULL;
  
  experience=0;
  level_at_experience=500;
  exp_level=1;
  free_skill_pointes=150;
  adaptation_points = 0;

  ID_of_level = "";

  next_action_time=0;
  turns_of_rest=0;

  group_affiliation = GROUP_HERO;

  for (int a=0;a<NUMBER_OF_RESISTS;a++)
     experience_in_skill[a]=0;

  action_to_repeat=REPEAT_ACTION_NONE;  
}


void HERO :: give_experience(unsigned long value)
{
  experience += value;
  free_skill_pointes += value;
  while( experience >= level_at_experience)
    got_level_up();
}

void HERO :: got_level_up()
{
  exp_level++;

// incrase attributes
  screen.console.add("* 레벨이 오른다! *\n",10);
  sounds.PlaySoundOnce("data/sounds/other/levelup.wav",255);

  if (exp_level%3==0)
  {
	  strength.val++;
	  strength.max++;
	  dexterity.val++;
	  dexterity.max++;
	  speed.val++; 
	  speed.max++; 
	  screen.console.add("* 힘이 강해지고 움직임이 빨라진다! *\n",10);
  }
  if (exp_level%4==0)
  {
	  endurance.val++; 
	  endurance.max++; 
	  screen.console.add("* 신체가 강인해진다 *\n",10);
  }
  level_at_experience*=2;
  int random_value = random(3)+1;

// proportional incrase of metabolism.
  int new_metabolism = ((metabolism.max+1)*(hit_points.max+random_value))/hit_points.max; 
  metabolism.val += (new_metabolism - metabolism.max);
  metabolism.max = new_metabolism;

  hit_points.max += random_value;
  hit_points.val += random_value;

// add adaptation points

  if (exp_level%3==0)
	adaptation_points+=(endurance.val/4);
}


void HERO :: show_attributes()
{
   string text,exp;
   set_color(7);
   text="HP: " + IntToStr(hit_points.GetValue()) + "/" + IntToStr(hit_points.max) + " EP: ";
   if (energy_points.val>0)
	   text+=IntToStr(energy_points.GetValue());
   else
	   text+='-';
   print_text(0,47,text);

   if (have_low_hp())
   {
	   if (have_critically_low_hp())
	   {
		   sounds.PlayHeartBeat();
		   set_color(4);
	   }
	   else
	   {
		   set_color(14);
		   sounds.StopHeartBeat();
	   }
	   text=IntToStr(hit_points.GetValue());
	   print_text(4,47,text);
	   set_color(7);	   
   }
   else
	   sounds.StopHeartBeat();
   
   if (options.number[OPTION_EXPERIENCE]==false)
		exp = " EXP: " + IntToStr(exp_level) + "/" + IntToStr(experience) + " (" + IntToStr(free_skill_pointes) + ")";
   else
		exp = " EXP: " + IntToStr(exp_level) + "\\" + IntToStr(level_at_experience - experience) + " (" + IntToStr(free_skill_pointes) + ")";

   text="힘: " + IntToStr(strength.GetValue()) + " 재주: " + IntToStr(dexterity.GetValue());
   int speed_pos = text.size();
   text += " 속도: " + IntToStr(speed.GetValue()) + " 체력: " + IntToStr(endurance.GetValue()) + " 지능: " + IntToStr(intelligence.GetValue()) + exp;
   print_text(20,47,text);
   if (isBurdened())
   {
	   if (isStrained())
		   set_color(12);
	   else
		   set_color(14);	   
	   print_text(21+speed_pos,47,"속도");
	   set_color(7);	   
   }

#ifdef SHOW_TURN_NUMBER
   print_text(70,48, "T: " +IntToStr(next_action_time));
#endif

   // show level name

   print_text(79-level.levels[ID_of_level].name.size(),49,level.levels[ID_of_level].name);
   
   if (weapon!=NULL)
   {
	   text="x " + (weapon->ITEM::show_name()).substr(0,25);
	  
      if (weapon->property_load_ammo())
      {		
		text="x " + (weapon->ITEM::show_name()).substr(0,17);
		RANGED_WEAPON *ranged_weapon = weapon->IsShootingWeapon();
		if (ranged_weapon!=NULL)
		{
			text+=" " + IntToStr ( ranged_weapon->ammo.quantity );
			text+="/" + IntToStr ( ranged_weapon->magazine_capacity);
			text+=" ";

			switch( ranged_weapon->fire_type)
			{
			  case FIRE_SINGLE:
				text+="(1)";
				break;
			  case FIRE_DOUBLE:
				text+="(2)";
				break;
			  case FIRE_TRIPLE:
				text+="(3)";
				break;
			  case FIRE_BURST:
				text+="(B)";
				break;
			}
		} // endof if
      }

	  if (weapon->energy_activated==1)
		set_color(13);
	  else
  	    set_color(7);
	  
      print_text(20,48,text);
	  weapon->draw_at(20,48);
   }

// armor always !=NULL
   text=(armor->ITEM::show_name()).substr(0,25);   

   if (armor->hit_points < armor->max_hit_points/2)
   {
	   if (armor->hit_points <= armor->max_hit_points/4)
	   {
			   set_color(4);
	   }
	   else
	   {
		   if (armor->energy_activated==1)
			   set_color(5);
		   else
			   set_color(14);
	   }
   }
   else
   {
	   if (armor->energy_activated==1)
		   set_color(13);
	   else
		   set_color(7);
   }

   print_text(53,48,text);
   armor->draw_at(51,48);
   
   // print enemy
   if (enemy!=NULL)
   {
      set_color(7);
      print_text(20,49,"목표: ");
	  
	  if (enemy->have_low_hp())
	  {
		  if (enemy->have_critically_low_hp())
			  set_color(4);
		  else
			  set_color(14);
	  }
	  else
		  set_color(9);
	  
		  
      print_text(28,49,enemy->name);
   }

   // print status
   if (IsRadiated())
   {
       set_color(14);
	   print_text(0,49,"오염");
   }
   if (IsPoisoned())
   {
       set_color(2);
	   print_text(5,49,"중독");
   }
   if (IsBurning())
   {
       set_color(4);
	   print_text(10,49,"화상");
   }
   else if (IsFreezed())
   {
       set_color(15);
	   print_text(10,49,"동상");
   }
   if (IsSlowed())
   {
       set_color(1);
	   print_text(15,49,"감속");
   }
   else if (IsHasted())
   {
       set_color(9);
	   print_text(15,49,"가속");
   }
   
}

void HERO :: display()
{
   TILE :: display();
   show_attributes();
}

void HERO::stop_repeating()
{
	turns_of_rest=0;
	travel_dest_x = -1;
	travel_dest_y = -1;
	action_to_repeat=REPEAT_ACTION_NONE;
    inventory_repeat_break=true;
}

void HERO::repeat_action(ERepeatAction a_to_repeat)
{
	action_to_repeat=a_to_repeat;
}


ACTION HERO :: get_action()
{	
	  if (IsStunned())
	  {
		  level.map.UnSeenMap();
		  level.map.HideFromPlayerSight();
		  
		  ptr_list::iterator m,_m;
		  MONSTER *temp;
		  
		  for(m=level.monsters.begin(),_m=level.monsters.end(); m!=_m; m++)
		  {
			  temp=(MONSTER *)*m;
			  if (temp!=level.player)
				  temp->seen_now = false;
		  }			  
		  return ACTION_WAIT_STUNNED;
	  }

	  // repeating

	  if (action_to_repeat==REPEAT_ACTION_REST)
	  {
		  if (turns_of_rest>0)
		  {
			  turns_of_rest--;
			  return ACTION_WAIT;
		  }
		  else
			  repeat_action(REPEAT_ACTION_NONE);
	  }

	  // show screen before player movement
	  // do fov calculation
	  look_around();

	  if (action_to_repeat==REPEAT_ACTION_EXPLORE)
	  {
		  // set_direction_to_unexplored
		  if (set_direction_to_closest_unexplored())
		  {
			  screen.draw();
			  myrefresh();
			  delay(20);
			  return ACTION_MOVE;
		  }
		  else
			return ACTION_NOTHING;
	  }
	  if (action_to_repeat==REPEAT_ACTION_TRAVEL)
	  {
		  if (set_direction_to_cell_by_shortest_path(travel_dest_x,travel_dest_y,true))
		  {
			  screen.draw();
			  myrefresh();
			  delay(20);
			  return ACTION_MOVE;
		  }
		  else
		  {
			  stop_repeating();
			  return ACTION_NOTHING;
		  }
	  }

	  if (options.number[OPTION_AIM_IF_NO_ENEMY]==true && enemy==NULL)
		  select_closest_enemy();
	  
	  if (options.number[OPTION_AUTO_AIM]==true)
		  select_closest_enemy();

	  if (level.player->is_repeating_action()==false)
	  {
		  screen.clear_all();						
		  screen.draw();
		  screen.console.clean();
		  
		  // print items pod nogami po of movement
		  if (last_pX!=pX() || last_pY!=pY())
		  {
			  level.player->show_laying_items();
		  }
		  
		  screen.console.show();
		  screen.console.zero();
	  }

	  int key=0;
	  if (is_repeating_action()==false)
	  {
		  if (options.number[OPTION_CURSOR]==true)
		  {
			  show_cursor();
			  print_text(level.player->pX(),level.player->pY(),"");
			  set_cursor(level.player->pX(),level.player->pY());
		  }		  
		  key=keyboard.wait_for_key();
		  if (options.number[OPTION_CURSOR]==true)
		  {
			  hide_cursor();
		  }		  
	  }      

      if (key==keyboard.n)
      {
        direction=_kn;
        return ACTION_MOVE;
      }
      else if (key==keyboard.ne)
      {
        direction=_kne;
        return ACTION_MOVE;
      }
      else if (key==keyboard.e)
      {
        direction=_ke;
        return ACTION_MOVE;
      }
      else if (key==keyboard.se)
      {
        direction=_kse;
        return ACTION_MOVE;
      }
      else if (key==keyboard.s)
      {
        direction=_ks;
        return ACTION_MOVE;
      }
      else if (key==keyboard.sw)
      {
        direction=_ksw;
        return ACTION_MOVE;
      }
      else if (key==keyboard.w)
      {
        direction=_kw;
        return ACTION_MOVE;
      }
      else if (key==keyboard.nw)
      {
        direction=_knw;
        return ACTION_MOVE;
      }
      else if (key==keyboard.quit)
      {
		  screen.console.add_and_zero("정말로 *저장하지 않고* 종료할까? (예Y/아니오N)\n",7);
		  if (keyboard.yes_no()==true)
			  return ACTION_QUIT;

		  return ACTION_NOTHING;		  
      }
      else if (key==keyboard.save && level.ID!="Tutorial")
      {
		  screen.console.add_and_zero("저장하고 종료할까? (예Y/아니오N)\n",7);
		  if (keyboard.yes_no()==true)
			  return ACTION_SAVE;
		  
		  return ACTION_NOTHING;		  
      }
      else if (key==keyboard.exchange)
      {
		  set_ready_weapon();
		  return ACTION_NOTHING;
      }	  
      else if (key==keyboard.up)
      {
        return ACTION_STAIRS_UP;
      }
      else if (key==keyboard.down)
      {
        return ACTION_STAIRS_DOWN;
      }
      else if (key==keyboard.rest)
      {
		  return ACTION_REST;
      }
	  else if (key==keyboard.explore)
	  {
		  return ACTION_EXPLORE;
	  }
	  else if (key==keyboard.travel)
	  {
		  return ACTION_TRAVEL;
	  }
      else if (key==keyboard.wait)
      {
        return ACTION_WAIT;
      }
      else if (key==keyboard.get)
      {
        return ACTION_GET;
      }
      else if (key==keyboard.look)
      {
        return ACTION_LOOK;
      }
	  else if (key==keyboard.show_visible)
	  {
		  print_visible_monsters_and_items();
		  return ACTION_NOTHING;
	  }	  
	  else if (key==keyboard.messagebuffer)
	  {
		  return ACTION_SHOW_MESSAGE_BUFFER;
	  }
      else if (key==keyboard.inventory || action_to_repeat==REPEAT_ACTION_INVENTORY)
      {
        return ACTION_INVENTORY;
      }
      else if (key==keyboard.fire)
      {
        return ACTION_FIRE;
      }
      else if (key==keyboard.help)
      {
        return ACTION_SHOW_HELP;
      }
      else if (key==keyboard.char_info)
      {
        return ACTION_SHOW_INFO;
      }
      else if (key==keyboard.inc_fire)
      {
        return ACTION_INC_FIRE;
      }
      else if (key==keyboard.dec_fire)
      {
        return ACTION_DEC_FIRE;
      }
      else if (key==keyboard.options)
      {
        return ACTION_OPTIONS;
      }
      else if (key==keyboard.reload)
      {
        return ACTION_RELOAD;
      }
      else if (key==keyboard._throw)
      {
        return ACTION_THROW;
      }
      else if (key==keyboard.activate_weapon)
      {
		  return ACTION_ACTIVATE_WEAPON;
      }
      else if (key==keyboard.activate_armor)
      {
		  return ACTION_ACTIVATE_ARMOR;
      }	  
      else if (key==keyboard.escape)
      {
         enemy=NULL; // zero enemy
         return ACTION_NOTHING;
      }
      else if (key==keyboard.nearest)
      {
		  if (enemy==NULL)
            select_closest_enemy();
		  else // go through all of enemies
		  {
			  ptr_list::iterator m,_m;
			  MONSTER *temp, *first=NULL, *next=NULL;
			  bool wybierz_kolejnego=false;
			  
			  for(m=level.monsters.begin(),_m=level.monsters.end(); m!=_m; m++)
			  {
				  temp=(MONSTER *)*m;
				  if (temp->seen_now && this->is_enemy(temp))
				  {
					  if (wybierz_kolejnego==true)
					  {
						  next=temp;
						  break;
					  }
					  if (first==NULL)
						  first=temp;
					  if (temp==enemy)
						  wybierz_kolejnego=true;
				  }
			  }
			  if (next!=NULL)
				  enemy=next;
			  else
				  enemy=first;
		  }
         return ACTION_NOTHING;
      }
      else if (key==keyboard.open)
      {
        return ACTION_OPEN;
      }
      else if (key==keyboard.close)
      {
        return ACTION_CLOSE;
      }
      else if (key==keyboard.attack)
      {
		  return ACTION_ATTACK;
      }
	  
      return ACTION_NOTHING;      
}

bool HERO::print_visible_monsters_and_items()
{
	ptr_list::iterator m,_m;
	MONSTER *monster;
	ITEM *item;
	screen.console.add("\n현재 시야에 보이는 괴물: ",7);
	bool something_seen=false;

	for(m=level.monsters.begin(),_m=level.monsters.end(); m!=_m; m++)
	{
		monster=(MONSTER *)*m;
		if (monster==this)
			continue;

		if (monster->seen_now)
		{
			if (is_enemy(monster)) // enemy
			{
				screen.console.add(string()+monster->tile,monster->color);
				screen.console.add(monster->name,7);
				something_seen=true;
			} 
		}	
	}
	if (!something_seen)
		screen.console.add("없음.",7);

	// seen items
	something_seen=false;
	screen.console.add("\n현재 시야에 보이는 물건: ",7);
	for(m=level.items_on_map.begin(),_m=level.items_on_map.end(); m!=_m; m++)
	{
		item=(ITEM *)*m;

		if (level.map.seen_by_camera(item->pX(),item->pY()) || (level.map.seen(item->pX(),item->pY()) && distance(item->pX(),item->pY(),pX(),pY())<fov_radius.GetValue()))
		{
			if (!item->property_controller() && !item->invisible)
			{

				screen.console.add(string()+item->tile,item->color);
				screen.console.add(item->show_name(),7);
				something_seen=true;
			}
		}	
	}
	if (!something_seen)
		screen.console.add("없음.\n",7);
	else
		screen.console.add("\n",7);
	return true;
}

bool HERO::look_around()
{
	level.map.HideFromPlayerSight();
	level.fov.Start(&level.map,level.player->pX(),level.player->pY(),level.player->fov_radius.GetValue());
	MONSTER::look_around();
	string to_print;
	bool was_stop=false;

	for(int x=0; x<MAPWIDTH; x++)
		for(int y=0; y<MAPHEIGHT; y++)
		{
			if ( (level.map.seen(x,y) && distance(x,y,level.player->pX(),level.player->pY()) < level.player->fov_radius.GetValue()) ||
				  level.map.seen_by_camera(x,y))
			{
				// stop repeating if player sees a new item			 
				if (action_to_repeat!=REPEAT_ACTION_NONE)
				{
					if (!level.map.seen_once(x,y) && level.map.GetNumberOfItems(x,y)!=0)
					{
						ptr_list items;
						level.list_of_items_from_cell(&items,x,y);
						ptr_list::iterator m,_m;
						ITEM *item;
						for (m=items.begin(),_m=items.end();m!=_m;++m)
						{
							ITEM *item = (ITEM *) *m;
							if (!item->property_controller() && !item->invisible) // gdy nie jest to sterujacy i gdy nie invisible
							{
								if (was_stop)
									to_print+=string(", ");
								else 
								{
									to_print+=string("Stop: ");
									was_stop=true;
								}
								to_print+=item->show_name();								
							}
						}
					}
				}
				level.map.setSeenByPlayer(x,y);			  		  
			}
		}		

    // seen monsters
	ptr_list::iterator m,_m;
	MONSTER *current_monster;
	
	for(m=level.monsters.begin(),_m=level.monsters.end(); m!=_m; m++)
	{
		current_monster=(MONSTER *)*m;	
		if (level.map.seen_by_player(current_monster->pX(),current_monster->pY()))
		{
			current_monster->seen_now=true;

			if (is_repeating_action())
			{
				if (current_monster!=level.player && current_monster->is_enemy(level.player)) // if player sees monster, then stop repeating
				{
					if (was_stop)
						to_print+=string(", ");
					else 
					{
						to_print+=string("Stop: ");
						was_stop=true;
					}
					to_print+=current_monster->name;								
				}
			}
			
			// to experience - player get for killing if seen recently
			current_monster->seen_last_time_in_turn=level.turn;
		}
		else // when not seen
		{
			current_monster->seen_now=false;
			if (current_monster==level.player->enemy) // current target
				level.player->enemy=NULL;
		}		
	}
	if (to_print.size()>0)
	{
		stop_repeating();
		to_print+='.';
		screen.console.add(to_print,7);
	}
	return true;
}



ITEM *HERO :: choose_item_from_backpack(int character)
{
   ptr_list::iterator m,_m;
   ITEM *temp;
   for(m=backpack.begin(),_m=backpack.end(); m!=_m; m++)
   {
       temp=(ITEM *)*m;
       if (temp->inventory_letter == character)
         return temp;
   }
   return NULL;
}

void HERO :: show_laying_items()
{
   ptr_list::iterator m,_m;
   ITEM *temp;
   ptr_list items;

   // check stairs
   STAIRS *stairs = level.map.StairsAt(pX(),pY());
   if (stairs!=NULL)
	   screen.console.add(stairs->name +"\n",15);

   // print items
   level.list_of_items_from_cell(&items,pX(),pY());
   if (items.size()==0) // when none
   {
     return;
   }
   else if (items.size()==1)
   {
     temp=(ITEM *)*items.begin();
     screen.console.add(string("이곳에 ") + temp->article_a() + temp->show_name() + "이 떨어져 있다.\n",7);
   }
   else if (items.size()<4)
   {
     screen.console.add("이곳에 있는 물건:\n",7);
     for(m=items.begin(),_m=items.end(); m!=_m; m++)
     {
       temp=(ITEM *)*m;
       screen.console.add(temp->article_a() + temp->show_name() + "\n",7);
     }
   }
   else
   {
     screen.console.add("이곳에 물건 여러 개가 떨어져 있다...",7);
   }
}

ITEM * HERO :: choose_item_to_pick_up()
{
   ptr_list items;
   level.list_of_items_from_cell(&items,pX(),pY());
   if (items.size()==0)
   {
     screen.console.add("주울 물건이 없다.",7);
     return NULL;
   }
   
   if (items.size()==1)
     return (ITEM *)*items.begin();

   // there are more
	return choose_item_from_list(items,"주울 물건을 선택하라");
}

bool HERO::throw_item_to (int x, int y, ITEM *item)
{
	COUNTABLE *countable = item->IsCountable();
	if (countable!=NULL && countable->quantity>1)
	{
		int posy=12;
		struct Screen_copy scr_copy;
		store_screen(&scr_copy);
		
		string text = item->show_name() + "을 몇 개 던질까? ";
		screen.draw_box(2,40-text.size()/2-2,posy,40+text.size()/2+1,posy+2);
		set_color(15);
		print_text(40-text.size()/2,posy,text);
		set_color(2);
		print_character(40-text.size()/2-1,posy,'[');
		print_character(40+text.size()/2,posy,']');
		
		set_color(7);
		int value = keyboard.get_value(38,posy+1,4);

		restore_screen(&scr_copy);
		if (value==0)
			return false;
		
		if (value!=-1 && value<countable->quantity) // split countable
		{
			// duplicate item
			COUNTABLE *new_one;
			int old_number = countable->quantity;
			countable->quantity=value;
			countable->ChangePosition(-1,-1);

			new_one = (COUNTABLE *) countable->duplicate();
			
			countable->quantity=old_number-value;
			item=new_one;
		}		
	}		
	return INTELLIGENT::throw_item_to (x,y,item);
}

int HERO::drop_item(ITEM *item,bool visible_dropping)
{
	COUNTABLE *countable = item->IsCountable();
	item->ChangePosition(pX(),pY());
	if (countable!=NULL && countable->quantity>1)
	{
		int posy=12;
		struct Screen_copy scr_copy;
		store_screen(&scr_copy);
		
		string text = item->show_name() + "을 몇 개 버릴까? ";
		screen.draw_box(2,40-text.size()/2-2,posy,40+text.size()/2+1,posy+2);
		set_color(15);
		print_text(40-text.size()/2,posy,text);
		set_color(2);
		print_character(40-text.size()/2-1,posy,'[');
		print_character(40+text.size()/2,posy,']');

		set_color(7);
		int value = keyboard.get_value(38,posy+1,4);

		restore_screen(&scr_copy);
		if (value==0)
			return false;

		if (value!=-1 && value<countable->quantity) // split countable
		{
			// duplicate item
			COUNTABLE *new_one;
			int old_number = countable->quantity;
			countable->quantity=value;
			new_one = (COUNTABLE *) countable->duplicate();
			countable->quantity=old_number-value;

			if (visible_dropping)
				if (this->seen_now==true)
				{
					text=this->name + "은 " + new_one->article_the() + new_one->show_name() + "을 버린다.";
					screen.console.add(text,3,false);
					return true;
				}							
		}
	}
	return INTELLIGENT::drop_item(item,visible_dropping);
}


int HERO :: get_action_for_item(ITEM *item)
{
       ITEM *temp;
       int character;
       
       while (1)
       {
         character=keyboard.wait_for_key();
         if (character=='x')
         {
			 descriptions.show_description(item->ITEM::show_name());
         }
         if (character=='u' && item->property_use())
         {
			 use_item(item);
			 return true;
         }
         if (character=='d' && item->property_drop())
         {
            drop_item(item,true);
            return true;
         }
         if (character=='t' && item->property_throw())
         {
            last_activated=item;
            if (do_action(ACTION_THROW)!=0)
              return true;
         }
         if (character=='p' && item->property_put_on())
         {
            if (item!=armor)
              set_armor(item);
            else
              set_armor(&no_armor);
            return true;
         }
         if (character=='w' && item->property_wield())
         {
            if (item!=weapon)
              set_weapon(item);
            else
              set_weapon(&unarmed);
            return true;
         }
         if (character=='a' && item->property_activate())
         {
            last_activated=item;
            screen.console.add(name + "은 " + item->show_name() + "을 작동시킨다.\n",3,false);
            item->activate();
            return true;
         }
         if (character=='n' && item->property_unload_ammo())
         {
			 if (item->unload_ammo(this)==true)
			 {
				 screen.console.add(name + "은 " + item->show_name() + "의 탄을 분리한다.\n",3,false);
				 return true;
			 }
         }
         if (character=='r' && item->property_ready())
         {
			 if (choose_ready_weapon(item)==true)
			 {
				 return true;
			 }
         }		 
         if (character=='b' && item->property_buildrobot())
         {
			 if (build_robot(item)==true)
			 {
				sounds.PlaySoundOnce("data/sounds/other/autorepair.wav");
				 return true;
			 }
			 return false;
         }		 
         if (character=='c' && item->property_program())
         {
			 if (reprogram(item->IsProgrammator())==true)
			 {
				 sounds.PlaySoundOnce("data/sounds/other/autorepair.wav");
				 return true;
			 }
			 return false;
         }		 
         if (character=='o' && item->property_buildrobot())
         {
			 if (turn_robot_on(item)!=1)
			 {
				 sounds.PlaySoundOnce("data/sounds/other/autorepair.wav");
				 return false;
			 }
			 return true;
         }		 
         if (character=='l' && item->property_load_ammo())
         {
			if (item->IsLoaded())
			{
				continue;
			}
            show_backpack(item, INV_SHOW_MATCH_AMMO);
            
            set_color(10);
            print_text(32-item->show_name().size()/2,0,string(" Select an ammo for ") + item->show_name() + " ");
            myrefresh();
            // choose ammo from backpack
            while (1)
            {
               character=keyboard.wait_for_key();
               if ((character>='a' && character<='z') || (character>='A' && character<='Z'))
               {
                  temp=choose_item_from_backpack(character);
                  if (temp!=NULL) // when something selected
                  {
					AMMO *am = temp->IsAmmo();
                    if (am != NULL  ) // is it really an ammo?
                    {
						   switch( item->load_ammo(am))
						   {
							 case 1:
								  continue;
							 case 0:
								  screen.console.add(name + "은 " + item->show_name() + "을 장전한다.\n",3,false);
								  return true;
						   }
                    } // endof if                       
                  } // endof if temp!=NULL
               }
               else if (character==keyboard.escape || character==keyboard.readmore || character==keyboard.readmore2)
               {
                  return false;
               }
            } // endof while(1)
            
            return true;
         }

         if (character=='m' && item->property_mend() && (item->hit_points!=item->max_hit_points || (item->IsRepairSet())) )
         {
			 REPAIR_KIT *repset=item->IsRepairSet();
			 if (repset!=NULL)
			 {
				 if (repset->can_be_used())					 
				 {
					 if (repset->purpose==PURPOSE_REPAIR_HP)
					 {
						 show_backpack(item, INV_SHOW_REPAIR_KIT);
						 set_color(10);
						 print_text(26-item->show_name().size()/2,0,string(" 수리에 사용할 물건을 골라라 ") + item->show_name() + " ");
					 }
					 else if (repset->purpose==PURPOSE_REPAIR_FIX_ARMOR)
					 {
						 show_backpack(item, INV_SHOW_IMPROVE_ARMOR);
						 set_color(10);
						 print_text(26-item->show_name().size()/2,0,string(" 개조에 사용할 물건을 골라라 ") + item->show_name() + " ");
					 }
				 }
			 }				 
			 else
			 {
				 show_backpack(item, INV_SHOW_MATCH_ITEM_TO_REPAIR);
				 set_color(10);
				 print_text(25-item->show_name().size()/2,0,string(" 수리에 사용할 물건을 골라라 ") + item->show_name() + " ");
			 }
			 myrefresh();			 
			 // choose item for mend
			 while (1)
			 {
				 character=keyboard.wait_for_key();
				 if ((character>='a' && character<='z') || (character>='A' && character<='Z'))
				 {
					 temp=choose_item_from_backpack(character);
					 if (temp!=NULL && temp!=item) // if selected item
					 {
						 if (repset!=NULL)
						 {
							  if (temp->fix_with_item(item,this)==true)
							  {
								  screen.console.add(name + "은 " + temp->show_name() + "을 수리한다.\n",3,false);
								  return true;
							  }
						 }
						 else if (item->fix_with_item(temp,this)==true)
						 {
							 screen.console.add(name + "은 " + item->show_name() + "을 수리한다.\n",3,false);
							 return true;
						 }
						 else
							 continue;
					 } // endof if temp!=NULL
				 }
				 else if (character==keyboard.escape || character==keyboard.readmore || character==keyboard.readmore2)
				 {
					 return false;
				 }
			 } // endof while(1)
		 }			 
         if (character==keyboard.escape || character==keyboard.readmore || character==keyboard.readmore2)
            break;
       }
       return false;
}


int HERO :: show_actions_of_item(ITEM *item)
{
   int cornerx;
   int cornery;
   int x,y,zm_x;
   int length, left, right, top, down;
   string text;

   cornerx=30;
   cornery=10;

   length=item->show_name().size();

   text=string("[ x ")+item->show_name()+" ]";

   left=cornerx;
   right=cornerx+text.size()+1;
   top=cornery;

   // count how much place takes printing vertically 
   item->print_attributes_vertical(left,top+2,&zm_x,&y);
   if (y>0)
   {
      if (right-left<16+zm_x)
        right=left+16+zm_x;
   }
   // place in the center of window
   cornerx=40-((right-left)/2);
   right=(cornerx+(right-left));
   left=cornerx;
   
   down=top+y+2;
   
   // show border
   screen.draw_box(2,left,top,right,down);
    
   // print 
   set_color(7);
   item->print_attributes_vertical(right-zm_x-1,top+2,&x,&y);
   
   x=left+2;

   set_color(15);
   print_text(left+1,cornery,text);
   set_color(2);
   print_character(left+1,cornery,'[');
   print_character(left+text.size(),cornery,']');
   
   item->draw_at(left+3,cornery);

   y=cornery+2;

      if (item->property_wield())
      {
		  if (item != weapon)
		  {
			  set_color(6);
			  print_text(x+1,y," 장비");
			  set_color(14);
			  print_text(x,y,"W");
		  }
		  else
		  {
			  set_color(6);
			  print_text(x+1,y," 해제");
			  set_color(14);
			  print_text(x,y,"W");
		  }
		  
         y++;
      }
      if (item->property_put_on())
      {
		  set_color(6);
		  print_text(x+1,y," 입기/벗기");
		  set_color(14);
		  print_text(x,y,"P");
         y++;
      }
      if (item->property_drop())
      {
         set_color(6);
         print_text(x+1,y," 버리기");
         set_color(14);
         print_text(x,y,"D");
         y++;
      }
      if (item->property_throw())
      {
         set_color(6);
         print_text(x+1,y," 던지기");
         set_color(14);
         print_text(x,y,"T");
         y++;
      }
      if (item->property_use())
      {
		  set_color(6);
		  print_text(x+1,y," 사용");
		  set_color(14);
		  print_text(x,y,"U");
		  y++;
      }
      if (item->property_load_ammo() && !item->IsLoaded())
      {
         set_color(6);
         print_text(x+1,y," 장전");
         set_color(14);
         print_text(x,y,"L");
         y++;
      }
      if (item->property_unload_ammo() && item->IsLoaded())
      {
         set_color(6);
         print_text(x+1,y," 탄 분리");
         set_color(14);
         print_text(x,y,"N");
         y++;
      }
      if (item->property_activate())
      {
		  HAND_WEAPON *hw = item->IsHandWeapon();
		  BASE_ARMOR *arm = item->IsArmor();
		  bool to_activate=false;
		  
		  if (hw!=NULL || arm!=NULL || (hw==NULL && arm==NULL))
		  {
			  if ( (hw!=NULL && hw->energy_activated==0) || (arm!=NULL && arm->energy_activated==0) || (hw==NULL && arm==NULL))
				  to_activate=true;
		  }
		  if (to_activate)
		  {
			  set_color(6);
			  print_text(x+1,y," 작동");			  
			  set_color(14);
			  print_text(x,y,"A");
			  y++;
		  }
		  else if ( (hw!=NULL && hw->energy_activated==1) || (arm!=NULL && arm->energy_activated==1))
		  {
			  set_color(6);
			  print_text(x+1,y," 작동 중단");
			  set_color(14);
			  print_text(x,y,"A");
			  y++;
		  }
         
      }
      if (item->property_mend())
      {
		  REPAIR_KIT *repset;
		  repset=item->IsRepairSet();
		  if ( item->hit_points!=item->max_hit_points || (repset!=NULL && repset->can_be_used()))
		  {
			  set_color(6);
			  print_text(x+1,y," 수리");
			  set_color(14);
			  print_text(x,y,"M");
			  y++;
		  }
      }	  

      if (item->property_program())
      {
		  set_color(6);
		  print_text(x+1,y," CPU 설계");
		  set_color(14);
		  print_text(x,y,"C");
		  y++;
      }	  	  

      if (item->property_buildrobot())
      {
		  set_color(6);
		  print_text(x+1,y," 로봇 조립");
		  set_color(14);
		  print_text(x,y,"B");
		  y++;
      }	  
      if (item->property_buildrobot())
      {
		  ROBOT_SHELL *r;
		  r=item->IsRobotShell();
		  if (r!=NULL && r->cpu!=NULL)
		  {
			  set_color(6);
			  print_text(x+1,y," 켜기");
			  set_color(14);
			  print_text(x,y,"O");
			  y++;
		  }
      }	  
      if (item->property_ready() && item!=weapon && item!=ready_weapon)
      {
		  set_color(6);
		  print_text(x+1,y," 보조 무기");
		  set_color(14);
		  print_text(x,y,"R");
		  y++;
      }	  
	  
	  set_color(6);
	  print_text(x+1,y," 조사");
	  set_color(14);
	  print_text(x,y,"X");
	  y++;
		  
      myrefresh();
	  return 0;
}


int HERO :: pick_up_item(ITEM *item, bool see_possible)
{
   string text;
   ptr_list::iterator m,_m;
   ITEM *temp;
   char *letters="abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
   string additional;
   // check if the item can be taken
   if (item==NULL)
     return false;
   switch (can_pick_up(item))
   {
      case 1:
           screen.console.add(string("너무 무거워서 가질 수 없다.\n"),7);
           return false;           
      case 2:
           screen.console.add(string("가방에 이것을 넣을 자리가 없다.\n"),7);
           return false;
   }

   COUNTABLE *countable = item->IsCountable();
   if (options.number[OPTION_GETALL]==false && see_possible && countable!=NULL && countable->quantity>1)
   {
	   int posy=12;
	   struct Screen_copy scr_copy;
	   store_screen(&scr_copy);
	   
	   string text = " 몇 개의 " + item->show_name() + "을 가질 것인가? ";
	   screen.draw_box(2,40-text.size()/2-2,posy,40+text.size()/2+1,posy+2);
	   set_color(15);
	   print_text(40-text.size()/2,posy,text);
	   set_color(2);
	   print_character(40-text.size()/2-1,posy,'[');
	   print_character(40+text.size()/2,posy,']');
	   
	   set_color(7);
	   int value = keyboard.get_value(38,posy+1,4);
	   
	   restore_screen(&scr_copy);
	   if (value==0)
		   return false;
	   
	   if (value!=-1 && value<countable->quantity) // split countable
	   {
		   // duplicate item
		   COUNTABLE *new_one;
		   int old_number = countable->quantity;
		   int old_x = countable->pX();
		   int old_y = countable->pY();
		   countable->ChangePosition(-1,-1);		   
		   countable->quantity=value;
		   new_one = (COUNTABLE *) countable->duplicate();
		   countable->quantity=old_number-value;
		   countable->ChangePosition(old_x,old_y);
		   item = new_one;
	   }
   }	   

// Assign letter
   additional=":";

   for(m=this->backpack.begin(),_m=this->backpack.end(); m!=_m; m++)
   {
       temp=(ITEM *)*m;
       additional+=temp->inventory_letter;
   }

// find not assigned

   int index;
   for (index=0;index<50;index++) // zalozmy, ze 50 liter, tyle nie bedzie - wyzej jest check, czy moze podniesc item
   {
      if (additional.find(letters[index])==-1)
        break;
   }
   
 // assign letter. Variable max_items_in_backpack guarantee range check
   item->inventory_letter=letters[index];


   if (INTELLIGENT :: pick_up_item(item,see_possible)==true)
   {
//      text=string("You get ") + item->show_name() +".\n";
//      screen.console.add(text,3);
   }
   return true;
}

// positions
#define INFO_BASIC_X      2
#define INFO_BASIC_Y      3
#define INFO_SKILLS_X     30
#define INFO_SKILLS_Y     3
#define INFO_STATUS_X     2
#define INFO_STATUS_Y     14
#define INFO_INVENTORY_X  30
#define INFO_INVENTORY_Y  20
#define INFO_RESISTS_X    2
#define INFO_RESISTS_Y    24
#define INFO_EXPERIENCE_X 30
#define INFO_EXPERIENCE_Y 28

#define NUMBER_OF_ATTRIBS 5

void HERO :: show_info()
{
     int a,x,y;
     screen.clear_all();
  
     set_color(9);
                      
     set_color(2);
     print_text(0,0, "--------------------------------------------------------------------------------");
     print_text(0,49,"--------------------------------------------------------------------------------");
     set_color(10);
     print_text(33,0," 인물 정보 ");

   // print basic info ---------------------

     x=INFO_BASIC_X;
     y=INFO_BASIC_Y;

     screen.draw_box(2,x,y,x+18,y+NUMBER_OF_ATTRIBS+4);
     set_color(10);
     x+=2;
     print_text(x,y++, "기본 정보");
     
     y++;

     set_color(7);
     print_text(x,y++,"이름: " + name);
     y++;

     for (a=0;a<NUMBER_OF_ATTRIBS;a++)
     {
       set_color(8);
       print_text(x,y+a,"...............");
     }

	 if (strength.mod!=0)
		 set_color(12);
	 else
		 set_color(7);	 
     print_number_right_align(x+13,y,strength.GetValue(),2);
     print_text(x,y++,"힘");

	 if (dexterity.mod!=0)
		 set_color(12);
	 else
		 set_color(7);	 
     print_number_right_align(x+13,y,dexterity.GetValue(),2);
     print_text(x,y++,"재주");

	 if (speed.mod!=0)
		 set_color(12);
	 else
		 set_color(7);	 
	 
     print_number_right_align(x+13,y,speed.GetValue(),2);
     print_text(x,y++,"행동 속도");

	 if (endurance.mod!=0)
		 set_color(12);
	 else
		 set_color(7);	 
	 
     print_number_right_align(x+13,y,endurance.GetValue(),2);
     print_text(x,y++,"체력");

	 if (intelligence.mod!=0)
		 set_color(12);
	 else
		 set_color(7);	 
	 
     print_number_right_align(x+13,y,intelligence.GetValue(),2);
     print_text(x,y++,"지능");


     // print skills -------------------------------------------
     
                      x=INFO_SKILLS_X;
                      y=INFO_SKILLS_Y;


                      screen.draw_box(2,x,y,x+23,y+NUMBER_OF_SKILLS+3);
                      x+=2;

                      for (a=0;a<NUMBER_OF_SKILLS;a++)
                      {
                        set_color(8);
                        print_text(x,y+a+2,"...................");
                        set_color(7);
                        print_number_right_align(x+17,y+a+2,skill[a],3);
                      }

                      set_color(10);
                      print_text(x,y++, "기술");
                      y++;
                      set_color(7);
                      print_text(x,y++,"소형 화기");
                      print_text(x,y++,"대형 화기");
                      print_text(x,y++,"에너지 무기");
                      print_text(x,y++,"격투");
                      print_text(x,y++,"근접 무기");
                      print_text(x,y++,"투척 무기");
                      print_text(x,y++,"의약품 지식");
                      print_text(x,y++,"무기 지식");
                      print_text(x,y++,"외계 과학");
                      print_text(x,y++,"회피");
                      print_text(x,y++,"기계 수리");
                      print_text(x,y++,"프로그램 설계");
					  

/// status info -------------------------------------

     x=INFO_STATUS_X;
     y=INFO_STATUS_Y;

     screen.draw_box(2,x,y,x+15,y+7);
     x+=2;
     set_color(10);
     print_text(x,y++,"상태");
     y++;

     set_color(8);

     char kolory_red[4]= {8,4,12,14};
     char kolory_blue[4]= {8,1,9,11};
     char kolory_green[4]= {8,2,10,14};
     int w;

           if (resist[RESIST_PARALYZE]!=0)
             w=abs(state[STATE_SPEED])/resist[RESIST_PARALYZE];
           else
             w=abs(state[STATE_SPEED]);
       
           if (w>3) w=3;
           if (w==0 && abs(state[STATE_SPEED])>0) w=1;
           set_color( kolory_blue[w] );
           print_text(x,y++,"마비");


           if (resist[RESIST_CHEM_POISON]!=0)
			   w=state[STATE_CHEM_POISON]/resist[RESIST_CHEM_POISON];
           else
             w=state[STATE_CHEM_POISON];
       
           if (w>3) w=3;
           if (w==0 && state[STATE_CHEM_POISON]>0) w=1;
           set_color( kolory_green[w] );
           print_text(x,y++,"중독");


           
		   if (resist[RESIST_RADIOACTIVE]!=0)
			   w=state[STATE_RADIOACTIVE]/resist[RESIST_RADIOACTIVE];
           else
             w=state[STATE_RADIOACTIVE];
       
           if (w>3) w=3;
           if (w==0 && state[STATE_RADIOACTIVE]>0) w=1;
           set_color( kolory_green[w] );
           print_text(x,y++,"방사능 오염");


           if (resist[RESIST_BLIND]!=0)
             w=state[STATE_BLIND]/resist[RESIST_BLIND];
           else
             w=state[STATE_BLIND];
       
           if (w>3) w=3;
           if (w==0 && state[STATE_BLIND]>0) w=1;
           set_color( kolory_blue[w] );
           print_text(x,y++,"시각 마비");           


		   w=state[STATE_TEMPERATURE];
		   if (w>0)
		   {
				if (resist[RESIST_FIRE]!=0)
					w=w/resist[RESIST_FIRE];
		   }
       
           if (w>3) w=3;
           if (w==0 && state[STATE_TEMPERATURE]>0) w=1;
           set_color( kolory_red[w] );
		   if (state[STATE_TEMPERATURE]>0)
				print_text(x,y++,"화상");
		   else
				print_text(x,y++,"동상");


           // slowed lub speeded up

/// backpack, weapon info -------------------------------------

     x=INFO_INVENTORY_X;
     y=INFO_INVENTORY_Y;

     screen.draw_box(2,x,y,x+27,y+5);
     x+=2;
     set_color(10);
     print_text(x,y++, "소지품");
     y++;

     for (a=0;a<3;a++)
     {
       set_color(8);
       print_text(x,y+a,".......................");
     }

       set_color(7);
       print_number_right_align(x+22,y,this->items_in_backpack,2);
       print_text(x,y++,"소지품 개수");
       print_number_right_align(x+22,y,this->get_backpack_weight(),2);
       print_text(x,y++,"소지품 중량");
       print_number_right_align(x+22,y,this->get_monster_capacity(),2);
       print_text(x,y++,"한계 중량");
     


     // print resist info -------------------

     x=INFO_RESISTS_X;
     y=INFO_RESISTS_Y;

     screen.draw_box(2,x,y,x+20,y+11);
     x+=2;
     set_color(10);
     print_text(x,y++, "저항력");

     for (a=0;a<NUMBER_OF_RESISTS;a++)
     {
            set_color(8);
            print_text(x,y+a+1,"................");
            set_color(7);
            print_number_right_align(x+14,y+a+1,resist[a],3);
     }
     set_color(7);
     y++;
     print_text(x,y++,"방사능");
     print_text(x,y++,"독");
     print_text(x,y++,"기절");
     print_text(x,y++,"마비");
     print_text(x,y++,"고온");
     print_text(x,y++,"저온");
     print_text(x,y++,"전기");
     print_text(x,y++,"시각 마비");
     print_text(x,y++,"혼란");


/// print experience info -------------------------------------

     x=INFO_EXPERIENCE_X;
     y=INFO_EXPERIENCE_Y;

     screen.draw_box(2,x,y,x+27,y+7);
     x+=2;
     set_color(10);
     print_text(x,y++, "경험");
     y++;

     for (a=0;a<5;a++)
     {
       set_color(8);
       print_text(x,y+a,".......................");
     }

       set_color(7);
       print_number_right_align(x+22,y,exp_level,2);
       print_text(x,y++,"레벨");
       print_number_right_align(x+22,y,experience,2);
       print_text(x,y++,"현재 경험");
       print_number_right_align(x+22,y,level_at_experience,2);
       print_text(x,y++,"다음 레벨 업");
       print_number_right_align(x+22,y,free_skill_pointes,2);
       print_text(x,y++,"남은 기술 점수");
       print_number_right_align(x+22,y,adaptation_points,2);
       print_text(x,y++,"남은 유전자 점수");
	   
}

void HERO :: show_player_info()
{
     show_info();
     myrefresh();
	 keyboard.wait_for_key();
}


TIME HERO :: do_action( ACTION action )
{
   int x,y;
   int returned;
   int key;
   string text;

   if (action==ACTION_NOTHING)
	   return 0;

   RANGED_WEAPON *ranged_weapon = weapon->IsShootingWeapon();

   last_pX = pX();
   last_pY = pY();   

   switch (action)
   {
      case ACTION_WAIT_STUNNED:
 					  return state[STATE_STUN];
      case ACTION_MOVE:
                      screen.draw_monsters(); // To make monsters know where player went
                      returned=move();
                      if (returned==1 || returned==3) // move or attack
                        return (TIME_MOVE/speed.GetValue())+1;
                      else if (returned==2) // can be opened
                      {
                        cell_in_current_direction(x,y);
						if (action_to_repeat!=REPEAT_ACTION_EXPLORE)
						{
							screen.console.add_and_zero(level.map.getCellName(x,y) + "을 열까? (y/n)",7);
							returned=keyboard.wait_for_key();
						}
						else
							returned='y';

                        if (returned=='y' || returned=='Y')
                        {
                          if (open_in_direction()==true)
                            return (TIME_MOVE/speed.GetValue())+1;
                        }
                        else
                        {
                          screen.console.clean();
                          screen.console.show();
                          return 0;
                        }
                      
                      }
                      else
                      {
                        screen.console.add("그곳은 막혀 있다.\n",7);
                        return 0;
                      }
      case ACTION_WAIT:
                      return (TIME_MOVE/speed.GetValue())+1;
      case ACTION_GET:
                      if (pick_up_item ( choose_item_to_pick_up(),true)==true)
                        return (TIME_MOVE/speed.GetValue())+1;
                      else
                        return 0;
      case ACTION_INVENTORY:
					  inventory_repeat_break=false;
                      if (show_inventory()==true)
					  {
						  if (!inventory_repeat_break)
							  repeat_action(REPEAT_ACTION_INVENTORY);
						  return (TIME_MOVE/speed.GetValue())+1;
					  }
                      else
					  {
						  stop_repeating();
						  return 0;
					  }
      case ACTION_FIRE:
                      if (!weapon->IsShootingWeapon())
                      {
                        screen.console.add(weapon->article_the() + weapon->show_name() + "을 발사할 수 없다.\n",7);
                        return 0;
                      }
                        
                      // if enemy targeted, then fire
                      if (enemy==NULL)
                        enemy=choose_target(&x,&y, weapon->show_name() + "을 발사할 목표를 선택하라.\n");
                      if (enemy!=NULL)
                      {
                         x=enemy->pX();
                         y=enemy->pY();
						 if (enemy==level.player)
							 enemy=NULL;
                      }
                      
					  if (x!=-1 && y!=-1)
					  {
						  shoot_into(x,y);						  
						  return (TIME_MOVE/speed.GetValue())+1;
					  }
					  return 0;
      case ACTION_SHOW_INFO:
                      show_player_info();
                      return 0;
      case ACTION_SHOW_HELP:
                      screen.clear_all();
                      set_color(11);                      
                      print_text(30,0,"제노사이드: 더 로그라이크");
                      set_color(15);
                      print_text(1,2 ,"기본 조작 키:");
                      print_text(1,4 ,"c - 문 닫기");
                      print_text(1,5 ,"e - 주 무기 / 보조 무기 전환");
                      print_text(1,6 ,"f - 조준 및 발사");
                      print_text(1,7 ,"g - 물건 줍기");
                      print_text(1,8 ,"i - 소지품 관리");
                      print_text(1,9 ,"l - 살펴보기");
                      print_text(1,10,"n - 인접한 적을 순서대로 조준");
                      print_text(1,11,"o - 문 열기, 로봇 켜기 / 끄기");
                      print_text(1,12,"r - 무기 재장전");
                      print_text(1,13,"t - 가장 최근에 활성화한 아이템 던지기");
                      print_text(1,14,"A - 근처의 적 및 사물 공격");
                      print_text(1,15,"M - 출력된 메시지 다시보기");
                      print_text(1,16,"Q - 게임 종료");
                      print_text(1,17,"R - 100 턴 동안 휴식 / 생명을 다 회복했거나 일이 생기면 중단됨");
                      print_text(1,18,"S - 저장 및 종료");
                      print_text(1,19,"escape - 조준 취소, 화면 빠져나가기");
                      print_text(1,20,"! - 옵션 설정");
                      print_text(1,21,"@ - 주인공 정보");
                      print_text(1,22,"] - 다음 발사 방식으로 전환");
                      print_text(1,23,"[ - 이전 발사 방식으로 전환");
                      print_text(1,24,"; - 무기 작동");
                      print_text(1,25,"' - 갑옷 작동");
                      print_text(1,26,"< - 올라가기");
                      print_text(1,27,"> - 내려가기");
					  print_text(1,28,"E - 자동 탐색 (시험중)");
					  print_text(1,29,"X - 자동 이동 (시험중)");
					  print_text(1,30 ,"L - 시야 내의 사물 출력");
                      print_text(1,35,"F11 - HTML 스크린샷 찍기");
                      print_text(1,48,"소지품창에서 아이템 메뉴를 열려면 아이템의 문자를 입력하라.");
                      myrefresh();
                      keyboard.wait_for_key();
                      screen.clear_all();
                      screen.draw2();
                      return 0;
      case ACTION_SHOW_MESSAGE_BUFFER:
					  screen.console.show_message_buffer(false);
					  return 0;
      case ACTION_INC_FIRE:						
                      if (ranged_weapon!=NULL)
                      {
                        ranged_weapon->incrase_fire_mode();
                        show_attributes();
                        myrefresh();
                      }
                      return 0;
      case ACTION_DEC_FIRE:
                      if (ranged_weapon!=NULL)
                      {
                        ranged_weapon->decrase_fire_mode();
                        show_attributes();
                        myrefresh();
                      }
                      return 0;
      case ACTION_REST:
				      repeat_action(REPEAT_ACTION_REST);
					  turns_of_rest = 100;
					  return 0;
	  case ACTION_EXPLORE:
					  repeat_action(REPEAT_ACTION_EXPLORE);
					  return 0;
	  case ACTION_TRAVEL:
  					  if (choose_travel_destination())
						  repeat_action(REPEAT_ACTION_TRAVEL);
					  return 0;
      case ACTION_OPTIONS:
                       screen.clear_all();
					   set_color(15);
					   print_text(1,1,"옵션");
					   set_color(9);
					   print_text(1,25,"[D] 현재 설정을 기본값으로 저장함");
					   set_color(14);
					   print_character(2,25,'D');
					   key=0;					   
                       while(1)
                       {
                         if (key==keyboard.escape)
                         {
						   TOSAVE::SaveOptions(string("save/") + name + ".opt");
                           break;
                         }
                         if (key=='d' || key=='D')
                         {
							 TOSAVE::SaveOptions("default.opt");
						 }							 
                         if (key-'0'>=0 && key-'0'<NUMBER_OF_OPTIONS)
                         {
                            options.number[key-'0']=!options.number[key-'0'];
                         }
                         for (int zz=0;zz<NUMBER_OF_OPTIONS;zz++)
                         {
                             switch(zz)
                             {
                               case OPTION_LETTERS:
									text=string("0: 소지품의 문자를 자동으로 정렬하지 않기: ");
                               break;
                               case OPTION_EXPERIENCE:
									text=string("1: 현재 경험 대신에 남은 경험 표시: ");
                               break;
                               case OPTION_AUTO_AIM:
								   text=string("2: 인접한 적을 자동으로 조준: ");
								   break;
                               case OPTION_DONT_LET_SWAP:
								   text=string("3: 동료들이 주인공과 위치를 바꾸지 못하도록 설정: ");
								   break;
                               case OPTION_CURSOR:
								   text=string("4: 주인공에게 커서 출력: ");
								   break;
                               case OPTION_GETALL:
								   text=string("5: 여러개의 물건이 있을 때 한꺼번에 줍기: ");
								   break;
                               case OPTION_AIM_IF_NO_ENEMY:
								   text=string("6: 적을 선택하지 않았을 때 자동으로 조준: ");
								   break;
                             }
                             if (options.number[zz]==true)
                               text+="사용    ";
                             else
                               text+="사용안함";
                             set_color(9);                               
                             print_text(1,5+zz,text);
                             set_color(14);                               
                             print_number(1,5+zz,zz);
                         } // endof for
                         myrefresh();
                         key=keyboard.wait_for_key();
                       } // endof while
                        screen.draw2();
                        return 0; 
      
      case ACTION_RELOAD:
                      if (reload_weapon()==true) // gdy pusty magazynek
                         return (TIME_MOVE/speed.GetValue())+1;
					  else if (ranged_weapon!=NULL) // gdy ma amunicje - wyladowanie jej
					  {
						  if (ranged_weapon->property_unload_ammo() && ranged_weapon->unload_ammo(this)==true)
						  {
							screen.console.zero();
							screen.console.add(name + "은 " + ranged_weapon->show_name() + "에 장전된 탄을 제거한다.\n",3,false);
							return (TIME_MOVE/speed.GetValue())+1;
						  }
					  }					  
                      else
                      {
                         screen.console.add(weapon->article_the() + weapon->show_name() + "에 탄을 장전할 수 없다.\n",7,false);
                         return 0;
                      }
					  return 0;					  
      case ACTION_QUIT:
                      death();
      case ACTION_SAVE:
                      game.SaveGame();
					  return 0;
      case ACTION_STAIRS_UP:
					  if (use_stairs(true)==true)
						return TIME_LEVEL_CHANGE;
					  return 0;
      case ACTION_STAIRS_DOWN:
					  if (use_stairs(false)==true)
						return TIME_LEVEL_CHANGE;
					  return 0;
      case ACTION_LOOK:
                      look_with_cursor();
                      return 0;
      case ACTION_ACTIVATE_ARMOR:
		  if (armor->property_activate())
		  {
			  screen.console.add(name + "은 " + armor->show_name() + "을 작동시킨다.\n",3,false);
			  armor->activate();
			  return (TIME_MOVE/speed.GetValue())+1;
		  }		  
		  return 0;
      case ACTION_ACTIVATE_WEAPON:
		  if (weapon->property_activate())
		  {
			  screen.console.add(name + "은 " + weapon->show_name() + "을 작동시킨다.\n",3,false);
			  weapon->activate();
			  return (TIME_MOVE/speed.GetValue())+1;
		  }		  
		  return 0;
      case ACTION_THROW:
                      if (last_activated!=NULL)
                      {
                         if (last_activated->property_throw())
                         {
                           struct Screen_copy scr_copy;
                           store_screen(&scr_copy);
						   screen.clear_all();
                           screen.draw2();						   
                           choose_target(&x,&y,last_activated->article_the() + last_activated->show_name() + " 던지기.\n");
                           if (x==-1 || y==-1)
                           {
                              restore_screen(&scr_copy);
                              myrefresh();
                              return 0;
                           }
                           throw_item_to(x,y,last_activated);
                           last_activated=NULL;
                           return (TIME_MOVE/speed.GetValue())+1;
                         }
                      }
                      last_activated=NULL;                      
                      return 0;
                      
                      
      case ACTION_OPEN:
                      screen.console.add_and_zero("어느 방향에 있는 것을 열까?",7);
      
                      if (set_move_direction()==true)
                      {
						int ox,oy;
						ROBOT *rob;
						MONSTER *monst;
						ITEM *przedm;
						ROBOT_SHELL *powl;
						cell_in_current_direction(ox,oy);
						ptr_list items_z_pola;
						ptr_list robot_shells;
						ptr_list::iterator om,_om;

						// is it a turning robot on/off?
						if (level.map.IsMonsterOnMap(ox,oy))
						{
							monst=level.monster_on_cell(ox,oy);
							rob = monst->IsRobot();
							if (rob!=NULL)
							{
								if (rob->is_enemy(this))
								{
									rob->tries_to_turn_off(this);
									return (TIME_MOVE/speed.GetValue())+1;								
								}
								else // friendly
								{
									build_robot(rob->shell);
									return (TIME_MOVE/speed.GetValue())+1;								
								}
							}
						}
						// or maybe robot building?
						robot_shells.clear();
						level.list_of_items_from_cell(&items_z_pola,ox,oy);
						if (items_z_pola.size()>0)
						{
							for (om=items_z_pola.begin(),_om=items_z_pola.end();om!=_om;om++)
							{
								przedm = (ITEM *) *om;
								powl = przedm->IsRobotShell();
								if (powl!=NULL)
									robot_shells.push_back(powl);
							}
							if (robot_shells.size()==1)
							{
								powl = (ROBOT_SHELL *) *robot_shells.begin();
								int wynik_akcji = build_robot(powl);
								switch(wynik_akcji) {
								case 1: // budowa robota
									return (TIME_MOVE/speed.GetValue())+1;
								case 2: // show_inventory
									return (TIME_MOVE/speed.GetValue())+1;
								default:;
									return (TIME_MOVE/speed.GetValue())+1;
								}
																	
							}
							else if (robot_shells.size()>1)
							{
								przedm = choose_item_from_list(robot_shells,"Choose a shell to assembly a new robot");
								if (przedm!=NULL)
								{
									if (build_robot(przedm)==true)
										return (TIME_MOVE/speed.GetValue())+1;								
								}
								else
									return 0;
							}

						}

						// nope, it's door opening
                        if (open_in_direction()==true)
						{
                          return (TIME_MOVE/speed.GetValue())+1;
						}
						else
							screen.console.add("그것은 열 수 없다.",7);                        
                      }
                      return 0;
      case ACTION_CLOSE:
                      screen.console.add_and_zero("어느 방향에 있는 것을 닫을까?",7);
                      if (set_move_direction()==true)
                      {
                        if (close_in_direction()==true)
                          return (TIME_MOVE/speed.GetValue())+1;
                        
                      }
                      screen.console.add("그것은 닫을 수 없다.",7);
                      return 0;
      case ACTION_ATTACK:
					  screen.console.add_and_zero("어느 방향에 있는 것을 공격할까?",7);
					  
					  if (set_move_direction()==true)
					  {
						  if (attack_in_direction()==true)
							  return (TIME_MOVE/speed.GetValue())+1;
						  
					  }
					  return 0;
		  
   }
   return 0;
}


bool HERO :: set_move_direction()
{
   int key;

   while(1)
   {
      key=keyboard.wait_for_key();

      if (key==keyboard.n)
      {
        direction=_kn;
        return true;
      }
      else if (key==keyboard.ne)
      {
        direction=_kne;
        return true;
      }
      else if (key==keyboard.e)
      {
        direction=_ke;
        return true;
      }
      else if (key==keyboard.se)
      {
        direction=_kse;
        return true;
      }
      else if (key==keyboard.s)
      {
        direction=_ks;
        return true;
      }
      else if (key==keyboard.sw)
      {
        direction=_ksw;
        return true;
      }
      else if (key==keyboard.w)
      {
        direction=_kw;
        return true;
      }
      else if (key==keyboard.nw)
      {
        direction=_knw;
        return true;
      }
      else if (key==keyboard.escape || key==keyboard.readmore)
        return false;
   } // endof while
}


void HERO :: look_with_cursor()
{
   int key;
   int x,y;
   int last_x,last_y;
   struct Screen_copy scr_copy;
   MONSTER *monst=NULL;
   ptr_list on_cell;
   ptr_list::iterator m,_m;
   ITEM *temp;
   int changex, changey;
   
   if (enemy!=NULL)
   {
     x=enemy->pX();
     y=enemy->pY();
   }
   else
   {
     x=this->pX();
     y=this->pY();
   }
   last_x=-1; last_y=-1;
   
   screen.console.zero();
   screen.console.clean();
   screen.console.show();
   
   store_screen(&scr_copy); // remember screen

   changex=0;
   changey=0;
   key=0;
   
   while(1)
   {
      x+=changex;
      y+=changey;
      if (key==keyboard.escape)
      {
         restore_screen(&scr_copy);
         screen.console.clean();
         screen.console.zero();
         screen.console.show();
         return;
      }
      else if (key=='m' || key=='M')
      {
		  if (monst!=NULL)
		  {
			  monst->ShowDescription();
		  }
      }
      else if (key==keyboard.look || key==keyboard.readmore ||
               key==keyboard.readmore2)
      {
         restore_screen(&scr_copy);
         screen.console.clean();
         screen.console.zero();
         screen.console.show();
         return;
      }

     restore_screen(&scr_copy);
      
     // limit of movement
     if (x<0 || x>79 || y<0 || y>39)
     {
       x=last_x; y=last_y;
     }
     else
     {
       if (x!=last_x || y!=last_y)
       {
         on_cell.clear();
         restore_screen(&scr_copy);
         screen.console.zero();
         screen.console.clean();
         screen.console.show();
         store_screen(&scr_copy);
         if (!(level.map.seen_by_player(x,y) || level.map.seen_by_camera(x,y)))
         {
           screen.console.add("이곳은 보이지 않는다.",7);
           screen.console.show();
           myrefresh();
           store_screen(&scr_copy);           
         }
         else
         {
            // look on a new place
            // is it a monster?
			 ROBOT *rob;
            monst=level.monster_on_cell(x,y);
            level.list_of_items_from_cell(&on_cell,x,y);
            if (monst!=NULL)
            {
				screen.console.add(string(" ") + monst->name,7);
				rob = monst->IsRobot();
				if (!rob)
				{
					screen.console.add(string(" / 무기: ") + monst->weapon->show_name() ,7);
					screen.console.add(string(" / 갑옷: ") + monst->armor->show_name() + "\n",7);
					screen.console.add(string("생명: ") + IntToStr(100*monst->hit_points.val/monst->hit_points.max) + "% ",7);
				}
				else
				{
					screen.console.add(string("\n피해: ") + IntToStr(100-(100*rob->shell->hit_points/rob->shell->max_hit_points)) + "% ",7);
				}
               screen.console.add("\n-- 자세한 정보를 보려면 'M'키를 눌러라. --",7);
               screen.console.show();
               screen.console.zero();
               myrefresh();
               store_screen(&scr_copy);
            }
            else if (on_cell.size()!=0) // items
            {
               if (on_cell.size()==1)
                  screen.console.add("이곳에 놓인 물건:",7);
               else
                  screen.console.add("이곳에 놓인 물건:",7);
               for(m=on_cell.begin(),_m=on_cell.end(); m!=_m;)
               {
                  temp=(ITEM *)*m;
                  if (++m!=_m)
                     screen.console.add(temp->show_name()+",",temp->color);
                  else
                     screen.console.add(temp->show_name()+".",temp->color);
               }
               screen.console.show();
               screen.console.zero();
               myrefresh();
               store_screen(&scr_copy);
            }
            else // terrain
            {
               screen.console.add(level.map.getCellName(x,y),7);
               screen.console.add(string("| ") + IntToStr(level.map.getPercentDamage(x,y)) + "% of damage.",7);
               screen.console.show();
               screen.console.zero();
               myrefresh();
               store_screen(&scr_copy);
            }
         } // you see this

       }
       last_x=x; last_y=y;
     }
     print_character(x,y,'?');
     myrefresh();

	 key = keyboard.get_direction(changex, changey);
     
   } // endof while
}


void HERO :: show_backpack(ITEM *passed_item, EInvShowType print_type)
{
   ptr_list::iterator m,_m;
   ITEM *temp;
   int pos;
   screen.clear_all();
   set_color(7);
   int print_count;
   char *letters="abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
   
   bool used;
   if (this->items_in_backpack==0)
   {
       print_text(1,2,string("가방은 비어 있다."));
       myrefresh();
       return;
   }
   used=false;
   print_count=0;
   string text;
   set_color(2);
   print_text(0,0, "--------------------------------------------------------------------------------");
   print_text(0,49,"--------------------------------------------------------------------------------");
   set_color(10);   

   string total_weight = IntToStr(get_backpack_weight());
   string total_capacity = IntToStr(get_monster_capacity());

   text =  string(" [ ") +  total_weight + "/" + total_capacity + " ] ";
   print_text(40-text.size()/2,49,text);
   if (isStrained())
   {
	   set_color(4);
   }
   else if (isBurdened())
   {
	   set_color(14);
   }
   print_text(40-text.size()/2+3,49,total_weight);

   pos=2;

   string addit_hit_points;
			   
   if ( !this->is_unarmed() )
   {
	   set_color(15);

	   if (print_type == INV_SHOW_REPAIR_KIT ||
		   print_type == INV_SHOW_MATCH_ITEM_TO_REPAIR)
	   {
		   addit_hit_points = string(" <") + IntToStr(100*weapon->hit_points/weapon->max_hit_points) + "%>";
		   if (weapon->hit_points!=weapon->max_hit_points)
			   set_color(7);
		   else
			   set_color(8);
	   }

	   
	   if (options.number[OPTION_LETTERS]==false)
		   this->weapon->inventory_letter=letters[print_count];
	   this->weapon->print_item_with_letter(1,pos,string("  (주 무기)") + addit_hit_points);
	   used=true;
	   pos++;
	   print_count++;
   }
   if ( this->in_armor() )
   {
	   set_color(15);

	   if (print_type == INV_SHOW_REPAIR_KIT ||
		   print_type == INV_SHOW_MATCH_ITEM_TO_REPAIR)
	   {
		   addit_hit_points = string(" <") + IntToStr(100*armor->hit_points/armor->max_hit_points) + "%>";
		   if (armor->hit_points!=armor->max_hit_points)
			   set_color(7);
		   else
			   set_color(8);
	   }
		   
	   
	   if (options.number[OPTION_LETTERS]==false)
		   this->armor->inventory_letter=letters[print_count];
	   this->armor->print_item_with_letter(1,pos,string("  (갑옷)") + addit_hit_points);
	   used=true;
	   pos++;
	   print_count++;
   }
   if ( this->ready_weapon!=NULL )
   {
	   set_color(7);

	   if (print_type == INV_SHOW_REPAIR_KIT ||
		   print_type == INV_SHOW_MATCH_ITEM_TO_REPAIR)
	   {
		   addit_hit_points = string(" <") + IntToStr(100*ready_weapon->hit_points/ready_weapon->max_hit_points) + "%>";
		   if (ready_weapon->hit_points!=ready_weapon->max_hit_points)
			   set_color(7);
		   else
			   set_color(8);
	   }
		   
	   
	   if (options.number[OPTION_LETTERS]==false)
		   ready_weapon->inventory_letter=letters[print_count];
	   ready_weapon->print_item_with_letter(1,pos,string("  (보조 무기)") + addit_hit_points) ;
	   used=true;
	   pos++;
	   print_count++;
   }
   
   if (used==true)
   {
	   pos++;
	   set_color(2);
	   print_text(0,pos, "--------------------------------------------------------------------------------");
	   set_color(10);
	   print_text(30,0," 사용 중인 장비 ");
	   print_text(35,pos," 소지품 ");
	   pos+=2;
   }
   else
   {
	   set_color(10);
	   print_text(35,0," 소지품 ");
   }

   for(m=backpack.begin(),_m=backpack.end(); m!=_m; m++)
   {
       temp=(ITEM *)*m;
       if (temp!=this->weapon && temp!=this->armor && temp!=ready_weapon) // juz wypisane
       {
		   AMMO *am;
		   RANGED_WEAPON *ranged_weapon;
		   REPAIR_KIT *repset;
		   BASE_ARMOR *armor;

		   switch (print_type)
		   {
		   case INV_SHOW_MATCH_AMMO:
			   set_color(8);
			   am = temp->IsAmmo();
			   ranged_weapon = passed_item->IsShootingWeapon();
			   
			   if (am!=NULL)
			   {
				   if (am->ammo_type==ranged_weapon->ammo_type)
					   set_color(7);
			   }
			   break;
		   case INV_SHOW_HIGHLIGHT_CORPSES:
			   if (temp->IsCorpse()!=NULL)
				   set_color(7);
			   else
				   set_color(8);
			   break;
		   case INV_SHOW_MATCH_ITEM_TO_REPAIR:
			   set_color(8);
			   repset=temp->IsRepairSet();
				   
			   if (((repset!=NULL && repset->can_be_used()) || (passed_item->IsArmor() && temp->IsArmor() && temp!=passed_item)))
			   {
					   set_color(7);
			   }
			   break;			   
		   case INV_SHOW_REPAIR_KIT:
			   
			   if (!temp->IsCountable() && temp->hit_points!=temp->max_hit_points)
				   set_color(7);
			   else
				   set_color(8);			   
			   break;			   
		   case INV_SHOW_IMPROVE_ARMOR:

			   set_color(8);
			   
			   armor = temp->IsArmor();
			   if (armor!=NULL)
			   { 
				   if (armor->MOD_DEX<=0 || armor->MOD_STR<=0 || armor->MOD_SPD<=0)
					  set_color(7);
			   }			   
			   break;			   
		   case INV_SHOW_HIGHLIGHT_REAPIR_KITS:			   
			   repset=temp->IsRepairSet();
			   if (repset!=NULL && repset->can_be_used() && repset->purpose==PURPOSE_REPAIR_HP)
				   set_color(7);
			   else
				   set_color(8);			   
			   break;			   
		   default:
			   set_color(7);
		   }

		   if (print_type == INV_SHOW_REPAIR_KIT ||
			   print_type == INV_SHOW_MATCH_ITEM_TO_REPAIR)
			   addit_hit_points = string(" <") + IntToStr(100*temp->hit_points/temp->max_hit_points) + "%>";
		   
         if (options.number[OPTION_LETTERS]==false)
            temp->inventory_letter=letters[print_count];
         temp->print_item_with_letter(1,pos,addit_hit_points);
         pos++;
         print_count++;
       }
   }
   myrefresh();
}

int HERO :: show_inventory()
{
   int character;
   ptr_list::iterator m,_m;
   ITEM *temp;
   string text;
   show_backpack(NULL, INV_SHOW_SIMPLE);
   while (1)
   {
         character=keyboard.wait_for_key();
         if ((character>='a' && character<='z') || (character>='A' && character<='Z'))
         {
            temp=choose_item_from_backpack(character);
            if (temp!=NULL)
            {
               show_actions_of_item(temp);
               if (get_action_for_item(temp)==true)
                 break;
               show_backpack(NULL, INV_SHOW_SIMPLE);
            }
         }
         if (character==keyboard.escape || character==keyboard.readmore || character==keyboard.readmore2)
         {
            return false;
         }
   }
   return true;
}

int HERO :: shoot_into (int x, int y)
{

  RANGED_WEAPON *ranged_weapon = weapon->IsShootingWeapon();
  if (ranged_weapon!=NULL && ranged_weapon->ammo.quantity==0)
  {
        screen.console.add(ranged_weapon->show_name() + "은 탄이 다 떨어졌다.\n",12);
        screen.console.show();

		RANGED_WEAPON *sound=(RANGED_WEAPON *) definitions.find_item(ranged_weapon->name);
		if (sound!=NULL)
			sounds.PlaySoundOnce(sound->sound_gun_empty);
        return false;
  }
  screen.console.clean();
  screen.console.add("사격!\n",3);
  screen.console.show();
  INTELLIGENT :: shoot_into(x,y);
  return true;
}

MONSTER * HERO :: choose_target(int *target_x, int *target_y, string text)
{
   int key;
   int x,y;
   int last_x,last_y;
   struct Screen_copy scr_copy;
   int dx,dy;
   
   select_closest_enemy();
   
   if (enemy==NULL)
   {
	   x=this->pX();
	   y=this->pY();
   }
   else
   {
	   x=enemy->pX();
	   y=enemy->pY();
   }

   last_x=x; last_y=y;
   
   store_screen(&scr_copy); 
   
   while(1)
   {
	   screen.console.add_and_zero(text,7);
	   screen.draw_targeting_line(this->pX(),this->pY(),x,y);
   
      key = keyboard.wait_for_key();
	  dx=0;
	  dy=0;

      if (key==keyboard.n)
      {
		  dy--;
      }
      else if (key==keyboard.ne)
      {
		  dy--;
		  dx++;
      }
      else if (key==keyboard.e)
      {
		  dx++;
      }
      else if (key==keyboard.se)
      {
		  dx++;
		  dy++;
      }
      else if (key==keyboard.s)
      {
		  dy++;
      }
      else if (key==keyboard.sw)
      {
		  dy++;
		  dx--;
      }
      else if (key==keyboard.w)      
      {
		  dx--;
      }
      else if (key==keyboard.nw)
      {
		  dx--;
		  dy--;
      }
	  if (dx!=0 || dy!=0)
	  {
		  select_closest_enemy();
		  x+=dx;
		  y+=dy;
	  }
   
      if (key==keyboard.escape)
      {
         *target_x=-1;
         *target_y=-1;
         restore_screen(&scr_copy);
         myrefresh();
         return NULL;
      }
	  else if (key==keyboard.nearest)
	  {
		  ptr_list::iterator m,_m;
		  MONSTER *temp, *first=NULL, *next=NULL;
		  bool wybierz_kolejnego=false;
		  
		  for(m=level.monsters.begin(),_m=level.monsters.end(); m!=_m; m++)
		  {
			  temp=(MONSTER *)*m;
			  if (temp->seen_now && this->is_enemy(temp))
			  {
				  if (wybierz_kolejnego==true)
				  {
					  next=temp;
					  break;
				  }
				  if (first==NULL)
					  first=temp;
				  if (temp==enemy)
					  wybierz_kolejnego=true;
			  }
		  }
		  if (next!=NULL)
			  enemy=next;
		  else
			  enemy=first;		  

		  if (enemy!=NULL)
		  {
			  x=enemy->pX();
			  y=enemy->pY();
		  }
	  }
      else if (key=='t' || key=='T' || key==keyboard.fire ||
               key==keyboard.readmore || key==keyboard.readmore2)
      {
         *target_x=x;
         *target_y=y;
         restore_screen(&scr_copy);
         myrefresh();
         return level.monster_on_cell(x,y);
      }

     restore_screen(&scr_copy);
      
     // limit of movement
     if (x<0 || x>79 || y<0 || y>39)
     {
       x=last_x; y=last_y;
     }
     else
     {
       last_x=x; last_y=y;
     }     
   } // endof while
}

bool HERO::death()
{
	if (level.ID!="튜토리얼")
	{
		myclear();
		screen.draw();
		screen.console.show();
#ifndef PLAYER_IS_IMMORTAL
		game.DeleteSaveGame();
#endif
		delete_file("death.htm");
		make_screenshot("death",true);
		show_info();
		make_screenshot("death",false);
		screen.console.show_message_buffer(true);
		make_screenshot("death",false);

		myclear();
		screen.draw();
		screen.console.show();

		set_color(14);
		sounds.StopHeartBeat();
		sounds.PlaySoundOnce("data/sounds/other/death.wav");
		print_text(1,1,string("죽었다!"));
		myrefresh();
		keyboard.wait_for_key();

	}
	game.Quit();
	return true;
}

void HERO :: select_closest_enemy()
{
   int min_distance;
   int dist;
   MONSTER *chosen_one=NULL;
   
   ptr_list::iterator m,_m;
   MONSTER *temp;

   if (enemy==NULL)
   {
	   min_distance=10000;
   }
   else
   {
	   if (abs(pX()-enemy->pX())<2 && abs(pY()-enemy->pY())<2) // not to change monsters near by
		   min_distance=1;
	   else
		   min_distance=distance(enemy->pX(),enemy->pY(),pX(),pY());
	   chosen_one=enemy;
   }

   for(m=level.monsters.begin(),_m=level.monsters.end(); m!=_m; m++)
   {
       temp=(MONSTER *)*m;
       if (temp!=level.player && is_enemy(temp)==true && temp->seen_now)
       {
			 dist = distance(temp->pX(),temp->pY(),pX(),pY());
             
             if (dist<min_distance)
             {
               min_distance=dist;
               chosen_one=temp;
             }
       }
   }
   enemy=chosen_one;
}

bool HERO::use_stairs(bool direction)
{
	int x,y;
	STAIRS * stairs = level.map.StairsAt(pX(),pY());
	if (stairs==NULL)
		return false;

	if (stairs->lead_up==direction)
	{
		if (stairs->to_where == "유전자 조작기")
		{
			places.GeneticLaboratory();
			return true;
		}
		// blok the cell - monsters will not enter the stairs
		x = level.player->pX();
		y = level.player->pY();
		
		level.map.setBlockMove(x,y);
		
		stairs_number_used=stairs->number;
		ID_of_last_level=level.ID;
		level.ChangeLevelTo(stairs->to_where);
		wait(5);
		return true;
	}

	return false;
}

void HERO::train_skill(int a_skill, int value)
{
	int ceiling;
	int procents;
	experience_in_skill[a_skill]+=value;
	ceiling = (skill[a_skill]/10)*(SKILL_MULTIPLIER*8) + 1; 
	skill[a_skill] = (skill[a_skill]/10) * 10;
	procents = experience_in_skill[a_skill]*10/ceiling; 
	if (procents>10)
		procents=10;
	skill[a_skill] += procents;
	if (procents==10) // next 10% achieved
	{
		experience_in_skill[a_skill]=0;

		if (a_skill==SKILL_UNARMED)
		{
			unarmed.DMG++;
			if ((skill[a_skill]/10)%2 == 0)
			{
				unarmed.HIT++;
				unarmed.DEF++;
			}
		}
	}
}

void HERO::skill_used(int a_skill)
{
	if (free_skill_pointes==0)
		return;

	int random_value = skill[a_skill]/10 * SKILL_MULTIPLIER +1;

	if (random_value>free_skill_pointes)
		random_value=free_skill_pointes;

	random_value = random(random_value);

	free_skill_pointes-=random_value;
	train_skill(a_skill,random_value);
}

int HERO::build_robot(ITEM *item)
{
	ROBOT_SHELL *shell;

	shell = item->IsRobotShell();
	if (shell==NULL) 
		return false;

	string temp;

	// print robot parts
	int character;
	int _state = false;
	string str;

	show_robot_build_screen(shell,-1);

	while (1)
	{
		character=keyboard.getkey();
		if (character=='o' || character=='O')
		{
			// 0 if not robot
			// 1 if done
			// 2 if something done in inventory 
			// 3 if robot turned off
			// -1 if not enough skill to turn
			// -2 if robot has no CPU
			// -3 if now place around to set robot as monster

			if (shell->owner==NULL || (shell->owner!=NULL && !shell->owner->IsRobot()))
			{
				_state = turn_robot_on(shell);
			}
			else // turn off
			{
				ROBOT *rob;
				rob = shell->owner->IsRobot();
				
				rob->tries_to_turn_off(this);
				return 3;
			}

			set_color(4);
			if (_state==1) // wlaczono, wiec przerywamy
			{
				for (int a=0;a<2;a++)
					skill_used(SKILL_MECHANIC);
				break;
			}
			else if (_state==-2)
			{           
				str = " 로봇이 작동하려면 CPU가 필요하다! ";
				print_text(40-str.size()/2,0,str);
			}
			else if (_state==-3)
			{
				str = " 로봇을 작동시킬 공간이 없다. ";
				print_text(40-str.size()/2,0,str);
			}
			else if (_state==-1) // no skill
			{
				str = " 현재 너의 능력으로는 이 로봇을 작동시킬 수 없다! ";
				print_text(40-str.size()/2,0,str);
				skill_used(SKILL_MECHANIC);				
			}
			keyboard.wait_for_key();
			show_robot_build_screen(shell,-1);							
		}
		else if (character=='r' || character=='R')
		{
			set_color(11);
			print_text(7,2,"                                                         ");
			shell->last_robot_name = keyboard.get_name(7,2,40);
			show_robot_build_screen(shell,-1);				
		}
		else if (character>='a' && character<='l')
		{
			if (shell->owner==NULL || (shell->owner!=NULL && !shell->owner->IsRobot()))
			{
				build_robot_change_field_number(shell,character-'a');
				show_robot_build_screen(shell,-1);				
			}
		}
		else if (character=='m' || character=='M')
		{
			show_robot_build_screen(shell,2);	
			set_color(14);
			print_character(6,4,'S');			
			if (build_robot_fix(shell))
				return 2;
			show_robot_build_screen(shell,-1);	
		}		
		else if (character==keyboard.escape)
			break;
	}
	return _state==1;
}

bool HERO::build_robot_fix(ROBOT_SHELL *shell)
{
	char character;
	int field_number, kind=0, index;
	ptr_list::iterator m;
	ITEM *item=NULL;
	set_color(10);
	print_text(25,0," 어느 부품을 수리할 것인가 ");
	myrefresh();			 
	// choose what to fix
	while (1)
	{
		character=keyboard.wait_for_key();
		if (character>='a' && character<='l')
		{
			field_number = character-'a';
			if (field_number>=shell->max_number_action_slots + shell->max_number_move_slots + 1)
				continue;
			
			if (field_number!=0) // not CPU
			{
				// check kind of field to fix
				if ( field_number-1 < shell->max_number_action_slots)
				{
					kind = 1;
					if (field_number-1 < shell->action_slots.size())
						kind = -1;
				}
				else
				{
					if ( field_number-1 - shell->max_number_action_slots < shell->max_number_move_slots)
					{
						kind = 2;
						if (field_number-1 - shell->max_number_action_slots < shell->move_slots.size())
							kind = -2;
					}			
				}
				
			}
			
			// CPU choosen
			if (field_number==0)
			{
				if (shell->cpu!=NULL)
				{
					item=shell->cpu;
					break;			
				}
			}		
			else if (kind==-1) // action slot selected 
			{
				for (index=0,m=shell->action_slots.begin();index<field_number-1;index++)
					m++;
				
				// fix this item
				item = (ITEM *) *m;
				break;
			}
			else if (kind==-2) // move slot selected
			{
				for (index=0,m=shell->move_slots.begin();index<field_number-1-shell->max_number_action_slots;index++)
					m++;
				
				// fix this item
				item = (ITEM *) *m;
				break;
			}
			
		}
		else if (character=='s')
		{
			item=shell;
			break;
		}
		else if (character==keyboard.escape || character==keyboard.readmore || character==keyboard.readmore2)
		{
			return false;
		}
	} // endof while(1)
	

	if (item==NULL)
		return false;
	// choose the repair kit
	ITEM *temp;
	show_backpack(NULL, INV_SHOW_HIGHLIGHT_REAPIR_KITS);
	set_color(10);
	print_text(25-item->show_name().size()/2,0,string(" 어떤 도구로 ") + item->show_name() + "을 수리할 것인가? ");
	myrefresh();			 
	while (1)
	{
		character=keyboard.wait_for_key();
		if ((character>='a' && character<='z') || (character>='A' && character<='Z'))
		{
			temp=choose_item_from_backpack(character);
			if (temp!=NULL) // when selected
			{
				REPAIR_KIT *repset;
				repset=temp->IsRepairSet();
				if (repset!=NULL && repset->can_be_used())
				{
					if (repset->fix(item,this))
						return true;
				}
			}
		}
		else if (character==keyboard.escape || character==keyboard.readmore || character==keyboard.readmore2)
		{
			return false;
		}
	} // endof while(1)

	return false;
}

void HERO::show_robot_build_screen(ROBOT_SHELL *shell, int show_letters)
{
	if (show_letters==-1)
	{
		if (shell->owner==NULL || (shell->owner!=NULL && !shell->owner->IsRobot()))
			show_letters=true;
	}

	myclear();
	string temp = " 로봇 조립 ";
    set_color(2);
    print_text(0,0, "--------------------------------------------------------------------------------");
    print_text(0,49,"--------------------------------------------------------------------------------");
    set_color(10);
	print_text(40-temp.size()/2,0,temp);

	set_color(7);	
	print_text(1,2,"이름: ");

	set_color(15);
	print_text(7,2,shell->last_robot_name);
	
	set_color(7);
	print_text(1,4,"본체: ");
	shell->draw_at(8,4);
	set_color(15);
	print_text(10,4,shell->name + " <" + IntToStr((100*shell->hit_points)/shell->max_hit_points) + "%>");

	set_color(7);
	temp = "로봇의 중량: ";
	temp+=IntToStr(shell->calculate_weight());
	print_text(1,6,temp);

	set_color(7);
	if (show_letters==1 || (show_letters==2 && shell->cpu!=NULL))
	{
		print_text(1,9,"CPU: a) ");
		set_color(14);
		print_text(6,9,"a");			
	}
	else
		print_text(1,9,"CPU:");
	
	
	if (shell->cpu!=NULL)
	{
		shell->cpu->draw_at(9,9);		
		set_color(15);	
		print_text(11,9,shell->cpu->show_name() + " <" + IntToStr((100*shell->cpu->hit_points)/shell->cpu->max_hit_points) + "%>");
	}
	else
	{
		temp="-------------------";
		set_color(7);	
		print_text(11,9,temp);
	}
	
	int y=11;
	char character='b';
	
	ITEM *it;
	ptr_list::iterator m;
	int a;
	
	if (shell->max_number_action_slots>0)
	{
		set_color(7);
		print_text(1,y,"[행동 슬롯]");
		y+=2;
		
		m=shell->action_slots.begin();
		for (a=0;a<shell->max_number_action_slots;a++)
		{
			if (a>=shell->action_slots.size())
			{
				set_color(7);
				print_text(6,y,"     -------------------");
			}
			else
			{
				it = (ITEM *) *m;
				it->draw_at(9,y);
				set_color(15);	
				print_text(11,y,it->show_name() + " <" + IntToStr(100*it->hit_points/it->max_hit_points) + "%>");
				m++;
			}

			if (show_letters==1 || (show_letters==2 && a<shell->action_slots.size()))
			{
				set_color(7);
				print_text(6,y," ) ");				
				set_color(14);
				print_character(6,y,character);
			}
			character++;
			y++;
		}
		y++;
	}

	if (shell->max_number_move_slots>0)
	{
		set_color(7);
		print_text(1,y,"[이동 슬롯]");
		y+=2;	
		
		m=shell->move_slots.begin();
		for (a=0;a<shell->max_number_move_slots;a++)
		{
			if (a>=shell->move_slots.size())
			{
				set_color(7);
				print_text(6,y,"     -------------------");
			}
			else
			{
				it = (ITEM *) *m;
				it->draw_at(9,y);
				set_color(15);	
				print_text(11,y,it->show_name() + " <" + IntToStr(100*it->hit_points/it->max_hit_points)+ "%>");
				m++;
			}
			
			if (show_letters==1 || (show_letters==2 && a<shell->move_slots.size()))
			{
				set_color(7);
				print_text(6,y," ) ");				
				set_color(14);
				print_character(6,y,character);
			}				
			character++;
			y++;
		}		
	}	

	// robot printed
	// show options

	screen.draw_box(2,58,3,75,9);

	set_color(6);
	print_text(60,5,"  이름 바꾸기");
	set_color(14);
	print_character(60,5,'R');

	set_color(6);
	if (shell->owner==NULL || (shell->owner!=NULL && !shell->owner->IsRobot()))
		print_text(60,6,"  켜기");
	else
		print_text(60,6,"  끄기");

	set_color(14);
	print_character(60,6,'O');

	set_color(6);
	print_text(60,7,"  수리하기");
	set_color(14);
	print_character(60,7,'M');
}

int HERO::build_robot_change_field_number(ROBOT_SHELL *shell, int field_number)
{
	ptr_list available_items;
	ptr_list available_CPUs;
	ptr_list available_legs;
	ptr_list::iterator m,_m;
	ITEM *temp;
	int index;

	// kind: (change to enums !!!)
	// 1 - free slot action
	// -1 - taken slot action
	// 2 - free slot move
	// -2 - taken slot move
	int kind=0;

	if (field_number>=shell->max_number_action_slots + shell->max_number_move_slots + 1)
		return false;

	if (field_number!=0) // nie CPU
	{
		// sprawdzamy jakiego rodzaju jest number pola w powloce
		if ( field_number-1 < shell->max_number_action_slots)
		{
			kind = 1;
			if (field_number-1 < shell->action_slots.size())
				kind = -1;
		}
		else
		{
			if ( field_number-1 - shell->max_number_action_slots < shell->max_number_move_slots)
			{
				kind = 2;
				if (field_number-1 - shell->max_number_action_slots < shell->move_slots.size())
					kind = -2;
			}			
		}

	}

	// when free, remove
	if (field_number==0)
	{
		if (shell->cpu!=NULL)
		{
			shell->uninstall_CPU();
			return true;			
		}
	}		
	else if (kind==-1) // wybrano zajety slot action. Usuwamy item z niego.
	{
		for (index=0,m=shell->action_slots.begin();index<field_number-1;index++)
			m++;

		shell->uninstall_from_action_slot((ITEM *) *m);
		return true;
	}
	else if (kind==-2) 
	{
		for (index=0,m=shell->move_slots.begin();index<field_number-1-shell->max_number_action_slots;index++)
			m++;
		
		// deinstall this item
		shell->uninstall_from_move_slot((ITEM *) *m);
		return true;
	}

	// when slot is empty
	build_robot_find_available_parts(available_items);
	
	// shell is not avail to install on itself
	available_items.remove(shell);	

	if (field_number==0) // CPU
	{
		for (m=available_items.begin(),_m=available_items.end();m!=_m;m++)
		{
			temp = (ITEM *) *m;
			if (temp->IsRobotCPU())
				available_CPUs.push_back(temp);
		}

		temp = choose_item_from_list(available_CPUs, "설치할 프로세서를 선택하라 (주변의 물건들)");
		shell->install_CPU(temp);
	}
	else if (kind==1) // any item
	{
		temp = choose_item_from_list(available_items, "행동 슬롯에 삽입할 물건을 선택하라 (주변의 물건들)");
		shell->install_in_action_slot(temp);
	}
	else if (kind==2)
	{
		for (m=available_items.begin(),_m=available_items.end();m!=_m;m++)
		{
			temp = (ITEM *) *m;
			if (temp->IsRobotLeg())
				available_legs.push_back(temp);
		}				
		temp = choose_item_from_list(available_legs,"이동 슬롯에 삽입할 물건을 선택하라 (주변의 물건들)");
		shell->install_in_move_slot(temp);
	}
		
	return true;
}

bool HERO::build_robot_find_available_parts(ptr_list &available_items)
{
	ptr_list::iterator m,_m;

	available_items.clear();

	// add items from player's backpack

	for (m=backpack.begin(),_m=backpack.end();m!=_m;m++)
		available_items.push_back(*m);

	// add items from ground

	level.list_of_items_from_cell(&available_items,pX(),pY());

	if (level.entering_on_cell_possible(pX()-1,pY()))
		level.list_of_items_from_cell(&available_items,pX()-1,pY());

	if (level.entering_on_cell_possible(pX()+1,pY()))
		level.list_of_items_from_cell(&available_items,pX()+1,pY());

	if (level.entering_on_cell_possible(pX(),pY()-1))
		level.list_of_items_from_cell(&available_items,pX(),pY()-1);
	
	if (level.entering_on_cell_possible(pX(),pY()+1))
		level.list_of_items_from_cell(&available_items,pX(),pY()+1);


	if (level.entering_on_cell_possible(pX()-1,pY()-1))
		level.list_of_items_from_cell(&available_items,pX()-1,pY()-1);
	
	if (level.entering_on_cell_possible(pX()+1,pY()-1))
		level.list_of_items_from_cell(&available_items,pX()+1,pY()-1);
	
	if (level.entering_on_cell_possible(pX()-1,pY()+1))
		level.list_of_items_from_cell(&available_items,pX()-1,pY()+1);
	
	if (level.entering_on_cell_possible(pX()+1,pY()+1))
		level.list_of_items_from_cell(&available_items,pX()+1,pY()+1);
	
	
	return true;
}

ITEM * HERO::choose_item_from_list(ptr_list &available_items, string header)
{
	struct Screen_copy scr_copy;

	if (available_items.size()==0)
		return NULL;

    store_screen(&scr_copy);

	char *letters="abcdefghijklmnopqrstuvwxyz0123456789";
	
	ITEM *returned=NULL;

	//
	int scroll=0;
	const int on_screen=36;
	int items_left = available_items.size();
	ptr_list::iterator m, first_m;
	ITEM *item;

	int index;
	int key;

	header = string(" ") + header + " ";
	
	m = available_items.begin();
	while (items_left>0)
	{
		myclear();
		set_color(2);
		print_text(0,0, "--------------------------------------------------------------------------------");
		print_text(0,49,"--------------------------------------------------------------------------------");
		set_color(10);
		print_text(40-header.size()/2,0,header);
		
		first_m = m;

		for (index=0; index < min(items_left,on_screen); index++)
		{
			item = (ITEM *) *m;
			set_color(14);
			print_character(1,index+4,letters[index]);
			set_color(7);
			print_character(2,index+4,')');

			set_color(7);			

			string to_show = item->show_name();

			if (item==this->weapon)
				to_show+=" (내 무기)";
			else if (item==this->armor)
				to_show+=" (내 갑옷)";

			print_text(6,index+4,to_show);
			item->draw_at(4,index+4);
			m++;
		}
		if (items_left>on_screen)
		{
			set_color(7);						
			print_text(6,index+6,"[ 계속 >> ]");
		}
		//
		while (1)
		{
			key = keyboard.getkey();
			m = first_m;
			for (int a=0;a<min(items_left,on_screen);a++)
			{
				if (key==letters[a])
				{
					returned = (ITEM *) *m;
					restore_screen(&scr_copy);	
					return returned;
				}
				m++;
			}

			if (key == keyboard.escape || key == keyboard.readmore || key == keyboard.readmore2)
			{
				break;
			}
		}
		items_left-=index;
		scroll+=index;
	}
		
    restore_screen(&scr_copy);	
	return returned;
}

void HERO::add_known_item(string real)
{
	known_items.insert(real);
}

void HERO::add_known_category_of_items(string category_to_add)
{
	list_of_names_for_categories l;
	list_of_names_for_categories::iterator m,_m;	
	string n;
	
	// no such category
	if (definitions.category_number.find(category_to_add)==definitions.category_number.end())
	{
		fprintf(stderr,"\nERROR: Try to learn player about unknown category: %s\n\n",category_to_add.c_str());
		keyboard.wait_for_key();
		exit(32);			
	}

	int kat = definitions.category_number[category_to_add];

	if (definitions.objects_in_category.find(kat)!=definitions.objects_in_category.end())
	{
		l=definitions.objects_in_category[kat];
		for (m=l.begin(),_m=l.end();m!=_m;m++)
		{
			n=*m;
			add_known_item(n);
		}
	}	
}

bool HERO::is_item_known(string real)
{
	if (known_items.find(real)!=known_items.end())
		return true;
	return false;
}

void HERO::create_list_of_monsters_following_player()
{
	// player is followed by:
	// 1. friendly monsters in FOV of player
	// 2. enemies that see player (50%)
	
	ptr_list::iterator sm,_sm;
	MONSTER *monster;
	ROBOT *rob;
	ENEMY *host;
	for (sm = level.monsters.begin(),_sm = level.monsters.end();sm!=_sm;sm++)
	{
		monster = (MONSTER *) *sm;
		if (monster->IsMadDoctor()) // MadDoctor doesn't go
			continue;
		if (monster->IsWorm()) // Worms don't go
			continue;

		host = monster->IsHostile();
		if (host==NULL)
			continue;
		rob = monster->IsRobot();
		
		if (host!=this && !host->IsStunned() && (host->speed.GetValue()>5 || (rob!=NULL && rob->get_robot_speed()>5)))
		{
				if (monster->seen_now)
				{
					if (host->is_friendly(level.player) ||
							(host->enemy==this && !host->have_critically_low_hp() &&
							   (coin_toss() ||
								  ( abs(host->pX()-pX())<2 && abs(host->pY()-pY())<2 )
							   )
							)
					   )
					{
						if (host->set_direction_to_cell_by_shortest_path(this->pX(),this->pY(),false))						
						{
							int time;
							if (rob!=NULL)
								time = TIME_MOVE/(rob->get_robot_speed()+1)+1;
							else
								time=TIME_MOVE/(speed.GetValue()+1)+1;
							host->wait( distance(pX(),pY(),host->pX(),host->pY()) * time );
							monsters_following_player_to_other_level.push_back(host);						
						}
					}
				}				
		}
	}	
}

int HERO::reprogram(PROGRAMMATOR *item)
{
	if (item==NULL)
		return false;
	
	ptr_list available_items;
	ptr_list available_CPUs;
	ptr_list::iterator m,_m;
	build_robot_find_available_parts(available_items);	
	ITEM *temp;

	for (m=available_items.begin(),_m=available_items.end();m!=_m;m++)
	{
		temp = (ITEM *) *m;
		if (temp->IsRobotCPU())
			available_CPUs.push_back(temp);
	}
	temp = choose_item_from_list(available_CPUs,"조작 할 프로세서를 선택하라 (주변의 물건들)");

	PROCESSOR *cpu;
	cpu = temp->IsRobotCPU();
	if (cpu==NULL)
		return false;

	if (cpu->group_affiliation == GROUP_HERO)
	{
		screen.console.add(item->show_name() + "그것은 이미 적대적이지 않다.\n",3);		
		return false;
	}
	
	if (cpu->quality > skill[SKILL_PROGRAMMING])
	{
		string str = " 현재 실력으로는 이 프로세서를 조작할 수 없다! ";
		print_text(40-str.size()/2,0,str);
		myrefresh();
		keyboard.wait_for_key();
		level.player->skill_used(SKILL_PROGRAMMING);		
		return false;
	}

	cpu->group_affiliation = GROUP_HERO;
	screen.console.add(name + "은 " + item->show_name() + "의 프로그램을 조작하여 동료로 만든다.\n",3,false);

	for (int a=0;a<5;a++) // 5 times
		level.player->skill_used(SKILL_PROGRAMMING);		
	return true;
}

bool HERO::set_direction_to_closest_unexplored()
{
	// find the closest unexplored by floodfill.
	if (travel_dest_x!=-1 && travel_dest_y!=-1 && !(pX()==travel_dest_x && pY()==travel_dest_y))
	{
		if (!level.map.seen(travel_dest_x,travel_dest_y) ||
			(
			level.map.seen(travel_dest_x,travel_dest_y) &&  (!level.map.blockMove(travel_dest_x,travel_dest_y) || (level.map.blockMove(travel_dest_x,travel_dest_y)) 
			&& level.map.isOpenable(travel_dest_x,travel_dest_y))
			)
			)
		{
			if (set_direction_to_cell_by_shortest_path(travel_dest_x,travel_dest_y,true))
				return true;
		}
	}

	POSITION pos,new_pos;
	list<POSITION> positions;
	list<POSITION>::iterator m;
	int tested[MAPWIDTH][MAPHEIGHT];
	int x,y;
	for (y=0;y<MAPHEIGHT;++y)
		for (x=0;x<MAPWIDTH;++x)
			tested[x][y]=0;

	pos.x=pX();
	pos.y=pY();
	positions.push_back(pos);
	tested[pos.x][pos.y]=1;

	m=positions.begin();
	while(m!=positions.end())
	{
		pos=*m;
		if (!level.map.seen_once(pos.x,pos.y))
		{
			for (int a=-1;a<=1;a++)
				for (int b=-1;b<=1;b++)
				{
					new_pos.x=pos.x+a;
					new_pos.y=pos.y+b;
					if (level.map.seen_once(new_pos.x,new_pos.y) && (!level.map.blockMove(new_pos.x,new_pos.y) || level.map.isOpenable(new_pos.x,new_pos.y)))
						if (set_direction_to_cell_by_shortest_path(new_pos.x,new_pos.y,true))
						{
							travel_dest_x=new_pos.x;
							travel_dest_y=new_pos.y;
							return true;
						}
				}
		}
		
		for (int a=-1;a<=1;a++)
			for (int b=-1;b<=1;b++)
			{
				new_pos.x=x=pos.x+a;
				new_pos.y=y=pos.y+b;
				
				if (level.map.onMap(x,y) && !tested[x][y] && (!level.map.blockLOS(x,y) || level.map.isOpenable(x,y)))
				{
					positions.push_back(new_pos);
					tested[x][y]=1;

				}
			}
		++m;
	}
	stop_repeating();
	return false;
}

bool HERO::choose_travel_destination()
{
	int key;
	int x,y;
	int last_x,last_y;
	struct Screen_copy scr_copy;
	int changex, changey;

	last_x=-1; last_y=-1;

	screen.console.zero();
	screen.console.clean();
	screen.console.show();

	store_screen(&scr_copy); // remember screen

	changex=0;
	changey=0;
	key=0;
	x=pX();
	y=pY();

	while(1)
	{
		x+=changex;
		y+=changey;
		if (key==keyboard.escape)
		{
			restore_screen(&scr_copy);
			screen.console.clean();
			screen.console.zero();
			screen.console.show();
			return false;
		}
		else if (key==keyboard.readmore || key==keyboard.readmore2 || key==10 || key==459) // || enter
		{
			if (level.map.seen_once(x,y))
			{
				travel_dest_x=x;
				travel_dest_y=y;
				restore_screen(&scr_copy);
				screen.console.clean();
				screen.console.zero();
				screen.console.show();
				return true;
			}
		}
		else if (key==keyboard.down || key==keyboard.up) // stairs
		{
				// find stairs
			ptr_list::iterator m,_m;
			
			// find current stairs
			bool stairs_found=false;
			for (m=level.map.stairs_of_map.begin(),_m=level.map.stairs_of_map.end();m!=_m;++m)
			{
				STAIRS *stairs = (STAIRS *) (*m);
				if (stairs->x==x && stairs->y==y)
				{
					++m;
					if (m==_m)
						break;

					stairs = (STAIRS *) (*m);
					if (level.map.seen_once(stairs->x,stairs->y)) // next stairs
					{
						x=stairs->x;
						y=stairs->y;
						stairs_found=true;
						break;
					}
				}
			}
			if (!stairs_found) // find first seen
			{
				for (m=level.map.stairs_of_map.begin(),_m=level.map.stairs_of_map.end();m!=_m;++m)
				{
					STAIRS *stairs = (STAIRS *) (*m);
					if (level.map.seen_once(stairs->x,stairs->y))
					{
						x=stairs->x;
						y=stairs->y;
						break;
					}
				}
			}
			
		}

		restore_screen(&scr_copy);

		// limit of movement
		if (x<0 || x>79 || y<0 || y>39)
		{
			x=last_x; y=last_y;
		}
		set_color(12);
		print_character(x,y,'@');
		screen.console.add_and_zero("목적지를 선택하거나 '<' 또는 '>' 키로 계단을 가리켜라.",7);
		screen.console.show();
		myrefresh();

		key = keyboard.get_direction(changex, changey);

	} // endof while
}