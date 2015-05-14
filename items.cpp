#include "mem_check.h"

#include "items.h"
#include "types.h"
#include "global.h"
#include "map.h"
#include "options.h"
#include "level.h"
#include "parser.h"
#include "tutorial.h"
#include "system.h"
#include "sounds.h"

#include <math.h>
#include <list>
using namespace std;

#include <assert.h>

extern MYSCREEN screen;
extern OPTIONS options;
extern DESCRIPTIONS descriptions;
extern DEFINITIONS definitions;
extern LEVEL level;
extern TUTORIAL tutorial;
extern SOUNDS sounds;

/*
  activities - DESCRIPTION
  g - get
  b - build robot
  d - drop
  t - throw
  w - wield
  L - load ammo (can be loaded with ammo)
  N - uNload ammo (some items can have load ammo but not option to unload)
  A - armor (wear - put on)
  u - use
  a - activate f.e. grenade
  r - ready
  m - mend
  p - program CPU
  
  // properties
  
  l - loadable (for ammo)
  i - countable - for packages
  
  

*/
bool ITEM::property_activate()
{
  if (this->activities.find('a')!=-1)
    return true;
  return false;
}

bool ITEM::property_program()
{
	if (this->activities.find('p')!=-1)
		return true;
	return false;
}

bool ITEM::property_drop()
{
  if (this->activities.find('d')!=-1)
    return true;
  return false;
}

bool ITEM::property_buildrobot()
{
	if (this->activities.find('b')!=-1)
		return true;
	return false;
}

bool ITEM::property_throw()
{
  if (this->activities.find('t')!=-1)
    return true;
  return false;
}

bool ITEM::property_wield()
{
  if (this->activities.find('w')!=-1)
    return true;
  return false;
}

bool ITEM::property_use()
{
	if (this->activities.find('u')!=-1)
		return true;
	return false;
}

bool ITEM::property_controller() // item sterujacy
{
  if (this->activities.find('!')!=-1)
    return true;
  return false;
}

bool ITEM::property_load_ammo()
{
  if (this->activities.find('L')!=-1)
    return true;
  return false;
}

bool ITEM::property_unload_ammo()
{
  if (this->activities.find('N')!=-1)
    return true;
  return false;
}

bool ITEM::property_put_on()
{
  if (this->activities.find('A')!=-1)
    return true;
  return false;
}

bool ITEM::property_to_load()
{
  if (this->activities.find('l')!=-1)
    return true;
  return false;
}

bool ITEM::property_join()
{
  if (this->activities.find('i')!=-1)
    return true;
  return false;
}

bool ITEM::property_get()
{
  if (this->activities.find('g')!=-1)
    return true;
  return false;
}

bool ITEM::property_ready()
{
	if (this->activities.find('r')!=-1)
		return true;
	return false;
}

bool ITEM::property_mend()
{
	if (this->activities.find('m')!=-1)
		return true;
	return false;
}


bool ITEM::ChangePosition(int x, int y)
{
	if (pX()==x && pY()==y)
		return false;

	if (on_ground)
	{
		level.map.DecraseNumberOfItems(pX(),pY());
		level.map.IncraseNumberOfItems(x,y);
	}
	TILE::ChangePosition(x,y);
	return true;
}

bool GAS::ChangePosition(int x, int y)
{
	if (pX()==x && pY()==y)
		return false;
	
	level.map.DecraseNumberOfGases(pX(),pY());
	level.map.IncraseNumberOfGases(x,y);
	TILE::ChangePosition(x,y);
	return true;
}

bool ITEM::IsDead()
{
	return is_dead;
}

bool ITEM::IsLoaded()
{
	if (property_load_ammo())
	{
		RANGED_WEAPON *ranged_weapon = IsShootingWeapon();  
		if (ranged_weapon!=NULL)
		{
			if (ranged_weapon->ammo.quantity==0)
				return false;
		}
	}
	return true;
}


HAND_WEAPON * ITEM::IsHandWeapon()
{
	return dynamic_cast<HAND_WEAPON *>(this);
};

RANGED_WEAPON * ITEM::IsShootingWeapon()
{
	return dynamic_cast<RANGED_WEAPON *>(this);
}

GRENADE * ITEM::IsGrenade()
{
	return dynamic_cast<GRENADE *>(this);
}

PILL * ITEM::IsPill()
{
	return dynamic_cast<PILL *>(this);
}

AMMO * ITEM::IsAmmo()
{
	return dynamic_cast<AMMO *>(this);
}

BASE_ARMOR * ITEM::IsArmor()
{
	return dynamic_cast<BASE_ARMOR *>(this);
}

CORPSE * ITEM::IsCorpse()
{
	return dynamic_cast<CORPSE *>(this);
}

TRASH * ITEM::IsGarbage()
{
	return dynamic_cast<TRASH *>(this);
}

REPAIR_KIT * ITEM::IsRepairSet()
{
	return dynamic_cast<REPAIR_KIT *>(this);
}

PROGRAMMATOR * ITEM::IsProgrammator()
{
	return dynamic_cast<PROGRAMMATOR *>(this);
}

BATTERY * ITEM::IsBattery()
{
	return dynamic_cast<BATTERY *>(this);
}

COUNTABLE * ITEM::IsCountable()
{
	return dynamic_cast<COUNTABLE *>(this);
}

PROCESSOR * ITEM::IsRobotCPU()
{
	return dynamic_cast<PROCESSOR *>(this);
}

ROBOT_LEG * ITEM::IsRobotLeg()
{
	return dynamic_cast<ROBOT_LEG *>(this);
}

ROBOT_SHELL * ITEM::IsRobotShell()
{
	return dynamic_cast<ROBOT_SHELL *>(this);
}



ITEM::ITEM()
{
	ClassName = "ITEM";

   category = 0;
   color = 7;   
   tile = '?';

   DMG=0;
   HIT=0;
   DEF=0;
   on_ground = false;
   energy_activated = -1;
   activities="dt"; // drop throw
   inventory_letter='*';
   purpose = PURPOSE_NONE;   
   weight=1;
   required_strength = 1;
   price=0;
   owner=NULL;
   in_robot_shell=NULL;
   properties = TYPE_NORMAL;
   is_dead=false;
   hit_points = 1;
   max_hit_points = 1;
   skill_to_use=SKILL_MELEE_WEAPONS;
}

ITEM::~ITEM()
{
	if (on_ground==true) // lays on ground
	{
		level.map.DecraseNumberOfItems(pX(),pY());
	}
}

AMMO::~AMMO()
{
}


BATTERY::BATTERY()
{
	ClassName = "BATTERY";
	tile = '$';
	activities+="g";
	max_capacity=0;
	regeneration_speed=0;
	capacity=0;	
}	

int ITEM::uses_energy(bool by_activation) // 0 - no changes, 1 - turned self off
{
	if (by_activation && energy_activated!=0)
		return 0;

	if (!by_activation && !(properties&TYPE_ENERGY))
		return 0;

	// if item can by powered

	if (owner==NULL)
	{
		this->turn_energy_off();
		return 1;
	}

	if (owner->use_energy(1)==false)
	{
		this->turn_energy_off();
		if (this->owner == level.player)
		{
			screen.console.add(this->show_name() + "은 동력이 소진되었다!",13);
		}			
		return 1;
	}
	return 0;	
}

int ITEM::evaluate_item_for_h2h_combat()
{
	int value;

	value = HIT*3 + DEF*3 + DMG;
	value *= 10;
	if (properties&TYPE_ARMOR_PERCING)
		value*=2;
	if (properties&TYPE_RADIOACTIVE)
		value=(value*3)/2;
	if (properties&TYPE_ELECTRIC)
		value=(value*3)/2;
	if (properties&TYPE_ENERGY)
		value=(value*3)/2;
	if (properties&TYPE_CHEM_POISON)
		value=(value*3)/2;
	if (properties&TYPE_PARALYZE)
		value=(value*3)/2;
	if (properties&TYPE_STUN)
		value=(value*3)/2;
	
	return value;
}

int RANGED_WEAPON::evaluate_item_for_ranged_combat()
{
	int value;
	value = (PWR + ammo.PWR)*20 + (ACC + ammo.ACC)*10;

	if (ammo.properties&TYPE_ARMOR_PERCING)
		value*=2;
	if (ammo.properties&TYPE_ELECTRIC)
		value=(value*3)/2;
	if (ammo.properties&TYPE_ENERGY)
		value=(value*3)/2;
	if (ammo.properties&TYPE_RADIOACTIVE)
		value=(value*3)/2;
	if (ammo.properties&TYPE_CHEM_POISON)
		value=(value*3)/2;
	if (ammo.properties&TYPE_PARALYZE)
		value=(value*3)/2;
	if (ammo.properties&TYPE_STUN)
		value=(value*3)/2;

	return value;	
}


string ITEM::article_a()
{
   char begin;
   string vowels=string("aeiouyAEIOUY");
   string digits=string("0123456789");
   begin=*show_name().c_str();
   if (digits.find(begin)!=-1)
   {
     return ("");
   }
   
   if (vowels.find(begin)!=-1)
   {
     return string("");
   }
   else
     return string("");
}

string ITEM::article_the()
{
   char begin;
   string digits=string("0123456789");
   begin=*show_name().c_str();
   if (digits.find(begin)!=-1)
   {
     return string("");
   }
   return string("그 ");
}


void ITEM::print_item_with_letter(int x,int y,string additional_text)
{
       string text;
       text="[ ] x "+ this->show_name();
       print_text(x,y,text+additional_text);
       set_color(14);
       print_character(x+1,y,this->inventory_letter);
	   this->draw_at(x+4,y);
}

// enemy - our target

bool RANGED_WEAPON::fire_missile(int x1, int y1, int x2, int y2,MONSTER * enemy, int distance)
{
	struct POSITION position[100];
	int cells, index;
	int tile_hit;
	int val;
	MONSTER *monster;
	bool terrain_hit;
	string descript;

	// use energy if weapon is activated (like laser scopes etc.)
	this->uses_energy(); 

	int power_multiplyer=1;

	if (properties&TYPE_ENERGY)
	{
		switch (fire_type)
		{
		case FIRE_DOUBLE:
			power_multiplyer=2;
			break;
		case FIRE_TRIPLE:
			power_multiplyer=3;
			break;
		case FIRE_BURST:
			power_multiplyer=6;
			break;
		}
	}
	else
		ammo.quantity--;

	cells=generate_bresenham_line(x1, y1, x2, y2, (POSITION *) position,100);
	int hit_power,hit_power_modifier;

	if (properties&TYPE_ENERGY)
		hit_power=random( this->PWR )/2 + this->PWR/2;
	else
		hit_power=random( this->PWR + this->ammo.PWR )/2 + this->ammo.PWR;

	hit_power*=power_multiplyer;
	terrain_hit=false;

	for (index=1;index<99;index++)
	{
		if (properties&TYPE_ENERGY)
		{
			set_color(this->color);
			int hx=position[index].x;
			int hy=position[index].y;
			bool was_seen=false;
			if (level.map.seen_by_player(hx,hy))
			{
				print_character(hx,hy,'*'); 
				myrefresh();
			}
		}
		if (index>=distance)
			terrain_hit=true;

		// HIT THE CELL
		if (level.map.blockMove(position[index].x,position[index].y) || terrain_hit)
		{
			set_color(14);

			// get random item
			int counter,randomized;
			ptr_list on_cell; ptr_list::iterator m;
			level.list_of_items_from_cell(&on_cell,position[index].x,position[index].y);
			ITEM *przedm; przedm=NULL;
			randomized=random(on_cell.size()+1);
			if (randomized!=0)
				for (m=on_cell.begin(),counter=0;counter<randomized;m++,counter++)
					przedm=(ITEM *)*m;
			on_cell.clear();
			// hit the item
			if (przedm!=NULL)
			{
				przedm->damage_it(random(hit_power)/2+hit_power/2);
			}

			// draw hit
			if (level.map.blockMove(position[index].x,position[index].y))
				tile_hit=level.map.getTile(position[index].x,position[index].y);
			else
				tile_hit='*';
			// damage terrain
			level.map.damage(position[index].x,position[index].y,random(hit_power)/2+hit_power/2);

			if (level.map.seen_by_player(position[index].x,position[index].y) ||
				level.map.seen_by_camera(position[index].x,position[index].y) )
			{
				print_character(position[index].x,position[index].y,tile_hit); // !!!
				myrefresh();
			}
			return false;
		}

		monster=level.monster_on_cell(position[index].x,position[index].y);
		// hit the monster?
		if (monster!=NULL)
		{      
			if (monster==enemy || enemy==NULL || enemy==this->owner) 
			{
				if (monster->evade_missile(NULL)==false || enemy==this->owner)
				{
					descript = ammo.name + "이 " + monster->name + "에 명중";
					descriptions.add_attack("HIT",descript);
					val = monster->hit_by_item(hit_power, (ITEM *)&ammo);
					if (val>0)
						descriptions.add_attack("HIT","하여 " + IntToStr(val) + "만큼의 피해를 입힌다");
					else if (val==0)
					{
						string text = descriptions.get_attack("HIT");
						if (text.find("damaging")==-1)
							descriptions.add_attack("HIT","하지만 아무 피해를 입히지 못한다");
					}
					else
						descriptions.add_attack("HIT","하여 목표를 살해한다");
					if (level.map.seen_by_player(position[index].x,position[index].y) ||
						level.map.seen_by_camera(position[index].x,position[index].y))
					{
						set_color(4);
						print_character(position[index].x,position[index].y,'*'); // !!!
						myrefresh();
					}
					if (!(properties&TYPE_ENERGY)) // energy goes further
						return true;
				}
			}
			if (monster!=enemy) // shooting above/by this one
			{
				if (enemy==NULL || lower_random(monster->size, monster->size + enemy->size*2)) // niestety, trafiony ten
				{
					if (monster->evade_missile(NULL)==false)
					{
						descript = ammo.name + "이 " + monster->name + "에 명중";
						descriptions.add_attack("HIT",descript);
						val = monster->hit_by_item(hit_power, (ITEM *)&ammo);
						if (val>0)
							descriptions.add_attack("HIT","하여 " + IntToStr(val) + "만큼의 피해를 입힌다");
						else if (val==0)
						{
							string text = descriptions.get_attack("HIT");
							if (text.find("damaging")==-1)
								descriptions.add_attack("HIT","하지만 아무런 피해를 입히지 못한다");
						}
						else
							descriptions.add_attack("HIT","하여 목표를 살해한다");

						if (level.map.seen_by_player(position[index].x,position[index].y) ||
							level.map.seen_by_camera(position[index].x,position[index].y))
						{
							set_color(4);
							print_character(position[index].x,position[index].y,'*'); // !!!
							myrefresh();
						}
						if (!(properties&TYPE_ENERGY)) // energy goes further
							return false;
					}
				}
			}
		}
	}
	return false;
}

void RANGED_WEAPON::print_item_with_letter(int x,int y,string additional_text)
{
       if (ammo.quantity>0)
         ITEM::print_item_with_letter(x,y," <" + ammo.show_name() + "> " + additional_text);
       else
         ITEM::print_item_with_letter(x,y, additional_text);
}

string ITEM::show_name()
{
	if (level.player!=NULL)
	{
		map_of_unknown_items::iterator mit;
		mit=definitions.constantly_unknown_items.find(name);
		if (mit!=definitions.constantly_unknown_items.end())
		{
			// is name hidden?
			if (level.player->is_item_known(name))
			{
				return name;
			}
			else
				return (*mit).second.name;
		}
		
	}
	return name;
}

string GRENADE::show_name()
{
   string name;
   name=ITEM::show_name();
   if (this->active==true)
    name+=" (작동중)";
   return name;
}

string HAND_WEAPON::show_name()
{
	string name;
	name=ITEM::show_name();
	if (this->energy_activated==true)
		name+=" (작동중)";
	return name;
}

string BASE_ARMOR::show_name()
{
	string name;
	name=ITEM::show_name();
	if (this->energy_activated==true)
		name+=" (작동중)";
	return name;
}


string PROCESSOR::show_name()
{
	string name;
	if (group_affiliation == GROUP_HERO)
		name = "우호적인 ";
	else if (group_affiliation == GROUP_ENEMIES)
		name = "적대적인 ";
	name+=ITEM::show_name();
	return name;
}

void CORPSE::every_turn()
{
   if (IsDead())
	   return;

   ITEM::every_turn();
   
   rotting_state++;
   if (rotting_state==1000)
   {
      rename(of_what+"의 썩은 시체");
   }
   if (rotting_state==2000)
   {
      color=7;
      this->weight/=10;
      rename(of_what+"의 뼈");
   }
   if (rotting_state==10000)
     this->death();
}

TRASH::TRASH()
{
	ClassName = "TRASH";
	
	activities+='g';
	tile=';';
}

REPAIR_KIT::REPAIR_KIT()
{
	ClassName = "REPAIR_KIT";
	
	regeneration_time=-1;
	regeneration_progress=-1;
	activities+="gm";
	tile='=';
}

PROGRAMMATOR::PROGRAMMATOR()
{
	ClassName = "PROGRAMMATOR";
	
	activities+="gp";
	tile='=';
}

PROCESSOR::PROCESSOR()
{
	ClassName = "PROCESSOR";
	
	activities+="gm";
	tile='-';
	color = 3;
	program="";
	frequency=20;
	where_placed = NULL;
	group_affiliation = GROUP_NEUTRAL;
}

ROBOT_LEG::ROBOT_LEG()
{
	ClassName = "ROBOT_LEG";
	
	activities+="gm";
	tile='-';
	color = 11;

	move_power = 10;
}

ROBOT_SHELL::ROBOT_SHELL()
{
	ClassName = "ROBOT_SHELL";

	last_robot_name = "Unnamed Robot";
	
	activities+="gbm";
	tile='R';
	color = 12;

	max_number_move_slots=0;
	max_number_action_slots=0;

	ARM=0;

	cpu = NULL;
}


CORPSE::CORPSE()
{
   ClassName = "CORPSE";

   activities+='g';
   tile='%';
   rotting_state=0;
}

int CORPSE::evaluate_item()
{
   return price;
}

int GRENADE::evaluate_item()
{
	if (active)
		return 0;
	return price + PWR + RNG;
}


int BASE_ARMOR::evaluate_item()
{
	return ARM*10+MOD_STR+MOD_DEX+MOD_SPD;
}


int PILL::evaluate_item()
{
	if (purpose == PURPOSE_INCRASE_RADIOACTIVE ||
		purpose == PURPOSE_INCRASE_CHEM_POISON ||
		purpose == PURPOSE_DECRASE_HP ||
		purpose == PURPOSE_DECRASE_SPEED ||
		purpose == PURPOSE_DECRASE_STRENGTH ||
		purpose == PURPOSE_DECRASE_ENDURANCE ||
		purpose == PURPOSE_DECRASE_INTELLIGENCE ||
		purpose == PURPOSE_DECRASE_METABOLISM ||
		purpose == PURPOSE_DECRASE_FOV ||
		purpose == PURPOSE_DECRASE_HP)
		return 0;
	else
		return price;
}


HAND_WEAPON::HAND_WEAPON()
{
	ClassName = "HAND_WEAPON";

	energy_DMG = 0;
	energy_HIT = 0;
	energy_DEF = 0;
	energy_properties=0;	
	energy_real_properties=TYPE_NORMAL;		

	tile='(';
   color=7;
   activities+="gwrm"; // wield
   skill_to_use=SKILL_MELEE_WEAPONS;   
}

int HAND_WEAPON::turn_energy_off()
{
	if (energy_activated==true)
	{
		DEF -= energy_DEF;
		HIT -= energy_HIT;
		DMG -= energy_DMG;
		energy_activated=false;
		properties = energy_real_properties;
		return true;
	}
	return false;
}

int HAND_WEAPON::turn_energy_on()
{
	if (energy_activated==false)
	{
		DEF += energy_DEF;
		HIT += energy_HIT;
		DMG += energy_DMG;
		energy_activated=true;
		properties = properties | energy_properties;
		return true;
	}
	return false;
}

int RANGED_WEAPON::turn_energy_off()
{
	if (energy_activated==true)
	{
		PWR -= energy_PWR;
		ACC -= energy_ACC;
		energy_activated=false;
		return true;
	}
	return false;
}

int RANGED_WEAPON::turn_energy_on()
{
	if (energy_activated==false)
	{
		PWR += energy_PWR;
		ACC += energy_ACC;
		energy_activated=true;
		return true;
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////
int BASE_ARMOR::turn_energy_off()
{
	if (energy_activated==true)
	{
		ARM -= energy_ARM;
		MOD_STR -= energy_MOD_STR;
		MOD_DEX -= energy_MOD_DEX;
		MOD_SPD -= energy_MOD_SPD;
		energy_activated=false;
		properties = energy_real_properties;
		return true;
	}
	return false;
}

int BASE_ARMOR::turn_energy_on()
{
	if (energy_activated==false)
	{
		ARM += energy_ARM;
		MOD_STR += energy_MOD_STR;
		MOD_DEX += energy_MOD_DEX;
		MOD_SPD += energy_MOD_SPD;
		energy_activated=true;
		properties = properties | energy_properties;
		return true;
	}
	return false;
}


bool ITEM::compare_activities_with (ITEM *object)
{
   if (this->activities == object->activities)
     return true;
   else
     return false;
}

COUNTABLE::COUNTABLE()
{
	activities+="i";
	quantity = 1;
}	

AMMO::AMMO()
{
	ClassName = "AMMO";

   tile='|';
   activities+="gl"; // loadable oraz ilosciowe
   color=3;
   weight=1;
   hit_points=1;
   DMG=-10;
   ACC=0;
   PWR=0;
   properties = 0;
   in_weapon = NULL;
   ammo_type=TYPE_NORMAL;
   price = 1;
}

BASE_ARMOR::BASE_ARMOR()
{
	ClassName = "BASE_ARMOR";

   MOD_STR=0;  
   MOD_DEX=0;  
   MOD_SPD=0; 

   energy_ARM=0;
   energy_MOD_STR=0;
   energy_MOD_DEX=0;
   energy_MOD_SPD=0;
   energy_properties=0;	
   energy_real_properties=TYPE_NORMAL;	   
	
   tile='[';
   activities+="gAm"; // wear
   color=7;
   weight=0;   
   properties=TYPE_NORMAL;
   price = 1;
}

NO_ARMOR::NO_ARMOR()
{
	ClassName = "NO_ARMOR";
	ARM=0;
}

ARMOR::ARMOR()
{
	ClassName = "ARMOR";
}

void COUNTABLE::add_and_destroy_object(ITEM *object)
{
	  COUNTABLE *am = object->IsCountable();
	  assert (object!=NULL);

      am->quantity += this->quantity;
      death(); // unicestwienie tego itemu
}

bool AMMO::compare_activities_with (ITEM *object)
{	
   AMMO *am = object->IsAmmo();
   if (am!=NULL)
   {
			 if ( am->properties  == this->properties &&
				  am->ammo_type == this->ammo_type )
			 return true;
   }
   return false;
}

bool PILL::compare_activities_with (ITEM *object)
{	
	PILL *p = object->IsPill();
	if (p!=NULL)
	{
		if ( p->purpose  == this->purpose &&
			p->PWR == this->PWR )
			return true;
	}
	return false;
}


string COUNTABLE::show_name()
{
	string temp;

	if (quantity==1)
		return ITEM::show_name();
	
	temp = IntToStr(quantity) + " x " + ITEM::show_name();
	return temp;
}

string AMMO::show_name()
{
   string name,to_add;
   name = COUNTABLE::show_name();

   to_add="";
   if (properties&TYPE_ARMOR_PERCING)
    to_add="AP";
   if (properties&TYPE_EXPLOSIVE)
   {
      if (to_add!="")
        to_add+="/EX";
      else
        to_add="EX";
   }
   if (properties&TYPE_ELECTRIC)
   {
	   if (to_add!="")
		   to_add+="/EM";
	   else
		   to_add="EM";
   }
   if (properties&TYPE_SMOKE)
   {
      if (to_add!="")
        to_add+="/SM";
      else
        to_add="SM";
   }
   if (properties&TYPE_RADIOACTIVE)
   {
      if (to_add!="")
        to_add+="/BP";
      else
        to_add="BP";
   }
   if (properties&TYPE_CHEM_POISON)
   {
      if (to_add!="")
        to_add+="/CP";
      else
        to_add="CP";
   }
   if (properties&TYPE_PARALYZE)
   {
      if (to_add!="")
        to_add+="/PA";
      else
        to_add="PA";
   }
   if (to_add!="")
   {
	   name+=string(" (") + to_add + ")";
   }
  
   return name;
}

bool RANGED_WEAPON::unload_ammo(INTELLIGENT *backpack_owner)
{
   ptr_list::iterator m,_m;
   ITEM *item;
   bool found;
   AMMO *am;

   if (this->ammo.quantity==0)
      return false;
   
 // 1. Find this ammo in backpack
   found=false;

   for(m=backpack_owner->backpack.begin(),_m=backpack_owner->backpack.end(); m!=_m; m++)
   {
       item=(ITEM *)*m;
	   am = item->IsAmmo();
       if ( am!=NULL ) // ammo
         if (this->ammo.name == item->name)
         {
           found=true;
           break;
         }
   }
   if (found==true)
   {
      am->quantity += this->ammo.quantity;
      this->ammo.quantity=0;
   }
   else
   {
      // create item
	   am = (AMMO *) ammo.duplicate();

      this->ammo.quantity=0;
      backpack_owner->pick_up_item(am,false);
   }
   return true;
 
}

bool REPAIR_KIT::can_be_used()
{
	if (regeneration_time>0)
	{
		if (regeneration_progress!=regeneration_time)
			return false;
	}
	return true;
}

bool REPAIR_KIT::fix(ITEM *item, MONSTER *who_fixes)
{
	if (purpose==PURPOSE_REPAIR_HP)
	{
		int points=random(who_fixes->skill[SKILL_MECHANIC]/4)+who_fixes->skill[SKILL_MECHANIC]/4;
		
		if (item->IsCountable())
			return false;
		if (item->IsRobotCPU())
			points/=4;
		else if (item->IsGrenade())
			points/=2;
		else if (item->IsRobotLeg())
			points/=2;
		
		item->fix_damage(points);
	}
	else if (purpose==PURPOSE_REPAIR_FIX_ARMOR)
	{
		BASE_ARMOR *pan;
		pan = item->IsArmor();
		if (pan==NULL || (pan->MOD_SPD>=0 && pan->MOD_STR>=0 && pan->MOD_DEX>=0))
			return false;
		
		// choose one negative to fix
		while (1)
		{
			int a=random(3);
			if (a==0)
			{
				if (pan->MOD_SPD<0)
				{
					if (pan->MOD_SPD<=-5)
						pan->MOD_SPD+=5;
					else
						pan->MOD_SPD=0;
					break;
				}
			}
			else if (a==1 && pan->MOD_DEX<0)
			{
				pan->MOD_DEX++;
				break;
			}
			else if (a==2 && pan->MOD_STR<0)
			{
				pan->MOD_STR++;
				break;
			}
		}

	}

	if (this->regeneration_time<0)
		death();
	else
	{
		regeneration_progress=0;
	}

	if (who_fixes==level.player)
		level.player->skill_used(SKILL_MECHANIC);	
	return true;
}


bool ITEM::fix_with_item(ITEM *parts, MONSTER *who_fixes)
{
	REPAIR_KIT *repairset;
	repairset = parts->IsRepairSet();
	if (repairset==NULL)
		return false;

	if (repairset->can_be_used())
	{
		repairset->fix(this, who_fixes);
		if (who_fixes==level.player)
		{
			level.player->skill_used(SKILL_MECHANIC);
		}
		return true;
	}	
	return false;
}

bool BASE_ARMOR::fix_with_item(ITEM *parts, MONSTER *who_fixes)
{
	REPAIR_KIT *repairset;
	repairset = parts->IsRepairSet();
	if (repairset!=NULL)
		return ITEM::fix_with_item(parts, who_fixes);
	BASE_ARMOR *ar;
	ar = parts->IsArmor();
	if (ar!=NULL) 
	{
		this->max_hit_points = (this->max_hit_points + ar->max_hit_points)/2;

		if (this->name == ar->name)
			this->hit_points += ar->hit_points;
		else
		{
			float quality1 = (float) this->hit_points/(float) this->max_hit_points;
			float quality2 = (float) ar->hit_points/(float) ar->max_hit_points;

			float quality=quality1+quality2;

			quality1 = quality1/quality;
			quality2 = quality2/quality;

//			int a = (int)((float) (quality1*(float) this->BLK));
//			int b = (int)((float) (quality2*(float) ar->BLK));
			
			this->hit_points += (int)(float)(quality2*ar->hit_points);
			this->max_hit_points = (int)(float)(quality1*this->max_hit_points + quality2*ar->max_hit_points);
			this->ARM = (int)(float)(quality1*this->ARM + quality2*ar->ARM);
//			this->BLK = (int)(float)(quality1*this->BLK + quality2*ar->BLK);

			if (name.find("개조된 ")==-1)
				rename("개조된 " + name);
		}

		if (this->hit_points > this->max_hit_points)
			this->hit_points = this->max_hit_points;

		parts->death();
		return true;
	}

	return false;
}


int RANGED_WEAPON::load_ammo(AMMO *a_ammo)
{
   if (this->ammo_type!=a_ammo->ammo_type)
     return 1; /// niezgodny type amunicji
   if (this->ammo.quantity>0)
     return 2; /// weapon juz zaladowana
     
   ammo=*a_ammo; // przekopiowanie amunicji do magazynka
   ammo.in_weapon = this;
   ammo.owner = NULL;
   ammo.in_robot_shell = NULL;
   
   if (this->magazine_capacity < a_ammo->quantity)
   { // nie all wlazly
     ammo.quantity=magazine_capacity;
     a_ammo->quantity-=magazine_capacity;
     return 0; // bez problemow zaladowane
   }
   else /// all pociski wlazly do magazynka
   {
     ammo.quantity=a_ammo->quantity;
     a_ammo->quantity=0;

     // pozbywamy sie zerowej liczby pociskow
     a_ammo->death(); // a nastepnie niszczymy go
     return 0; // magazynek przestaje istniec
   }
}

RANGED_WEAPON::RANGED_WEAPON()
{
	ClassName = "RANGED_WEAPON";

   tile='\\';
   energy_PWR=0;
   energy_ACC=0;
   color=3;
   activities+="gwLNrm"; // get wield Load uNload
   ammo.quantity=0;
   ammo.weight=0;
   hit_points=1;
   max_hit_points=1;
   ACC=0;
   PWR=0;
   fire_type=FIRE_SINGLE;
   category_of_magazine = 0;
}


int AMMO::evaluate_item()
{
   int value;
   value=price+PWR*10+ACC*10;
   value=value*quantity;
   return value;
}

int RANGED_WEAPON::evaluate_item()
{
   int value;
   value=price+ACC*10;
   value=value*(hit_points/max_hit_points);
   return value;
}



int HAND_WEAPON::evaluate_item()
{
   int value;
   value=price+DMG*10+HIT*10+DEF*10;
   value=value*(hit_points/max_hit_points);
   return value;
}


int ITEM::used_by_on(MONSTER *who, MONSTER *on_who, int a_skill)
{
   int power;
   int damage;
   int skill;

   power = (who->strength.GetValue() + who->skill[a_skill]/10) * this->DMG;
   power /= 10;

   if (power<0)
     power=0;

   int result = on_who->hit_by_item(random(power),this);
   if (result>=0)
   {
	   if (who->seen_now || on_who->seen_now)
	   {
		   ITEM *sound=(ITEM *) definitions.find_item(this->name);
		   if (sound!=NULL)
			   sounds.PlaySoundOnce(sound->sound_used);
	   }
   }
   if (result>0)
   {
	   if (this->properties&TYPE_VAMPIRIC)
	   {
		   if (&who->unarmed==this && (on_who->IsAnimal() || on_who->IsIntelligent()))
		   {		   
			   if (who->heal(random(result)/2+result/2)==true && who->seen_now==true)
			   {			   
				   descriptions.add_attack("HIT",", 피를 빨아들이고,");
			   }
		   }
	   }
   }
   return result;
}

int ITEM::calculate_weight()
{
   return weight;
}

int COUNTABLE::calculate_weight()
{
	return weight*quantity;
}

int ROBOT_SHELL::calculate_weight()
{
	int total=weight;

	if (cpu!=NULL)
		total += cpu->calculate_weight();

	ptr_list::iterator m,_m;
	ITEM *t;

	for (m=move_slots.begin(),_m=move_slots.end();m!=_m;m++)
	{
		t = (ITEM *)*m;
		total+=t->calculate_weight();
	}

	for (m=action_slots.begin(),_m=action_slots.end();m!=_m;m++)
	{
		t = (ITEM *)*m;
		total+=t->calculate_weight();
	}
	
	return total;
}

int RANGED_WEAPON::calculate_weight()
{
   return weight+ammo.calculate_weight();
}

void ITEM::print_attributes_vertical(int x, int y, int *changex, int *changey)
{
  print_text(x,y,IntToStr(calculate_weight()) + " 그램");
  print_text(x,y+2,"내구도: "+IntToStr(hit_points)+"/"+IntToStr(max_hit_points));
}

void TRASH::print_attributes_vertical(int x,int y, int *changex, int *changey)
{
	ITEM::print_attributes_vertical(x,y,changex,changey);
	*changex=16;
	*changey=4;
}

void PROGRAMMATOR::print_attributes_vertical(int x,int y, int *changex, int *changey)
{
	ITEM::print_attributes_vertical(x,y,changex,changey);
	*changex=16;
	*changey=4;
}

void BATTERY::print_attributes_vertical(int x,int y, int *changex, int *changey)
{
	ITEM::print_attributes_vertical(x,y,changex,changey);
	print_text(x,y+3,string("용량: ") + IntToStr(max_capacity) + " EP");
	if (regeneration_speed>0)
		print_text(x,y+4,string("충전속도: ") + IntToStr(regeneration_speed) + "/1");
	else if (regeneration_speed<0)
		print_text(x,y+4,string("충전속도: 1/") + IntToStr(-regeneration_speed));

	*changex=16;
	*changey=6;
}


void REPAIR_KIT::print_attributes_vertical(int x,int y, int *changex, int *changey)
{
	ITEM::print_attributes_vertical(x,y,changex,changey);

	y+=4;
	string text;

	if (regeneration_time>0)
	{
		int progress = 100*regeneration_progress/regeneration_time;
		text = "잔량: ";
		text+=IntToStr(progress) + "%";
		print_text(x,y++,text);
		*changey=7;
	}
	else
		*changey=6;
	
	
	*changex=16;
}

void PILL::print_attributes_vertical(int x,int y, int *changex, int *changey)
{
	ITEM::print_attributes_vertical(x,y,changex,changey);
	*changex=16;
	*changey=5;
}

void CORPSE::print_attributes_vertical(int x,int y, int *changex, int *changey)
{
  ITEM::print_attributes_vertical(x,y,changex,changey);
  *changex=16;
  *changey=4;
}

void HAND_WEAPON::print_attributes_vertical(int x,int y, int *changex, int *changey)
{
           ITEM::print_attributes_vertical(x,y,changex,changey);
           y+=4;
           string text;

		   if (this->DMG>=0)
			   text=string("근거리 위력: +");
		   else
			   text=string("근거리 위력: ");
		   text+=IntToStr(this->DMG);
		   print_text(x,y,text);

           if (this->HIT>=0)
               text=string("근거리 명중: +");
           else
               text=string("근거리 명중: ");
           text+=IntToStr(this->HIT);
           print_text(x,y+1,text);
           

           if (this->DEF>=0)
               text=string("근거리 방어: +");
           else
               text=string("근거리 방어: ");
           text+=IntToStr(this->DEF);
           print_text(x,y+2,text);

           *changex=16;
           *changey=7;
}

void RANGED_WEAPON::print_attributes_vertical(int x,int y, int *changex, int *changey)
{
           ITEM::print_attributes_vertical(x,y,changex,changey);
           y+=4;
           string text;
           if (this->ACC>=0)
               text=string("원거리 명중: +");
           else
               text=string("원거리 명중: ");
           text+=IntToStr(this->ACC);
           print_text(x,y,text);

           if (this->PWR>=0)
               text=string("원거리 위력: +");
           else
               text=string("원거리 위력: ");
           text+=IntToStr(this->PWR);
           print_text(x,y+1,text);

           if (this->HIT>=0)
               text=string("근거리 명중: +");
           else
               text=string("근거리 명중: ");
           text+=IntToStr(this->HIT);
           print_text(x,y+3,text);
           

           if (this->DEF>=0)
			   text=string("근거리 방어: +");
           else
               text=string("근거리 방어: ");
           text+=IntToStr(this->DEF);
           print_text(x,y+4,text);

           if (this->DMG>=0)
               text=string("근거리 위력: +");
           else
               text=string("근거리 위력: ");
           text+=IntToStr(this->DMG);
           print_text(x,y+5,text);

           
           if (ammo.quantity>0)
           {
             print_text(x,y+7,ammo.show_name());
             ammo.print_attributes_vertical(x,y+8,changex,changey);
             // set size to name
             if (ammo.show_name().size()>*changex)
               *changex=ammo.show_name().size();
             *changey+=12;
           }
           else
           {
             text="탄: 0/";
             text+=IntToStr(this->magazine_capacity);
             print_text(x,y+7,text);
             *changex=16;
             *changey=12;
           }
}

void AMMO::print_attributes_vertical(int x,int y, int *changex, int *changey)
{
			print_text(x,y,IntToStr(calculate_weight()) + " 그램");
			string text;
		   *changey=1;
           if (this->ACC!=0)
		   {
			   text=string("원거리 명중: ");
			   if (this->ACC>0)
				   text+="+";
			   text+=IntToStr(this->ACC);
			   print_text(x,y+*changey,text);
			   (*changey)++;
		   }

           if (this->PWR!=0)
		   {
			   if (properties&TYPE_ENERGY)
				   text=string("동력 소모량: ");
			   else
					text=string("원거리 위력: ");
			   if (this->PWR>0)
				   text+="+";
			   text+=IntToStr(this->PWR);
			   print_text(x,y+*changey,text);
		   }


           *changex=16;
		   *changey+=2;
}

void GRENADE::print_attributes_vertical(int x,int y, int *changex, int *changey)
{
           ITEM::print_attributes_vertical(x,y,changex,changey);
           y+=4;
           string text;
           text=string("위력: ") + IntToStr(this->PWR);
           print_text(x,y,text);

           text=string("범위: ") + IntToStr(this->RNG);
           print_text(x,y+1,text);
           if (this->active==true)
             text=string("작동됨!");
           else
             text=string("미사용");
           print_text(x,y+3,text);

           *changex=16;
           *changey=8;
}

void ARMOR::print_attributes_vertical(int x,int y, int *changex, int *changey)
{
           ITEM::print_attributes_vertical(x,y,changex,changey);
           y+=4;
           string text;
           text=string("방어: ") + IntToStr(this->ARM);
           print_text(x,y,text);

		   int zmy=0;

		   if (MOD_STR!=0)
		   {
			   zmy++;
			   if (MOD_STR>0)
				text=string(" 힘 : ") + "+" + IntToStr(this->MOD_STR);
			   else 
				text=string(" 힘 : ") + IntToStr(this->MOD_STR);
			   print_text(x,y+zmy,text);
		   }
		   if (MOD_DEX!=0)
		   {
			   zmy++;
			   if (MOD_DEX>0)
				text=string("재주: ") + "+" + IntToStr(this->MOD_DEX);
			   else
				text=string("재주: ") + IntToStr(this->MOD_DEX);
			   print_text(x,y+zmy,text);
		   }
		   if (MOD_SPD!=0)
		   {
			   zmy++;
			   if (MOD_SPD>0)
				text=string("속도: ") + "+" + IntToStr(this->MOD_SPD);
			   else
				text=string("속도: ") + IntToStr(this->MOD_SPD);
			   print_text(x,y+zmy,text);
		   }
		   
           *changex=16;
           *changey=6+zmy;
}

void PROCESSOR::print_attributes_vertical(int x,int y, int *changex, int *changey)
{
	ITEM::print_attributes_vertical(x,y,changex,changey);
	y+=4;
	string text;
	text=string(" 정밀도 : ") + IntToStr(this->quality);
	print_text(x,y,text);
	
	text=string("  속도  : ") + IntToStr(this->frequency);
	print_text(x,y+1,text);
	
	text=string("프로그램: ") + IntToStr(this->program.size());
	print_text(x,y+2,text);
	
	*changex=16;
	*changey=8;
}

void ROBOT_LEG::print_attributes_vertical(int x,int y, int *changex, int *changey)
{
	ITEM::print_attributes_vertical(x,y,changex,changey);
	y+=4;
	string text;
	text=string("기동력: ") + IntToStr(this->move_power);
	print_text(x,y,text);
		
	*changex=15;
	*changey=7;
}


void ROBOT_SHELL::print_attributes_vertical(int x,int y, int *changex, int *changey)
{
	ITEM::print_attributes_vertical(x,y,changex,changey);
	y+=4;
	string text;
	text=string("이동 슬롯: ") + IntToStr(this->max_number_move_slots);
	print_text(x,y,text);

	ptr_list::iterator m;
	int a;
	ITEM *t;
	for (a=0, m=move_slots.begin(); a<move_slots.size(); a++)
	{
		t = (ITEM *)*m;
		text = t->show_name().substr(0,23);
		print_text(x,y+1+a,text);		
	}

	text=string("행동 슬롯: ") + IntToStr(this->max_number_action_slots);
	print_text(x,y+2+move_slots.size(),text);

	for (a=0, m=action_slots.begin(); a<action_slots.size(); a++,m++)
	{
		t = (ITEM *)*m;
		text = t->show_name().substr(0,23);
		print_text(x,y+3+move_slots.size() + a,text);
	}
	
		
	*changex=23;
	*changey=8 + action_slots.size() + move_slots.size();
}

void ITEM::death()
{
   if (this->is_dead)
	   return;
   activities = "";

   if (level.player!=NULL)
   {
	   if (this == level.player->last_activated)
	   {
		   level.player->last_activated = NULL;
	   }
   }

   if (owner!=NULL)
   {
	   if (this==owner->weapon)
		   owner->weapon = &owner->unarmed;
	   
	   if (this==owner->armor)
	   {
		   INTELLIGENT *intel = owner->IsIntelligent();
		   if (intel!=NULL)
			  intel->remove_armor();
		   else
		   {
			   assert(0);
			   owner->armor = &owner->no_armor;   // tego nie powinno byc
		   }
	   }
	   
	   INTELLIGENT *intel = owner->IsIntelligent();
	   if (intel!=NULL)
	   {
		   intel->take_out_from_backpack(this);
		   if (this==intel->ready_weapon)
				intel->ready_weapon = NULL;
	   }
   } 
   // not in backpack and not shell 
   if (this->in_robot_shell!=NULL) // is item in a robot shell?
   {
	   ROBOT_SHELL *shell = in_robot_shell->IsRobotShell();
	   assert(shell!=NULL);

	   if (this==shell->cpu)
	   {
		   shell->uninstall_CPU();
	   }
	   else
	   {
		   ptr_list::iterator m,_m;
		   ITEM *temp, *found=NULL;
		   // znalezienie itemu na listach
		   m=shell->action_slots.begin();
		   _m=shell->action_slots.end();
		   for (;m!=_m;m++)
		   {
			   temp = (ITEM *) *m;
			   if (temp==this)
			   {
				   found = this;
				   shell->uninstall_from_action_slot(this);
				   break;
			   }
		   }
		   if (found==NULL)
		   {
			   m=shell->move_slots.begin();
			   _m=shell->move_slots.end();
			   for (;m!=_m;m++)
			   {
				   temp = (ITEM *) *m;
				   if (temp==this)
				   {
					   shell->uninstall_from_move_slot(this);
					   break;
				   }
			   }
			   
		   } // endof if			   
	   } // endof else it's not processor
   }

   level.remove_from_items_on_map(this); // if is on level, then remove
   // we don't remove now from all items list, to prevent damage of ptr_list in iteration of time_for_items
   level.items_to_delete.push_back(this);
   is_dead=true;
}

int NO_ARMOR::damage_it(int value)
{   
   return owner->cause_damage(value);
}

int ITEM::damage_it(int value)
{
  if (value<=0)
	  return DAMAGE_NONE;

  hit_points-=value;
      
  if (hit_points<=0)
  {
    this->death();
    return DAMAGE_DEATH;
  }
  return value;
}

int ITEM::fix_damage(int value)
{
	if (value<=0)
		return 0;
	
	hit_points+=value;
	
	if (hit_points>max_hit_points)
	{
		hit_points=max_hit_points;
		return 1;
	}
	return 1;
}


void BASE_ARMOR::death()
{
	if (owner!=NULL && this==owner->armor)
	{
		INTELLIGENT *intel = owner->IsIntelligent();
		if (intel!=NULL)
			intel->set_armor(&owner->no_armor);
	}	       
	
	ITEM::death();
}

void HAND_WEAPON::death()
{
	if (owner!=NULL && this==owner->weapon)
	{
		INTELLIGENT *intel = owner->IsIntelligent();
		if (intel!=NULL)
			intel->set_weapon(&owner->unarmed);
	}	       
	
	ITEM::death();
}

int RANGED_WEAPON::damage_it(int value)
{
  int which_damage = ITEM::damage_it(value);
  if (this->IsDead())
  {
	 if (owner!=NULL && this==owner->weapon)
	 {
		 INTELLIGENT *intel = owner->IsIntelligent();
		 if (intel!=NULL)
			 intel->set_weapon(&owner->unarmed);
	 }	       
  }
  return which_damage;
}


int GRENADE::damage_it(int value)
{
  hit_points-=value;
  if (hit_points<=0)
  {
     if (properties&TYPE_SENSOR)
     {
	   // remove sensor type from item
       PROPERTIES mask;
       mask=TYPE_SENSOR;
       mask=~mask;
       properties=properties&mask;
     }
     active=false;
     works_now=false;
     activate();
     counter=0;
  }
  return 0;
}


int COUNTABLE::damage_it(int value)
{
  if (value<=0)
	  return DAMAGE_NONE;

  quantity-=value;
  if (quantity<=0)
  {
     death();
	 return DAMAGE_DEATH;
  }
  return value;
}

int COUNTABLE::fix_damage(int value)
{
	return 0;
}


void RANGED_WEAPON::incrase_fire_mode()
{
   switch (fire_type)
   {
     case FIRE_SINGLE:
         if (available_fire_types&FIRE_DOUBLE)
         {
           fire_type=FIRE_DOUBLE;
           break;
         }
     case FIRE_DOUBLE:
         if (available_fire_types&FIRE_TRIPLE)
         {
           fire_type=FIRE_TRIPLE;
           break;
         }
     case FIRE_TRIPLE:
         if (available_fire_types&FIRE_BURST)
         {
           fire_type=FIRE_BURST;
           break;
         }

   }
}

void RANGED_WEAPON::decrase_fire_mode()
{
   switch (fire_type)
   {
     case FIRE_BURST:
         if (available_fire_types&FIRE_TRIPLE)
         {
           fire_type=FIRE_TRIPLE;
           break;
         }
     case FIRE_TRIPLE:
         if (available_fire_types&FIRE_DOUBLE)
         {
           fire_type=FIRE_DOUBLE;
           break;
         }
     case FIRE_DOUBLE:
         if (available_fire_types&FIRE_SINGLE)
         {
           fire_type=FIRE_SINGLE;
           break;
         }

   }
}


GRENADE::GRENADE()
{
	ClassName = "GRENADE";

   tile='`';
   color=5;
   activities+="gam"; // activate
   counter=-1;
   active=false;
   works_now=false;
   properties=TYPE_NORMAL;
   weight=400;
   hit_points=20;
   max_hit_points=20;
   PWR=0;
   RNG=0;
   price = 1; // aby nie 0, bo potworki nie podnosza wtedy
}

int GRENADE::create_shield()
{
    int x,y;
    int x1,y1;
    int dist;

	INTELLIGENT *intel=NULL;

    if (owner!=NULL)
	{
		intel = owner->IsIntelligent();
		if (intel!=NULL)
           intel->drop_item(this,false);
	}

    for (x=-RNG;x<=RNG;x++)
    for (y=-RNG;y<=RNG;y++)
    {
         x1=pX()+x;
         y1=pY()+y;
         dist=distance(pX(),pY(),x1,y1);
		 if (DLY==0 && dist==2) // dla personal shield
			 dist--;

         if (dist <= RNG )
         {
             level.map.addShield(x1,y1,PWR);
         }
    }
  return 0;
}

int GRENADE::remove_shield()
{
    int x,y;
    int x1,y1;
    int dist;

    for (x=-RNG;x<=RNG;x++)
    for (y=-RNG;y<=RNG;y++)
    {
         x1=pX()+x;
         y1=pY()+y;
         dist=distance(pX(),pY(),x1,y1);
		 if (DLY==0 && dist==2) // dla personal shield
			 dist--;

         if (dist <= RNG )
         {
           level.map.removeShield(x1,y1);
         }
    }
    return 0;
}

void ITEM::every_turn()
{
	if (purpose == PURPOSE_TEMP_MISSILE)
	{
		death();
	}
	if (owner==NULL && energy_activated==true)
	{
		turn_energy_off();
	}
	if (owner!=NULL)
	{
		ChangePosition(owner->pX(),owner->pY());
	}
	else if (in_robot_shell!=NULL)
	{
		ChangePosition(in_robot_shell->pX(),in_robot_shell->pY());
	}	
}

void ARMOR::every_turn()
{
	ITEM::every_turn();
	if (owner!=NULL && this->energy_activated)
	{
		if (energy_MOD_DEX>0 || energy_MOD_STR || energy_MOD_SPD || energy_ARM)
			uses_energy();
	}	
}

void REPAIR_KIT::every_turn()
{
	if (regeneration_time>0)
		regeneration_progress++;

	if (regeneration_progress>regeneration_time)
		regeneration_progress=regeneration_time;
}

void BATTERY::every_turn()
{
	if (owner!=NULL && owner->energy_points.GetValue()<max_capacity)
	{
		if (regeneration_speed>0)
		{
			owner->add_energy(regeneration_speed);
		}
		else if (regeneration_speed<0)
		{
			if (random(-regeneration_speed)==0)
				owner->add_energy(1);
		}
	}	
}

void GRENADE::every_turn()
{
   if (IsDead())
	   return;

   ITEM::every_turn();

   bool exploded = false;

   // if not active and not works now or it's sensor and works now
   // and no one have it in backpack
   // or if someone has and it's not a power shield
   if (((active==true && !works_now) || (properties&TYPE_SENSOR && works_now)) && (owner==NULL || properties&TYPE_POWER_SHIELD))
   {
	   counter--;
	   if (counter<=0)
	   {
		   works_now=true;
		   if (properties&TYPE_SENSOR && works_now)
		   {
			   if (is_monster_around()==false)
			   {
				   counter=0;
				   return;
			   }		   
		   }              
		   if (properties&TYPE_STUN && !(properties&TYPE_SMOKE))
		   {
			   if (level.map.seen_by_player(pX(),pY()) ||
				   level.map.seen_by_player(pX(),pY()))		   
			   {
				   level.player->add_known_item(name);
			   }		   
			   stun_explosion();
			   exploded = true;
		   }
		   if (properties&TYPE_EXPLOSIVE)
		   {
			   if (level.map.seen_by_player(pX(),pY()))		   
			   {
				   level.player->add_known_item(name);
			   }		   
			   explode();
			   exploded = true;
		   }
		   else if (properties&TYPE_ELECTRIC)
		   {
			   if (level.map.seen_by_player(pX(),pY()))		   
			   {
				   level.player->add_known_item(name);
			   }		   
			   emp_explosion();
			   exploded = true;
		   }
		   else if (properties&TYPE_INCENDIARY)
		   {
			   if (level.map.seen_by_player(pX(),pY()))		   
			   {
				   level.player->add_known_item(name);
			   }		   
			   fire_explosion();
			   exploded = true;
		   }
		   else if (properties&TYPE_POWER_SHIELD) // power shield sie uaktywnia od razu
		   {
			   if (level.map.seen_by_player(pX(),pY()))		   
			   {
				   level.player->add_known_item(name);
			   }		   
			   if (PWR!=0)
				   create_shield();
			   else
				   remove_shield();

			   death();
			   return;
		   }
		   if (properties&TYPE_SMOKE)
		   {
			   counter=PWR;     
			   if (level.map.seen_by_player(pX(),pY()))	
			   {
				   GRENADE *sound=(GRENADE *) definitions.find_item(name);
				   if (sound!=NULL)
					   sounds.PlaySoundOnce(sound->sound_explosion);

			   }
		   }

	   }     
   }
   if (exploded == true)
   {
		this->death();
		return;
   }
   
   if (works_now)
   {
       counter--;
       
       if (properties&TYPE_SMOKE)
       {
		   if (level.map.seen_by_player(pX(),pY()))		   
		   {
			   level.player->add_known_item(name);
		   }		   
           if (counter>=0)
             produce_gas();
           else
             this->death();
       }
   }
}

bool ITEM::is_monster_around()
{
   if (level.map.IsMonsterOnMap(pX(),pY()))
     return true;
   if (level.map.IsMonsterOnMap(pX(),pY()-1))
     return true;
   if (level.map.IsMonsterOnMap(pX(),pY()+1))
     return true;
   if (level.map.IsMonsterOnMap(pX()-1,pY()))
     return true;
   if (level.map.IsMonsterOnMap(pX()+1,pY()))
     return true;
   if (level.map.IsMonsterOnMap(pX()-1,pY()-1))
     return true;
   if (level.map.IsMonsterOnMap(pX()+1,pY()-1))
     return true;
   if (level.map.IsMonsterOnMap(pX()-1,pY()+1))
     return true;
   if (level.map.IsMonsterOnMap(pX()+1,pY()+1))
     return true;
   return false;
}

int GRENADE::produce_gas()
{
    level.add_gas(this->pX(),this->pY(),properties,this->RNG*5);
	return 0;
}

int GRENADE::activate()
{
    if (active!=true)
    {
       counter=DLY;
       active=true;
       hit_points = 1; // active jest bardzo wrazliwy (aby mozna bylo strzalem np. mine rozwalic)
	   return true;
    }
  return false;
}

int HAND_WEAPON::activate()
{
	if (energy_activated==-1)
		return 0;

	if (energy_activated==false)
	{
		this->turn_energy_on();
		if (uses_energy()==true)
			return false;
		return true;
	}
	else if (energy_activated==true)
	{
		this->turn_energy_off();
		return true;
	}
	return false;
}

int RANGED_WEAPON::activate()
{
	if (energy_activated==-1)
		return 0;
	
	if (energy_activated==false)
	{
		this->turn_energy_on();
		if (uses_energy()==true)
			return false;
		return true;
	}
	else if (energy_activated==true)
	{
		this->turn_energy_off();
		return true;
	}
	return false;
}

int BASE_ARMOR::activate()
{
	if (energy_activated==-1)
		return 0;
	
	if (energy_activated==false)
	{
		this->turn_energy_on();
		if (uses_energy()==true)
			return false;
		return true;
	}
	else if (energy_activated==true)
	{
		this->turn_energy_off();
		return true;
	}
	return false;
}

int GRENADE::calculate_explosion(int *data)
{
    int x,y;
    int index;
    int x1,y1;
    int dist;
    int power;
    struct POSITION position[100]; // tyle sie miescie na ekranie po przekatnej

    for (x=0;x<MAPWIDTH;x++)
     for (y=0;y<MAPHEIGHT;y++)
       data[x+y*MAPWIDTH]=0;
     

    // count bresenhamem places, gdzie dociera fala wybuchu

    for (x=-RNG;x<=RNG;x++)
    for (y=-RNG;y<=RNG;y++)
    {
     if (x==-RNG || x==RNG || y==-RNG || y==RNG)
     {
       generate_bresenham_line(this->pX(),this->pY(),this->pX()+x,this->pY()+y, (POSITION *) position, RNG );
       for (index=0;index<RNG;index++) // kolejne pola
       {
         x1=position[index].x;
         y1=position[index].y;         
         
         if (!level.map.onMap(x1,y1))
           break;

         dist=distance(pX(),pY(),x1,y1);
         if (dist >= RNG )
           data[x1+y1*MAPWIDTH]=-1;
           
         if (data[x1+y1*MAPWIDTH]==-1) // to pole it_blocks
           break;
           
         // czy cos it_blocks
         if (level.map.blockMove(x1,y1)==true)
         {
           if (level.map.onMap(position[index+1].x,position[index+1].y))
             data[position[index+1].x+(position[index+1].y)*MAPWIDTH]=-1;
         }
         power=PWR-dist*(PWR/RNG);
         
         if (power<1)
           power=1;
           
         data[x1+y1*MAPWIDTH]=power;
       } // endof for
     } // endof if
    } // endof for
  return 0;
}

int GRENADE::explode()
{
    int x,y;
    ptr_list on_cell;
    ptr_list::iterator m,_m;
    ITEM *temp;

    MONSTER *monster;

    int data[MAPWIDTH*MAPHEIGHT+1];
    calculate_explosion((int*) &data);

    if (screen.draw_explosion(data, PWR+1))
	{
		GRENADE *sound=(GRENADE *) definitions.find_item(name);
		if (sound!=NULL)
			sounds.PlaySoundOnce(sound->sound_explosion);
	}

	// add damage on cells, where explosion reach
    on_cell.clear();
    
    for (x=0;x<MAPWIDTH;x++)
     for (y=0;y<MAPHEIGHT;y++)
     {
       if (data[x+y*MAPWIDTH]>0)
       {
         // add smoke
         level.add_gas(x,y,TYPE_SMOKE,random(data[x+y*MAPWIDTH]*20));

         // do damage
         monster=level.monster_on_cell(x,y);
         if (monster!=NULL)
           monster->hit_by_explosion(random(data[x+y*MAPWIDTH]/2)+data[x+y*MAPWIDTH]/2);
           
         // damage itms
         level.list_of_items_from_cell(&on_cell,x,y);
         for(m=on_cell.begin(),_m=on_cell.end(); m!=_m; m++)
         {
           temp=(ITEM *)*m;
           temp->damage_it(random(data[x+y*MAPWIDTH]/2)+data[x+y*MAPWIDTH]/2);
         }
         on_cell.clear();
         
         // damage terrain
         level.map.damage(x,y,random(data[x+y*MAPWIDTH]/2)+data[x+y*MAPWIDTH]/2);
       }
     }
     
  return 0;
}


int GRENADE::fire_explosion()
{
    int x,y;

    MONSTER *monster;

    int data[MAPWIDTH*MAPHEIGHT+1];

    calculate_explosion((int*) &data);

	bool fire_visible=level.map.seen_by_player(pX(),pY());

	for (x=0;x<MAPWIDTH;x++)
		for (y=0;y<MAPHEIGHT;y++)
		{
			if (data[x+y*MAPWIDTH]>0)
			{
				if (level.map.seen_by_player(x,y))
					fire_visible=true;

				// dodanie ognia		  
				if (random(6)!=0)
				{
					monster=level.monster_on_cell(x,y);
					if (monster!=NULL)
						monster->hit_changes_status(TYPE_INCENDIARY,random(PWR));

					if (!level.map.blockMove(x,y))
						level.add_gas(x,y,TYPE_INCENDIARY,random(PWR));
				}
				else if (!level.map.blockMove(x,y))			 
					level.add_gas(x,y,TYPE_SMOKE,random(PWR*50));
			}
		}
	if (fire_visible)
	{
		GRENADE *sound=(GRENADE *) definitions.find_item(name);
		if (sound!=NULL)
			sounds.PlaySoundOnce(sound->sound_explosion);
	}

  return 0;
}

int GRENADE::stun_explosion()
{
    int x,y;
    MONSTER *monster;

    int data[MAPWIDTH*MAPHEIGHT+1];
    calculate_explosion((int*) &data);

    for (x=0;x<MAPWIDTH;x++)
     for (y=0;y<MAPHEIGHT;y++)
     {
       if (data[x+y*MAPWIDTH]>0)
       {
         monster=level.monster_on_cell(x,y);
         if (monster!=NULL)
			monster->hit_changes_status(TYPE_STUN,(random(data[x+y*MAPWIDTH]/2)+data[x+y*MAPWIDTH]/2));
       }
     }
	 if (screen.draw_stunning_explosion(data))
	 {
		 GRENADE *sound=(GRENADE *) definitions.find_item(name);
		 if (sound!=NULL)
			 sounds.PlaySoundOnce(sound->sound_explosion);
	 }


  return 0;
}

int GRENADE::emp_explosion()
{
	int x,y;
	MONSTER *monster;

	int data[MAPWIDTH*MAPHEIGHT+1];
	calculate_explosion((int*) &data);

	if (screen.draw_emp_explosion(data, PWR+1))
	{
		GRENADE *sound=(GRENADE *) definitions.find_item(name);
		if (sound!=NULL)
			sounds.PlaySoundOnce(sound->sound_explosion);
	}

	for (x=0;x<MAPWIDTH;x++)
		for (y=0;y<MAPHEIGHT;y++)
		{
			if (data[x+y*MAPWIDTH]>0)
			{
				monster=level.monster_on_cell(x,y);
				if (monster!=NULL)
				{
					if (monster->IsRobot())
						monster->hit_changes_status(TYPE_ELECTRIC,(random(data[x+y*MAPWIDTH]/2)+data[x+y*MAPWIDTH]/2));
				}
			}
		}
		return 0;
}


void FIRE::every_turn()
{
   if (is_dead)
	   return;

   if (random(20)==0)
     density--;

   if (density<=0 || level.map.blockMove(pX(),pY()))
   {
     death();
     return;
   }

   level.add_gas(pX(),pY(),TYPE_SMOKE,random(8)+5);
   
   act_on_monster();
   act_on_items();

/// move fire
   if (random(40)==0)
   {
   int flow;
   
   flow=random(density);
   density-=flow;
   

   switch(random(8))
   {
     case 0:
     level.add_gas(pX()-1,pY(),properties,flow);
     break;
     case 1:
     level.add_gas(pX()+1,pY(),properties,flow);
     break;
     case 2:
     level.add_gas(pX(),pY()+1,properties,flow);
     break;
     case 3:
     level.add_gas(pX(),pY()-1,properties,flow);
     break;
     case 4:
     level.add_gas(pX()-1,pY()-1,properties,flow);
     break;
     case 5:
     level.add_gas(pX()+1,pY()-1,properties,flow);
     break;
     case 6:
     level.add_gas(pX()-1,pY()+1,properties,flow);
     break;
     case 7:
     level.add_gas(pX()+1,pY()+1,properties,flow);
     break;
   }
   }
}


void GAS::every_turn()
{
   if (this->is_dead)
	   return;

   act_on_monster();   

   int podzial,x,y;
   if (density<=0)
   {
     death();
     return;
   }
   density--;
   podzial=density/2;

   x=this->pX();
   y=this->pY();

    if (podzial>1)
    {
     if (random(6)==0)
     level.add_gas(x-1,y,properties,random(podzial));
     if (random(6)==0)
     level.add_gas(x+1,y,properties,random(podzial));
     if (random(6)==0)
     level.add_gas(x,y+1,properties,random(podzial));
     if (random(6)==0)
     level.add_gas(x,y-1,properties,random(podzial));

     if (random(6)==0)
     level.add_gas(x-1,y-1,properties,random(podzial));
     if (random(6)==0)
     level.add_gas(x+1,y-1,properties,random(podzial));
     if (random(6)==0)
     level.add_gas(x-1,y+1,properties,random(podzial));
     if (random(6)==0)
     level.add_gas(x+1,y+1,properties,random(podzial));
   
    }
   density=random(density/3)+density/2;
}

void GAS::death()
{
   if (this->is_dead)
	   return;

   this->is_dead = true;
   level.map.backBlockLOS(this->pX(),this->pY());
   ChangePosition(-1,-1);
   // don't delete from gases_on_map directly, to avoid iteration damage in ptr_list during time_for_gases
   level.gases_to_delete.push_back(this);
}

GAS::GAS()
{
	ClassName = "GAS";

	tile='&';
	is_dead = false;
}

FIRE::FIRE()
{
	ClassName = "FIRE";
}

void FIRE::death()
{
   GAS::death();
}


void GAS::display()
{
     if (density>50)
     {
       if (density>300)
       {
        level.map.setBlockLOS(this->pX(),this->pY());
       }
       else
        level.map.backBlockLOS(this->pX(),this->pY());
     
       if (density>200)
	   {
		   if (properties&TYPE_CHEM_POISON)
		   {
			   color=2;
		   }
		   else if (properties&TYPE_RADIOACTIVE)
		   {
			   color=5;
		   }
		   else if (properties&TYPE_STUN)
		   {
			   color=7;
		   }
		   else if (properties&TYPE_PARALYZE)
		   {
			   color=1;
		   }
		   else // np. TYPE_SMOKE
		   {
			   color=8;
		   }
	   }
       else
	   {
		   if (properties&TYPE_CHEM_POISON)
		   {
			   color=10;
		   }
		   else if (properties&TYPE_RADIOACTIVE)
		   {
			   color=13;
		   }
		   else if (properties&TYPE_STUN)
		   {
			   color=15;
		   }
		   else if (properties&TYPE_PARALYZE)
		   {
			   color=9;
		   }
		   else // other f.e. TYPE_SMOKE
		   {
			   color=7;
		   }
	   }
       TILE::display();
       level.map.setLastTile(pX(),pY(),' ');
     }
}


void FIRE::display()
{
     char kolory[5]={8,4,12,14,15};
     TILE::display();

     if (density>20)
       color=kolory[ random(2)+3 ];
     else if (density>10)
       color=kolory[ random(2)+2 ];
     else if (density>4)
       color=kolory[ random(2)+1 ];
     else color=kolory[ random(2)+0 ];
     
     TILE::display();     
}


CONTROLLER::CONTROLLER()
{
	ClassName = "CONTROLLER";
	tile = '$';

	invisible = true;
	activities = '!';
	price = 0;
}

PILL::PILL()
{
	ClassName = "PILL";
	tile = ',';

	DMG=-10;
	HIT=-10;
	DEF=-10;
	hit_points = 3;
	max_hit_points = 3;
	weight = 10;
	price = 100;
	
	activities += "gu";
}

int PILL::use_on(MONSTER *who)
{
	int power;
	if (who==level.player)
	{
		screen.console.add("그 알약을 먹는다.",11);		
		sounds.PlaySoundOnce("data/sounds/other/swallow.wav");
	}
	
	power = random(PWR/2) + PWR/2;
	switch(purpose) {
	case PURPOSE_INCRASE_HP:
		if (who->hit_points.GetValue() != who->hit_points.max)
		{
			if (who==level.player)
			{
				level.player->stop_repeating();
				screen.console.add("미세한 로봇들이 상처를 치료한다.",11);		
				level.player->add_known_item(name);
			}
			else if (who->seen_now)
				screen.console.add(who->name + "은 치료받는다.",11);
			who->heal(power);
		}
		else if (who==level.player)
			screen.console.add("아무 일도 일어나지 않은 것처럼 보인다.",11);				
		break;
	case PURPOSE_DECRASE_HP:
		if (who==level.player)
		{
			level.player->stop_repeating();
			screen.console.add("온몸에서 고통이 느껴진다!",11);		
			level.player->add_known_item(name);
		}
		else if (who->seen_now)
			screen.console.add(who->name + "은 고통에 몸부림친다.",11);
		who->cause_damage(power);
		break;
	case PURPOSE_INCRASE_RADIOACTIVE:
		if (who==level.player)
		{
			screen.console.add("방사능에 오염된 느낌이다!!",11);		
			level.player->add_known_item(name);
		}
		else if (who->seen_now)
			screen.console.add(who->name + "은 방사능에 오염된 것 같다.",11);		
		who->state[STATE_RADIOACTIVE]+=power;
		break;
	case PURPOSE_DECRASE_RADIOACTIVE:
		if (who->state[STATE_RADIOACTIVE]>0)
		{
			if (who==level.player)
			{
				level.player->stop_repeating();
				screen.console.add("방사능이 배출된 느낌이다.",11);		
				level.player->add_known_item(name);
			}
			else if (who->seen_now)
				screen.console.add(who->name + "은 방사능을 배출한 것 같다.",11);		
			who->state[STATE_RADIOACTIVE]-=power;
			if (who->state[STATE_RADIOACTIVE]<0)
				who->state[STATE_RADIOACTIVE]=0;
		}
		else if (who==level.player)
			screen.console.add("아무 일도 일어나지 않은 것처럼 보인다.",11);		
		break;
	case PURPOSE_INCRASE_CHEM_POISON:
		if (who==level.player)
		{
			level.player->stop_repeating();
			screen.console.add("피가 썩어 들어간다!",11);		
			level.player->add_known_item(name);
		}
		else if (who->seen_now)
			screen.console.add(who->name + "은 중독된 것 같다.",11);		
		who->state[STATE_CHEM_POISON]+=power;
		break;
	case PURPOSE_DECRASE_CHEM_POISON:
		if (who->state[STATE_CHEM_POISON]>0)
		{
			if (who==level.player)
			{
				level.player->stop_repeating();
				screen.console.add("피가 다시 맑아지는 기분이다.",11);		
				level.player->add_known_item(name);
			}
			else if (who->seen_now)
				screen.console.add(who->name + "은 해독된 것처럼 보인다.",11);		
			who->state[STATE_CHEM_POISON]-=power;
			if (who->state[STATE_CHEM_POISON]<0)
				who->state[STATE_CHEM_POISON]=0;
		}
		else if (who==level.player)
			screen.console.add("아무 일도 일어나지 않은 것처럼 보인다.",11);		
		
		break;
	case PURPOSE_INCRASE_SPEED:
		if (who->state[STATE_SPEED]<40)
		{
			if (who==level.player)
			{
				level.player->stop_repeating();
				screen.console.add("갑자기 주변의 모든 것이 느려진 것 같다.",11);		
				level.player->add_known_item(name);
			}
			else if (who->seen_now)
				screen.console.add(who->name + "은 움직임이 빨라졌다.",11);		
			who->state[STATE_SPEED]+=power;
		}
		else if (who==level.player)
			screen.console.add("아무 일도 일어나지 않은 것처럼 보인다.",11);		
		
		break;
	case PURPOSE_DECRASE_SPEED:
		if (who->state[STATE_SPEED]>-40)
		{
			if (who==level.player)
			{
				level.player->stop_repeating();
				screen.console.add("갑자기 주변의 모든 것이 빨라진 것 같다.",11);		
				level.player->add_known_item(name);
			}
			else if (who->seen_now)
				screen.console.add(who->name + "은 움직임이 느려졌다.",11);		
			who->state[STATE_SPEED]+=power;
		}
		else if (who==level.player)
			screen.console.add("아무 일도 일어나지 않은 것처럼 보인다.",11);		
		
		break;
	case PURPOSE_INCRASE_STRENGTH:
		if (who->state[STATE_STRENGTH]<60)
		{
			if (who==level.player)
			{
				level.player->stop_repeating();
				screen.console.add("힘이 세진 느낌이다.",11);		
				level.player->add_known_item(name);
			}
			else if (who->seen_now)
				screen.console.add(who->name + "은 힘이 세진 것 같다.",11);		
			who->state[STATE_STRENGTH]+=power;
		}
		else if (who==level.player)
			screen.console.add("아무 일도 일어나지 않은 것처럼 보인다.",11);		
		
		break;
	case PURPOSE_DECRASE_STRENGTH:
		if (who->state[STATE_STRENGTH]>60)
		{
			if (who==level.player)
			{
				level.player->stop_repeating();
				screen.console.add("힘이 약해진 느낌이다.",11);		
				level.player->add_known_item(name);
			}
			else if (who->seen_now)
				screen.console.add(who->name + "은 힘이 약해진 것 같다.",11);		
			who->state[STATE_STRENGTH]+=power;
		}
		else if (who==level.player)
			screen.console.add("아무 일도 일어나지 않은 것처럼 보인다.",11);		
		
		break;
	}
	quantity--;
	if (quantity<=0)
		death();
	return 0;
}

void PILL::every_turn()
{
	ITEM::every_turn();	
}

void CONTROLLER::every_turn()
{	
   if (IsDead())
	   return;

	if (purpose == PURPOSE_AUTO_DOOR_OPEN)
	{
			if (is_monster_around()==true)
			{
				if (level.map.open(pX(),pY())==true) // otworzyly sie
				{
					if (level.map.seen_by_player(pX(),pY()))
					{
						if (!level.player->is_repeating_action() || 
							 level.player->is_repeating_action() && !(abs(pX()-level.player->pX())<2 && abs(pY()-level.player->pY())<2)
						   )
						screen.console.add("자동문이 저절로 열린다.",7);
						sounds.PlaySoundOnce("data/sounds/other/openclose.wav");
					}
				}
			}
			else if (level.map.isClosable(pX(),pY()))
			{
				if (level.map.close(pX(),pY())==true)
				{
					if (level.map.seen_by_player(pX(),pY()))
					{
						if (!level.player->is_repeating_action())
							screen.console.add("자동문이 저절로 닫힌다.",7);
						sounds.PlaySoundOnce("data/sounds/other/openclose.wav");
					}
				}
			}
			if (level.map.getPercentDamage(pX(),pY())>5) // > 5%
				death();
	}
	else if (purpose == PURPOSE_WEAK_CORRIDOR_IN_MINES)
	{
		if (level.map.getPercentDamage(pX(),pY())>5) // > 5%
		{
			MONSTER *on_cell;

			level.map.CollapseCeiling(this->pX(),this->pY());			

			for (int a=-1;a<=1;a++)
				for (int b=-1;b<=1;b++)
				{
					on_cell = level.monster_on_cell(this->pX()+a,this->pY()+b);
					if (on_cell!=NULL)
					{
						if (on_cell->seen_now)
						{
							screen.console.add(on_cell->name + "은 무너진 천장에 깔려 부서졌다.",4);
						}
						// monster on cell die
						if (a==0 && b==0)
							on_cell->death();					
						else
							on_cell->cause_damage(50+random(50));					
					}
				}
			death();			
		}
	}
	else if (purpose == PURPOSE_SMOKE_GENERATOR)
	{
		if (random(30)==0)
		{
			level.add_gas(pX(),pY(),TYPE_SMOKE,20000);			
		}
	}
	else if (purpose == PURPOSE_TUTORIAL_ADVANCE)
	{
		if (level.player!=NULL)
		{
			if ( abs(level.player->pX()-pX())<2 && abs(level.player->pY()-pY())<2 )
			{
				tutorial.show_next();
				death();
			}
		}
	}
	
		
}

ITEM * TRASH::duplicate()
{
	TRASH *new_one;
	new_one=new TRASH;
	*new_one=*this;

	map_of_unknown_items::iterator mit;
	mit=definitions.constantly_unknown_items.find(name);
	if (mit!=definitions.constantly_unknown_items.end())
	{
		new_one->color = (*mit).second.color;
	}

	new_one->owner=NULL;
	new_one->in_robot_shell=NULL;
	new_one->rename(this->name);

	level.add_to_items_on_map(new_one);
	level.all_items.push_back(new_one);
	
	return (ITEM *) new_one;
}

ITEM * REPAIR_KIT::duplicate()
{
	REPAIR_KIT *new_one;
	new_one=new REPAIR_KIT;
	*new_one=*this;
	
	map_of_unknown_items::iterator mit;
	mit=definitions.constantly_unknown_items.find(name);
	if (mit!=definitions.constantly_unknown_items.end())
	{
		new_one->color = (*mit).second.color;
	}
	
	new_one->regeneration_progress = new_one->regeneration_time;
	new_one->owner=NULL;
	new_one->in_robot_shell=NULL;
	new_one->rename(this->name);
	
	level.add_to_items_on_map(new_one);
	level.all_items.push_back(new_one);
	
	return (ITEM *) new_one;
}

ITEM * PROGRAMMATOR::duplicate()
{
	PROGRAMMATOR *new_one;
	new_one=new PROGRAMMATOR;
	*new_one=*this;
	
	map_of_unknown_items::iterator mit;
	mit=definitions.constantly_unknown_items.find(name);
	if (mit!=definitions.constantly_unknown_items.end())
	{
		new_one->color = (*mit).second.color;
	}
	
	new_one->owner=NULL;
	new_one->in_robot_shell=NULL;
	new_one->rename(this->name);
	
	level.add_to_items_on_map(new_one);
	level.all_items.push_back(new_one);	
	return (ITEM *) new_one;
}

ITEM * BATTERY::duplicate()
{
	BATTERY *new_one;
	new_one=new BATTERY;
	*new_one=*this;
	
	map_of_unknown_items::iterator mit;
	mit=definitions.constantly_unknown_items.find(name);
	if (mit!=definitions.constantly_unknown_items.end())
	{
		new_one->color = (*mit).second.color;
	}
	
	new_one->owner=NULL;
	new_one->in_robot_shell=NULL;
	new_one->rename(this->name);
	new_one->capacity = random(new_one->max_capacity/2) + new_one->max_capacity/2;
	
	level.add_to_items_on_map(new_one);
	level.all_items.push_back(new_one);	
	return (ITEM *) new_one;
}

ITEM * PROCESSOR::duplicate()
{
	PROCESSOR *new_one;
	new_one=new PROCESSOR;
	*new_one=*this;
	map_of_unknown_items::iterator mit;
	mit=definitions.constantly_unknown_items.find(name);
	if (mit!=definitions.constantly_unknown_items.end())
	{
		new_one->color = (*mit).second.color;
	}
	
	new_one->owner=NULL;
	new_one->in_robot_shell=NULL;	
	new_one->rename(this->name);
    level.add_to_items_on_map(new_one);
    level.all_items.push_back(new_one);
	return (ITEM *) new_one;
}

ITEM * ROBOT_LEG::duplicate()
{
	ROBOT_LEG *new_one;
	new_one=new ROBOT_LEG;
	*new_one=*this;
	map_of_unknown_items::iterator mit;
	mit=definitions.constantly_unknown_items.find(name);
	if (mit!=definitions.constantly_unknown_items.end())
	{
		new_one->color = (*mit).second.color;
	}
	
	new_one->owner=NULL;
	new_one->in_robot_shell=NULL;
	new_one->rename(this->name);
    level.add_to_items_on_map(new_one);
    level.all_items.push_back(new_one);
	return (ITEM *) new_one;
}

ITEM * ROBOT_SHELL::duplicate()
{
	ITEM *temp, *temp2;
	ptr_list::iterator m,_m;

	ROBOT_SHELL *new_one;
	new_one=new ROBOT_SHELL;
	*new_one=*this;
	map_of_unknown_items::iterator mit;
	mit=definitions.constantly_unknown_items.find(name);
	if (mit!=definitions.constantly_unknown_items.end())
	{
		new_one->color = (*mit).second.color;
	}
	
	new_one->owner=NULL;
	new_one->in_robot_shell=NULL;
	new_one->rename(this->name);
	
	new_one->action_slots.clear();
	new_one->move_slots.clear();

	// copy legs

	for (m=move_slots.begin(),_m=move_slots.end();m!=_m;m++)
	{
		temp = (ITEM *) *m;
		temp2 = temp->duplicate();

		level.remove_last_item_on_map();
		new_one->install_in_move_slot(temp2);
	}

	// copy "hands"

	for (m=action_slots.begin(),_m=action_slots.end();m!=_m;m++)
	{
		temp = (ITEM *) *m;
		temp2 = temp->duplicate();
		level.remove_last_item_on_map();
		new_one->install_in_action_slot(temp2);
	}

	// and cpu

	if (cpu!=NULL)
	{
		new_one->cpu=NULL;
		temp2 = cpu->duplicate();
		level.remove_last_item_on_map();
		new_one->install_CPU(temp2);
	}
	
    level.add_to_items_on_map(new_one);
    level.all_items.push_back(new_one);
	return (ITEM *) new_one;
}

ITEM * GRENADE::duplicate()
{
   GRENADE *new_one;
   new_one=new GRENADE;
   *new_one=*this;

   map_of_unknown_items::iterator mit;
   mit=definitions.constantly_unknown_items.find(name);
   if (mit!=definitions.constantly_unknown_items.end())
   {
	   new_one->color = (*mit).second.color;
   }
   
   new_one->owner=NULL;
   new_one->in_robot_shell=NULL;
   new_one->rename(this->name);
   level.add_to_items_on_map(new_one);
   level.all_items.push_back(new_one);
   return (ITEM *) new_one;
}

ITEM * CONTROLLER:: duplicate()
{
   CONTROLLER *new_one;
   new_one=new CONTROLLER;
   *new_one=*this;
   new_one->owner=NULL;
   new_one->in_robot_shell=NULL;
   new_one->rename(this->name);
   level.add_to_items_on_map(new_one);
   level.all_items.push_back(new_one);
   return (ITEM *) new_one;
}

ITEM * PILL:: duplicate()
{
	PILL *new_one;
	new_one=new PILL;
	*new_one=*this;

	map_of_unknown_items::iterator mit;
	mit=definitions.constantly_unknown_items.find(name);
	if (mit!=definitions.constantly_unknown_items.end())
	{
		new_one->color = (*mit).second.color;
	}
	
	new_one->owner=NULL;
	new_one->in_robot_shell=NULL;
	new_one->rename(this->name);
    level.add_to_items_on_map(new_one);
    level.all_items.push_back(new_one);
	return (ITEM *) new_one;
}


ITEM * RANGED_WEAPON::duplicate()
{
   RANGED_WEAPON *new_one;
   new_one=new RANGED_WEAPON;
   *new_one=*this;

   map_of_unknown_items::iterator mit;
   mit=definitions.constantly_unknown_items.find(name);
   if (mit!=definitions.constantly_unknown_items.end())
   {
	   new_one->color = (*mit).second.color;
   }
   
   new_one->owner=NULL;
   new_one->in_robot_shell=NULL;
   new_one->rename(this->name);   
   new_one->ammo.in_weapon = new_one;
   level.add_to_items_on_map(new_one);
   level.all_items.push_back(new_one);
   return (ITEM *) new_one;
}

ITEM * AMMO::duplicate()
{
   AMMO *new_one;
   new_one=new AMMO;
   *new_one=*this;

   map_of_unknown_items::iterator mit;
   mit=definitions.constantly_unknown_items.find(name);
   if (mit!=definitions.constantly_unknown_items.end())
   {
	   new_one->color = (*mit).second.color;
   }
   
   new_one->owner=NULL;
   new_one->in_robot_shell=NULL;
   new_one->in_weapon=NULL;
   new_one->rename(this->name);   
   level.add_to_items_on_map(new_one);
   level.all_items.push_back(new_one);
   return (ITEM *) new_one;
}

ITEM * ARMOR::duplicate()
{
   ARMOR *new_one;
   new_one=new ARMOR;
   *new_one=*this;

   map_of_unknown_items::iterator mit;
   mit=definitions.constantly_unknown_items.find(name);
   if (mit!=definitions.constantly_unknown_items.end())
   {
	   new_one->color = (*mit).second.color;
   }
   
   new_one->owner=NULL;
   new_one->in_robot_shell=NULL;
   new_one->rename(this->name);   
   level.add_to_items_on_map(new_one);
   level.all_items.push_back(new_one);
   return (ITEM *) new_one;
}

ITEM * NO_ARMOR::duplicate()
{
   NO_ARMOR *new_one;
   new_one=new NO_ARMOR;
   *new_one=*this;

   map_of_unknown_items::iterator mit;
   mit=definitions.constantly_unknown_items.find(name);
   if (mit!=definitions.constantly_unknown_items.end())
   {
	   new_one->color = (*mit).second.color;
   }
   
   new_one->owner=NULL;
   new_one->in_robot_shell=NULL;
   new_one->rename(this->name);   
   level.add_to_items_on_map(new_one);
   level.all_items.push_back(new_one);
   return (ITEM *) new_one;
}

ITEM * BASE_ARMOR::duplicate()
{
   BASE_ARMOR *new_one;
   new_one=new BASE_ARMOR;
   *new_one=*this;

   map_of_unknown_items::iterator mit;
   mit=definitions.constantly_unknown_items.find(name);
   if (mit!=definitions.constantly_unknown_items.end())
   {
	   new_one->color = (*mit).second.color;
   }
   
   new_one->owner=NULL;
   new_one->in_robot_shell=NULL;
   new_one->rename(this->name);   
   level.add_to_items_on_map(new_one);
   level.all_items.push_back(new_one);
   return (ITEM *) new_one;
}

ITEM * HAND_WEAPON::duplicate()
{
   HAND_WEAPON *new_one;
   new_one=new HAND_WEAPON;
   *new_one=*this;

   map_of_unknown_items::iterator mit;
   mit=definitions.constantly_unknown_items.find(name);
   if (mit!=definitions.constantly_unknown_items.end())
   {
	   new_one->color = (*mit).second.color;
   }
   
   new_one->owner=NULL;
   new_one->in_robot_shell=NULL;
   new_one->rename(this->name);
   level.add_to_items_on_map(new_one);
   level.all_items.push_back(new_one);
   return (ITEM *) new_one;
}

ITEM * CORPSE::duplicate()
{
   CORPSE *new_one;
   new_one=new CORPSE;
   *new_one=*this;

   map_of_unknown_items::iterator mit;
   mit=definitions.constantly_unknown_items.find(name);
   if (mit!=definitions.constantly_unknown_items.end())
   {
	   new_one->color = (*mit).second.color;
   }
   
   new_one->owner=NULL;
   new_one->in_robot_shell=NULL;
   new_one->rename(this->name);
   level.add_to_items_on_map(new_one);
   level.all_items.push_back(new_one);
   return (ITEM *) new_one;
}

void FIRE::act_on_monster()
{
   int random_value;
   MONSTER *monster;
   monster=level.monster_on_cell(pX(),pY());
   if (monster!=NULL)
   {
     random_value = random(density/4) + density/4;
     if (random_value!=0)
       monster->hit_changes_status(TYPE_INCENDIARY,random_value);
   }
}

void GAS::act_on_monster()
{
	MONSTER *monster;
	monster=level.monster_on_cell(pX(),pY());
	if (monster!=NULL)
	{
		if (properties!=TYPE_SMOKE) // gdy nie tylko sam dym
		{
			if (density>200)
				density=200;
			monster->hit_changes_status(this->properties,density/40);
		}
	}
}

void FIRE::act_on_items()
{
    ptr_list on_cell;
    ptr_list::iterator m,_m;
    ITEM *item;
    int random_value;

    level.list_of_items_from_cell(&on_cell,pX(),pY());
    for(m=on_cell.begin(),_m=on_cell.end(); m!=_m; m++)
    {
      item=(ITEM *)*m;
      random_value=random(density/3);
      if (random_value>0)
        item->damage_it(random_value);
    }
}

bool ROBOT_SHELL::install_CPU(ITEM *item)
{
	INTELLIGENT *CPU_owner=NULL;
	PROCESSOR *proc;

	if (item==NULL)
		return false;	

	proc=item->IsRobotCPU();
	if (proc==NULL)
		return false;

	if (proc->owner!=NULL)
		CPU_owner = proc->owner->IsIntelligent();

	if (CPU_owner!=NULL)
		CPU_owner->drop_item(proc,false);

	// lays on map, so move to robot
	level.remove_from_items_on_map(proc);
	cpu = proc;
	cpu->in_robot_shell = this;
	return true;
}

bool ROBOT_SHELL::install_in_action_slot(ITEM *item)
{
	INTELLIGENT *item_owner=NULL;

	if (item==NULL)
		return false;
	
	if (action_slots.size()>=max_number_action_slots)
		return false;

	if (item->owner!=NULL)
		item_owner = item->owner->IsIntelligent();
	
	if (item_owner!=NULL)
		item_owner->drop_item(item,false);
	
	// lays on map, so move to robot
	level.remove_from_items_on_map(item);	

	action_slots.push_back(item);
	item->in_robot_shell = this;	
	return true;
}

bool ROBOT_SHELL::install_in_move_slot(ITEM *item)
{
	INTELLIGENT *item_owner=NULL;
	ROBOT_LEG *leg;
	
	if (item==NULL)
		return false;

	leg = item->IsRobotLeg();
	if (leg==NULL)
		return false;
	
	if (move_slots.size()>=max_number_move_slots)
		return false;
	
	if (item->owner!=NULL)
		item_owner = item->owner->IsIntelligent();
	
	if (item_owner!=NULL)
		item_owner->drop_item(item,false);
	
	// lays on map, so move to robot
	level.remove_from_items_on_map(item);	
	
	move_slots.push_back(item);
	item->in_robot_shell = this;		
	return true;
}

bool ROBOT_SHELL::uninstall_CPU()
{
	if (cpu==NULL)
		return false;

	separate_item(cpu);	
	cpu = NULL;	
	ROBOT *rob;
	rob = owner->IsRobot();
	if (rob!=NULL) 
	{
		if (rob->shell == this)
		{
			rob->death();
		}
	}
	return true;
}

bool ROBOT_SHELL::uninstall_from_action_slot(ITEM *item)
{
	if (item==NULL)
		return false;

	separate_item(item);

	action_slots.remove(item);	
	return true;
}

bool ROBOT_SHELL::uninstall_from_move_slot(ITEM *item)
{
	if (item==NULL)
		return false;
	
	separate_item(item);
	move_slots.remove(item);
	return true;
}

bool ROBOT_SHELL::separate_item(ITEM *item)
{
	// if shell has an owner, then put it to the owner's backpack

	item->in_robot_shell=NULL;
	
	// lays on map, so move to robot
	if (this->owner!=NULL)
	{
		item->ChangePosition(owner->pX(),owner->pY());
		level.add_to_items_on_map(item);
		INTELLIGENT *intel = owner->IsIntelligent();
		if (intel!=NULL)
			intel->pick_up_item(item,false);
	}
	else
	{
		item->ChangePosition(pX(),pY());	
		level.add_to_items_on_map(item);	
	}
	return true;
}

void ROBOT_SHELL::death()
{
	ROBOT *rob = NULL;
	if (owner!=NULL)
		rob = owner->IsRobot();
	if (rob!=NULL)
	{
		if (rob->seen_now)
		{
			MONSTER *sound=(MONSTER *) definitions.find_monster(rob->name);
			if (sound!=NULL)
				sounds.PlaySoundOnce(sound->sound_dying);		

		}
		rob->death();
	}
	owner = NULL;	

	ITEM *temp;
	// uninstall all
	uninstall_CPU();
	while(action_slots.size()>0)
	{
		temp = (ITEM *) *action_slots.begin();
		uninstall_from_action_slot(temp);
	}
	while(move_slots.size()>0)
	{
		temp = (ITEM *) *move_slots.begin();
		uninstall_from_move_slot(temp);
	}
	
	ITEM::death();
}
