// #define DONT_CREATE_MONSTERS

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
#include "mt19937int.h"
#include "level.h"
#include "system.h"
#include "sounds.h"

#include <algorithm>
#include <cctype>

#ifndef min
# define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#include "directions.h"

LEVEL level;
extern GAME game;
extern MYSCREEN screen;
extern OPTIONS options;
extern DEFINITIONS definitions;
extern SOUNDS sounds;

LEVEL::LEVEL()
{
	ClassName = "LEVEL";
	UniqueNumber = LEVEL_UNIQUE_NUMBER;
}

bool LEVEL :: entering_on_cell_possible(int x,int y)
{
	if (!map.blockMove(x,y) && map.IsMonsterOnMap(x,y)==false)
	{
		return true;
	}
	else
		return false;
}

int LEVEL :: get_gas_density(int x, int y)
{
   ptr_list::iterator m,_m;
   GAS *temp;
   int density;
   density=0;
   if (gases_on_map.size()!=0)
   for(m=gases_on_map.begin(),_m=gases_on_map.end(); m!=_m; m++)
   {
      temp=(GAS *)*m;
      if (temp->pY()==y && temp->pX()==x)
      {
        density+=temp->density;
      }
   }
   return density;
}


void LEVEL :: clear_lists_to_delete()
{
   ptr_list::iterator m,_m;
   ITEM *temp;
   MONSTER *temp2;
   GAS *temp3;

   // delete monsters
   for(m=monsters_to_delete.begin(),_m=monsters_to_delete.end(); m!=_m; m++)
   {
       temp2=(MONSTER *)*m;

	   monsters.remove(temp2);
       delete temp2;
   }
   // delete items
   for(m=items_to_delete.begin(),_m=items_to_delete.end(); m!=_m; m++)
   {
       temp=(ITEM *)*m;
       all_items.remove(temp);
       delete temp;
   }
   // delete gases
   for(m=gases_to_delete.begin(),_m=gases_to_delete.end(); m!=_m; m++)
   {
       temp3=(GAS *)*m;
       gases_on_map.remove(temp3); // if is on level, to sie kasuje
       delete temp3;
   }
   //
   monsters_to_delete.clear();
   items_to_delete.clear();
   gases_to_delete.clear();

}


void LEVEL :: list_of_items_from_cell(ptr_list *items,int x,int y)
{
   ptr_list::iterator m,_m;
   ITEM *item;

   if (map.GetNumberOfItems(x,y)==0)
	  return;
	  
   for(m=items_on_map.begin(),_m=items_on_map.end(); m!=_m; m++)
   {
       item=(ITEM *)*m;
       if (item->pX()==x && item->pY()==y)
       {
		   if (!item->property_controller()) // don't add controllers
				items->push_back(item);
       }
   }
}


MONSTER * LEVEL :: monster_on_cell(int x,int y)
{
      ptr_list::iterator m,_m;
      MONSTER *monster;

	  if (!map.IsMonsterOnMap(x,y))
		  return NULL;

      for(m=monsters.begin(),_m=monsters.end(); m!=_m; m++)
      {
            monster=(MONSTER *)*m;
            if (x==monster->pX() && y==monster->pY())
              return monster;
      }
      return NULL;
};

bool LEVEL :: add_gas(int x,int y, PROPERTIES properties, int density)
{
   ptr_list::iterator m,_m;
   GAS *gas;

   bool found;
   
   if (map.blockMove(x,y)==true)
     return false;

   found=false;
   if (level.map.GetNumberOfGases(x,y)>0)
   {
	   for(m=gases_on_map.begin(),_m=gases_on_map.end(); m!=_m; m++)
	   {
		   gas=(GAS *)*m;
		   if (gas->pX()==x && gas->pY()==y) // is there
		   {
			   // the same gas?
			   if (gas->properties==properties)
			   {
				   found=true;
				   break;
			   }
		   }
	   }
	   if (found==true)
		   gas->density+=density;
   }

   if (!found)
   {
     if (properties&TYPE_INCENDIARY)
     {
       gas=(GAS *) new FIRE;
     }
     else
       gas=new GAS;

     gas->properties=properties;
     gas->density=density;
	 gas->ChangePosition(x,y);
     gases_on_map.push_front(gas);
   }
   return true;
}

void LEVEL::remove_monsters()
{
	ptr_list::iterator m,_m;
	for(m=monsters.begin(),_m=monsters.end(); m!=_m; m++)
	{
		MONSTER *temp2=(MONSTER *)*m;
		delete temp2;
	}
	monsters.clear();
}

void LEVEL::FreeLevelMemory()
{
   ptr_list::iterator m,_m;
   ptr_list::iterator j,_j;
   ITEM *temp;
   MONSTER *temp2;
   GAS *temp3;
   STAIRS *temp4;

   // delete monsters
   remove_monsters();

   // delete items
   for(m=all_items.begin(),_m=all_items.end(); m!=_m; m++)
   {
		temp=(ITEM *)*m;
		delete temp;
   }
   // delete gases
   for(m=gases_on_map.begin(),_m=gases_on_map.end(); m!=_m; m++)
   {
       temp3=(GAS *)*m;
       delete temp3;
   }

   // delete stairs
   for (m=map.stairs_of_map.begin(),_m=map.stairs_of_map.end();m!=_m;m++)
   {
       temp4=(STAIRS *)*m;
       delete temp4;
   }


   //////////////////////////////////////////////////////////////////////////
   
   items_on_map.clear();
   all_items.clear();
   gases_on_map.clear();

   map.stairs_of_map.clear();
   stairs_up.clear();
   stairs_down.clear();

   ListToFix.clear();
   AddressesOfObjects.clear();
}

void LEVEL::ChangeLevelTo(string ID_new_one)
{
	if (ID!="START")
	{
		player->create_list_of_monsters_following_player();
		
		// zero all enemies in monsters - important, because in level change enemy ptr can become invalid
		ptr_list::iterator m,_m;
		MONSTER *temp;
		for(m=monsters.begin(),_m=monsters.end(); m!=_m; m++)
		{
			temp=(MONSTER *)*m;
			temp->enemy=NULL;
		}
		for(m=player->monsters_following_player_to_other_level.begin(),_m=player->monsters_following_player_to_other_level.end(); m!=_m; m++)
		{
			temp=(MONSTER *)*m;
			temp->enemy=NULL;
		}
		

		SaveObject(); // save level and player
		FreeLevelMemory(); // remove this level
		SaveLastUNToFile();
		player = TOSAVE::LoadPlayer(game.name_of_player);
	}

	ID = ID_new_one;
	player->ID_of_level = ID_new_one;

	if (LoadObject()==false)
	{
		if (turn!=0)
		{
			screen.console.clean();
			screen.console.zero();
			screen.console.add("맵 생성 중...",7);
			screen.console.show();
		}
		myrefresh();

		// set player FOV according to level
		player->fov_radius.val = levels[ID].fov_range;
		
		CreateLevel(levels[ID_new_one].type);
		CreateItemsOnLevel(ID_new_one);
		
		// set player on stairs if available
		ptr_list :: iterator m,_m;
		STAIRS *stairs;
		for (m=map.stairs_of_map.begin(),_m=map.stairs_of_map.end();m!=_m;m++)
		{
			stairs = (STAIRS *) *m;
			if (stairs->to_where == player->ID_of_last_level && stairs->number == player->stairs_number_used)
			{
				player->ChangePosition(stairs->x, stairs->y);
				break;
			}
		}
#ifndef DONT_CREATE_MONSTERS
		CreateMonstersOnLevel(ID_new_one);
#endif
		sounds.PlayMusic(levels[ID_new_one].music);

	}
	else // level loaded
	{
			sounds.PlayMusic(levels[ID_new_one].music);

			// set player FOV
			player->fov_radius.val = levels[ID].fov_range;
			// set player on stairs if available
			ptr_list :: iterator m,_m;
			STAIRS *stairs;
			for (m=map.stairs_of_map.begin(),_m=map.stairs_of_map.end();m!=_m;m++)
			{
				stairs = (STAIRS *) *m;
				if (stairs->to_where == player->ID_of_last_level && stairs->number == player->stairs_number_used)
				{
					player->ChangePosition(stairs->x, stairs->y);
					break;
				}
			}
			// if less than 10000 turns passed, then nothing changes
			screen.console.clean();
			screen.console.zero();
			screen.console.add("변화를 계산하는 중...",7);
			screen.console.show();
			myrefresh();

			if (player->next_action_time > turn + 50000) // a loooot of time passed
			{
				monsters.remove(player); // player will be added on his turn
				remove_monsters();
				turn = player->next_action_time;
				CreateMonstersOnLevel(ID);
			}
			else if (player->next_action_time > turn+1000) // a bit of time passed
			{
				turn = player->next_action_time - 1000;
			}

			// time pass on level but without player
			// player doesn't see what's going on level
			map.HideFromPlayerSight();
			screen.draw();

			monsters.remove(player); // player will be added on his turn
			is_player_on_level = false;
	}	
}

void LEVEL::CreateLevel(string type)
{
	if (type=="Shuttle")
	{
	  map.CreateMapShuttle();
	}
	else if (type=="Capsule")
	{
		map.CreateMapCapsule();
	}
	else if (type=="City NW")
	{
		map.CreateMapCityNW();
	}
	else if (type=="City SW")
	{
		map.CreateMapCitySW();
	}
	else if (type=="Genetic Lab")
	{
		map.CreateMapGeneticLab();
	}
	else if (type=="Doctor's Cave")
	{
		map.CreateMapDoctorsCave();
	}
	else if (type=="Capsule")
	{
		map.CreateMapCapsule();
	}
	else if (type=="Caves")
	{
	  map.CreateMapCaves();
	}
	else if (type=="Mines")
	{
		map.CreateMapMines();
	}
	else if (type=="Building")
	{
	  map.CreateMapBuilding();
	}
	else if (type=="Bunker Enterance")
	{
	  map.CreateMapBunker();
	}
	else if (type=="Underground")
	{
	  map.CreateMapUnderground(false);
	}
	else if (type=="Tutorial")
	{
		map.CreateMapTutorial();
	}
	else
	{
		fprintf(stderr,"ERROR: NIEZNANY TYP POZIOMU: %s",type.c_str());
		exit(49);
	}
}


void LEVEL::CreateItemsOnLevel(string id)
{
	if (levels.find(id)==levels.end())
	{
		fprintf(stderr,"ERROR: UNKNOWN LEVEL ID: %s",id.c_str());
		exit(45);		
	}

	int number = levels[id].number_of_items;
	int deviation = (number * levels[id].number_of_items_deviation) / 100; 

	if (coin_toss())
		number+=random(deviation);
	else
		number-=random(deviation);

	int x,y;
	int number_of_concentrated = min(number,random(4)+1);
		
	for (int a=0;a<number;a++)
	{
		if (levels[id].items_concentration==0 || a==0) // when first
		{
			if (find_empty_place(x,y)==true)
				CreateItemOnLevel(id,x,y);
		}
		else
		{
			for (int b=0;b<number_of_concentrated;b++)
			{
				x+= random(levels[id].items_concentration * 2) - levels[id].items_concentration;
				y+= random(levels[id].items_concentration * 2) - levels[id].items_concentration;
				if (!map.blockMove(x,y))
				{
				  CreateItemOnLevel(id,x,y);
				  a++;
				}
			}
			find_empty_place(x,y);
		}
	}
}	

void LEVEL::CreateItemOnLevel(string id,int x,int y)
{
	map_of_probabilities::iterator m,_m;
	pair_of_probabilities p;

	int category=-1;
	int lowest_one = 999999999;

	// choose random item category

	int random_value = random( levels[id].sum_of_items_probabilities );

	// find the lowest one with this conditions, gives category
	for (m=levels[id].item_probability.begin(),_m=levels[id].item_probability.end();m!=_m;m++)
	{
		p = *m;
		if (random_value < p.second && p.second < lowest_one)
		{
			lowest_one = p.second;
			category = p.first;
		}
	}
	// choose random item from this category
	if (category!=-1)
	{
		int parametr1, parametr2;
		
		parametr1 = random (levels[id].items_param1);
		parametr2 = random (levels[id].items_param2);
		
		CreateRandomItemFromCategory(category,x,y,parametr1,parametr2);
		return;
	}
}

ITEM *LEVEL::CreateRandomItemFromCategory(int category, int x, int y, int parametr1, int parametr2)
{
	string selected_name;
	
	if ( definitions.objects_in_category.find(category) == definitions.objects_in_category.end() )
		return NULL;
	
	list_of_names_for_categories::iterator k;
	int list_size = definitions.objects_in_category[category].size();
	int random_value = random(list_size);
	
	k = definitions.objects_in_category[category].begin();
	for (int a=0;a<random_value;a++,k++); // przejscie do wylosowanej nazwy
	
	selected_name = *k;
		
	return create_item(x,y,selected_name,parametr1,parametr2);		
}

void LEVEL::CreateMonstersOnLevel(string id)
{
	if (levels.find(id)==levels.end())
	{
		fprintf(stderr,"ERROR: NIEZNANE ID POZIOMU: %s",id.c_str());
		exit(45);		
	}
	
	int number = levels[id].number_of_monsters;
	int deviation = (number * levels[id].number_of_monsters_deviation) / 100;
	
	if (coin_toss())
		number+=random(deviation);
	else
		number-=random(deviation);
	
	int x,y;
	
	for (int a=0;a<number;a++)
	{
		if (id=="s01")
		{
			if (find_empty_not_visible_place(x,y)==true)
				CreateMonsterOnLevel(id,x,y,true);
		}
		else
		{
			if (find_empty_place(x,y)==true)
				CreateMonsterOnLevel(id,x,y,true);			
		}
	}
}	

void LEVEL::CreateMonsterOnLevel(string id,int x,int y, bool create_not_moving)
{
	map_of_probabilities::iterator m,_m;
	pair_of_probabilities p;
	
	int category=-1;
	int najmniejsza = 999999999;
	
	// choose monster category
	
	int random_value = random( levels[id].sum_of_monster_probabilities );

	// find the lowest one with this conditions, gives category
	for (m=levels[id].monster_probability.begin(),_m=levels[id].monster_probability.end();m!=_m;m++)
	{
		p = *m;
		if (random_value < p.second && p.second < najmniejsza)
		{
			najmniejsza = p.second;
			category = p.first;
		}
	}
	// random monster from this category
	if (category!=-1)
	{
		string selected_name;
		
		if ( definitions.objects_in_category.find(category) == definitions.objects_in_category.end() )
		{
			return;
		}
		
		list_of_names_for_categories::iterator k;
		int list_size = definitions.objects_in_category[category].size();
		k = definitions.objects_in_category[category].begin();
		random_value = random(list_size);
		
		for (int a=0;a<random_value;a++,k++); // przejscie do wylosowanej nazwy
		
		selected_name = *k;

		if (!create_not_moving) 
		{
			// can it move?
			MONSTER *from_definition;
			from_definition=definitions.find_monster(selected_name);
			if (from_definition!=NULL)
			{
				if (from_definition->speed.GetValue()==0)
					return;
			}
		}
	
		create_monster(x,y,selected_name,0,0);	
		return;
	}
}

bool LEVEL::find_empty_not_visible_place(int &x, int &y)
{
	for(int a=0;a<1000 && find_empty_place(x,y)==true;a++)
	{
		if (player==NULL)
			return true;

		if (distance(x,y,player->pX(),player->pY()) > player->fov_radius.GetValue() && !level.map.seen_by_camera(x,y))
			return true;
	}
	return false;
}
	
bool LEVEL::find_empty_place(int &x, int &y)
{
	for (int a=0;a<20000;a++)
	{
		x=random(MAPWIDTH);
		y=random(MAPHEIGHT);
		if (!map.blockMove(x,y))
		{
			if (map.IsMonsterOnMap(x,y)==false)
				return true;
		}
	}
	return false;
}

void LEVEL::time_for_items()
{
   ptr_list::iterator m,_m;
   ITEM *temp;
   for(m=all_items.begin(),_m=all_items.end(); m!=_m; m++)
   {
       temp=(ITEM *)*m;
       temp->every_turn();
   }
}

void LEVEL::time_for_monsters()
{
   ptr_list::iterator m,_m;
   MONSTER *monster;
   for(m=monsters.begin(),_m=monsters.end(); m!=_m; m++)
   {
       monster=(MONSTER *)*m;
       monster->every_turn();
   }
}

void LEVEL::time_for_gases()
{
   ptr_list::iterator m,_m;
   GAS *gas;
   for(m=gases_on_map.begin(),_m=gases_on_map.end(); m!=_m; m++)
   {
       gas=(GAS *)*m;
       gas->every_turn();
   }
}

void LEVEL::TimePass()
{
	turn++;

	// inform player about long calculations with rotating sign

	if (player!=NULL && (player->IsStunned() || player->next_action_time-40 > turn) || gases_on_map.size()>100 || player->is_repeating_action()==REPEAT_ACTION_REST)
	{
		if ((turn/48)%4==0 || gases_on_map.size()>100)
		{
			int l;
			if (gases_on_map.size()<100)
			{
				l = (turn/12)%4;
			}
			else
				l = turn%4;
			
			set_color(15);
			switch(l) {
			case 0:
				print_character(0,0,'-');
				break;
			case 1:
				print_character(0,0,'\\');
				break;
			case 2:
				print_character(0,0,'|');
				break;
			case 3:
				print_character(0,0,'/');
				break;
				
			}
			myrefresh();
		}
	}	

   time_for_items();
   time_for_monsters();
   time_for_gases();
   time_for_level();

   clear_lists_to_delete();
}

void LEVEL::time_for_level()
{
	int x,y;
	// special behaviour on level according to it's type

	// weaken shields on all level
	for (x=0;x<MAPWIDTH;x++)
		for (y=0;y<MAPHEIGHT;y++)
		{
			map.addShield(x,y,-1);
		}			


	// create monsters on level
#ifndef DONT_CREATE_MONSTERS
	if (turn%levels[this->ID].monster_every_n_turns==0) // time for a new one
	{
		if (monsters.size()<levels[this->ID].max_number_of_monsters) // we can add a new one on map
		{
			if (is_player_on_level==true)
			{
				if (find_empty_not_visible_place(x,y)==true)
					CreateMonsterOnLevel(this->ID,x,y,false);
			}
			else if (find_empty_place(x,y)==true)
			{
				CreateMonsterOnLevel(this->ID,x,y,false);
			}
		}
	}
#endif
}


void LEVEL::add_to_items_on_map(ITEM *item)
{
	if (item!=NULL)
	{
		items_on_map.push_back(item);
		item->on_ground = true;
		map.IncraseNumberOfItems(item->pX(),item->pY());
	}
}

void LEVEL::remove_from_items_on_map(ITEM *item)
{
	if (item!=NULL)
	{
		items_on_map.remove(item);
		item->on_ground = false;
		map.DecraseNumberOfItems(item->pX(),item->pY());
	}	
}

void LEVEL::remove_last_item_on_map()
{
	if (items_on_map.size()==0)
		return;
	ITEM *item;

	ptr_list::iterator m;
	m=items_on_map.end();
	m--; // last item
	item = (ITEM *) *m;
	remove_from_items_on_map(item);
}

void LEVEL::AddMonstersFollowingPlayerToNewLevel()
{
	if (is_player_on_level==true && player->monsters_following_player_to_other_level.size()>0) 
	{
		MONSTER *new_on_level;
		ptr_list :: iterator p,_p;
		STAIRS *stairs;
		bool stairs_exist=false;
		for (p=map.stairs_of_map.begin(),_p=map.stairs_of_map.end();p!=_p;p++)
		{
			stairs = (STAIRS *) *p;
			if (stairs->to_where == player->ID_of_last_level && stairs->number == player->stairs_number_used)
			{
				stairs_exist=true;
				break;
			}
		}			 
		if (!stairs_exist) 
		{ 
			// no stairs back, delete monsters, that passed
			p=player->monsters_following_player_to_other_level.begin();
			_p=player->monsters_following_player_to_other_level.end();
			for (;p!=_p;)
			{
				new_on_level = (MONSTER *) *p;
				monsters_to_delete.push_back(new_on_level);
				p++;
				player->monsters_following_player_to_other_level.pop_front();
			}				 				 
		}
		else // stairs are
		{
			// add max 4 monsters, if place is valid for entering
			int posx,posy;

			for (int a=0;a<4 && player->monsters_following_player_to_other_level.size()>0;a++)
			{
				posx=stairs->x+random(3)-1;
				posy=stairs->y+random(3)-1;
				if (entering_on_cell_possible(posx,posy))
				{
					p=player->monsters_following_player_to_other_level.begin();
					_p=player->monsters_following_player_to_other_level.end();
					for (;p!=_p;p++)
					{
						new_on_level = (MONSTER *) *p;
						if (new_on_level->next_action_time<=turn)
						{
							player->monsters_following_player_to_other_level.remove(new_on_level);
							new_on_level->ChangePosition(posx,posy);
							monsters.push_front(new_on_level);
							break; // one per level
						}
					} // endof for
				} // endof if
			}
		} // endof else
	}
}

