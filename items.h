// Dodajac new_one wlasciwosc do itemu pamietaj o dodaniu jej do Load/Save

#ifndef ITEMS_H
#define ITEMS_H

#pragma warning (disable: 4786)
#pragma warning (disable: 4788)
// disable warning (compare bool with int)
#pragma warning (disable: 4805)
#pragma warning (disable: 4503)

#include <list>
using namespace std;

#include "tiles.h"
#include "types.h"
#include "map.h"
#include "loadsave.h"

#define DAMAGE_NONE 0
#define DAMAGE_DEATH -1

class MONSTER;
class INTELLIGENT;
class RANGED_WEAPON;
class HAND_WEAPON;
class GRENADE;
class AMMO;
class TRASH;
class PILL;
class BASE_ARMOR;
class ARMOR;
class CORPSE;
class COUNTABLE;
class ROBOT_SHELL;
class ROBOT_LEG;
class PROCESSOR;
class REPAIR_KIT;
class PROGRAMMATOR;
class BATTERY;

typedef list<void *> ptr_list;

void bad_virtual_call(string method);

class ITEM : public TILE {
private:
	bool is_dead; // when death() for this item was called
	
public:
   int category; // category of item - for probability of setting on the level
   
   MONSTER *owner; // who go nosi lub NULL
   ITEM *in_robot_shell;
   int purpose;   

   bool on_ground; // if lays on ground
   
   int DMG; // Hand 2 hand combat
   int HIT; // Hand 2 hand combat
   int DEF; // Hand 2 hand combat

   int energy_activated; // -1 cannot, 0 - when on, 1 - when off

   PROPERTIES properties; // Armor Piercing / Explosive etc.

   int hit_points;
   int max_hit_points;
   int required_strength;
   int weight;
   int price;
   int skill_to_use; // which skill to concern when using
   string activities;   

   string sound_used;

   int inventory_letter; // used only by player

   ITEM();
   ~ITEM();
   
   virtual bool ChangePosition(int x, int y);
   
// ----------------------------------------
   /// dopasowane do konkretnych rodzajow
   virtual int used_by_on(MONSTER *who, MONSTER *on_who, int skill);
   virtual int uses_energy(bool by_activation=true); // return: 0 - nothing changed, 1 - turned self off
   virtual int turn_energy_on()  { bad_virtual_call("Item: turn_energy_on"); return 0; };
   virtual int turn_energy_off()  { bad_virtual_call("Item: turn_energy_off"); return 0; };
   
   // for granade
   virtual int activate()  { bad_virtual_call("Item: activate"); return 0; };
   // for pills - should be changed to used_by_on?
   virtual int use_on(MONSTER *who) { bad_virtual_call("Item: use_on"); return 0; };
   
   //  for ammo ('l')
   virtual int  load_ammo(AMMO *ammo) { bad_virtual_call("Item: load_ammo"); return 0;};
   virtual bool unload_ammo(INTELLIGENT *czyj_backpack) { bad_virtual_call("Item: unload_ammo"); return 0;};
   
   /// dla countable ('i')
   virtual void add_and_destroy_object (ITEM *object) { bad_virtual_call("Item: add_and_destroy_object"); };
// ----------------------------------------
   // ogolne
   virtual bool compare_activities_with (ITEM *object); 
   
   virtual void print_item_with_letter(int x,int y,string additional_text);
   virtual int evaluate_item() { return 0;}; // check the item value, base on "price" mostly
   virtual int evaluate_item_for_h2h_combat();
   virtual int evaluate_item_for_ranged_combat() { return 0; };
   virtual void print_attributes_vertical(int x,int y, int *changex, int *changey);
   virtual int calculate_weight();
   virtual int damage_it(int value);
   virtual int fix_damage(int value);   
   virtual bool fix_with_item(ITEM *parts, MONSTER *who_fixes);

   virtual void every_turn(); // cos, co sie wykonuje co ture z tym objectem
   virtual void death();  // Call this method - at the end of turn is removes object from memory
   virtual string show_name();
   virtual string article_a();
   virtual string article_the();

   bool is_monster_around();

   // fulfill activities
   bool property_get();
   bool property_drop();
   bool property_buildrobot();
   bool property_throw();
   bool property_wield();
   bool property_use(); 
   bool property_controller(); 
   bool property_load_ammo();
   bool property_unload_ammo();
   bool property_put_on();
   bool property_to_load();
   bool property_join(); // f.e. with pills or ammo
   bool property_activate(); // f.e. grenades
   bool property_ready();
   bool property_mend();
   bool property_program();
   
   // informacje o przedmiocie
   HAND_WEAPON * IsHandWeapon();
   RANGED_WEAPON * IsShootingWeapon();
   GRENADE * IsGrenade();
   PILL * IsPill();
   AMMO * IsAmmo();
   BASE_ARMOR * IsArmor();
   CORPSE * IsCorpse();
   TRASH * IsGarbage();
   REPAIR_KIT * IsRepairSet();
   COUNTABLE * IsCountable();
   ROBOT_SHELL * IsRobotShell();
   ROBOT_LEG * IsRobotLeg();
   PROCESSOR *IsRobotCPU();
   PROGRAMMATOR *IsProgrammator();
   BATTERY *IsBattery();
   
   bool IsLoaded(); // is loaded with ammo
   bool IsDead();

   // duplicate on creation
   virtual ITEM * duplicate()  { bad_virtual_call("Item: duplicate"); return 0; };

   /// INHERITED FROM ToSave
	virtual unsigned long SaveObject();
	virtual bool LoadObject();
};

class TRASH : public ITEM {
public:
	TRASH();
	virtual void every_turn() {}; // cos, co sie wykonuje co ture z tym objectem - tu nic
	virtual void print_attributes_vertical(int x,int y, int *changex, int *changey);
	virtual ITEM * duplicate();
	
	/// INHERITED FROM ToSave
	virtual unsigned long SaveObject();
	virtual bool LoadObject();
};

class REPAIR_KIT : public ITEM {
public:
	REPAIR_KIT();
	int regeneration_time;
	int regeneration_progress;

	bool can_be_used();
	bool fix(ITEM *item, MONSTER *who_fixes);
	virtual void every_turn(); // cos, co sie wykonuje co ture z tym objectem - tu nic
	virtual void print_attributes_vertical(int x,int y, int *changex, int *changey);
	virtual ITEM * duplicate();
	
	/// INHERITED FROM ToSave
	virtual unsigned long SaveObject();
	virtual bool LoadObject();
};

class PROGRAMMATOR : public ITEM {
public:
	PROGRAMMATOR();
	
	virtual void print_attributes_vertical(int x,int y, int *changex, int *changey);
	virtual ITEM * duplicate();
	
	/// INHERITED FROM ToSave
	virtual unsigned long SaveObject();
	virtual bool LoadObject();
};

class BATTERY : public ITEM {
public:
	BATTERY();
	int max_capacity;
	int capacity; 

	int regeneration_speed; // regeneruje maks. do swojej pojemnosci, ale u posiadacza

	virtual void every_turn(); // cos, co sie wykonuje co ture z tym objectem - tu nic
	
	virtual void print_attributes_vertical(int x,int y, int *changex, int *changey);
	virtual int evaluate_item() { return max_capacity + (regeneration_speed!=0)*100;};	
	virtual ITEM * duplicate();
	
	/// INHERITED FROM ToSave
	virtual unsigned long SaveObject();
	virtual bool LoadObject();
};


class COUNTABLE : public ITEM { // it the item can be in packages (ammo, pills)
public:
	COUNTABLE();
	int quantity;
	virtual void add_and_destroy_object (ITEM *object);
	virtual int damage_it(int value);
	virtual int fix_damage(int value);
	virtual string show_name();
	virtual int calculate_weight();
	
	/// INHERITED FROM ToSave
	virtual unsigned long SaveObject();
	virtual bool LoadObject();
};

class PILL : public COUNTABLE {
public:
	int PWR;
	
	PILL();
	
    virtual void every_turn();
	virtual int evaluate_item();
	virtual void print_attributes_vertical(int x,int y, int *changex, int *changey);
	virtual bool compare_activities_with (ITEM *object);
	virtual int use_on(MONSTER *who);
    virtual ITEM * duplicate();   

	/// INHERITED FROM ToSave
	virtual unsigned long SaveObject();
	virtual bool LoadObject();
};


class CORPSE : public ITEM {
public:
   string of_what;
   int rotting_state;
   CORPSE();
   virtual void every_turn(); // cos, co sie wykonuje co ture z tym objectem
   virtual int evaluate_item();
   virtual void print_attributes_vertical(int x,int y, int *changex, int *changey);
   virtual ITEM * duplicate();

   /// INHERITED FROM ToSave
	virtual unsigned long SaveObject();
	virtual bool LoadObject();
};

class AMMO : public COUNTABLE {
public:
   AMMO();
   ~AMMO();
   
   int PWR; // power of missile
   int ACC; // accuracy 
   AMMO_TYPE ammo_type; // f.e. 8mm
   
   RANGED_WEAPON *in_weapon;
   virtual int evaluate_item();
   virtual void print_attributes_vertical(int x,int y, int *changex, int *changey);
   virtual string show_name();

   virtual bool compare_activities_with (ITEM *object);
   virtual ITEM * duplicate();   

   /// INHERITED FROM ToSave
	virtual unsigned long SaveObject();
	virtual bool LoadObject();
};


class RANGED_WEAPON : public ITEM {
public:
   RANGED_WEAPON();

   AMMO_TYPE ammo_type; // must be the same in ammo and weapon
   int PWR; // additional power for ammo
   int ACC; // accuracy for weapon

   int energy_PWR; // additional power with energy on
   int energy_ACC; // additional accuracy with energy on
   
   int magazine_capacity;
   int category_of_magazine; // category from which is randomized ammo on item creation. Not saved.
   
   FIRE_TYPE fire_type;
   FIRE_TYPE available_fire_types;

   AMMO ammo;

   // Sounds are not saved, are taken from definitions
   string sound_file_single; 
   string sound_file_double; 
   string sound_file_triple;
   string sound_file_burst;
   string sound_reload;
   string sound_gun_empty;

   void incrase_fire_mode();
   void decrase_fire_mode();

   virtual int turn_energy_on(); 
   virtual int turn_energy_off();
   virtual int activate();
   
   
   virtual int damage_it(int value);
   virtual int calculate_weight();
   virtual int load_ammo(AMMO *ammo);
   virtual bool unload_ammo(INTELLIGENT *backpack_owner);
   virtual int evaluate_item();
   virtual int evaluate_item_for_ranged_combat();      
   virtual void print_attributes_vertical(int x,int y, int *changex, int *changey);
   virtual void print_item_with_letter(int x,int y,string additional_text);
   bool fire_missile(int x1, int y1, int x2, int y2,MONSTER * enemy, int distance);
   virtual ITEM * duplicate();

   /// INHERITED FROM ToSave
	virtual unsigned long SaveObject();
	virtual bool LoadObject();
};


class BASE_ARMOR : public ITEM {
public:
   int ARM; // how much it blocks
   // modifiers
   int MOD_STR; // 
   int MOD_DEX; // 
   int MOD_SPD; // 

   // additional modifiers for energy on
   int energy_ARM;
   int energy_MOD_STR;
   int energy_MOD_DEX;
   int energy_MOD_SPD;
   PROPERTIES energy_properties;
   PROPERTIES energy_real_properties;
   
   virtual int turn_energy_on(); 
   virtual int turn_energy_off();
   virtual int activate();
   

   BASE_ARMOR();
   
   int evaluate_item();
   virtual void death();  // Call this method - at the end of turn is removes object from memory   
   virtual void print_attributes_vertical(int x,int y, int *changex, int *changey) {};
   virtual string show_name();
   virtual ITEM * duplicate() ;

   virtual bool fix_with_item(ITEM *parts, MONSTER *who_fixes);

   /// INHERITED FROM ToSave
	virtual unsigned long SaveObject();
	virtual bool LoadObject();
};

class NO_ARMOR : public BASE_ARMOR { // all OBRAZENIA ZADAWANE SA TEJ ZBROI czyli skorze
public:
   NO_ARMOR();

   virtual int damage_it(int value);
   virtual ITEM * duplicate();   

   /// INHERITED FROM ToSave
	virtual unsigned long SaveObject();
	virtual bool LoadObject();
};


class ARMOR : public BASE_ARMOR {
public:
   ARMOR();
   
   virtual void every_turn(); // cos, co sie wykonuje co ture z tym objectem   
   virtual void print_attributes_vertical(int x,int y, int *changex, int *changey);
   virtual ITEM * duplicate() ;

   /// INHERITED FROM ToSave
	virtual unsigned long SaveObject();
	virtual bool LoadObject();
};

class HAND_WEAPON : public ITEM {
public:
   int energy_DMG;
   int energy_HIT;
   int energy_DEF;
   PROPERTIES energy_properties;
   PROPERTIES energy_real_properties;
   HAND_WEAPON();

   virtual int turn_energy_on(); 
   virtual int turn_energy_off();
   virtual int activate();
   
   virtual string show_name();
   
   virtual void death();  // Call this method - at the end of turn is removes object from memory
   //   void used_by_on(MONSTER *who, MONSTER *on_who);
   virtual int evaluate_item();
   virtual void print_attributes_vertical(int x,int y, int *changex, int *changey);
   virtual ITEM * duplicate();

   /// INHERITED FROM ToSave
	virtual unsigned long SaveObject();
	virtual bool LoadObject();
};


typedef HAND_WEAPON WEAPON_UNARMED;

class GRENADE : public ITEM {
private:
   int counter; // odlicza do 0, gdy 0 to wybuch. -1 nieactive
   bool works_now;
   int calculate_explosion(int *data);   
public:
   bool active;

   int PWR;
   int RNG;
   int DLY; // value poczatkowa dla countera

   string sound_explosion;

   GRENADE();
   virtual string show_name();

   virtual int evaluate_item();   
   virtual void print_attributes_vertical(int x,int y, int *changex, int *changey);
   virtual void every_turn(); // cos, co sie wykonuje co ture z tym objectem
   virtual int activate();
   virtual int damage_it(int value);
   
   int explode();
   int fire_explosion();
   int stun_explosion();
   int emp_explosion();

   int produce_gas();
   int create_shield();
   int remove_shield();

   virtual ITEM * duplicate();   

   /// INHERITED FROM ToSave
	virtual unsigned long SaveObject();
	virtual bool LoadObject();
};


class GAS : public TILE {
public:
   GAS();
   bool is_dead; // gdy zostalo wywalane death() dla tego gazu
   int density; //   density 0 - 100 invisible 101-200 visible 201+ blocks fov
   PROPERTIES properties;

   virtual bool ChangePosition(int x, int y);
   
   virtual void act_on_monster();
   virtual void display();
   virtual void death(); // usuwa ten gaz z map
   virtual void every_turn(); // gaz sie rozprzestrzenia, dziala na monsters itd.

   /// INHERITED FROM ToSave
	virtual unsigned long SaveObject();
	virtual bool LoadObject();
};

class FIRE : public GAS {
public:
   FIRE();
   virtual void act_on_monster();
   virtual void act_on_items();
   virtual void display();
   virtual void death(); // usuwa ten ogien z map
   virtual void every_turn(); // ogien sie rozprzestrzenia, dziala na monsters itd.

   /// INHERITED FROM ToSave
	virtual unsigned long SaveObject();
	virtual bool LoadObject();
};


class CONTROLLER : public ITEM {
public:
	int PWR;

	CONTROLLER();

    void display() {};
    virtual void every_turn();
    virtual ITEM * duplicate();   
   /// INHERITED FROM ToSave
	virtual unsigned long SaveObject();
	virtual bool LoadObject();
};

class ROBOT_LEG : public ITEM {
public:
	int move_power;
	ROBOT_LEG();

//	virtual void death();  // Call this method - at the end of turn is removes object from memory   
	
	virtual void every_turn() {}; 
	virtual int evaluate_item() { return 0; };
	virtual void print_attributes_vertical(int x,int y, int *changex, int *changey);
	virtual ITEM * duplicate();
	
	/// INHERITED FROM ToSave
	virtual unsigned long SaveObject();
	virtual bool LoadObject();
};

class PROCESSOR : public ITEM {
public:
	string program;
	int frequency;
	int quality;
	int group_affiliation;
	ROBOT_SHELL *where_placed;

	PROCESSOR();
//	virtual void death();  // Call this method - at the end of turn is removes object from memory   
	
	virtual void every_turn() {};
	virtual int evaluate_item() { return 0; };
	virtual void print_attributes_vertical(int x,int y, int *changex, int *changey);
	virtual ITEM * duplicate();
	virtual string show_name();
	
	/// INHERITED FROM ToSave
	virtual unsigned long SaveObject();
	virtual bool LoadObject();
};


class ROBOT_SHELL : public ITEM {
public:
	int max_number_move_slots;
	int max_number_action_slots;

	int ARM; // how much it blocks	
	
	ptr_list move_slots;
	ptr_list action_slots;

	PROCESSOR *cpu;

	string last_robot_name;

	ROBOT_SHELL();
	virtual void death();  // Call this method - at the end of turn is removes object from memory   
	
	virtual int evaluate_item() { return 0; };
	virtual void print_attributes_vertical(int x,int y, int *changex, int *changey);
	virtual int calculate_weight();
	virtual ITEM * duplicate();

	virtual bool install_CPU(ITEM *item);
	virtual bool install_in_action_slot(ITEM *item);
	virtual bool install_in_move_slot(ITEM *item);

	virtual bool separate_item(ITEM *item);
	virtual bool uninstall_CPU();
	virtual bool uninstall_from_action_slot(ITEM *item);
	virtual bool uninstall_from_move_slot(ITEM *item);
	
	/// INHERITED FROM ToSave
	virtual unsigned long SaveObject();
	virtual bool LoadObject();
};



#endif


