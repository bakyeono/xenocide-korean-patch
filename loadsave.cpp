#include "mem_check.h"

// #define LOAD_DEBUG

#include "loadsave.h"
#include "items.h"
#include "monster.h"
#include "attrib.h"
#include "level.h"
#include "game.h"
#include "options.h"
#include "parser.h"
#include "keyboard.h"
#include <assert.h>

extern GAME game;
extern OPTIONS options;
extern KEYBOARD keyboard;
extern DEFINITIONS definitions;
extern MYSCREEN screen;

#pragma warning (disable: 4786)
#pragma warning (disable: 4788)
#pragma warning (disable: 4805)

#include <string>
#include <list>
#include <map>
using namespace std;

#include <stdlib.h>
#include "monster.h"
#include "items.h"
#include "global.h"
#include "game.h"
#include <math.h>

extern LEVEL level;

// static

FILE *TOSAVE::FilePointer; 
unsigned long TOSAVE::LastUniqueNumber = FIRST_UNIQUE_NUMBER;
to_fix TOSAVE::ListToFix;
addresses TOSAVE::AddressesOfObjects;
list_of_saved TOSAVE::saved;

TOSAVE *TOSAVE::CreateObject()
{
#ifdef LOAD_DEBUG
	printf("* TOSAVE::CreateObject\n");
#endif

	string s;
	unsigned long UN;
	TOSAVE *new_one;

	UN = 0;

	unsigned long FilePos;

	FilePos = ftell(FilePointer);

	LoadString(s);
	LoadLong(UN);

	// place w pliku jeszcze raz na poczatek, aby load CLASS_NAME w LoadObject bylo mozliwe.
	fseek(FilePointer,FilePos,SEEK_SET);   
	
	new_one = NULL;

   if (s == "CORPSE")
   {
                new_one=new CORPSE;
				level.all_items.push_back(new_one);
   }
   else if (s == "AMMO")
   {
                new_one=new AMMO;
				level.all_items.push_back(new_one);
   }
   else if (s == "TRASH")
   {
				new_one=new TRASH;
				level.all_items.push_back(new_one);
   }
   else if (s == "REPAIR_KIT")
   {
	   new_one=new REPAIR_KIT;
	   level.all_items.push_back(new_one);
   }
   else if (s == "RANGED_WEAPON")
   {
       new_one=new RANGED_WEAPON;
	   level.all_items.push_back(new_one);
   }
   else if (s == "HAND_WEAPON")
   {
       new_one = new HAND_WEAPON;
	   level.all_items.push_back(new_one);
   }
   else if (s == "GRENADE")
   {
       new_one=new GRENADE;
	   level.all_items.push_back(new_one);
   }
   else if (s == "CONTROLLER")
   {
       new_one=new CONTROLLER;
	   level.all_items.push_back(new_one);
   }
   else if (s == "PILL")
   {
	   new_one=new PILL;
	   level.all_items.push_back(new_one);
   }
   else if (s == "PROCESSOR")
   {
	   new_one=new PROCESSOR;
	   level.all_items.push_back(new_one);
   }
   else if (s == "ROBOT_LEG")
   {
	   new_one=new ROBOT_LEG;
	   level.all_items.push_back(new_one);
   }
   else if (s == "ROBOT_SHELL")
   {
	   new_one=new ROBOT_SHELL;
	   level.all_items.push_back(new_one);
   }
   else if (s == "NO_ARMOR")
   {
       new_one=new NO_ARMOR;
	   level.all_items.push_back(new_one);
   }
   else if (s == "ARMOR")
   {
       new_one=new ARMOR;
	   level.all_items.push_back(new_one);
   }
   else if (s == "PROGRAMMATOR")
   {
	   new_one=new PROGRAMMATOR;
	   level.all_items.push_back(new_one);
   }
   else if (s == "BATTERY")
   {
	   new_one=new BATTERY;
	   level.all_items.push_back(new_one);
   }   
   else if (s == "ANIMAL")
   {
       new_one=new ANIMAL;
   }
   else if (s == "WORM")
   {
	   new_one=new WORM;
   }
   else if (s == "MADDOCTOR")
   {
	   new_one=new MADDOCTOR;
   }
   else if (s == "SEARCHLIGHT")
   {
	   new_one=new SEARCHLIGHT;
   }
   else if (s == "INTELLIGENT")
   {
       new_one=new INTELLIGENT;
   }
   else if (s == "ROBOT")
   {
	   new_one=new ROBOT;
   }
   else if (s == "GAS")
   {
       new_one=new GAS;
   }
   else if (s == "FIRE")
   {
       new_one=new FIRE;
   }
   else if (s == "STAIRS")
   {
       new_one=new STAIRS;
   }
   else if (s == "NULL_OBJECT")
   {
	   new_one=NULL;
	   LoadString(s); // there won't be load object, so move file ptr by string
	   return new_one;
   }
   else
   {
	   fprintf(stderr,"ERROR READ: Not defined object for reading: %s\n",s.c_str());
	   keyboard.wait_for_key(); 
	   exit(199);	   
   }

   if (new_one==NULL)
   {
	   fprintf(stderr,"ERROR READ: Creating object failed %s\n",s.c_str());
	   keyboard.wait_for_key(); 
	   exit(200);
   }

   return new_one;
}

bool TOSAVE::LoadMessageBuffer(string file_name)
{
#ifdef LOAD_DEBUG
	printf("TOSAVE::LoadMessageBuffer\n");
#endif

	FILE *fp, *old_fp;
	
	old_fp = FilePointer;
	
	fp = fopen(string(string("save/") + game.name_of_player + ".msg").c_str(),"rb");
	
	if (fp==NULL)
	{
		fprintf(stderr,"ERROR READ: Can not open file save/%s.msg",level.player->name.c_str());		
		keyboard.wait_for_key(); 
		exit(83);
	}
	InitFilePointer(fp);
	
	int linii;
	string line;

	screen.console.message_buffer.clear();
	
	LoadInt(linii);
	for (int a=0;a<linii;a++)
	{
		LoadString(line);
		screen.console.message_buffer.push_back(line);
	}
	
	fclose(fp);
	InitFilePointer(old_fp);	
	return true;
}	

bool TOSAVE::SaveMessageBuffer(string file_name)
{
#ifdef LOAD_DEBUG
	printf("TOSAVE::LoadMessageBuffer\n");
#endif
	list<string>::iterator m,_m;
	m=screen.console.message_buffer.begin();
	_m=screen.console.message_buffer.end();

	FILE *fp, *old_fp;
	
	old_fp = FilePointer;

	fp = fopen(string(string("save/") + game.name_of_player + ".msg").c_str(),"wb+");
	if (fp==NULL)
	{
		fprintf(stderr,"ERROR SAVE: Error creating file save/%s.msg",level.player->name.c_str());		
		keyboard.wait_for_key(); 
		exit(83);
	}
	InitFilePointer(fp);
	
	SaveInt(screen.console.message_buffer.size());
	for (;m!=_m;m++)
	{
		string line = *m;
		SaveString(line);
	}

	fclose(fp);
	InitFilePointer(old_fp);	
	return true;
}	


bool TOSAVE::LoadOptions(string file_name)
{
#ifdef LOAD_DEBUG
	printf("TOSAVE::LoadOptions\n");
#endif
	
	string line;
	CFILE file;
	if (!(file.open(file_name)))
		return false;

	line=file.get_line();
	while (line!="")
	{
		bool wybrane = false;
		int option=-1;
		
		if (get_boolean(line,file.line_number)==true)
			wybrane = true;
		if (line.find("Constant letters in inventory:")!=-1)
			option = OPTION_LETTERS;
		else if (line.find("Show experience needed for new level:")!=-1)
			option = OPTION_EXPERIENCE;
		else if (line.find("Auto aim nearest enemy:")!=-1)
			option = OPTION_AUTO_AIM;
		else if (line.find("Auto aim if no enemy selected:")!=-1)
			option = OPTION_AIM_IF_NO_ENEMY;
		else if (line.find("Don't let friendly monsters to swap place with you:")!=-1)
			option = OPTION_DONT_LET_SWAP;
		else if (line.find("Show cursor on player:")!=-1)
			option = OPTION_CURSOR;
		else if (line.find("Always get the whole package of items:")!=-1)
			option = OPTION_GETALL;
		else if (line.find("")!=-1)
			option = OPTION_DONT_LET_SWAP;
		
		if (option!=-1)
			options.number[option] = wybrane;	

		line=file.get_line();
	}

	file.close();
	return true;
}	

bool TOSAVE::SaveOptions(string file_name)
{
#ifdef LOAD_DEBUG
	printf("TOSAVE::SaveOptions\n");
#endif

	CFILE file;
	if (!file.create(file_name))
		file.error_create();
	
	string line;
	string text;
	file.save_line("# OPTIONS\n");
	
	for (int zz=0;zz<NUMBER_OF_OPTIONS;zz++)
	{
		switch(zz)
		{
			
           case OPTION_LETTERS:
			   text="Constant letters in inventory: ";
			   break;
           case OPTION_EXPERIENCE:
			   text="Show experience needed for new level: ";
			   break;
           case OPTION_AUTO_AIM:
			   text="Auto aim nearest enemy: ";
			   break;
           case OPTION_AIM_IF_NO_ENEMY:
			   text="Auto aim if no enemy selected: ";
			   break;
           case OPTION_DONT_LET_SWAP:
			   text="Don't let friendly monsters to swap place with you: ";
			   break;								   
           case OPTION_CURSOR:
			   text="Show cursor on player: ";
			   break;								   
           case OPTION_GETALL:
			   text="Always get the whole package of items: ";
			   break;								   
		   default:
			   text="Unknown option!!! : ";
		}

	    if (options.number[zz]==true)
		   text+="TRUE";
	    else
		   text+="FALSE";

		file.save_line(text);		
	}		
	file.close();
	return true;
}	


HERO *TOSAVE::LoadPlayer(string name)
{
#ifdef LOAD_DEBUG
	printf("TOSAVE::LoadPlayer\n");
#endif

	FILE *fp, *old_fp;

	HERO *new_one;
	string s;
	unsigned long UN;

	old_fp = FilePointer;

	fp = fopen(string(string("save/") + name + ".sav").c_str(),"rb+");
	if (fp==NULL)
	{
		fprintf(stderr,"ERROR READ: Error opening file %s\n",string(string("save/") + name + ".sav").c_str());		
		keyboard.wait_for_key(); 
		exit(84);
	}
	TOSAVE::InitFilePointer(fp);

	LoadString(s);
	LoadLong(UN);

	if (s != "HERO")
	{
	   fprintf(stderr,"ERROR READ: HERO string expected, not %s\n",s.c_str());
	   keyboard.wait_for_key(); 
	   exit(215);
	}


	new_one = new HERO;
	if (new_one == NULL)
	{
	   fprintf(stderr,"ERROR READ: Error creating HERO\n");
	   keyboard.wait_for_key(); 
	   exit(216);
	}


	fseek(fp,0,SEEK_SET);
	new_one->LoadObject();

	fclose(fp);
	TOSAVE::InitFilePointer(old_fp);
	TOSAVE::LoadOptions(string("save/") + name + ".opt");
	TOSAVE::LoadMessageBuffer(string("save/") + name + ".msg");			

	return new_one;
}

TOSAVE::TOSAVE()
{
	UniqueNumber = ++LastUniqueNumber;
        if (UniqueNumber==0) // UniqueNumber overflow - it should NEVER happen
	{
           fprintf(stderr,"Wow, the universe has been destroyed! Contact the author!\n\n");
           keyboard.wait_for_key(); 
		   exit(3);
	}
	ClassName = "UNDEFINED";
	IsAlreadySaved = false;
}

void TOSAVE::SaveList(ptr_list &value)
{
   int size;
   ptr_list::iterator m,_m;
   TOSAVE *temp;

	SaveString("LIST");
	size = value.size(); // size listy
	SaveInt(size); 

   for(m=value.begin(),_m=value.end(); m!=_m; m++)
   {
       temp=(TOSAVE *)*m;
 	   temp->SaveObject();
   }
}

void TOSAVE::LoadList(ptr_list &value)
{
#ifdef LOAD_DEBUG
	printf("* TOSAVE::LoadList\n");
#endif

	string t;
	int size;
	LoadString(t);

	if (t != "LIST")
	{
	   fprintf(stderr,"ERROR READ: string LIST expected, not %s\n",t.c_str());
	   keyboard.wait_for_key(); 
	   exit(215);
	}

	LoadInt(size);
	TOSAVE *new_one;

	for (int index = 0; index<size;index++)
	{
		new_one = CreateObject();
#ifdef LOAD_DEBUG
	printf("-> Element listy: %d,%d\n",index,size);
#endif
		new_one->LoadObject();
		value.push_back(new_one);
	}
}

void TOSAVE::InitFilePointer(FILE *fp) // static
{
	FilePointer = fp;
}

void TOSAVE::SaveUniqueNumber(unsigned long value)
{
	fwrite(&value,sizeof(unsigned long),1,FilePointer);
}

void TOSAVE::LoadUniqueNumber(TOSAVE **ptr)
{
	para p;
	unsigned long value;
	fread(&value,sizeof(unsigned long),1,FilePointer);
	if (value>LastUniqueNumber) // this shouldn't happen
		LastUniqueNumber = value+1;

	p.first = ptr;
	p.second = value;
	ListToFix.push_back(p);

#ifdef LOAD_DEBUG
	printf("LoadUN: %d\n",(int) value);
	if (value==786445)
	{
		int z=0;
	}
#endif
}


void TOSAVE::SaveBool(bool value)
{
	fwrite(&value,sizeof(bool),1,FilePointer);
}

void TOSAVE::LoadBool(bool &value)
{
	fread(&value,sizeof(bool),1,FilePointer);
#ifdef LOAD_DEBUG
	printf("LoadBool: %d\n",(int) value);
#endif
}


void TOSAVE::SaveChar(char value)
{
	fwrite(&value,sizeof(char),1,FilePointer);
}

void TOSAVE::LoadChar(char &value)
{
	fread(&value,sizeof(char),1,FilePointer);
#ifdef LOAD_DEBUG
	if (value != 7)
		printf("LoadChar: %c (%d, 0x%x)\n",value,value,value);
	else
		printf("LoadChar: <7> (%d, 0x%x)\n",value,value);
#endif
}


void TOSAVE::SaveInt(int value)
{
   fwrite(&value,sizeof(int),1,FilePointer);
}

void TOSAVE::LoadInt(int &value)
{
	fread(&value,sizeof(int),1,FilePointer);
#ifdef LOAD_DEBUG
	printf("LoadInt: %d\n",value);
#endif
}

void TOSAVE::SaveLong(unsigned long value)
{
	fwrite(&value,sizeof(long),1,FilePointer);
}

void TOSAVE::LoadLong(unsigned long &value)
{
	fread(&value,sizeof(long),1,FilePointer);
#ifdef LOAD_DEBUG
	printf("LoadLong: %d\n",(int) value);
#endif
}


void TOSAVE::SaveString(string value)
{
	int string_size;
	string_size=value.size();
	fwrite(&string_size,sizeof(int),1,FilePointer);
	fwrite(value.c_str(),string_size,1,FilePointer);	
}

void TOSAVE::LoadString(string &value)
{
	char t[1000];
	unsigned int string_size;
	fread(&string_size,sizeof(int),1,FilePointer);
   if (string_size>=1000)
   {
      fprintf(stderr,"ERROR READ: size stringa >1000\n");
      keyboard.wait_for_key(); 
	  exit(222);
   }   
	fread(t,sizeof(char),string_size,FilePointer);
	t[string_size]='\0';
	value = string(t);
#ifdef LOAD_DEBUG
	string to_print=value;
	size_t pos = to_print.find(7);
	if (pos!=string::npos)
		to_print[pos]='+';
	printf("LoadString: %s (size: %d)\n",to_print.c_str(),string_size);
#endif
}

unsigned long TOSAVE::SaveObject()
{
	if (IsAlreadySaved == true)
	{
	   fprintf(stderr,"ERROR SAVE:  TOSAVE already saved\n");
	   keyboard.wait_for_key(); 
	   exit(230);
	}

	SaveString(ClassName);
	SaveLong(UniqueNumber);

	IsAlreadySaved = true;
	saved.push_back(this); // dodajemy object do zapisanych

	return UniqueNumber;
}

void TOSAVE::SaveNULLObject()
{
	SaveString("NULL_OBJECT");
}

bool TOSAVE::LoadObject()
{

	LoadString(ClassName);
	LoadLong(UniqueNumber);

#ifdef LOAD_DEBUG
	printf("* TOSAVE::LoadObject - %s\n",ClassName.c_str());
#endif

	if (AddressesOfObjects.find(UniqueNumber)!=AddressesOfObjects.end())
	{
		TOSAVE *adr = AddressesOfObjects[UniqueNumber];
		   fprintf(stderr,"ERROR: Map has assigned address for %d\n",UniqueNumber);
		   keyboard.wait_for_key(); 
		   exit(234);
	}
	AddressesOfObjects[UniqueNumber] = this;

	return true;
}

void TOSAVE::UpdatePointers()
{
	to_fix::iterator m,_m;
	addresses::iterator l,_l;

	para p;
	pair <unsigned long, TOSAVE *> para_UN;

	TOSAVE *adres;
	TOSAVE **mod;

	unsigned long UN;

	for (m=ListToFix.begin(),_m = ListToFix.end();m!=_m;m++)
	{
		p = *m;
		UN = p.second;
		if (UN!=0 && AddressesOfObjects.find(UN)==AddressesOfObjects.end())
		{
		   fprintf(stderr,"ERROR UPDATE POINT: Not found for %d (%x)\n",UN,UN);
		   keyboard.wait_for_key(); 
		   exit(231);
		}

		adres = AddressesOfObjects [UN];
		mod = p.first;
		*mod = adres;
	}

	return;
}

unsigned long TILE::SaveObject()
{
	if (IsAlreadySaved == true)
	{
	   fprintf(stderr,"ERROR SAVE:  TOSAVE already saved\n");
	   keyboard.wait_for_key(); 
	   exit(230);
	}

	TOSAVE::SaveObject();

	SaveInt(positionX);
	SaveInt(positionY);
	SaveString(name);
	SaveChar(color);
	SaveChar(tile);

	return UniqueNumber;
}

bool TILE::LoadObject()
{
#ifdef LOAD_DEBUG
	printf("* TILE::LoadObject\n");
#endif

	TOSAVE::LoadObject();

	LoadInt(positionX);
	LoadInt(positionY);
	LoadString(name);
	LoadChar(color);
	LoadChar(tile);
	return true;
}

unsigned long STAIRS::SaveObject()
{
	if (IsAlreadySaved == true)
	{
	   fprintf(stderr,"ERROR SAVE:  TOSAVE already saved\n");
	   keyboard.wait_for_key(); 
	   exit(230);
	}

	TOSAVE::SaveObject();

	SaveBool(lead_up);
	SaveInt(number);
	SaveInt(x);
	SaveInt(y);
	SaveString(to_where);
	SaveString(name);

	return UniqueNumber;
}

bool STAIRS::LoadObject()
{
#ifdef LOAD_DEBUG
	printf("* STAIRS::LoadObject\n");
#endif

	TOSAVE::LoadObject();

	LoadBool(lead_up);
	LoadInt(number);
	LoadInt(x);
	LoadInt(y);
	LoadString(to_where);
	LoadString(name);
	return true;
}



unsigned long ITEM::SaveObject()
{
	if (IsAlreadySaved == true)
	{
	   fprintf(stderr,"ERROR SAVE:  TOSAVE already saved\n");
	   keyboard.wait_for_key(); 
	   exit(230);
	}

	TILE::SaveObject();

	SaveInt(category);
	
	if (owner!=NULL)
		SaveUniqueNumber(owner->UniqueNumber);
	else
		SaveUniqueNumber(NULL_UNIQUE_NUMBER);

	if (in_robot_shell!=NULL)
		SaveUniqueNumber(in_robot_shell->UniqueNumber);
	else
		SaveUniqueNumber(NULL_UNIQUE_NUMBER);	

	SaveInt(DMG);
	SaveInt(HIT);
	SaveInt(DEF);
	SaveBool(on_ground);
	SaveInt(energy_activated);	
	SaveLong(properties);
	SaveInt(hit_points);
	SaveInt(max_hit_points);
	SaveInt(required_strength);
	SaveInt(weight);
	SaveInt(price);
	SaveInt(skill_to_use);
	SaveString(activities);
	SaveInt(inventory_letter);
	SaveBool(is_dead);
	SaveInt(purpose);	

	return UniqueNumber;
}

bool ITEM::LoadObject()
{
#ifdef LOAD_DEBUG
	printf("* ITEM::LoadObject\n");
#endif
	TILE::LoadObject();

	LoadInt(category);
	
	LoadUniqueNumber( (TOSAVE **) &owner);
	LoadUniqueNumber( (TOSAVE **) &in_robot_shell);
	
	LoadInt(DMG);
	LoadInt(HIT);
	LoadInt(DEF);
	LoadBool(on_ground);
	LoadInt(energy_activated);
	LoadLong(properties);
	LoadInt(hit_points);
	LoadInt(max_hit_points);
	LoadInt(required_strength);
	LoadInt(weight);
	LoadInt(price);
	LoadInt(skill_to_use);
	LoadString(activities);
	LoadInt(inventory_letter);
	LoadBool(is_dead);
	LoadInt(purpose);
	

	return true;
}

unsigned long CORPSE::SaveObject()
{
	if (IsAlreadySaved == true)
	{
	   fprintf(stderr,"ERROR SAVE:  TOSAVE already saved\n");
	   keyboard.wait_for_key(); 
	   exit(230);
	}

	ITEM::SaveObject();

	SaveString(of_what);
	SaveInt(rotting_state);

	return UniqueNumber;
}

bool CORPSE::LoadObject()
{
#ifdef LOAD_DEBUG
	printf("* CORPSE::LoadObject\n");
#endif
	ITEM::LoadObject();

	LoadString(of_what);
	LoadInt(rotting_state);
	return true;
}

unsigned long TRASH::SaveObject()
{
	if (IsAlreadySaved == true)
	{
		fprintf(stderr,"ERROR SAVE:  TOSAVE already saved\n");
		keyboard.wait_for_key(); 
		exit(230);
	}
	
	ITEM::SaveObject();
	
	return UniqueNumber;
}

bool TRASH::LoadObject()
{
#ifdef LOAD_DEBUG
	printf("* TRASH::LoadObject\n");
#endif
	ITEM::LoadObject();
	
	return true;
}


unsigned long REPAIR_KIT::SaveObject()
{
	if (IsAlreadySaved == true)
	{
		fprintf(stderr,"ERROR SAVE:  TOSAVE already saved\n");
		keyboard.wait_for_key(); 
		exit(230);
	}
	
	ITEM::SaveObject();

	SaveInt(regeneration_progress);
	SaveInt(regeneration_time);
	
	return UniqueNumber;
}

bool REPAIR_KIT::LoadObject()
{
#ifdef LOAD_DEBUG
	printf("* REPAIR_KIT::LoadObject\n");
#endif
	ITEM::LoadObject();
	
	LoadInt(regeneration_progress);
	LoadInt(regeneration_time);

	return true;
}

//////////////////////////////////////////////////////////////////////////
unsigned long PROGRAMMATOR::SaveObject()
{
	if (IsAlreadySaved == true)
	{
		fprintf(stderr,"ERROR SAVE:  TOSAVE already saved\n");
		keyboard.wait_for_key(); 
		exit(230);
	}
	
	ITEM::SaveObject();
	return UniqueNumber;
}

bool PROGRAMMATOR::LoadObject()
{
#ifdef LOAD_DEBUG
	printf("* PROGRAMMATOR::LoadObject\n");
#endif
	ITEM::LoadObject();
	return true;
}
//////////////////////////////////////////////////////////////////////////
unsigned long BATTERY::SaveObject()
{
	if (IsAlreadySaved == true)
	{
		fprintf(stderr,"ERROR SAVE:  TOSAVE already saved\n");
		keyboard.wait_for_key(); 
		exit(230);
	}
	
	ITEM::SaveObject();
	SaveInt(max_capacity);
	SaveInt(capacity); 
	SaveInt(regeneration_speed); 
	
	return UniqueNumber;
}

bool BATTERY::LoadObject()
{
#ifdef LOAD_DEBUG
	printf("* BATTERY::LoadObject\n");
#endif
	ITEM::LoadObject();
	LoadInt(max_capacity);
	LoadInt(capacity); 
	LoadInt(regeneration_speed); 
	return true;
}


unsigned long PILL::SaveObject()
{
	if (IsAlreadySaved == true)
	{
		fprintf(stderr,"ERROR SAVE:  TOSAVE already saved\n");
		keyboard.wait_for_key(); 
		exit(230);
	}
	
	COUNTABLE::SaveObject();
	
	SaveInt(PWR);
	
	return UniqueNumber;
}

bool PILL::LoadObject()
{
#ifdef LOAD_DEBUG
	printf("* PILL::LoadObject\n");
#endif
	COUNTABLE::LoadObject();
	
	LoadInt(PWR);
	return true;
}

unsigned long PROCESSOR::SaveObject()
{
	if (IsAlreadySaved == true)
	{
		fprintf(stderr,"ERROR SAVE:  TOSAVE already saved\n");
		keyboard.wait_for_key(); 
		exit(230);
	}
	
	ITEM::SaveObject();

	SaveInt(frequency);	
	SaveInt(quality);	
	SaveInt(group_affiliation);	
	SaveString(program);	

	if (where_placed!=NULL)
		SaveUniqueNumber(where_placed->UniqueNumber);
	else
		SaveUniqueNumber(NULL_UNIQUE_NUMBER);
		
	return UniqueNumber;
}

bool PROCESSOR::LoadObject()
{
#ifdef LOAD_DEBUG
	printf("* PROCESSOR::LoadObject\n");
#endif
	ITEM::LoadObject();

	LoadInt(frequency);	
	LoadInt(quality);
	LoadInt(group_affiliation);	
	LoadString(program);	

	LoadUniqueNumber( (TOSAVE **) &where_placed);	
	
	return true;
}

unsigned long ROBOT_LEG::SaveObject()
{
	if (IsAlreadySaved == true)
	{
		fprintf(stderr,"ERROR SAVE:  TOSAVE already saved\n");
		keyboard.wait_for_key(); 
		exit(230);
	}
	
	ITEM::SaveObject();
	
	SaveInt(move_power);	
	
	return UniqueNumber;
}

bool ROBOT_LEG::LoadObject()
{
#ifdef LOAD_DEBUG
	printf("* ROBOT_LEG::LoadObject\n");
#endif
	ITEM::LoadObject();
	
	LoadInt(move_power);	
	
	return true;
}



unsigned long CONTROLLER::SaveObject()
{
	if (IsAlreadySaved == true)
	{
	   fprintf(stderr,"ERROR SAVE:  TOSAVE already saved\n");
	   keyboard.wait_for_key(); 
	   exit(230);
	}

	ITEM::SaveObject();

	return UniqueNumber;
}

bool CONTROLLER::LoadObject()
{
#ifdef LOAD_DEBUG
	printf("* CONTROLLER::LoadObject\n");
#endif
	ITEM::LoadObject();

	return true;
}

unsigned long COUNTABLE::SaveObject()
{
	if (IsAlreadySaved == true)
	{
		fprintf(stderr,"ERROR SAVE:  TOSAVE already saved\n");
		keyboard.wait_for_key(); 
		exit(230);
	}
	ITEM::SaveObject();
	
	SaveInt(quantity);	
	return UniqueNumber;
}

bool COUNTABLE::LoadObject()
{
#ifdef LOAD_DEBUG
	printf("* COUNTABLE::LoadObject\n");
#endif
	ITEM::LoadObject();
	LoadInt(quantity);
	return true;
}


unsigned long AMMO::SaveObject()
{
	if (IsAlreadySaved == true)
	{
	   fprintf(stderr,"ERROR SAVE:  TOSAVE already saved\n");
	   keyboard.wait_for_key(); 
	   exit(230);
	}

	COUNTABLE::SaveObject();

	SaveInt(PWR);
	SaveInt(ACC);
	SaveInt(ammo_type);

	if (in_weapon!=NULL)
		SaveUniqueNumber(in_weapon->UniqueNumber);
	else
		SaveUniqueNumber(NULL_UNIQUE_NUMBER);

	return UniqueNumber;
}

bool AMMO::LoadObject()
{
#ifdef LOAD_DEBUG
	printf("* AMMO::LoadObject\n");
#endif

	COUNTABLE::LoadObject();
	
	LoadInt(PWR);
	LoadInt(ACC);
	LoadInt(ammo_type);

	LoadUniqueNumber( (TOSAVE **) &in_weapon);
	return true;
}

unsigned long RANGED_WEAPON::SaveObject()
{
	if (IsAlreadySaved == true)
	{
	   fprintf(stderr,"ERROR SAVE:  TOSAVE already saved\n");
	   keyboard.wait_for_key(); 
	   exit(230);
	}

	ITEM::SaveObject();

	SaveInt(ammo_type);
	SaveInt(PWR);
	SaveInt(ACC);
	SaveInt(energy_PWR);
	SaveInt(energy_ACC);	
	SaveInt(magazine_capacity);
	SaveInt(fire_type);
	SaveInt(available_fire_types);
	ammo.SaveObject(); // ??? Czy moze jako wskaznik?

	return UniqueNumber;
}

bool RANGED_WEAPON::LoadObject()
{
#ifdef LOAD_DEBUG
	printf("* RANGED_WEAPON::LoadObject\n");
#endif

	ITEM::LoadObject();

	LoadInt(ammo_type);
	LoadInt(PWR);
	LoadInt(ACC);
	LoadInt(energy_PWR);
	LoadInt(energy_ACC);
	LoadInt(magazine_capacity);
	LoadInt(fire_type);
	LoadInt(available_fire_types);
	ammo.LoadObject(); // ??? Czy moze jako wskaznik?
	return true;
}

unsigned long BASE_ARMOR::SaveObject()
{
	if (IsAlreadySaved == true)
	{
	   fprintf(stderr,"ERROR SAVE:  TOSAVE already saved\n");
	   keyboard.wait_for_key(); 
	   exit(230);
	}

	ITEM::SaveObject();

	SaveInt(ARM);
	SaveInt(MOD_STR);
	SaveInt(MOD_DEX);
	SaveInt(MOD_SPD);

	SaveInt(energy_ARM);
	SaveInt(energy_MOD_STR);
	SaveInt(energy_MOD_DEX);
	SaveInt(energy_MOD_SPD);
	SaveLong(energy_properties);
	SaveLong(energy_real_properties);	
	
	return UniqueNumber;
}

bool BASE_ARMOR::LoadObject()
{
#ifdef LOAD_DEBUG
	printf("* BASE_ARMOR::LoadObject\n");
#endif

	ITEM::LoadObject();

	LoadInt(ARM);
	LoadInt(MOD_STR);
	LoadInt(MOD_DEX);
	LoadInt(MOD_SPD);

	LoadInt(energy_ARM);
	LoadInt(energy_MOD_STR);
	LoadInt(energy_MOD_DEX);
	LoadInt(energy_MOD_SPD);
	LoadLong(energy_properties);
	LoadLong(energy_real_properties);

	return true;
}

unsigned long NO_ARMOR::SaveObject()
{
	if (IsAlreadySaved == true)
	{
	   fprintf(stderr,"ERROR SAVE:  TOSAVE already saved\n");
	   keyboard.wait_for_key(); 
	   exit(230);
	}

	BASE_ARMOR::SaveObject();

	return UniqueNumber;
}

bool NO_ARMOR::LoadObject()
{
#ifdef LOAD_DEBUG
	printf("* NO_ARMOR::LoadObject\n");
#endif

	BASE_ARMOR::LoadObject();
	return true;
}

unsigned long ARMOR::SaveObject()
{
	if (IsAlreadySaved == true)
	{
	   fprintf(stderr,"ERROR SAVE:  TOSAVE already saved\n");
	   keyboard.wait_for_key(); 
	   exit(230);
	}

	BASE_ARMOR::SaveObject();	
	return UniqueNumber;
}

bool ARMOR::LoadObject()
{
#ifdef LOAD_DEBUG
	printf("* ARMOR::LoadObject\n");
#endif

	BASE_ARMOR::LoadObject();	
	return true;
}

unsigned long HAND_WEAPON::SaveObject()
{
	if (IsAlreadySaved == true)
	{
	   fprintf(stderr,"ERROR SAVE:  TOSAVE already saved\n");
	   keyboard.wait_for_key(); 
	   exit(230);
	}

	ITEM::SaveObject();

	SaveInt(energy_DMG);
	SaveInt(energy_HIT);
	SaveInt(energy_DEF);
	SaveLong(energy_properties);	
	SaveLong(energy_real_properties);
	

	return UniqueNumber;
}

bool HAND_WEAPON::LoadObject()
{
#ifdef LOAD_DEBUG
	printf("* HAND_WEAPON::LoadObject\n");
#endif

	ITEM::LoadObject();

	LoadInt(energy_DMG);
	LoadInt(energy_HIT);
	LoadInt(energy_DEF);
	LoadLong(energy_properties);
	LoadLong(energy_real_properties);
	
	return true;
}

unsigned long GRENADE::SaveObject()
{
	if (IsAlreadySaved == true)
	{
	   fprintf(stderr,"ERROR SAVE:  TOSAVE already saved\n");
	   keyboard.wait_for_key(); 
	   exit(230);
	}

	ITEM::SaveObject();

	SaveInt(counter);
	SaveBool(active);
	SaveBool(works_now);
	SaveInt(PWR);
	SaveInt(RNG);
	SaveInt(DLY);

	return UniqueNumber;
}

bool GRENADE::LoadObject()
{
#ifdef LOAD_DEBUG
	printf("* GRENADE::LoadObject\n");
#endif

	ITEM::LoadObject();

	LoadInt(counter);
	LoadBool(active);
	LoadBool(works_now);
	LoadInt(PWR);
	LoadInt(RNG);
	LoadInt(DLY);
	return true;
}


unsigned long GAS::SaveObject()
{
	if (IsAlreadySaved == true)
	{
	   fprintf(stderr,"ERROR SAVE:  TOSAVE already saved\n");
	   keyboard.wait_for_key(); 
	   exit(230);
	}

	TILE::SaveObject();

	SaveInt(density);
	SaveLong(properties);

	return UniqueNumber;
}

bool GAS::LoadObject()
{
#ifdef LOAD_DEBUG
	printf("* GAS::LoadObject\n");
#endif

	TILE::LoadObject();

	LoadInt(density);
	LoadLong(properties);
	return true;
}

unsigned long FIRE::SaveObject()
{
	if (IsAlreadySaved == true)
	{
	   fprintf(stderr,"ERROR SAVE:  TOSAVE already saved\n");
	   keyboard.wait_for_key(); 
	   exit(230);
	}

	GAS::SaveObject();

	return UniqueNumber;
}

bool FIRE::LoadObject()
{
#ifdef LOAD_DEBUG
	printf("* FIRE::LoadObject\n");
#endif

	GAS::LoadObject();
	return true;
}

unsigned long ATTRIBUTE::SaveObject()
{
	if (IsAlreadySaved == true)
	{
	   fprintf(stderr,"ERROR SAVE:  TOSAVE already saved\n");
	   keyboard.wait_for_key(); 
	   exit(230);
	}

	SaveInt(val);
	SaveInt(max);
	SaveInt(mod);

	SaveInt(type);
	if (owner!=NULL)
		SaveUniqueNumber(owner->UniqueNumber);
	else
		SaveUniqueNumber(NULL_UNIQUE_NUMBER);
	

	return UniqueNumber;
}

bool ATTRIBUTE::LoadObject()
{
#ifdef LOAD_DEBUG
	printf("* ATTRIBUTE::LoadObject\n");
#endif
	
	LoadInt(val);
	LoadInt(max);
	LoadInt(mod);
	
	LoadInt(type);	
	LoadUniqueNumber( (TOSAVE **) &owner);
	
	return true;
}


unsigned long MONSTER::SaveObject()
{
	int index;

	if (IsAlreadySaved == true)
	{
	   fprintf(stderr,"ERROR SAVE:  TOSAVE already saved\n");
	   keyboard.wait_for_key(); 
	   exit(230);
	}

	TILE::SaveObject();

	SaveInt(last_pX);
	SaveInt(last_pY);
	SaveInt(special_properties);
	
	SaveInt(category);
	
	SaveInt(self_treatment);
	SaveBool(is_dead);
	
	fov_radius.SaveObject();

	SaveInt(size);
	SaveInt(weight);

	hit_points.SaveObject();
	energy_points.SaveObject();
	strength.SaveObject();
	dexterity.SaveObject();
	endurance.SaveObject();
	intelligence.SaveObject();
	speed.SaveObject();
	metabolism.SaveObject();

	SaveInt(group_affiliation);
	SaveInt(direction);

	unarmed.SaveObject();
	no_armor.SaveObject();

	if (weapon!=NULL)
		SaveUniqueNumber(weapon->UniqueNumber);
	else
		SaveUniqueNumber(NULL_UNIQUE_NUMBER);

	if (armor!=NULL)
		SaveUniqueNumber(armor->UniqueNumber);
	else
		SaveUniqueNumber(NULL_UNIQUE_NUMBER);

	SaveLong(next_action_time);

	SaveLong(seen_last_time_in_turn);
	SaveLong(experience_for_kill);

// player nie zapisuje of enemy, aby przy zmianie poziomu nie bylo bledu
	if (enemy!=NULL && this!=level.player) 
		SaveUniqueNumber(enemy->UniqueNumber);
	else
		SaveUniqueNumber(NULL_UNIQUE_NUMBER);

	for (index = 0; index < NUMBER_OF_SKILLS ; index++)
		SaveInt(skill[index]);

	for (index = 0; index < NUMBER_OF_RESISTS ; index++)
		SaveInt(resist[index]);

	for (index = 0; index < NUMBER_OF_STATES ; index++)
		SaveInt(state[index]);

	return UniqueNumber;
}

bool MONSTER::LoadObject()
{
#ifdef LOAD_DEBUG
	printf("* MONSTER::LoadObject\n");
#endif

	int index;

	TILE::LoadObject();

	LoadInt(last_pX);
	LoadInt(last_pY);
	LoadInt(special_properties);
	
	LoadInt(category);
	
	LoadInt(self_treatment);
	LoadBool(is_dead);
	
	fov_radius.LoadObject();

	LoadInt(size);
	LoadInt(weight);

	hit_points.LoadObject();
	energy_points.LoadObject();
	strength.LoadObject();
	dexterity.LoadObject();
	endurance.LoadObject();
	intelligence.LoadObject();
	speed.LoadObject();
	metabolism.LoadObject();

	LoadInt(group_affiliation);
	LoadInt(direction);

	unarmed.LoadObject();
	no_armor.LoadObject();

	LoadUniqueNumber( (TOSAVE **) &weapon);

	LoadUniqueNumber( (TOSAVE **) &armor);

	LoadLong(next_action_time);

	seen_now=false;
	
	LoadLong(seen_last_time_in_turn);
	LoadLong(experience_for_kill);

	LoadUniqueNumber( (TOSAVE **) &enemy);

	for (index = 0; index < NUMBER_OF_SKILLS ; index++)
		LoadInt(skill[index]);

	for (index = 0; index < NUMBER_OF_RESISTS ; index++)
		LoadInt(resist[index]);

	for (index = 0; index < NUMBER_OF_STATES ; index++)
		LoadInt(state[index]);
	return true;
}

unsigned long ENEMY::SaveObject()
{
	if (IsAlreadySaved == true)
	{
	   fprintf(stderr,"ERROR SAVE:  TOSAVE already saved\n");
	   keyboard.wait_for_key(); 
	   exit(230);
	}

	MONSTER::SaveObject();

	SaveInt(camping);
	SaveLong(enemy_last_seen_in_turn);		
	SaveInt(last_x_of_enemy);
	SaveInt(last_y_of_enemy);
	SaveInt(last_direction_of_enemy);
	SaveBool(is_at_enemy_position);
	SaveInt(last_direction_of_enemy);
	SaveInt(actual_behaviour);	
	SaveBool(turned_to_fight);
	return UniqueNumber;
}

bool ENEMY::LoadObject()
{
#ifdef LOAD_DEBUG
	printf("* ENEMY::LoadObject\n");
#endif

	MONSTER::LoadObject();

	LoadInt(camping);
	LoadLong(enemy_last_seen_in_turn);		
	LoadInt(last_x_of_enemy);
	LoadInt(last_y_of_enemy);
	LoadInt(last_direction_of_enemy);
	LoadBool(is_at_enemy_position);
	LoadInt(last_direction_of_enemy);
	LoadInt(actual_behaviour);
	LoadBool(turned_to_fight);
	return true;
}


unsigned long ANIMAL::SaveObject()
{
	if (IsAlreadySaved == true)
	{
	   fprintf(stderr,"ERROR SAVE:  TOSAVE already saved\n");
	   keyboard.wait_for_key(); 
	   exit(230);
	}

	ENEMY::SaveObject();

	return UniqueNumber;
}

bool ANIMAL::LoadObject()
{
#ifdef LOAD_DEBUG
	printf("* ANIMAL::LoadObject\n");
#endif

	ENEMY::LoadObject();
	return true;
}

unsigned long WORM::SaveObject()
{
	if (IsAlreadySaved == true)
	{
		fprintf(stderr,"ERROR SAVE:  TOSAVE already saved\n");
		keyboard.wait_for_key(); 
		exit(230);
	}

	ENEMY::SaveObject();
	if (prev!=NULL)
		SaveUniqueNumber(prev->UniqueNumber);
	else
		SaveUniqueNumber(NULL_UNIQUE_NUMBER);

	if (next!=NULL)
		SaveUniqueNumber(next->UniqueNumber);
	else
		SaveUniqueNumber(NULL_UNIQUE_NUMBER);

	SaveInt(length_left);
	SaveString(segment_name);
	SaveString(tail_name);

	return UniqueNumber;
}

bool WORM::LoadObject()
{
#ifdef LOAD_DEBUG
	printf("* WORM::LoadObject\n");
#endif

	ENEMY::LoadObject();

	LoadUniqueNumber( (TOSAVE **) &prev);
	LoadUniqueNumber( (TOSAVE **) &next);
	LoadInt(length_left);
	LoadString(segment_name);
	LoadString(tail_name);
	return true;
}


unsigned long MADDOCTOR::SaveObject()
{
	if (IsAlreadySaved == true)
	{
		fprintf(stderr,"ERROR SAVE:  TOSAVE already saved\n");
		keyboard.wait_for_key(); 
		exit(230);
	}
	
	INTELLIGENT::SaveObject();
	SaveInt(yelling_chance);
	tail.SaveObject();	
	
	return UniqueNumber;
}

bool MADDOCTOR::LoadObject()
{
#ifdef LOAD_DEBUG
	printf("* MADDOCTOR::LoadObject\n");
#endif
	
	INTELLIGENT::LoadObject();
	LoadInt(yelling_chance);
	tail.LoadObject();	
	
	return true;
}


unsigned long SEARCHLIGHT::SaveObject()
{
	if (IsAlreadySaved == true)
	{
		fprintf(stderr,"ERROR SAVE:  TOSAVE already saved\n");
		keyboard.wait_for_key(); 
		exit(230);
	}

	ENEMY::SaveObject();
	SaveInt(destination_x);
	SaveInt(destination_y);
	
	return UniqueNumber;
}

bool SEARCHLIGHT::LoadObject()
{
#ifdef LOAD_DEBUG
	printf("* SEARCHLIGHT::LoadObject\n");
#endif
	
	ENEMY::LoadObject();
	LoadInt(destination_x);
	LoadInt(destination_y);
	return true;
}



unsigned long INTELLIGENT::SaveObject()
{
	if (IsAlreadySaved == true)
	{
	   fprintf(stderr,"ERROR SAVE:  TOSAVE already saved\n");
	   keyboard.wait_for_key(); 
	   exit(230);
	}

	ENEMY::SaveObject();

	SaveLong(AI_restrictions);	
	SaveLong(turns_of_searching_for_enemy);	
	SaveLong(turns_of_calm);	
	SaveInt(misses_in_shooting);
	SaveInt(run_away_to_x);
	SaveInt(run_away_to_y);
	SaveList(backpack);
	SaveInt(max_items_in_backpack);
	SaveInt(items_in_backpack);

	if (ready_weapon!=NULL) 
		SaveUniqueNumber(ready_weapon->UniqueNumber);
	else
		SaveUniqueNumber(NULL_UNIQUE_NUMBER);
	
	return UniqueNumber;
}

bool INTELLIGENT::LoadObject()
{
#ifdef LOAD_DEBUG
	printf("* INTELLIGENT::LoadObject\n");
#endif

	ENEMY::LoadObject();

	LoadLong(AI_restrictions);	
	LoadLong(turns_of_searching_for_enemy);	
	LoadLong(turns_of_calm);	
	LoadInt(misses_in_shooting);
	LoadInt(run_away_to_x);
	LoadInt(run_away_to_y);
	LoadList(backpack);
	LoadInt(max_items_in_backpack);
	LoadInt(items_in_backpack);

	LoadUniqueNumber( (TOSAVE **) &ready_weapon);
	
	return true;
}

unsigned long ROBOT_SHELL::SaveObject()
{
	if (IsAlreadySaved == true)
	{
		fprintf(stderr,"ERROR SAVE:  TOSAVE already saved\n");
		keyboard.wait_for_key(); 
		exit(230);
	}
	
	ITEM::SaveObject();

	SaveInt(max_number_move_slots);
	SaveInt(max_number_action_slots);
	SaveInt(ARM);

	SaveList(move_slots);
	SaveList(action_slots);
	
	if (cpu!=NULL) 
		cpu->SaveObject();
	else
		SaveNULLObject();

	SaveString(last_robot_name);
	
	return UniqueNumber;
}

bool ROBOT_SHELL::LoadObject()
{
#ifdef LOAD_DEBUG
	printf("* ROBOT_SHELL::LoadObject\n");
#endif
	
	ITEM::LoadObject();
	
	LoadInt(max_number_move_slots);
	LoadInt(max_number_action_slots);
	LoadInt(ARM);

	LoadList(move_slots);
	LoadList(action_slots);

	cpu = (PROCESSOR *) CreateObject();
	if (cpu!=NULL)
		cpu->LoadObject();

	LoadString(last_robot_name);
	
	return true;
}
//////////////////////////////////////////////////////////////////////////
unsigned long ROBOT::SaveObject()
{
	if (IsAlreadySaved == true)
	{
		fprintf(stderr,"ERROR SAVE:  TOSAVE already saved\n");
		keyboard.wait_for_key(); 
		exit(230);
	}
	
	ENEMY::SaveObject();

	SaveInt(last_x_of_creator);
	SaveInt(last_y_of_creator);
	SaveBool(sees_creator);
	SaveLong(Creator_ID);
	

	assert(shell!=NULL); // tak byc nie powinno. Robot nie istnieje bez powloki!

	shell->SaveObject();
	
	return UniqueNumber;
}

bool ROBOT::LoadObject()
{
#ifdef LOAD_DEBUG
	printf("* ROBOT::LoadObject\n");
#endif
	
	ENEMY::LoadObject();

	LoadInt(last_x_of_creator);
	LoadInt(last_y_of_creator);
	LoadBool(sees_creator);
	LoadLong(Creator_ID);
	

	shell = (ROBOT_SHELL *) CreateObject();
	if (shell!=NULL)
		shell->LoadObject();
	
	return true;
}


//////////////////////////////////////////////////////////////////////////

unsigned long HERO::SaveObject()
{
	if (IsAlreadySaved == true)
	{
	   fprintf(stderr,"ERROR SAVE:  TOSAVE already saved\n");
	   keyboard.wait_for_key(); 
	   exit(230);
	}

	// Inicjalizacja
	FILE *fp, *old_fp;

	old_fp = FilePointer;

	fp = fopen(string(string("save/") + level.player->name + ".sav").c_str(),"wb+");
	if (fp==NULL)
	{
		fprintf(stderr,"ERROR SAVE: Error creating file save/%s.sav",level.player->name.c_str());		
		keyboard.wait_for_key(); 
		exit(83);
	}
	TOSAVE::InitFilePointer(fp);

	// Zapisanie
	INTELLIGENT::SaveObject();

	SaveInt(exp_level);
	SaveLong(experience);
	SaveLong(level_at_experience);
	SaveLong(free_skill_pointes);
	SaveLong(adaptation_points);
	
	for (int index = 0; index < NUMBER_OF_SKILLS ; index++)
		SaveInt(experience_in_skill[index]);

	SaveString(level.ID); // jest na tym poziomie
	SaveString(ID_of_last_level);
	SaveInt(stairs_number_used);

	// save potworkow, ktore ida za graczem na new_one poziom
	SaveList(monsters_following_player_to_other_level);	
  
	// save pol¹czeñ poziomów
	// schody w dol
	 map_of_level_links::iterator m,_m;
	 list <string>::iterator j,_j;
	 pair < string , list < string > > para;

	 SaveInt(level.stairs_down.size());
	 for (m=level.stairs_down.begin(),_m=level.stairs_down.end();m!=_m;m++)
	 {
		 para = *m;
		 SaveString(para.first);
		 SaveInt(para.second.size()); // size listy
		 for (j = para.second.begin(),_j = para.second.end();j!=_j;j++)
		 {
			SaveString(*j); // name
		 }
	 }
	 // schody w gore
	 SaveInt(level.stairs_up.size());
	 for (m=level.stairs_up.begin(),_m=level.stairs_up.end();m!=_m;m++)
	 {
		 para = *m;
		 SaveString(para.first);
		 SaveInt(para.second.size()); // size listy
		 for (j = para.second.begin(),_j = para.second.end();j!=_j;j++)
		 {
			SaveString(*j); // name
		 }
	 }

	 // save znanych graczowi nazw

	 set_of_known_names::iterator nn;

	 // save liczby znanych
	 SaveInt(known_items.size());

	 // save kolejnych

	 for (nn=known_items.begin();nn!=known_items.end();nn++)
	 {
		 SaveString((*nn));
	 }

   // Zapisanie map wszystkich nieznanych items
	 // save sizei map
	 SaveInt(definitions.constantly_unknown_items.size());
	 // save wszystkich elementow
	 map_of_unknown_items::iterator mnp,_mnp;
	 mnp=definitions.constantly_unknown_items.begin();
	 _mnp=definitions.constantly_unknown_items.end();
	 
	 for (;mnp!=_mnp;mnp++)
	 {
		 SaveString((*mnp).first);
		 SaveString((*mnp).second.name);
		 SaveInt((*mnp).second.color);
	 }
	 


	//////////////////////////////////////////////////////////////////////////	
	fclose(fp);

	TOSAVE::InitFilePointer(old_fp);
	TOSAVE::SaveOptions(string("save/") + name + ".opt");		
	TOSAVE::SaveMessageBuffer(string("save/") + name + ".msg");		
	return UniqueNumber;
}

bool HERO::LoadObject()
{
#ifdef LOAD_DEBUG
	printf("* HERO::LoadObject\n");
#endif
	int index;

	INTELLIGENT::LoadObject();

	LoadInt(exp_level);
	LoadLong(experience);
	LoadLong(level_at_experience);
	LoadLong(free_skill_pointes);
	LoadLong(adaptation_points);	

	for (index = 0; index < NUMBER_OF_SKILLS ; index++)
		LoadInt(experience_in_skill[index]);

	LoadString(ID_of_level); // jest na tym poziomie
	LoadString(ID_of_last_level);
	LoadInt(stairs_number_used);

	// load potworkow, ktore ida za graczem na new_one poziom
	LoadList(monsters_following_player_to_other_level);		

	// load polaczen poziomow

	 int number_a, number_b, count;
	 string temp_a,temp_b;

	 if (level.stairs_down.size()!=0 || level.stairs_up.size()!=0)
	 {
	   fprintf(stderr,"ERROR READ:  Sa juz definitions stairs!\n");
	   keyboard.wait_for_key(); 
	   exit(192);
	 }

	// schody w dol

	 LoadInt(number_a);
	 for (index=0;index<number_a;index++)
	 {
		 LoadString(temp_a);
		 LoadInt(number_b); // size listy
		 for (count=0;count<number_b;count++)
		 {
			 LoadString(temp_b);
			 level.stairs_down[temp_a].push_back(temp_b);
		 }
	 }

	// schody w gore

	 LoadInt(number_a);
	 for (index=0;index<number_a;index++)
	 {
		 LoadString(temp_a);
		 LoadInt(number_b); // size listy
		 for (count=0;count<number_b;count++)
		 {
			 LoadString(temp_b);
			 level.stairs_up[temp_a].push_back(temp_b);
		 }
	 }

	 // load zestawu znanych graczowi nazw
	 
	 known_items.clear();
	 
	 // load liczby nieznanych
	 int number_nazw;
	 LoadInt(number_nazw);
	 string wczytany;
	 
	 // load kolejnych
	 
	 for (index=0;index<number_nazw;index++)
	 {
		 LoadString(wczytany);
		 add_known_item(wczytany);
	 }

 // Wczytanie map wszystkich nieznanych items
	 definitions.constantly_unknown_items.clear();
	 // load sizei map
	 int size_mapy;
	 unknown_item np;
	 LoadInt(size_mapy);
	 // Wczytanie wszystkich elementow

	 for (index=0;index<size_mapy;index++)
	 {
		 LoadString(wczytany);
		 LoadString(np.name);
		 LoadInt(np.color);	
		 definitions.constantly_unknown_items[wczytany]=np;
	 }	 	 

	UpdatePointers();

	return true;
}

unsigned long LEVEL::SaveObject()
{
	if (IsAlreadySaved == true)
	{
	   fprintf(stderr,"ERROR SAVE:  TOSAVE already saved\n");
	   keyboard.wait_for_key(); 
	   exit(230);
	}
    clear_lists_to_delete();

	// Inicjalizacja
	FILE *fp;
	fp = fopen(string(string("save/") + player->name + "." + ID).c_str(),"wb+");
	if (fp==NULL)
	{
		fprintf(stderr,"ERROR SAVE: Error creating file save/%s",(level.player->name + "." + ID).c_str());				
		keyboard.wait_for_key(); 
		exit(82);
	}
	TOSAVE::InitFilePointer(fp);

	// save

	TOSAVE::SaveObject();

	SaveString(ID);
	SaveInt(fov_range);
	SaveLong(turn);

	ptr_list::iterator sm,_sm;
	MONSTER *monster;
	// usuniecie ich z listy monsters na poziomie
	sm=player->monsters_following_player_to_other_level.begin();
	_sm=player->monsters_following_player_to_other_level.end();
	for (;sm!=_sm;sm++)
	{
		monster=(MONSTER *) *sm;
		monster->ChangePosition(-1,-1);
		monsters.remove(monster);
	}
	
	SaveList(items_on_map);
	monsters.remove(player);
	SaveList(monsters);
	SaveList(gases_on_map);

	//////////////////////////////////////////////////////////////////////////	
	// Save map

	map.SaveObject();

	fclose(fp);

	// Save the player for case of game's crash

	level.player->SaveObject();

	// Set all saved items that are not saved, to make save possible again
	list_of_saved::iterator m,_m;

	for (m=saved.begin(),_m=saved.end();m!=_m;m++)
		((TOSAVE *) *m)->IsAlreadySaved = false;

	saved.clear();

	return UniqueNumber;
}

bool LEVEL::LoadObject()
{
#ifdef LOAD_DEBUG
	printf("* LEVEL::LoadObject\n");
#endif
	// Initialize
	
	FILE *fp;
	fp = fopen(string(string("save/") + game.name_of_player + "." + level.ID).c_str(),"rb+");
	if (fp==NULL) // No such level!
	{
#ifdef LOAD_DEBUG
		fprintf(stderr,"ERROR READ: Can not open file level save/%s",(level.player->name + "." + level.ID).c_str());						
#endif
		return false;
	}

	TOSAVE::InitFilePointer(fp);

	// save

	TOSAVE::LoadObject();

	LoadString(ID);	

	LoadInt(fov_range);	
	LoadLong(turn);

	LoadList(items_on_map);
	LoadList(monsters);
	LoadList(gases_on_map);

	// load map
	map.LoadObject();

	fclose(fp);

	TOSAVE::InitFilePointer(NULL);

	UpdatePointers();

	ListToFix.clear();
	AddressesOfObjects.clear();
   
	return true;
}

unsigned long MAP::SaveObject()
{
	if (IsAlreadySaved == true)
	{
	   fprintf(stderr,"ERROR SAVE:  TOSAVE already saved\n");
	   keyboard.wait_for_key(); 
	   exit(230);
	}

	TOSAVE::SaveObject();

	// save map
	SaveList(stairs_of_map);

	int x,y;

	for (x=0;x<MAPWIDTH;x++)
		for (y=0;y<MAPHEIGHT;y++)
			cells[x][y].SaveObject();

	return UniqueNumber;
}

bool MAP::LoadObject()
{
#ifdef LOAD_DEBUG
	printf("* MAP::LoadObject\n");
#endif

	TOSAVE::LoadObject();

// load map
	LoadList(stairs_of_map);

	int x,y;

	for (x=0;x<MAPWIDTH;x++)
		for (y=0;y<MAPHEIGHT;y++)
			cells[x][y].LoadObject();
	return true;
}

unsigned long CELL::SaveObject()
{
	SaveBool(seen);
	SaveChar(last_tile);
	SaveChar(seen_at_least_once);
	SaveString(name);
	SaveChar(tile);
	SaveInt(color);
	SaveBool(block_los);
	SaveBool(real_block_los);
	SaveBool(block_move);
	SaveBool(real_block_move);
	SaveBool(is_stairs);
	SaveBool(is_monster);
	SaveInt(number_of_items);
	SaveInt(number_of_gases);
	SaveInt(shield);
	SaveInt(hit_points);
	SaveInt(max_hit_points);
	SaveInt(about_open);
	SaveString(destroyed_to);
	return 0;
}


bool CELL::LoadObject()
{
	LoadBool(seen);
	LoadChar(last_tile);
	LoadChar(seen_at_least_once);
	LoadString(name);
	LoadChar(tile);
	LoadInt(color);
	LoadBool(block_los);
	LoadBool(real_block_los);
	LoadBool(block_move);
	LoadBool(real_block_move);
	LoadBool(is_stairs);
	LoadBool(is_monster);
	LoadInt(number_of_items);
	LoadInt(number_of_gases);
	LoadInt(shield);
	LoadInt(hit_points);
	LoadInt(max_hit_points);
	LoadInt(about_open);
	LoadString(destroyed_to);
	return true;
}

void TOSAVE::SaveLastUNToFile()
{
	FILE *fp, *old_fp;

	old_fp = FilePointer;

	fp = fopen(string(string("save/") + game.name_of_player + ".un").c_str(),"wb+");
	if (fp==NULL)
	{
		fprintf(stderr,"ERROR SAVE UniqueNumber");								
		keyboard.wait_for_key(); 
		exit(81);
	}
	InitFilePointer(fp);
	SaveLong(LastUniqueNumber);
	InitFilePointer(old_fp);

	fclose(fp);
}

void TOSAVE::LoadLastUNFromFile()
{
	FILE *fp, *old_fp;

	old_fp = FilePointer;

	fp = fopen(string(string("save/") + game.name_of_player + ".un").c_str(),"rb+");
	if (fp==NULL)
	{
		fprintf(stderr,"ERROR READ UniqueNumber");								
		keyboard.wait_for_key(); 
		exit(80);
	}
	InitFilePointer(fp);
	LoadLong(LastUniqueNumber);
	InitFilePointer(old_fp);

	fclose(fp);
}























