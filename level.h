#ifndef LEVEL_H
#define LEVEL_H


#pragma warning (disable: 4786)
#pragma warning (disable: 4788)
// disable warning (compare bool with int)
#pragma warning (disable: 4805)
#pragma warning (disable: 4503)

#include "fov.h"
#include "map.h"
#include "monster.h"
#include <string>
#include "attrib.h"

typedef map < int , int > map_of_probabilities;
typedef pair < int, int > pair_of_probabilities;

class LEVEL_DEFINITION
{
public:
	string name;
	string type;

	string music;

	int fov_range;

	int number_of_items;
	int number_of_items_deviation;

	int number_of_monsters;
	int number_of_monsters_deviation;

	int max_number_of_monsters;
	int monster_every_n_turns;

	int items_param1;
	int items_param2;

	int items_concentration;
	
	map_of_probabilities item_probability;
	map_of_probabilities monster_probability;

	int sum_of_monster_probabilities;
	int sum_of_items_probabilities;

	void reset() {
		name="";
		type="";
		fov_range = 12;
		monster_every_n_turns = 99999999;
		number_of_monsters = 0;
		number_of_items = 0;
		max_number_of_monsters = 0;
		sum_of_monster_probabilities = 0;
		sum_of_items_probabilities = 0;
		number_of_monsters_deviation = 0;
		number_of_items_deviation = 0;
		item_probability.clear();			
		monster_probability.clear();			
		items_param1=0;
		items_param2=0;
		items_concentration=0;		
	};
};

typedef map < string , list < string > > map_of_level_links;
typedef map < string , LEVEL_DEFINITION > map_of_levels;

class LEVEL : public TOSAVE {
private:
	void CreateItemOnLevel(string type,int x, int y);
	void CreateMonsterOnLevel(string type,int x, int y, bool create_not_moving);

	void time_for_items();
	void time_for_monsters();
	void time_for_gases();
	void time_for_level();

	void remove_monsters();
	
public:
	bool find_empty_place(int &x, int &y);
	bool find_empty_not_visible_place(int &x, int &y);
	
	  bool is_player_on_level; // must not be saved!
	  
	  string ID; // ID (text) of this level
	  int fov_range;
      HERO *player;
      MUTUAL_FOV fov;
      MAP map;
      unsigned long turn;
      ptr_list monsters;
      ptr_list items_on_map;
      ptr_list gases_on_map;

      ptr_list all_items;     // all, jakie istnieja na tym poziomie

      ptr_list monsters_to_delete;     // po kazdej turze jest zwalniana pamiec
      ptr_list items_to_delete;  // po kazdej turze jest zwalniana pamiec
      ptr_list gases_to_delete;

	  void add_to_items_on_map(ITEM *item);
	  void remove_from_items_on_map(ITEM *item);
	  void remove_last_item_on_map();
	  
      void clear_lists_to_delete();
      void list_of_items_from_cell(ptr_list *items,int x,int y);

	  LEVEL();

      void TimePass(); // up³ynê³a w grze chwilka
	  
      bool entering_on_cell_possible(int x,int y);
      MONSTER *monster_on_cell(int x,int y);

      // w def_mons.cpp
      MONSTER *create_monster(int x, int y, string name, int param1, int param2);
      HERO *create_player(int x, int y, string name, int param1, int param2);
      
      // w def_item.cpp
      ITEM  *create_base_item(string ClassName);
      ITEM  *create_item(int x, int y, string name, int param1, int param2);

      int get_gas_density(int x, int y);
      
      bool add_gas(int x,int y, PROPERTIES properties, int density);

	  map_of_level_links stairs_up;
	  map_of_level_links stairs_down;
	  map_of_levels levels;

	  void FreeLevelMemory();
	  void CreateLevel(string type);
	  void CreateItemsOnLevel(string id);
	  void CreateMonstersOnLevel(string id);
	  ITEM *CreateRandomItemFromCategory(int category,int x,int y,int parametr1,int parametr2);


	  void AddMonstersFollowingPlayerToNewLevel();
	  
	  void ChangeLevelTo(string ID_new_one);

   /// INHERITED FROM ToSave
	virtual unsigned long SaveObject();
	virtual bool LoadObject();
};


#endif
