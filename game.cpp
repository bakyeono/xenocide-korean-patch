#include "mem_check.h"

#pragma warning (disable: 4786)
#pragma warning (disable: 4788)
// disable warning (compare bool with int)
#pragma warning (disable: 4805)

#include <list>
using namespace std;

#include <stdlib.h>
#include "monster.h"
#include "items.h"
#include "global.h"
#include "level.h"	
#include "keyboard.h"
#include "options.h"
#include "game.h"
#include "parser.h"
#include "system.h"
#include "sounds.h"
#include <math.h>

extern LEVEL level;
extern MYSCREEN screen;
extern DEFINITIONS definitions;
extern KEYBOARD keyboard;
extern OPTIONS options;
extern SOUNDS sounds;

extern char player_name_8[9]; // z system.cpp

void GAME :: Begin()
{
	  init_system();

	  sounds.PlayMusic("data/music/menu.mp3");

	  if (definitions.read_definition()==false)
	  {
		 printf("\n\nERROR READING DEFINITIONS\n\n");
		 exit(29);
	  }
	  create_directory("save");

	  screen.init();
	  screen.clear_all();
  
	  set_color(10);

	  string to_print;

	  to_print = "... 제 노 사 이 드 ...";
	  print_text(40-to_print.size()/2,2,to_print);

	  set_color(2);

	  to_print = string("dev version ") + __DATE__;
	  print_text(40-to_print.size()/2,3,to_print);

	  to_print = "(c) Copyright 2002-2007 Jakub Debski";
	  print_text(40-to_print.size()/2,5,to_print);

	  to_print = string("  한국어판 컨버전");
	  print_text(60,7,to_print);
	  to_print = string("박연오 2009-02-02");
	  print_text(60,8,to_print);

	  to_print = "If you want to play with the sources, don't forget to write me:";
	  print_text(40-to_print.size()/2,45,to_print);
	  to_print = "jakub@arcabit.pl or debski.jakub@wp.pl or jkn@konto.pl";
	  print_text(40-to_print.size()/2,46,to_print);
	  to_print = "Probably new version of sources is available.";
	  print_text(40-to_print.size()/2,47,to_print);

	  set_color(7);
	  print_text(1,9,"이름을 입력하라 :");

	  set_color(15);
	  myrefresh();

	  if (name_of_player=="")
		name_of_player = keyboard.get_name(20,9,20);
	  else
		  print_text(20,9,name_of_player);

	  strncpy(player_name_8,name_of_player.c_str(),8);
	  player_name_8[8]='\0';

	  if (CheckToLoad() == false)
		  NewGame();
	  else
		  LoadGame();
}


void GAME :: NewGame()
{
	int character;
	string chosen;

	set_color(7);
	print_text(1,11,"가장 자신있는 분야는?");

	set_color(7);
	
	print_text(3,13,"a) 전투");
	print_text(3,14,"b) 의학");
	print_text(3,15,"c) 과학");
	print_text(3,16,"d) 기계");
	print_text(3,18,"t) 게임 방법 배우기");
	print_text(3,20,"q) 게임 종료");
	
	set_color(14);
	
	print_character(3,13,'a');
	print_character(3,14,'b');
	print_character(3,15,'c');
	print_character(3,16,'d');
	print_character(3,18,'t');
	print_character(3,20,'q');
	
	myrefresh();
	
	while(1)
	{
		character=keyboard.wait_for_key();
		if (character=='a' || character=='a')
		{
			chosen = "Combat";
			break;
		}
		else if (character=='b' || character=='b')
		{
			chosen = "Medic";
			break;
		}
		else if (character=='c'  || character=='c')
		{
			chosen = "Scientist";
			break;
		}
		else if (character=='d'  || character=='d')
		{
			chosen = "Mechanic";
			break;
		}
		else if (character=='t'  || character=='T')
		{
			chosen = "Tutorial";
			break;
		}
		else if (character=='x' || character=='q' || character=='Q')
		{
			Quit();			
		}
	}

	TOSAVE::LoadOptions("default.opt");
		
	screen.console.add(name_of_player + string(", 행운을 빈다!\n"),15);
	level.turn=100;
	level.ID = "START";

	// Add PLAYER monster ;) on the level
	level.player = level.create_player(-1,-1,"Player " + chosen,0,0); // player
	level.is_player_on_level = true;

	ptr_list::iterator m,_m;
	
	if (chosen=="Combat")
	{
		level.player->add_known_category_of_items("GRENADE NORMAL 1");
		level.player->add_known_category_of_items("GRENADE NORMAL 2");
		level.player->add_known_category_of_items("GRENADE MINE 1");
		level.player->add_known_category_of_items("GRENADE MINE 2");
		level.player->add_known_category_of_items("GRENADE ENERGY 1");
//		level.player->add_known_item("Booster Pill");
	}
	else if (chosen=="Medic")
	{
		level.player->add_known_category_of_items("PILLS POPULAR");
	}
	else if (chosen=="Scientist")
	{
		level.player->add_known_category_of_items("GRENADE NORMAL 1");
		level.player->add_known_category_of_items("GRENADE ENERGY 1");
		level.player->add_known_category_of_items("GRENADE ENERGY 2");
//		level.player->add_known_item("Nanorebuilding Pill");
		
	}
	else if (chosen=="Mechanic")
	{
		// change own CPUs to friendly 
		PROCESSOR *cpu;
		for (m=level.player->backpack.begin(),_m=level.player->backpack.end();m!=_m;m++)
		{
			cpu = ((ITEM *) *m)->IsRobotCPU();
			if (cpu!=NULL)
				cpu->group_affiliation = GROUP_HERO;
		}
	}

	// all wearing items as known
	ITEM *temp;
	for (m=level.player->backpack.begin(),_m=level.player->backpack.end();m!=_m;m++)
	{
		temp = (ITEM *) *m;
		level.player->add_known_item(temp->name);
	}
	
	if (chosen=="Tutorial")
	{
		level.player->add_known_category_of_items("GRENADE NORMAL 1");
		level.player->add_known_category_of_items("PILLS POPULAR");
		level.ChangeLevelTo("Tutorial");
	}
	else
		level.ChangeLevelTo("s01");
}

GAME :: GAME()
{
}

void GAME :: Main(void)
{
   int someone_moves;
   TIME action_time;
   // main loop
   ptr_list::iterator m,_m;
   MONSTER *current_monster=NULL;

   while (1)
   {
new_level:
		 if (level.player->next_action_time==level.turn)
		 {
			 // to make player move as the last of monsters in this turn
			 level.monsters.remove(level.player);

			 if (level.is_player_on_level==false)
			 {
				 level.is_player_on_level=true;
				 level.map.backBlockMove(level.player->pX(),level.player->pY());
			 }
			 
			 level.monsters.push_back(level.player); // this also adds player when entering new level
		 }	// endof if

		 level.AddMonstersFollowingPlayerToNewLevel();

         do
         {
            someone_moves=false;
            for(m=level.monsters.begin(),_m=level.monsters.end(); m!=_m; m++)
            {
               current_monster=(MONSTER *)*m;

			   if (current_monster->is_dead) // when monster is dead, then continue
				   continue;

               if (current_monster->next_action_time<=level.turn)
               {
                  someone_moves=true;

                  action_time=current_monster->do_action( current_monster->get_action() );
				  if (action_time==TIME_LEVEL_CHANGE) // player changed the level in this action
				  {					  					  
					  goto new_level; // we are already on a new level
				  }

				  // if this monster caused player death then no one else moves
				  if (level.player!=NULL && level.player->is_dead)
					  someone_moves = false;

                  current_monster->wait( action_time );

				  // show screen after the player action
				  if (current_monster==level.player)
					level.player->look_around();
				  
                  if (current_monster==level.player && action_time!=0 && level.player->is_repeating_action()==false)
                  {
					if (!current_monster->IsStunned())
					{
						// do fov calculation
						screen.clear_all();						
						screen.draw();
						screen.console.clean();
						screen.console.show();
					}
                  } // endof if
               } // endof if
            } // endof for
         } while (someone_moves);

         // all the monsters moved, the time on level can pass
         level.TimePass();         
   }
   return;
}


bool GAME::SaveGame()
{
	level.SaveObject();
	level.SaveLastUNToFile();
#ifdef MSC
//	_CrtDumpMemoryLeaks();
#endif
	Quit();
	return true;
}

bool GAME::CheckToLoad()
{
	FILE *fp;

	fp = fopen(string(string("save/") + name_of_player + ".sav").c_str(),"rb+");
	if (fp==NULL)
		return false;
	else
		fclose(fp);

	return true;
}

bool GAME::LoadGame()
{
	level.LoadLastUNFromFile();
	level.FreeLevelMemory();
	level.player = TOSAVE::LoadPlayer(name_of_player);
	level.ID = level.player->ID_of_level;
	level.LoadObject();
	sounds.PlayMusic(level.levels[level.ID].music);
	return true;
}

void GAME::Quit()
{
   myclear();
   myrefresh();
   end_graph();
   exit(0);
}

void GAME::DeleteSaveGame()
{
	string name = string(string("save/") + name_of_player + ".");
	pair <string , LEVEL_DEFINITION> para;
	map_of_levels::iterator m,_m;

	delete_file(name + "sav");
	delete_file(name + "un");
	delete_file(name + "opt");
	delete_file(name + "msg");
	for (m=level.levels.begin(),_m=level.levels.end();m!=_m;m++)
	{
		para = *m;
		delete_file(name + para.first);
	}
}
