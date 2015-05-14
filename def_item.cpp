#include "mem_check.h"

#include "global.h"
#include "types.h"
#include "parser.h"
#include "level.h"
#include <assert.h>

extern LEVEL level;
extern DEFINITIONS definitions;
extern unsigned int random(unsigned int range);
extern bool coin_toss();

ITEM * LEVEL :: create_base_item(string ClassName)
{
   ITEM *new_one;
   
   AMMO *ammo;
   HAND_WEAPON *handw;
   RANGED_WEAPON *shooting;
   ARMOR *armor;
   CORPSE *body;
   GRENADE *grenade;
   CONTROLLER *controller;
   PILL *pill;
   TRASH *garbage;
   ROBOT_LEG *leg;
   ROBOT_SHELL *shell;
   PROCESSOR *processor;
   REPAIR_KIT *repairset;
   PROGRAMMATOR *programmator;
   
   new_one=NULL;

   if (ClassName == "CORPSE")
   {
                body=new CORPSE;
                new_one=body;
   }
   else if (ClassName == "AMMO")
   {
                ammo=new AMMO;
                new_one=ammo;
   }
   else if (ClassName == "RANGED_WEAPON")
   {
                shooting=new RANGED_WEAPON;
                new_one=shooting;
   }
   else if (ClassName == "HAND_WEAPON")
   {
                handw = new HAND_WEAPON;
                new_one=handw;
   }
   else if (ClassName == "GRENADE")
   {
                grenade=new GRENADE;
                new_one=grenade;
   }
   else if (ClassName == "TRASH")
   {
	   garbage=new TRASH;
	   new_one=garbage;
   }
   else if (ClassName == "REPAIR_KIT")
   {
	   repairset=new REPAIR_KIT;
	   new_one=repairset;
   }
   else if (ClassName == "PROGRAMMATOR")
   {
	   programmator=new PROGRAMMATOR;
	   new_one=programmator;
   }
   else if (ClassName == "CONTROLLER")
   {
       controller=new CONTROLLER;
       new_one=controller;
   }
   else if (ClassName == "PROCESSOR")
   {
	   processor=new PROCESSOR;
	   new_one=processor;
   }
   else if (ClassName == "ROBOT_LEG")
   {
	   leg=new ROBOT_LEG;
	   new_one=leg;
   }
   else if (ClassName == "ROBOT_SHELL")
   {
	   shell=new ROBOT_SHELL;
	   new_one=shell;
   } 
   else if (ClassName == "PILL")
   {
	   pill=new PILL;
	   new_one=pill;
   }
   else if (ClassName == "ARMOR")
   {
                armor=new ARMOR;
                new_one=armor;
   }
   else
   {
     fprintf(stderr,"ERROR: Unknown class to duplicate: %s.\n",ClassName.c_str());
     exit(123);
   }
   level.add_to_items_on_map(new_one);
   level.all_items.push_back(new_one);
   return new_one;
}

ITEM * LEVEL :: create_item(int x, int y, string name, int param1, int param2)
{
   ITEM *new_one;
   // find item on definitions list (by name)
   ptr_list::iterator m,_m;
   ITEM *from_definition;
   
   from_definition=definitions.find_item(name);

   if (from_definition==NULL)
   {
     fprintf(stderr,"ERROR: Try to create non-existing item %s.\n",name.c_str());
     exit(123);
   }
   else
   {
     new_one=from_definition->duplicate();
   
	 if (new_one->ClassName == "AMMO")
	 {
				AMMO *am = new_one->IsAmmo();
				assert (am!=NULL);

                am->quantity=param1+1;
     }
	 else if (new_one->ClassName == "RANGED_WEAPON")
	 {
		 // create ammo in magazine
		 RANGED_WEAPON *ranged_weapon = new_one->IsShootingWeapon();
		 assert (ranged_weapon!=NULL);

   		    // randomize ammo
			if (ranged_weapon->category_of_magazine!=0)
			{
				string selected_name;
				
				if ( definitions.objects_in_category.find(ranged_weapon->category_of_magazine) != definitions.objects_in_category.end() )
				{
						list_of_names_for_categories::iterator k;
						int list_size = definitions.objects_in_category[ranged_weapon->category_of_magazine].size();
						k = definitions.objects_in_category[ranged_weapon->category_of_magazine].begin();
						int random_value = random(list_size);
						
						for (int a=0;a<random_value;a++,k++); // move to selected name
						
						selected_name = *k;

						from_definition=definitions.find_item(selected_name);
						
						if (from_definition==NULL)
						{
							fprintf(stderr,"ERROR: Try to choose non-existing ammo %s.\n",selected_name.c_str());
							exit(159);
						}

						AMMO *am = from_definition->IsAmmo();
						assert (am!=NULL);

						if (ranged_weapon->ammo_type==am->ammo_type)
						{
							ranged_weapon->ammo = *am; // copy ammo

							// number of missiles
							if (ranged_weapon->properties&TYPE_ENERGY)
							{
								ranged_weapon->ammo.quantity=1;
							}
							else
							{
								ranged_weapon->ammo.quantity=random(ranged_weapon->magazine_capacity) + random(ranged_weapon->magazine_capacity);
								if (ranged_weapon->ammo.quantity>ranged_weapon->magazine_capacity)
									ranged_weapon->ammo.quantity=ranged_weapon->magazine_capacity;
							}

						}
				}
			}

		 
     }

   }

   if (new_one!=NULL)
		new_one->ChangePosition(x,y);   

   return new_one;
}


