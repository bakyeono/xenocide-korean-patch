#include "global.h"
#include "monster.h"
#include "items.h"
#include "map.h"
#include <map>

bool get_boolean(string line, int line_number);

typedef list<string> lista_ammo;

struct unknown_item {
	string name;
	int color;
};

typedef list<unknown_item> unknown_items_list;

typedef list<string> list_of_names_for_categories;
typedef map <int, list_of_names_for_categories> map_of_categories;
typedef map <string, int> map_of_category_names;
typedef map <string, list <int> > map_of_items_for_monster;
typedef map <string, unknown_item> map_of_unknown_items;

class DEFINITIONS {
private:
  bool read_keyboard_mapping();
  bool read_categories();
  bool read_ammo_types();
  bool read_items_definitions();
  bool read_monsters_definitions();
  bool read_gases_definitions();
  bool read_terrains_definitions();
  bool read_level_definitions();

  bool read_pills_types();
  bool read_grenades_types();
  bool create_map_of_game_unknown_items();  

  int check_ammo_type(string line);
  
public:
  map_of_unknown_items constantly_unknown_items;
  
  ptr_list items;
  ptr_list monsters;
  ptr_list terrains;
  
  lista_ammo ammo_types; // stringi z typami amunicji (AMMOTYPE.LST)
  unknown_items_list unknown_pills; 
  unknown_items_list unknown_grenades; 

  map_of_categories objects_in_category;
  map_of_category_names category_number;
  map_of_items_for_monster lista_kategorii_losowych_itemow_potwora;  
  
  bool read_definition();
  ITEM *find_item(string name);
  MONSTER *find_monster(string name);
  CELL *find_terrain(string name);
};

#define FILE_BUF_SIZE 5000

class CFILE {
private:
	FILE *fp;
public:
	CFILE(); 
	~CFILE();
	string file_name;
	char *buf;
	int line_number;
	bool open(string name);
	bool create(string name);
	void close();
	string get_line();
	void save_line(string line);
	void error_open()	{ fprintf(stderr,"\nERROR: unable to open file %s.\n\n",file_name.c_str()); exit(0);	};
	void error_create() { fprintf(stderr,"\nERROR: unable to create file %s.\n\n",file_name.c_str()); exit(0);	};
};
