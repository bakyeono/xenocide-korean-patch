#include "mem_check.h"

#include "global.h"
#include "parser.h"
#include "level.h"
#include "keyboard.h"

extern DEFINITIONS definitions;
extern LEVEL level;
extern GAME game;
extern KEYBOARD keyboard;

HERO * LEVEL :: create_player(int x, int y, string name, int param1, int param2)
{
  HERO *new_one;
  MONSTER *from_definition;
  INTELLIGENT *intel;
  ptr_list::iterator m,_m;
  ITEM *temp, *temp2;
  
  from_definition=definitions.find_monster(name);

  if (from_definition==NULL)
  {
     fprintf(stderr,"ERROR: Try to create non-existing monster %s as player.\n",name.c_str());
     exit(128);
  }
  if (from_definition->ClassName!="INTELLIGENT")
  {
     fprintf(stderr,"ERROR: Player can base only on INTELLIGENT, not on %s.\n",name.c_str());
     exit(129);
  }
  intel=(INTELLIGENT *) from_definition;
  
  new_one=new HERO;

  ITEM *old_armor= intel->armor;
  if (old_armor!=&intel->no_armor)
  {
	  intel->remove_armor();	   
  }
  
  *((INTELLIGENT *)new_one)=*intel;
  if (old_armor!=&intel->no_armor)
  {
	  intel->set_armor(old_armor);
  }

  // change owner of attributes

  new_one->set_attributes_on_self();

  new_one->energy_points.val=0;  
  new_one->energy_points.mod=0;  
  new_one->energy_points.max=0;  
  
  new_one->ClassName = "HERO";
  new_one->UniqueNumber = PLAYER_UNIQUE_NUMBER;  

  // assign inventory
  new_one->weapon=&new_one->unarmed;
  new_one->armor=&new_one->no_armor;
  new_one->weapon->owner = new_one;
  new_one->armor->owner = new_one;  
  
  new_one->backpack.clear();
  new_one->items_in_backpack=0;
  
  new_one->ChangePosition(-1,-1);

  for(m=intel->backpack.begin(),_m=intel->backpack.end(); m!=_m; m++)
  {
         temp=(ITEM *)*m;
         temp2=temp->duplicate();

		 new_one->ChangePosition(-1,-1);

         new_one->pick_up_item(temp2,false);
         if (temp==from_definition->weapon)
         {
           new_one->weapon=temp2;
         }
         else if (temp==from_definition->armor)
         {
		   ((INTELLIGENT *) new_one)->set_armor(temp2);
         }
         else if (temp==intel->ready_weapon)
         {
			 new_one->ready_weapon=temp2;
         }
         
  }

  // Create random items, if have
  if (definitions.lista_kategorii_losowych_itemow_potwora.find(name)!=definitions.lista_kategorii_losowych_itemow_potwora.end())
  {
	  list <int>::iterator k,l;
	  int list_size = definitions.lista_kategorii_losowych_itemow_potwora[name].size();
	  k = definitions.lista_kategorii_losowych_itemow_potwora[name].begin();
	  l = definitions.lista_kategorii_losowych_itemow_potwora[name].end();
	  int category_number;
	  
	  // go through the category list from which is selected
	  for (;k!=l;k++)
	  {
		  category_number = *k;
		  // znalezienie losowego itemu z danej kategorii
		  new_one->pick_up_item(level.CreateRandomItemFromCategory(category_number, -1, -1, 0, 0),false);			  
	  }
  }
  
  // assign the rest
  new_one->rename(game.name_of_player);
  new_one->no_armor.owner=new_one;
  new_one->enemy=NULL;

  new_one->group_affiliation = GROUP_HERO;

  new_one->ChangePosition(x,y);

  new_one->next_action_time=level.turn+1;
  level.monsters.push_back(new_one);
  return new_one;
  
}

MONSTER * LEVEL :: create_monster(int x, int y, string name, int param1, int param2)
{
   MONSTER *new_one;
   MONSTER *from_definition;

   new_one=level.monster_on_cell(x,y);
   if (new_one!=NULL || !level.entering_on_cell_possible(x,y) || map.AreStairs(x,y)) return NULL;

   from_definition=definitions.find_monster(name);

   if (from_definition==NULL)
   {
     fprintf(stderr,"ERROR: Try to create non-existing monster %s.\n",name.c_str());
	 keyboard.wait_for_key();
     exit(124);
   }

   new_one=from_definition->duplicate();
   new_one->rename(from_definition->name);
   
         
// --------------- set pointers ---------------------------------------
   new_one->no_armor.owner=new_one;
   new_one->enemy=NULL;
   

// --------------- set position ---------------------------------------
   
   new_one->ChangePosition(x,y);
   new_one->next_action_time=level.turn;
   level.monsters.push_back(new_one);

// --- set FOV radius to level - !!! ADD CHANGE OF IT WITH MONSTER TRAVELLING
	
   new_one->fov_radius.val = levels[level.ID].fov_range;
   return new_one;
}

