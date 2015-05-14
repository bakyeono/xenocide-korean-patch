#include "mem_check.h"

// #define SHOW_LINES
//#define DEBUG_PARSER

#include "parser.h"
#include "keyboard.h"
#include "global.h"
#include "map.h"
#include "level.h"
#include <stdio.h>

// ponizsze dwa do toupper
#include <algorithm>

#include <cctype>
#include <vector>

extern LEVEL level;
extern KEYBOARD keyboard;
extern DEFINITIONS definitions;
extern DESCRIPTIONS descriptions;

CFILE::CFILE()
{
	buf = new char[FILE_BUF_SIZE];
}

CFILE::~CFILE()
{
	delete buf;	
}

bool CFILE :: open(string name)
{
  line_number=0;
  file_name = name;
  fp=fopen(name.c_str(),"rt");
  if (fp==NULL)
	  return false;
  return true;
}

bool CFILE :: create(string name)
{
	line_number=0;
	file_name = name;
	fp=fopen(name.c_str(),"wt+");
	if (fp==NULL)
		return false;
	return true;
}

string CFILE :: get_line()
{
  string line;
  if (fgets(buf, FILE_BUF_SIZE, fp)==NULL)
  {
    return string("");
  }
#ifdef SHOW_LINES
  printf("\n%d: %s",line_number,buf);
#endif
  line_number++;
  // change all underscores to spaces
  for (int a=0;a<strlen(buf);a++)
   if (buf[a]=='_')
     buf[a]=' ';
  
  return string(buf);
}

void CFILE :: save_line(string line)
{
	fprintf(fp,"%s\n",line.c_str());
}

void CFILE :: close()
{
  fclose(fp);
}

ITEM * DEFINITIONS :: find_item(string name)
{
   ITEM *from_definition;
   ptr_list :: iterator m,_m;

   from_definition=NULL;
   for(m=items.begin(),_m=items.end(); m!=_m; m++)
   {
       from_definition=(ITEM *)*m;
       if (from_definition->name==name)
          return from_definition;
   }
   return NULL;
}


CELL * DEFINITIONS :: find_terrain(string name)
{
   CELL *from_definition;
   ptr_list :: iterator m,_m;

   from_definition=NULL;
   for(m=terrains.begin(),_m=terrains.end(); m!=_m; m++)
   {
       from_definition=(CELL *)*m;
       if (from_definition->name==name)
          return from_definition;
   }

  // 
     fprintf(stderr,"\nERROR! Not found in definitions of terrain %s\n\n",name.c_str());
	 keyboard.wait_for_key();
     exit(100);
   return NULL;
}

MONSTER * DEFINITIONS :: find_monster(string name)
{
   MONSTER *from_definition;
   ptr_list :: iterator m,_m;

   from_definition=NULL;
   for(m=monsters.begin(),_m=monsters.end(); m!=_m; m++)
   {
       from_definition=(MONSTER *)*m;
       if (from_definition->name==name)
          return from_definition;
   }
   return NULL;
}

string get_from_quotations(string line, int number)
{
	string nowa;
	int poz1,poz2;
	poz1=line.find('"')+1;
	poz2=line.rfind('"');
	nowa=line.substr(poz1,poz2-poz1);
	if (nowa.size()<1)
	{
		fprintf(stderr,"\nERROR in cutting from quotations #: %d %s\n\n",number,line.c_str());
		keyboard.wait_for_key();
		exit(30);
	}
	return nowa;
}


int get_category(string line, int line_number)
{
	string temp;
	temp = get_from_quotations(line,line_number);
	
	if (definitions.category_number.find(temp)!=definitions.category_number.end())
	{
		return definitions.category_number[temp];
	}
	else
	{
		fprintf(stderr,"\nUnknown category in line #%d:\n%s\n",line_number,line.c_str());
		keyboard.wait_for_key(); 
		exit(45);
		return 0;    
	}	
}




bool DEFINITIONS :: read_ammo_types()
{
  CFILE file;
  string line;
  string kind;
  int read=0;

  printf("\nReading file data/ammotype.lst\n");
  if (!file.open("data/ammotype.lst"))
	  file.error_open();

  line=file.get_line();
  while (line!="")
  {
	  if (line.find("#")==-1)
	  {
		  kind = get_from_quotations(line, file.line_number);
		  if (kind!="")
		  {
			  ammo_types.push_back(kind);  
			  read++;
		  }
	  }
	  
	  line=file.get_line();
  }
  file.close();
  printf("  -- Done. %d types loaded.\n",read);    
  return true;  
}

bool DEFINITIONS :: read_categories()
{
	CFILE file;
	string line;
	string cat;
	int read=0;
	
	printf("\nReading file data/category.lst\n");
	if (!file.open("data/category.lst"))
		file.error_open();
	
	line=file.get_line();
	while (line!="")
	{
		if (line.find("#")==-1)
		{
			cat = get_from_quotations(line, file.line_number);
			if (cat!="")
			{
				category_number[cat] = read+1;
				read++;
			}
		}
		line=file.get_line();
	}
	file.close();
	printf("  -- Done. %d categories loaded.\n",read);    
	return true;  
}


int get_value(string line, int line_number)
{
  string number;
  string digits=string("-0123456789");
  int pos=line.find('=');
  if (pos==-1)
    return -1;

  pos=line.find_first_of(digits,pos+1);
  if (pos==-1)
    return -1;
  number=line.substr(pos,(unsigned long) -1);
  int value=atoi( number.c_str() );
  return value;
}

bool DEFINITIONS :: read_pills_types()
{
	CFILE file;
	string line;
	unknown_item p;
	int read=0;
	
	printf("\nReading file data/pills.lst\n");	
	if (!file.open("data/pills.lst"))
		file.error_open();	
	
	line=file.get_line();
	while (line!="")
	{
		p.name = get_from_quotations(line, file.line_number);
		p.color = get_value(line, file.line_number);
		unknown_pills.push_back(p);  
		line=file.get_line();
		read++;
	}
	file.close();
	printf("  -- Done. %d types loaded.\n",read);
	// randmize unknown pills
	shuffle_list(unknown_pills);
	return true;	
}

bool DEFINITIONS :: read_grenades_types()
{
	CFILE file;
	string line;
	unknown_item p;
	int read=0;
	
	printf("\nReading file data/grenades.lst\n");	
	if (!file.open("data/grenades.lst"))
		file.error_open();	
	
	line=file.get_line();
	while (line!="")
	{
		p.name = get_from_quotations(line, file.line_number);
		p.color = get_value(line, file.line_number);
		unknown_grenades.push_back(p);  
		line=file.get_line();
		read++;
	}
	file.close();
	printf("  -- Done. %d types loaded.\n",read);    		
	// randomize unknown names
	shuffle_list(unknown_grenades);
	return true;	
}

bool DEFINITIONS::create_map_of_game_unknown_items()
{
	CFILE file;
	string line;
	int category;
	list_of_names_for_categories l;
	list_of_names_for_categories::iterator m,_m;
	unknown_items_list::iterator n;
	string name;

	////////////////// GRENADES //////////////////////
	
	printf("\nReading file data/un_gren.lst\n");	
	if (!file.open("data/un_gren.lst"))
		file.error_open();	
	
	line=file.get_line();
	while (line!="")
	{
		category = get_category(line, file.line_number);
		// create map of unknown items from this category

		if (objects_in_category.find(category)!=objects_in_category.end())
		{
			l=objects_in_category[category];
			n=unknown_grenades.begin();
			for (m=l.begin(),_m=l.end();m!=_m;m++,n++)
			{
				if (n==unknown_grenades.end())
				{
					fprintf(stderr,"\nERROR: Not enough items in file data/grenades.lst\n\n");
					keyboard.wait_for_key();
					exit(32);			
				}
				name = *m; // name of item
#ifdef DEBUG_PARSER
				printf(">ITEM %s assign name %s and color %d\n",name.c_str(),(*n).name.c_str(),(*n).color);
#endif			
				constantly_unknown_items[name]=*n;
			}			
		} // endof objecty sa w kategorii
		line=file.get_line();
	}
	file.close();

	////////////////// PILLS //////////////////////
	
	printf("\nReading file data/un_pills.lst\n");	
	if (!file.open("data/un_pills.lst"))
		file.error_open();	
	
	line=file.get_line();
	while (line!="")
	{
		category = get_category(line, file.line_number);
		// create map of unknown items from this category
		
		if (objects_in_category.find(category)!=objects_in_category.end())
		{
			l=objects_in_category[category];
			n=unknown_pills.begin();
			for (m=l.begin(),_m=l.end();m!=_m;m++,n++)
			{
				if (n==unknown_pills.end())
				{
					fprintf(stderr,"\nERROR: Za malo items w pliku data/pills.lst\n\n");
					keyboard.wait_for_key();
					exit(32);			
				}
				name = *m; // name itemu
#ifdef DEBUG_PARSER
				printf(">ITEM %s assign name %s and color %d\n",name.c_str(),(*n).name.c_str(),(*n).color);
#endif			
				constantly_unknown_items[name]=*n;
			}			
		} // endof objecty sa w kategorii
		line=file.get_line();
	}
	file.close();
	
	return true;
}


bool get_boolean(string line, int line_number)
{
  if (line.find("FALSE") != -1 || line.find("false") != -1)
    return false;
  return true;
}


int get_skill(string line, int line_number)
{
  if (line.find("SMALL") != -1)
    return SKILL_SMALL_GUNS;
  else if (line.find("BIG") != -1)
    return SKILL_BIG_GUNS;
  else if (line.find("ENERGY") != -1)
    return SKILL_ENERGY_WEAPONS;
  else if (line.find("UNARMED") != -1)
    return SKILL_UNARMED;
  else if (line.find("MELEE") != -1)
    return SKILL_MELEE_WEAPONS;
  else if (line.find("THROWING") != -1)
    return SKILL_THROWING;
  else if (line.find("MEDICINE") != -1)
    return SKILL_MEDICINE;
  else if (line.find("DODGING") != -1)
    return SKILL_DODGING;
  else if (line.find("WEAPON KNOWLEDGE") != -1)
	  return SKILL_ALIEN_SCIENCE;
  else if (line.find("ALIEN SCIENCE") != -1)
	  return SKILL_ALIEN_SCIENCE;
  else if (line.find("MECHANIC") != -1)
	  return SKILL_MECHANIC;
  else if (line.find("PROGRAMMING") != -1)
	  return SKILL_PROGRAMMING;
  else
  {
	  fprintf(stderr,"\nUnknown skill in line #%d:\n%s\n",line_number,line.c_str());
	  keyboard.wait_for_key(); exit(45);
  }

  return 0;    
}

int get_resist(string line, int line_number)
{
	if (line.find("RADIOACTIV") != -1)
		return RESIST_RADIOACTIVE;
	else if (line.find("POISON") != -1)
		return RESIST_CHEM_POISON;
	else if (line.find("STUN") != -1)
		return RESIST_STUN;
	else if ((line.find("PARALYZ") != -1) || (line.find("SLOW") != -1))
		return RESIST_PARALYZE;
	else if (line.find("FIRE") != -1)
		return RESIST_FIRE;
	else if (line.find("ELECTRIC") != -1)
		return RESIST_ELECTRICITY;
	else if (line.find("BLIND") != -1)
		return RESIST_BLIND;
	else
	{
		fprintf(stderr,"\nNieznany resist in line #%d:\n%s\n",line_number,line.c_str());
		keyboard.wait_for_key(); exit(45);
	}
	
	return 0;    
}


int get_special_properties(string line, int line_number)
{
	if (line.find("SPLITTER") != -1)
		return SPECIAL_PROPERTY_SPLITTER;
	else
	{
		fprintf(stderr,"\nNieznana specjalna wlasciwosc in line #%d:\n%s\n",line_number,line.c_str());
		keyboard.wait_for_key(); exit(45);
	}
	
	return 0;    
}


unsigned long get_AI_restrictions(string line, int line_number)
{
	AI_CONTROL k = AI_NOT_LIMITED;
	
	if (line.find("DONT USE GRENADES")!=-1)
		k=k|AI_DONT_USE_GRENADES;
	if (line.find("DONT USE HEALING")!=-1)
		k=k|AI_DONT_USE_HEALING;
	if (line.find("DONT USE BOOSTERS")!=-1)
		k=k|AI_DONT_USE_BOOSTERS;
	if (line.find("DONT USE RANGED")!=-1)
		k=k|AI_DONT_USE_RANGED_WEAPONS;
	if (line.find("DONT MOVE")!=-1)
		k=k|AI_DONT_MOVE;
	if (line.find("DONT COOPEATE")!=-1)
		k=k|AI_DONT_COOPERATE_WITH_FRIENDS;
	if (line.find("DONT RUN")!=-1)
		k=k|AI_DONT_RUN_AWAY;
	if (line.find("DONT PICK")!=-1)
		k=k|AI_DONT_PICK_UP_ITEMS;
	if (line.find("DONT USE OTHER WEAPON")!=-1)
		k=k|AI_DONT_USE_OTHER_WEAPONS;
	if (line.find("DONT USE OTHER ARMOR")!=-1)
		k=k|AI_DONT_USE_OTHER_ARMORS;	
	
	if (k==AI_NOT_LIMITED)
	{
		fprintf(stderr,"ERROR: Nie znane ograniczenie AI w %s.\n",line.c_str());
		keyboard.wait_for_key();
		exit(245);
	}
	return k;	
}

int get_group(string line, int line_number)
{
	if (line.find("HERO") != -1)
		return GROUP_HERO;
	else if (line.find("ENEM") != -1)
		return GROUP_ENEMIES;
	else if (line.find("NEUTRAL") != -1)
		return GROUP_NEUTRAL;
	else if (line.find("CRAZY") != -1)
		return GROUP_CRAZY;
	else
	{
		fprintf(stderr,"\nUnknown group in line #%d:\n%s\n",line_number,line.c_str());
		keyboard.wait_for_key(); exit(46);
	}
	
	return 0;    
}

int DEFINITIONS :: check_ammo_type(string line)
{
   lista_ammo::iterator m,_m;
   bool found;
   int a;

   a=0;
   found=false;
   for(m=ammo_types.begin(),_m=ammo_types.end(); m!=_m; m++)
   {
       if (*m==line)
       {
         found=true;
         break;
       }
       a++;
   }
   if (found==true)
     return a;
   else
     return -1;
}

int get_fire_modes(string line, int line_number)
{
  int w=0;
  if (line.find("FIRE SINGLE")!=-1)
   w=w|FIRE_SINGLE;
  if (line.find("FIRE DOUBLE")!=-1)
   w=w|FIRE_DOUBLE;
  if (line.find("FIRE TRIPLE")!=-1)
   w=w|FIRE_TRIPLE;
  if (line.find("FIRE BURST")!=-1)
   w=w|FIRE_BURST;
  if (w==0)
  {
     fprintf(stderr,"\nERROR in file modes in line #%d\n\n",line_number);
     keyboard.wait_for_key(); exit(35);
  }
  return w;
}

int get_purpose(string line, int line_number)
{
  int w=0;
  if (line.find("AUTO DOOR OPEN")!=-1)
	w=PURPOSE_AUTO_DOOR_OPEN;
  else if (line.find("WEAK CORRIDOR IN MINES")!=-1)
	  w=PURPOSE_WEAK_CORRIDOR_IN_MINES;
  else if (line.find("SMOKE GENERATOR")!=-1)
	  w=PURPOSE_SMOKE_GENERATOR;
  else if (line.find("TUTORIAL ADVANCE")!=-1)
	  w=PURPOSE_TUTORIAL_ADVANCE;
  else if (line.find("INCRASE HP")!=-1)
	  w=PURPOSE_INCRASE_HP;
  else if (line.find("DECRASE HP")!=-1)
	  w=PURPOSE_DECRASE_HP;
  else if (line.find("INCRASE SPEED")!=-1)
	  w=PURPOSE_INCRASE_SPEED;
  else if (line.find("DECRASE SPEED")!=-1)
	  w=PURPOSE_DECRASE_SPEED;
  else if (line.find("INCRASE STRENGTH")!=-1)
	  w=PURPOSE_INCRASE_STRENGTH;
  else if (line.find("DECRASE STRENGTH")!=-1)
	  w=PURPOSE_DECRASE_STRENGTH;
  else if (line.find("INCRASE ENDURANCE")!=-1)
	  w=PURPOSE_INCRASE_ENDURANCE;
  else if (line.find("DECRASE ENDURANCE")!=-1)
	  w=PURPOSE_DECRASE_ENDURANCE;
  else if (line.find("INCRASE METABOLISM")!=-1)
	  w=PURPOSE_INCRASE_METABOLISM;
  else if (line.find("DECRASE METABOLISM")!=-1)
	  w=PURPOSE_DECRASE_METABOLISM;
  else if (line.find("INCRASE FOV")!=-1)
	  w=PURPOSE_INCRASE_FOV;
  else if (line.find("DECRASE FOV")!=-1)
	  w=PURPOSE_DECRASE_FOV;
  else if (line.find("INCRASE RADIOACTIVE")!=-1)
	  w=PURPOSE_INCRASE_RADIOACTIVE;
  else if (line.find("DECRASE RADIOACTIVE")!=-1)
	  w=PURPOSE_DECRASE_RADIOACTIVE;
  else if (line.find("INCRASE CHEM POISON")!=-1)
	  w=PURPOSE_INCRASE_CHEM_POISON;
  else if (line.find("DECRASE CHEM POISON")!=-1)
	  w=PURPOSE_DECRASE_CHEM_POISON;
  else if (line.find("REPAIR HP")!=-1)
	  w=PURPOSE_REPAIR_HP;
  else if (line.find("REPAIR FIX ARMOR")!=-1)
	  w=PURPOSE_REPAIR_FIX_ARMOR;
  else if (line.find("MACHINE CARD")!=-1)
	  w=PURPOSE_GENETIC_MACHINE_CARD;
  else if (line.find("TEMP MISSILE")!=-1)
	  w=PURPOSE_TEMP_MISSILE;
  else if (w==0)
  {
     fprintf(stderr,"\nERROR Nieznane purpose in line #%d\n\n",line_number);
     keyboard.wait_for_key(); exit(35);
  }
  return w;
}


PROPERTIES get_properties(string line)
{
  PROPERTIES w=0;

  if (line.find("NORMAL")!=-1)
    w=w|TYPE_NORMAL;
  if (line.find("ARMOR PERCING")!=-1)
    w=w|TYPE_ARMOR_PERCING;
  if (line.find("CONVERT CPU")!=-1)
	  w=w|TYPE_CONVERT_CPU;
  if (line.find("EXPLOSIVE")!=-1)
    w=w|TYPE_EXPLOSIVE;
  if (line.find("WRITE PROGRAM")!=-1)
	  w=w|TYPE_WRITE_PROGRAM;
  if (line.find("SMOKE")!=-1)
    w=w|TYPE_SMOKE;
  if (line.find("RADIOACTIVE")!=-1)
    w=w|TYPE_RADIOACTIVE;
  if (line.find("CHEM POISON")!=-1)
    w=w|TYPE_CHEM_POISON;
  if (line.find("PARALYZE")!=-1)
    w=w|TYPE_PARALYZE;
  if (line.find("STUN")!=-1)
    w=w|TYPE_STUN;
  if (line.find("POWER SHIELD")!=-1)
    w=w|TYPE_POWER_SHIELD;
  if (line.find("INCENDIARY")!=-1)
    w=w|TYPE_INCENDIARY;
  if (line.find("SENSOR")!=-1)
    w=w|TYPE_SENSOR;
  if (line.find("INVISIBLE")!=-1)
    w=w|TYPE_INVISIBLE;
  if (line.find("VAMPIRIC")!=-1)
	w=w|TYPE_VAMPIRIC;
  if (line.find("ELECTRIC")!=-1)
	  w=w|TYPE_ELECTRIC;
  if (line.find("ENERGY")!=-1)
	  w=w|TYPE_ENERGY;
  return w;
}

bool DEFINITIONS :: read_items_definitions()
{
  CFILE file;
  string line, real;
  int value;
  int read = 0;

   ITEM *new_one=NULL;
   
   AMMO *ammo=NULL;
   HAND_WEAPON *handw=NULL;
   RANGED_WEAPON *shooting=NULL;
   ARMOR *armor=NULL;
   GRENADE *grenade=NULL;
   NO_ARMOR *skin=NULL;
   CONTROLLER *controller=NULL;
   PILL *pill=NULL;
   TRASH *garbage=NULL;  
   REPAIR_KIT *repairset=NULL;
   CORPSE *cialo=NULL;  
   PROCESSOR *processor=NULL;
   ROBOT_LEG *leg=NULL;
   ROBOT_SHELL *shell=NULL;
   PROGRAMMATOR *programmator=NULL;
   BATTERY *battery=NULL;
   
  printf("\nReading file data/items.def\n");   
  if (!file.open("data/items.def"))
  	  file.error_open();
  
  line=string("something");
  while (line!="")
  {
    real=file.get_line();
    line=string(real) + "";
	transform(line.begin(), line.end(), line.begin(), toupper);
    
    
    // loog for [
    if (*line.c_str()=='#' || *line.c_str()=='\n') // komentarz
      continue;

    if (line.find('[')!=-1) // new_one item
    {
      read++;
      if (line.find("[AMMO]")!=-1)
      {
        ammo=new AMMO;
        new_one=(ITEM *) ammo;
      }
      else if (line.find("[RANGED WEAPON]")!=-1)
      {
        shooting=new RANGED_WEAPON;
        new_one=(ITEM *) shooting;
      }
      else if (line.find("[HAND WEAPON]")!=-1)
      {
        handw=new HAND_WEAPON;
        new_one=(ITEM *) handw;
      }
      else if (line.find("[ARMOR]")!=-1)
      {
        armor=new ARMOR;
        new_one=(ITEM *) armor;
      }
      else if (line.find("[GRENADE]")!=-1)
      {
        grenade=new GRENADE;
        new_one=(ITEM *) grenade;
      }
      else if (line.find("[CONTROLLER]")!=-1)
      {
        controller=new CONTROLLER;
        new_one=(ITEM *) controller;
      }
      else if (line.find("[ROBOT SHELL]")!=-1)
      {
		  shell=new ROBOT_SHELL;
		  new_one=(ITEM *) shell;
      }	  
      else if (line.find("[ROBOT LEG]")!=-1)
      {
		  leg=new ROBOT_LEG;
		  new_one=(ITEM *) leg;
      }	  
      else if (line.find("[PROCESSOR]")!=-1)
      {
		  processor=new PROCESSOR;
		  new_one=(ITEM *) processor;
      }	  	  
      else if (line.find("[PILL]")!=-1)
      {
		  pill=new PILL;
		  new_one=(ITEM *) pill;
      }
      else if (line.find("[SKIN]")!=-1)
      {
        skin=new NO_ARMOR;
        new_one=(ITEM *) skin;
      }
      else if (line.find("[TRASH]")!=-1)
      {
		  garbage=new TRASH;
		  new_one=(ITEM *) garbage;		  
      }
      else if (line.find("[PROGRAMMATOR]")!=-1)
      {
		  programmator=new PROGRAMMATOR;
		  new_one=(ITEM *) programmator;		  
      }
      else if (line.find("[BATTERY]")!=-1)
      {
		  battery=new BATTERY;
		  new_one=(ITEM *) battery;		  
      }
      else if (line.find("[REPAIR KIT]")!=-1)
      {
		  repairset=new REPAIR_KIT;
		  new_one=(ITEM *) repairset;
      }
      else if (line.find("[CORPSE]")!=-1)
      {
		  cialo=new CORPSE;
		  new_one=(ITEM *) cialo;
      }
      else if (line.find("[COPYOF ")!=-1)
      {
        line=get_from_quotations(real, file.line_number);
        new_one = this->find_item(line);
        if (new_one==NULL)
        {
           fprintf(stderr,"ERROR: There is no item %s.\n",line.c_str());
           keyboard.wait_for_key(); exit(201);
        }
        new_one = new_one->duplicate(); // stworznie kopii znalezionego

		// items w definicjach nie ma na mapie ani na liscie wszystkich
		level.remove_last_item_on_map();
		level.all_items.pop_back();
        
        // nie wiemy, jaki kind, wiec przypisujemy all...
        ammo=(AMMO *)new_one;
        shooting=(RANGED_WEAPON *)new_one;
        handw=(HAND_WEAPON*)new_one;
        armor=(ARMOR*)new_one;
        grenade=(GRENADE*)new_one;
        skin=(NO_ARMOR *)new_one;
		controller=(CONTROLLER *)new_one;
		pill = (PILL *)new_one;
		garbage = (TRASH *)new_one;
		repairset = (REPAIR_KIT *)new_one;
		cialo = (CORPSE *)new_one;
		processor=(PROCESSOR *)new_one;
		leg = (ROBOT_LEG*)new_one;
		shell = (ROBOT_SHELL*)new_one;		
      }
      else
      {
        fprintf(stderr,"Unknown item type %s in line #%d", real.c_str(),file.line_number);
		keyboard.wait_for_key(); exit(202);
      }
      items.push_back(new_one);
    }
    
                  else if (line.find("NAME =")!=-1)
                  {
                     line=get_from_quotations(real, file.line_number);
					 if (new_one->ClassName == "CORPSE")
					 {
						 cialo->rename( line + "ÀÇ ½ÃÃ¼");
						 cialo->of_what = line;
					 }
					 else
						 new_one->rename( line );
                  }				  
                  else if (line.find("TILE =")!=-1)
                  {
                     line=get_from_quotations(real, file.line_number);
                     new_one->tile = *line.c_str();
                  }
                  else if (line.find("DMG =")!=-1)
                  {
                     new_one->DMG = get_value(line,file.line_number);
                  }
                  else if (line.find("HIT =")!=-1)
                  {
                     new_one->HIT = get_value(line,file.line_number);
                  }
                  else if (line.find("DEF =")!=-1)
                  {
                     new_one->DEF = get_value(line,file.line_number);
                  }
				  else if (line.find("SOUND USED =")!=-1)
				  {
					  new_one->sound_used = get_from_quotations(line,file.line_number);
				  }
				  else if (line.find("SOUND FIRE SINGLE =")!=-1)
				  {
					  shooting->sound_file_single = get_from_quotations(line,file.line_number);
				  }
				  else if (line.find("SOUND EXPLOSION =")!=-1)
				  {
					  grenade->sound_explosion = get_from_quotations(line,file.line_number);
				  }
				  else if (line.find("SOUND RELOAD =")!=-1)
				  {
					  shooting->sound_reload = get_from_quotations(line,file.line_number);
				  }
				  else if (line.find("SOUND FIRE DOUBLE =")!=-1)
				  {
					  shooting->sound_file_double = get_from_quotations(line,file.line_number);
				  }
				  else if (line.find("SOUND FIRE TRIPLE =")!=-1)
				  {
					  shooting->sound_file_triple = get_from_quotations(line,file.line_number);
				  }
				  else if (line.find("SOUND GUN EMPTY =")!=-1)
				  {
					  shooting->sound_gun_empty = get_from_quotations(line,file.line_number);
				  }
				  else if (line.find("SOUND FIRE BURST =")!=-1)
				  {
					  shooting->sound_file_burst = get_from_quotations(line,file.line_number);
				  }
                  else if (line.find("REGENERATION TIME =")!=-1)
                  {
					  repairset->regeneration_time = get_value(line,file.line_number);
                  }				  
                  else if (line.find("MAX HIT POINTS =")!=-1)
                  {
                     new_one->max_hit_points = get_value(line,file.line_number);
                  }
                  else if (line.find("HIT POINTS =")!=-1)
                  {
                     new_one->hit_points = get_value(line,file.line_number);
                     if (new_one->max_hit_points < new_one->hit_points)
                       new_one->max_hit_points=new_one->hit_points;
                  }
                  else if (line.find("REQ STRENGTH =")!=-1)
                  {
                     new_one->required_strength = get_value(line,file.line_number);
                  }
                  else if (line.find("MAGAZINE CAPACITY =")!=-1)
                  {
                     shooting->magazine_capacity = get_value(line,file.line_number);
                  }
                  else if (line.find("BATTERY CAPACITY =")!=-1)
                  {
					  battery->max_capacity = get_value(line,file.line_number);
                  }
                  else if (line.find("REGENERATION SPEED =")!=-1)
                  {
					  battery->regeneration_speed = get_value(line,file.line_number);
                  }
                  else if (line.find("FIRE MODE =")!=-1)
                  {
                     shooting->fire_type = get_fire_modes(line,file.line_number);
                  }
                  else if (line.find("SKILL TO USE =")!=-1)
                  {
                     new_one->skill_to_use = get_skill(line,file.line_number);
                  }
				  else if (line.find("INVISIBLE =")!=-1)
				  {
					  new_one->invisible = get_boolean(line,file.line_number);
				  }				  
                  else if (line.find("AVAILABLE FIRE MODES =")!=-1)
                  {
                     shooting->available_fire_types = get_fire_modes(line,file.line_number);
                  }
                  else if (line.find("MOVE SLOTS =")!=-1)
                  {
					  shell->max_number_move_slots = get_value(line,file.line_number);
                  }
                  else if (line.find("ACTION SLOTS =")!=-1)
                  {
					  shell->max_number_action_slots = get_value(line,file.line_number);
                  }
                  else if ((line.find("MOVE PWR =")!=-1) || (line.find("MOVE POWER =")!=-1))
                  {
					  leg->move_power = get_value(line,file.line_number);
                  }
                  else if (line.find("FREQ =")!=-1)
                  {
					  processor->frequency = get_value(line,file.line_number);
                  }				  
                  else if (line.find("QUALITY =")!=-1)
                  {
					  processor->quality = get_value(line,file.line_number);
                  }				  
                  else if (line.find("PROGRAM =")!=-1)
                  {
					  processor->program = get_from_quotations(real, file.line_number);;
                  }
                  else if (line.find("GROUP =")!=-1)
                  {
					  processor->group_affiliation = get_group(line,file.line_number);
                  }				  				  
                  else if (line.find("CATEGORY =")!=-1)
                  {
					  new_one->category = get_category(line,file.line_number);
					  objects_in_category[new_one->category].push_back(new_one->name);
                  }
                  else if (line.find("IN MAGAZINE =")!=-1)
                  {
					  shooting->category_of_magazine = get_category(line,file.line_number);
                  }
                  else if (line.find("PURPOSE =")!=-1)
                  {
					new_one->purpose = get_purpose(line,file.line_number);
                  }
                  else if (line.find("WEIGHT =")!=-1)
                  {
                     new_one->weight = get_value(line,file.line_number);
                  }
                  else if (line.find("ARM =")!=-1)
                  {
                     value = get_value(line,file.line_number);
					 if (new_one->ClassName == "NO_ARMOR")
					 {
                         skin->ARM = value;
					 }
					 else if (new_one->ClassName == "ARMOR")
					 {
                         armor->ARM = value;
					 }
					 else if (new_one->ClassName == "ROBOT_SHELL")
					 {
                         shell->ARM = value;
					 }
					 else
					 {
                       fprintf(stderr,"\nERROR: %s not for this item in #%d\n\n",line.c_str(),file.line_number);
					   keyboard.wait_for_key(); exit(13);
					 }
                  }
                  else if (line.find("ARM ENERGY =")!=-1)
                  {
					  new_one->energy_activated = 0;
					  new_one->activities+='a';
					  if (new_one->ClassName != "ARMOR")
					  {
						  fprintf(stderr,"\nERROR: %s not for this item in #%d\n\n",line.c_str(),file.line_number);
						  keyboard.wait_for_key(); exit(13);
					  }
						  
					  armor->energy_ARM = get_value(line,file.line_number);
                  }				  
                  else if (line.find("SPD ENERGY =")!=-1)
                  {
					  new_one->energy_activated = 0;
					  new_one->activities+='a';
					  
					  if (new_one->ClassName != "ARMOR")
					  {
						  fprintf(stderr,"\nERROR: %s not for this item in #%d\n\n",line.c_str(),file.line_number);
						  keyboard.wait_for_key(); exit(13);
					  }
					  
					  armor->energy_MOD_SPD = get_value(line,file.line_number);
                  }				  
                  else if (line.find("STR ENERGY =")!=-1)
                  {
					  new_one->energy_activated = 0;
					  new_one->activities+='a';
					  if (new_one->ClassName != "ARMOR")
					  {
						  fprintf(stderr,"\nERROR: %s not for this item in #%d\n\n",line.c_str(),file.line_number);
						  keyboard.wait_for_key(); exit(13);
					  }
					  
					  armor->energy_MOD_STR = get_value(line,file.line_number);
                  }				  
                  else if (line.find("DEX ENERGY =")!=-1)
                  {
					  new_one->energy_activated = 0;
					  new_one->activities+='a';
					  if (new_one->ClassName != "ARMOR")
					  {
						  fprintf(stderr,"\nERROR: %s not for this item in #%d\n\n",line.c_str(),file.line_number);
						  keyboard.wait_for_key(); exit(13);
					  }
					  
					  armor->energy_MOD_DEX = get_value(line,file.line_number);
                  }				  
                  else if (line.find("DMG ENERGY =")!=-1)
                  {
					  new_one->energy_activated = 0;
					  new_one->activities+='a';
					  if (new_one->ClassName != "HAND_WEAPON")
					  {
						  fprintf(stderr,"\nERROR: %s not for this item in #%d\n\n",line.c_str(),file.line_number);
						  keyboard.wait_for_key(); exit(13);
					  }
					  
					  handw->energy_DMG = get_value(line,file.line_number);
                  }				  				  
                  else if (line.find("HIT ENERGY =")!=-1)
                  {
					  new_one->energy_activated = 0;
					  new_one->activities+='a';
					  if (new_one->ClassName != "HAND_WEAPON")
					  {
						  fprintf(stderr,"\nERROR: %s not for this item in #%d\n\n",line.c_str(),file.line_number);
						  keyboard.wait_for_key(); exit(13);
					  }
					  
					  handw->energy_HIT = get_value(line,file.line_number);
                  }				  				  
                  else if (line.find("DEF ENERGY =")!=-1)
                  {
					  new_one->energy_activated = 0;
					  new_one->activities+='a';
					  if (new_one->ClassName != "HAND_WEAPON")
					  {
						  fprintf(stderr,"\nERROR: %s not for this item in #%d\n\n",line.c_str(),file.line_number);
						  keyboard.wait_for_key(); exit(13);
					  }
					  
					  handw->energy_DEF = get_value(line,file.line_number);
                  }				  				  
                  else if (line.find("MOD STR =")!=-1)
                  {
					  if (new_one->ClassName != "ARMOR")
					  {
						  fprintf(stderr,"\nERROR: %s not for this item in #%d\n\n",line.c_str(),file.line_number);
						  keyboard.wait_for_key(); exit(13);
					  }
					  armor->MOD_STR = get_value(line,file.line_number);
                  }
                  else if (line.find("MOD DEX =")!=-1)
                  {
					  if (new_one->ClassName != "ARMOR")
					  {
						  fprintf(stderr,"\nERROR: %s not for this item in #%d\n\n",line.c_str(),file.line_number);
						  keyboard.wait_for_key(); exit(13);
					  }
                      armor->MOD_DEX = get_value(line,file.line_number);
                  }
                  else if (line.find("MOD SPD =")!=-1)
                  {
					  if (new_one->ClassName != "ARMOR")
					  {
						  fprintf(stderr,"\nERROR: %s not for this item in #%d\n\n",line.c_str(),file.line_number);
						  keyboard.wait_for_key(); exit(13);
					  }
					  armor->MOD_SPD = get_value(line,file.line_number);
                  }
                  else if (line.find("RNG =")!=-1)
                  {
					  if (new_one->ClassName != "GRENADE")
					  {
						  fprintf(stderr,"\nERROR: %s not for this item in #%d\n\n",line.c_str(),file.line_number);
						  keyboard.wait_for_key(); exit(13);
					  }
					  grenade->RNG = get_value(line,file.line_number);
                  }
                  else if (line.find("PRICE =")!=-1)
                  {
                     new_one->price = get_value(line,file.line_number);
                  }
                  else if (line.find("ACC =")!=-1)
                  {
                     value=get_value(line,file.line_number);
					 if (new_one->ClassName == "AMMO")
					 {
                         ammo->ACC = value;
					 }
					 else if (new_one->ClassName == "RANGED_WEAPON")
					 {
                         shooting->ACC = value;
					 }
					 else
					 {
                       fprintf(stderr,"\nERROR: %s not for this item in #%d\n\n",real.c_str(),file.line_number);
					   keyboard.wait_for_key(); exit(13);
					 }
                  }
                  else if (line.find("DLY =")!=-1)
                  {
                         value=get_value(line,file.line_number);
                         grenade->DLY = value;
                  }
                  else if (line.find("PWR =")!=-1)
                  {
                     value=get_value(line,file.line_number);
					 if (new_one->ClassName == "AMMO")
					 {
                         ammo->PWR = value;
					 }
					 else if (new_one->ClassName == "RANGED_WEAPON")
					 {
                         shooting->PWR = value;
					 }
					 else if (new_one->ClassName == "GRENADE")
					 {
                         grenade->PWR = value;
					 }
					 else if (new_one->ClassName == "PILL")
					 {
                         pill->PWR = value;
					 }
					 else
					 {
                       fprintf(stderr,"\nERROR: %s not for this item in #%d\n\n",line.c_str(),file.line_number);
					   keyboard.wait_for_key(); exit(13);
					 }
                  }
                  else if (line.find("COLOR =")!=-1)
                  {
                     new_one->color = get_value(line,file.line_number);
                  }
                  else if (line.find("PROPERTIES =")!=-1)
                  {
                     value = get_properties(line);
					 if (value&TYPE_ENERGY && new_one->ClassName=="RANGED_WEAPON")
					 {
						    new_one->activities="gwrm"; // without Load uNload
					 }
					 new_one->properties = value;
					 if (new_one->ClassName == "ARMOR")
						 armor->energy_real_properties = value;
					 else if (new_one->ClassName == "HAND_WEAPON")
						 handw->energy_real_properties = value;					 
                  }
                  else if (line.find("PROPERTIES ENERGY =")!=-1)
                  {
					  value = get_properties(line);
					  if (new_one->ClassName == "ARMOR")
						armor->energy_properties = value;
					  else if (new_one->ClassName == "HAND_WEAPON")
						handw->energy_properties = value;
					  else
					  {
						  fprintf(stderr,"\nERROR: %s not for this item in #%d\n\n",line.c_str(),file.line_number);
						  keyboard.wait_for_key(); exit(13);
					  }					  
                  }
                  else if (line.find("DESCRIPTION =")!=-1)
                  {
					  line=get_from_quotations(real, file.line_number);
					  descriptions.add_description(new_one->name,line);
                  }				  
                  else if (line.find("ATTACK =")!=-1)
                  {
					  line=get_from_quotations(real, file.line_number);
					  descriptions.add_attack(new_one->name,line);
                  }				  
                  else if (line.find("TYPE =")!=-1)
                  {
                     line=get_from_quotations(real, file.line_number);
                     value=check_ammo_type(line);
                     if (value==-1)
                     {
                        fprintf(stderr,"Ammo type not defined %s in line #%d", line.c_str(),file.line_number);
                        keyboard.wait_for_key(); exit(31);
                     }
					 if (new_one->ClassName == "AMMO")
					 {
                         ammo->ammo_type = value;
					 }
					 else if (new_one->ClassName == "RANGED_WEAPON")
					 {
                         shooting->ammo_type = value;
					 }
					 else
					 {
                       fprintf(stderr,"\nERROR: %s not for this item in #%d\n\n",line.c_str(),file.line_number);
					   keyboard.wait_for_key(); exit(13);
					 }
                  }
                  else if (line.find(" =")!=-1)
                  {
                        fprintf(stderr,"Unknown assign %s in line #%d", line.c_str(),file.line_number);
                        keyboard.wait_for_key(); exit(27);
                  }
                  
  } // endof while
  file.close();
  printf("  -- Done. %d definitions loaded.\n",read);
  return true;  
}

bool DEFINITIONS :: read_monsters_definitions()
{
  CFILE file;
  string line, real;
  int value;
  int index;
  int read=0;

  ITEM *item;
  MONSTER *new_one;
   
   ANIMAL *zwierz;
   SEARCHLIGHT *searchlight;
   INTELLIGENT *intel;
   ROBOT *robot;
   MADDOCTOR *doctor;
   WORM *worm;

   printf("\nReading file data/monsters.def\n");
   if (!file.open("data/monsters.def"))
	   file.error_open();   

  line=string("something");
  while (line!="")
  {
    real=file.get_line();
    line=string(real) + "";
	transform(line.begin(), line.end(), line.begin(), toupper);
    // szukamy of character [
    if (*line.c_str()=='#' || *line.c_str()=='\n') // komentarz
      continue;
    
    if (line.find('[')!=-1) // new_one monster
    {
	  read++;
      if (line.find("[ANIMAL]")!=-1)
      {
        zwierz=new ANIMAL;
        new_one=(MONSTER *) zwierz;
      }
      else if (line.find("[MADDOCTOR]")!=-1)
      {
		  doctor=new MADDOCTOR;
		  new_one=(MONSTER *) doctor;
		  item = find_item("Scorpion Tail");
		  if (item==NULL)
		  {
			  fprintf(stderr,"Not defined item Scorpion Tail dla Mad Doctor'a");
			  keyboard.wait_for_key(); exit(40);
		  }
		  doctor->tail=*(HAND_WEAPON *)item;
		  
      }
      else if (line.find("[SEARCHLIGHT]")!=-1)
      {
		  searchlight=new SEARCHLIGHT;
		  new_one=(MONSTER *) searchlight;
      }
	  else if (line.find("[WORM]")!=-1)
	  {
		  worm=new WORM;
		  new_one=(MONSTER *) worm;
	  }
      else if (line.find("[INTELLIGENT]")!=-1)
      {
        intel=new INTELLIGENT;
        new_one=(MONSTER *) intel;
        intel->backpack.clear();
      }
      else if (line.find("[ROBOT]")!=-1)
      {
		  robot=new ROBOT;
		  new_one=(MONSTER *) robot;
		  robot->shell=NULL;
      }	  
      else if (line.find("[COPYOF ")!=-1)
      {
        line=get_from_quotations(real, file.line_number);
        new_one = this->find_monster(line);
        if (new_one==NULL)
        {
           fprintf(stderr,"ERROR: No such monster %s.\n",line.c_str());
           keyboard.wait_for_key(); exit(200);
        }
        new_one = new_one->duplicate(); // create copy of found
        
        // we don't know the type, so assign all
        zwierz=(ANIMAL *)new_one;
        intel=(INTELLIGENT *)new_one;
        robot=(ROBOT *)new_one;
      }
      else
      {
        fprintf(stderr,"Not defined monster type %s in line #%d", line.c_str(),file.line_number);
        keyboard.wait_for_key(); exit(29);
      }
      monsters.push_back(new_one);
    }


    
                  else if (line.find("NAME =")!=-1)
                  {
                     line=get_from_quotations(real, file.line_number);
                     new_one->rename( line );
                  }
                  else if (line.find("DESCRIPTION =")!=-1)
                  {
					  line=get_from_quotations(real, file.line_number);
					  descriptions.add_description(new_one->name,line);
                  }				  
                  else if (line.find("CATEGORY =")!=-1)
                  {
					  new_one->category = get_category(line,file.line_number);
					  objects_in_category[new_one->category].push_back(new_one->name);
                  }				  
                  else if (line.find("GROUP =")!=-1)
                  {
					  new_one->group_affiliation = get_group(line,file.line_number);
                  }				  
                  else if (line.find("AI RESTRIRCTIONS =")!=-1)
                  {
					  ((INTELLIGENT *)new_one)->AI_restrictions = get_AI_restrictions(line,file.line_number);
                  }				  
                  else if (line.find("TILE =")!=-1)
                  {
                     line=get_from_quotations(real, file.line_number);
                     new_one->tile = *line.c_str();
                  }
                  else if (line.find("MAX HIT POINTS =")!=-1)
                  {
                     new_one->hit_points.val = get_value(line,file.line_number);
                  }
                  else if (line.find("HIT POINTS =")!=-1)
                  {
                     new_one->hit_points.val = get_value(line,file.line_number);
                     if (new_one->hit_points.max < new_one->hit_points.val)
                       new_one->hit_points.max = new_one->hit_points.val;
                  }
                  else if (line.find("WEIGHT =")!=-1)
                  {
                     new_one->weight = get_value(line,file.line_number);
                  }
                  else if (line.find("EXPERIENCE =")!=-1)
                  {
                     new_one->experience_for_kill = get_value(line,file.line_number);
                  }
                  else if (line.find("FEATURE =")!=-1)
                  {
					  new_one->special_properties = get_special_properties(line,file.line_number);
                  }
				  else if (line.find("INVISIBLE =")!=-1)
				  {
					  new_one->invisible = get_boolean(line,file.line_number);
				  }
                  else if (line.find("SIZE =")!=-1)
                  {
                     new_one->size = get_value(line,file.line_number);
                  }
                  else if (line.find("COLOR =")!=-1)
                  {
                     new_one->color = get_value(line,file.line_number);
                  }
                  else if (line.find("STRENGTH =")!=-1)
                  {
                     new_one->strength.val = get_value(line,file.line_number);
                     new_one->strength.max = new_one->strength.val;
                  }
                  else if (line.find("ENDURANCE =")!=-1)
                  {
                     new_one->endurance.val = get_value(line,file.line_number);
                     new_one->endurance.max = new_one->endurance.val;
                  }
                  else if (line.find("INTELLIGENCE =")!=-1)
                  {
                     new_one->intelligence.val = get_value(line,file.line_number);
                     new_one->intelligence.max = new_one->intelligence.val;
                  }
                  else if (line.find("DEXTERITY =")!=-1)
                  {
                     new_one->dexterity.val = get_value(line,file.line_number);
                     new_one->dexterity.max = new_one->dexterity.val;
                  }
                  else if (line.find("SPEED =")!=-1)
                  {
                     new_one->speed.val = get_value(line,file.line_number);
                     new_one->speed.max = new_one->speed.val;
                  }
                  else if (line.find("METABOLISM =")!=-1)
                  {
                     new_one->metabolism.val = get_value(line,file.line_number);
                     new_one->metabolism.max = new_one->metabolism.val;
                  }
				  else if (line.find("SOUND DYING =")!=-1)
				  {
					  new_one->sound_dying = get_from_quotations(line,file.line_number);
				  }
				  else if (line.find("SOUND PAIN =")!=-1)
				  {
					  new_one->sound_pain = get_from_quotations(line,file.line_number);
				  }
				  else if (line.find("SOUND PAIN =")!=-1)
				  {
					  new_one->sound_pain = get_from_quotations(line,file.line_number);
				  }
				  else if (line.find("CATEGORY ITEM =")!=-1)
				  {
				     line=get_from_quotations(real, file.line_number);
					 if (category_number.find(line) != category_number.end())
					 {
						lista_kategorii_losowych_itemow_potwora[new_one->name].push_back( category_number[line] );
					 }
					 else
					 {
						 printf("Unknown category in line %d:\n%s",file.line_number,real.c_str());
						 keyboard.wait_for_key(); exit(99);
					 }
				  }
				  else if (line.find("LENGTH =")!=-1)
				  {
					  worm->length_left = get_value(line,file.line_number);
				  }
				  else if (line.find("SEGMENT =")!=-1)
				  {
					  worm->segment_name = get_from_quotations(real, file.line_number);
				  }
				  else if (line.find("TAIL =")!=-1)
				  {
					  worm->tail_name = get_from_quotations(real, file.line_number);
				  }
					
                  else if (line.find("INVENTORY =")!=-1)
                  {
                     value=get_value(line,file.line_number);
                     if (value==-1)
                       value=1;
                     line=get_from_quotations(real, file.line_number);

                     item = find_item(line);
                     if (item==NULL)
                     {
                        fprintf(stderr,"Not defined item %s in line #%d", line.c_str(),file.line_number);
                        keyboard.wait_for_key(); exit(40);
                     }
					 
                     if (item->IsCountable())
                     {
                          item = level.create_item(-1, -1, line, 0, 0);
						  COUNTABLE *licz = item->IsCountable();
						  licz->quantity = value;						  
                          ((INTELLIGENT *)new_one)->pick_up_item(item,false);
						  level.all_items.pop_back(); // definitions are not on the level
                     }
                     else
                     {
                        for (index=0;index<value;index++)
                        {
					
                          item = level.create_item(-1, -1, line, 0, 0);
                          ((INTELLIGENT *)new_one)->pick_up_item(item,false);
						  level.all_items.pop_back(); // definitions are not on the level
                          
                          if (real.find("AS ARMOR")!=-1)
						  {
                            ((INTELLIGENT *)new_one)->set_armor(item);
						  }
                          else if (real.find("AS WEAPON")!=-1)
                            ((INTELLIGENT *)new_one)->weapon = item;
                          else if (real.find("AS READY")!=-1)
							  ((INTELLIGENT *)new_one)->ready_weapon = item;
                        }
                     }
                     
                  }
                  else if (line.find("UNARMED =")!=-1)
                  {
                     line=get_from_quotations(real, file.line_number);
                     item = find_item(line);
                     if (item==NULL)
                     {
                        fprintf(stderr,"Not defined item %s in line #%d", line.c_str(),file.line_number);
                        keyboard.wait_for_key(); exit(40);
                     }
                     new_one->unarmed=*(WEAPON_UNARMED *)item;
                  }
                  else if (line.find("SHELL =")!=-1)
                  {
					  line=get_from_quotations(real, file.line_number);
					  item = find_item(line);
					  if (item==NULL)
					  {
						  fprintf(stderr,"Not defined item %s in line #%d", line.c_str(),file.line_number);
						  keyboard.wait_for_key(); exit(40);
					  }
					  item = level.create_item(-1, -1, line, 0, 0);
					  robot->shell=(ROBOT_SHELL *) item;
					  level.all_items.pop_back(); // definitions are not on the level
                  }				  
                  else if (line.find("CPU =")!=-1)
                  {
					  line=get_from_quotations(real, file.line_number);
					  item = find_item(line);
					  if (item==NULL)
					  {
						  fprintf(stderr,"Not defined item %s in line #%d", line.c_str(),file.line_number);
						  keyboard.wait_for_key(); exit(40);
					  }
					  if (robot->shell==NULL)
					  {
						  fprintf(stderr,"ERROR - robot has no shell to install CPU %s in line #%d", line.c_str(),file.line_number);
						  keyboard.wait_for_key(); exit(41);
					  }
					  item = level.create_item(-1, -1, line, 0, 0);
					  robot->shell->install_CPU(item);
					  level.all_items.pop_back(); // definitions are not on the level							  
                  }	
                  else if (line.find("ACTION SLOT =")!=-1)
                  {
					  if (robot->shell==NULL)
					  {
						  fprintf(stderr,"ERROR - robot has no shell to install in ACTION SLOT %s in line #%d", line.c_str(),file.line_number);
						  keyboard.wait_for_key(); exit(41);
					  }
					  
					  value=get_value(line,file.line_number);
					  if (value==-1)
						  value=1;
					  line=get_from_quotations(real, file.line_number);
					  
					  item = find_item(line);
					  if (item==NULL)
					  {
						  fprintf(stderr,"Not defined item %s in line #%d", line.c_str(),file.line_number);
						  keyboard.wait_for_key(); exit(40);
					  }
					  
					  if (item->IsCountable())
					  {
                          item = level.create_item(-1, -1, line, 0, 0);
						  COUNTABLE *licz = item->IsCountable();
						  licz->quantity = value;
						  robot->shell->install_in_action_slot(item);
						  level.all_items.pop_back(); // definitions are not on the level
					  }
					  else
					  {
						  for (index=0;index<value;index++)
						  {
							  
							  item = level.create_item(-1, -1, line, 0, 0);
							  robot->shell->install_in_action_slot(item);
							  level.all_items.pop_back(); // definitions are not on the level							  
						  }
					  }					  
                  }	 // endof ACTION SLOT			  
                  else if (line.find("MOVE SLOT =")!=-1)
                  {
					  if (robot->shell==NULL)
					  {
						  fprintf(stderr,"ERROR - robot has no shell to install in MOVE SLOT %s in line #%d", line.c_str(),file.line_number);
						  keyboard.wait_for_key(); exit(41);
					  }
					  
					  value=get_value(line,file.line_number);
					  if (value==-1)
						  value=1;
					  line=get_from_quotations(real, file.line_number);
					  
					  item = find_item(line);
					  if (item==NULL)
					  {
						  fprintf(stderr,"Not defined item %s in line #%d", line.c_str(),file.line_number);
						  keyboard.wait_for_key(); exit(40);
					  }
					  
					  if (item->IsCountable())
					  {
                          item = level.create_item(-1, -1, line, 0, 0);
						  COUNTABLE *licz = item->IsCountable();
						  licz->quantity = value;
						  robot->shell->install_in_move_slot(item);
						  level.all_items.pop_back(); // definitions are not on the level
					  }
					  else
					  {
						  for (index=0;index<value;index++)
						  {
							  
							  item = level.create_item(-1, -1, line, 0, 0);
							  robot->shell->install_in_move_slot(item);
							  level.all_items.pop_back(); // definitions are not on the level							  
						  }
					  }					  
                  }	 // endof MOVE SLOT
                  else if (line.find("SKIN =")!=-1)
                  {
                     line=get_from_quotations(real, file.line_number);
                     item = find_item(line);
                     if (item==NULL)
                     {
                        fprintf(stderr,"Not defined item %s in line #%d", line.c_str(),file.line_number);
                        keyboard.wait_for_key(); exit(40);
                     }
                     new_one->no_armor=*(NO_ARMOR *)item;
                  }
                  else if (line.find("SKILL =")!=-1)
                  {
                     value = get_skill(line,file.line_number);
                     new_one->skill[value] = get_value(line,file.line_number);
                  }                  
                  else if (line.find("RESIST =")!=-1)
                  {
					  value = get_resist(line,file.line_number);
					  new_one->resist[value] = get_value(line,file.line_number);
                  }                                  
				  else if (line.find(" =")!=-1)
                  {
                        fprintf(stderr,"Unknown assign %s in line #%d", line.c_str(),file.line_number);
                        keyboard.wait_for_key(); exit(27);
                  }
                  
  } // endof while
  file.close();
  printf("  -- Done. %d definitions loaded.\n",read);  
  return true;
  
}

bool DEFINITIONS :: read_keyboard_mapping()
{
  CFILE file;
  string line,temp,real;
  int value;
  printf("\nReading file data/keyboard.def\n");  
  if (!file.open("data/keyboard.def"))
	  file.error_open();
  

  line="something";
  while (line!="")
  {
    real=file.get_line();
    line=string(real) + "";
	transform(line.begin(), line.end(), line.begin(), toupper);
    
      if (line.find("\"")!=-1)
      {
        temp=get_from_quotations(real, file.line_number);
        value=*temp.c_str();
      }
      else
        value=get_value(line,file.line_number);

             
      if (line.find(" SW =")!=-1)
        keyboard.sw=value;
      else if (line.find(" S =")!=-1)
        keyboard.s=value;
      else if (line.find(" SE =")!=-1)
        keyboard.se=value;
      else if (line.find(" W =")!=-1)
        keyboard.w=value;
      else if (line.find(" E =")!=-1)
        keyboard.e=value;
      else if (line.find(" NW =")!=-1)
        keyboard.nw=value;
      else if (line.find(" N =")!=-1)
        keyboard.n=value;
      else if (line.find(" NE =")!=-1)
        keyboard.ne=value;
      else if (line.find("GET =")!=-1)
        keyboard.get=value;
      else if (line.find("QUIT =")!=-1)
        keyboard.quit=value;
      else if (line.find("WAIT =")!=-1)
        keyboard.wait=value;
      else if (line.find("ESCAPE =")!=-1)
        keyboard.escape=value;
      else if (line.find("BACKSPACE =")!=-1)
        keyboard.backspace=value;
      else if (line.find("READ MORE =")!=-1)
        keyboard.readmore=value;
      else if (line.find("READ MORE 2 =")!=-1)
        keyboard.readmore2=value;
      else if (line.find("INC FIRE =")!=-1)
        keyboard.inc_fire=value;
      else if (line.find("DEC FIRE =")!=-1)
        keyboard.dec_fire=value;
      else if (line.find("INVENTORY =")!=-1)
        keyboard.inventory=value;
      else if (line.find("RELOAD =")!=-1)
        keyboard.reload=value;
      else if (line.find("OPTIONS =")!=-1)
        keyboard.options=value;
      else if (line.find("FIRE =")!=-1)
        keyboard.fire=value;
      else if (line.find("HELP =")!=-1)
        keyboard.help=value;
      else if (line.find("CHAR INFO =")!=-1)
        keyboard.char_info=value;
      else if (line.find("LOOK =")!=-1)
        keyboard.look=value;
      else if (line.find("OPEN =")!=-1)
        keyboard.open=value;
      else if (line.find("CLOSE =")!=-1)
        keyboard.close=value;
      else if (line.find("THROW =")!=-1)
        keyboard._throw=value;
      else if (line.find("SAVE =")!=-1)
        keyboard.save=value;
      else if (line.find("UP =")!=-1)
        keyboard.up=value;
      else if (line.find("DOWN =")!=-1)
        keyboard.down=value;
      else if (line.find("REST =")!=-1)
		  keyboard.rest=value;
	  else if (line.find("EXPLORE =")!=-1)
		  keyboard.explore=value;
	  else if (line.find("TRAVEL =")!=-1)
		  keyboard.travel=value;
      else if (line.find("AIM NEAR =")!=-1)
		  keyboard.nearest=value;
      else if (line.find("MESSAGE BUFFER =")!=-1)
		  keyboard.messagebuffer=value;	  
      else if (line.find("ATTACK =")!=-1)
		  keyboard.attack=value;
	  else if (line.find("EXCHANGE =")!=-1)
		  keyboard.exchange=value;
	  else if (line.find("ACTIVATE ARMOR =")!=-1)
		  keyboard.activate_armor=value;
	  else if (line.find("ACTIVATE WEAPON =")!=-1)
		  keyboard.activate_weapon=value;
	  else if (line.find("SHOW VISIBLE =")!=-1)
		  keyboard.show_visible=value;
	  else if (line.find(" =")!=-1)
      {
        fprintf(stderr,"\nUnknown key in line #: %d %s\n\n",file.line_number,line.c_str());
        keyboard.wait_for_key(); exit(37);
      }
              
  }
  file.close();
  printf("  -- Done. Keyboard mapping read.\n");    				
  return true;
}


bool DEFINITIONS :: read_terrains_definitions()
{
  CFILE file;
  string line, real;
  int read=0;

  CELL *new_one;
  
  printf("\nReading file data/terrains.def\n");  
  if (!file.open("data/terrains.def"))
	  file.error_open();
  
  line="something";
  while (line!="")
  {
    real=file.get_line();
    line=string(real) + "";
	transform(line.begin(), line.end(), line.begin(), toupper);
    // szukamy of character [
    if (*line.c_str()=='#' || *line.c_str()=='\n') // komentarz
      continue;
    
    if (line.find('[')!=-1) // new_one teren
    {
	  read++;
      if (line.find("[TERRAIN]")!=-1)
        new_one=new CELL;
      else
	  {
        fprintf(stderr,"Nieznane wyrazenie %s z '[' in line #%d", line.c_str(),file.line_number);
		keyboard.wait_for_key(); exit(126);
	  }
        
      terrains.push_back(new_one);
    }


    
                  else if (line.find("NAME =")!=-1)
                  {
                     line=get_from_quotations(real, file.line_number);
#ifdef DEBUG_PARSER                     
                     printf("wyciete: %s\n",line.c_str());
#endif                     
                     new_one->name= string("") + line;
                  }
                  else if (line.find("DESCRIPTION =")!=-1)
                  {
					  line=get_from_quotations(real, file.line_number);
					  descriptions.add_description(new_one->name,line);
                  }				  
                  else if (line.find("TILE =")!=-1)
                  {
                     line=get_from_quotations(real, file.line_number);
                     new_one->tile = *line.c_str();
                  }
                  else if (line.find("COLOR =")!=-1)
                  {
                     new_one->color = get_value(line,file.line_number);
                  }
                  else if (line.find("OPENED =")!=-1)
                  {
                     new_one->about_open = get_boolean(line,file.line_number);
                  }
                  else if (line.find("BLOCKLOS =")!=-1)
                  {
                     new_one->block_los = get_boolean(line,file.line_number);
                     new_one->real_block_los = new_one->block_los;
                  }
                  else if (line.find("BLOCKMOVE =")!=-1)
                  {
                     new_one->block_move = get_boolean(line,file.line_number);
                     new_one->real_block_move = new_one->block_move;
                  }
                  else if (line.find("HIT POINTS =")!=-1)
                  {
                     new_one->hit_points = get_value(line,file.line_number);
                     new_one->max_hit_points = new_one->hit_points;
                  }
                  else if (line.find("DESTROYED =")!=-1)
                  {
                     line=get_from_quotations(real, file.line_number);
                     new_one->destroyed_to= string("") + line;
                  }
                  else if (line.find(" =")!=-1)
                  {
                        fprintf(stderr,"Unknown assign %s in line #%d", line.c_str(),file.line_number);
                        keyboard.wait_for_key(); exit(27);
                  }
  } // endof while

  file.close();
  printf("  -- Done. %d definitions loaded.\n",read);  
  return true;
  
}


bool DEFINITIONS :: read_definition()
{
  if (read_keyboard_mapping()==false)
    return false;
  if (read_categories()==false)
	  return false;
  if (read_ammo_types()==false)
    return false;
  if (read_items_definitions()==false)
    return false;
  if (read_monsters_definitions()==false)
    return false;
  if (read_terrains_definitions()==false)
    return false;
  if (read_level_definitions()==false)
    return false;
  if (read_pills_types()==false)
	  return false;
  if (read_grenades_types()==false)
	  return false;
  if (create_map_of_game_unknown_items()==false)
	  return false;
  return true;
}

bool DEFINITIONS::read_level_definitions()
{
      CFILE file;
      string line, real;
	  vector < string > wejscia;
	  string enterance;
	  string music;
	  string ID;
	  LEVEL_DEFINITION dp;
	  int read=0;

	  printf("\nReading file data/levels.def\n");	  
	  if (!file.open("data/levels.def"))
		  file.error_open();	  

	  ID="START";
	  line="something";
	  while (line!="")
	  {
		  real=file.get_line();
		  line=string(real) + "";
		  transform(line.begin(), line.end(), line.begin(), toupper);
		  // szukamy of character [
		  if (*line.c_str()=='#' || *line.c_str()=='\n') // komentarz
			  continue;

		  if (line.find('[')!=-1) // new_one poziom
		  {
			  if (line.find("[LEVEL]")!=-1 || line.find("[END OF LEVELS]")!=-1)
			  {
				  read++;
				  // dodanie wyjsc z tego poziomu przed stworzeniem nowego
				  if (ID!="START")
				  {
					  if (wejscia.size()>0)
					  {
						  enterance = wejscia[random(wejscia.size())];
						  level.stairs_down[enterance].push_back(ID);
						  level.stairs_up[ID].push_back(enterance);
					  }
					  level.levels[ID] = dp;
				  }
				  wejscia.clear();
				  dp.reset();
			  }
			  else
			  {
				  fprintf(stderr,"Unknown expression %s z '[' in line #%d", line.c_str(),file.line_number);
				  keyboard.wait_for_key(); exit(123);
			  }

		  }
		  else if (line.find("ID =")!=-1)
		  {
			  line=get_from_quotations(real, file.line_number);
			  ID= string("") + line;
		  }
		  else if (line.find("NAME =")!=-1)
		  {
			  line=get_from_quotations(real, file.line_number);
			  dp.name= string("") + line;
		  }
		  else if (line.find("TYPE =")!=-1)
		  {
			  line=get_from_quotations(real, file.line_number);
			  dp.type= string("") + line;
		  }
		  else if (line.find("FOV =")!=-1)
		  {
			  dp.fov_range = get_value(line,file.line_number);
		  }
		  else if (line.find("ENTERANCE =")!=-1)
		  {
			  line=get_from_quotations(real, file.line_number);
			  wejscia.push_back(line);
		  }
		  else if (line.find("NUMBER OF ITEMS =")!=-1)
		  {
			  dp.number_of_items = get_value(line,file.line_number);
		  }
		  else if (line.find("NUMBER OF ITEMS DEVIATION =")!=-1)
		  {
			  dp.number_of_items_deviation = get_value(line,file.line_number);
		  }
		  else if (line.find("NUMBER OF MONSTERS =")!=-1)
		  {
			  dp.number_of_monsters = get_value(line,file.line_number);
		  }
		  else if (line.find("ITEMS PARAMETER 1 =")!=-1)
		  {
			  dp.items_param1 = get_value(line,file.line_number);
		  }
		  else if (line.find("ITEMS PARAMETER 2 =")!=-1)
		  {
			  dp.items_param2 = get_value(line,file.line_number);
		  }
		  else if (line.find("NUMBER OF MONSTERS DEVIATION =")!=-1)
		  {
			  dp.number_of_monsters_deviation = get_value(line,file.line_number);
		  }
		  else if (line.find("MAXIMUM MONSTERS =")!=-1)
		  {
			  dp.max_number_of_monsters = get_value(line,file.line_number);
		  }
		  else if (line.find("ITEMS CONCENTRATION =")!=-1)
		  {
			  dp.items_concentration = get_value(line,file.line_number);
		  }
		  else if (line.find("MUSIC =")!=-1)
		  {
			  dp.music=get_from_quotations(real, file.line_number);
		  }
		  else if (line.find("MONSTER EVERY N TURNS")!=-1)
		  {
			  dp.monster_every_n_turns = get_value(line,file.line_number);
		  }					  
		  else if (line.find("PROBABILITY OF ITEM:")!=-1)
		  {
			  int kat = get_category(line,file.line_number);
			  int wart_kat = get_value(line,file.line_number);
			  // w definicji poziomu - suma wystepowania to suma wszystkich wystepowan
			  // dp.item_probability[category] ma value:
			  /*
			  przyklad:
			  [level]
			  prawd przed: ammo_1 = 4
			  prawd przed: weap_1 = 2
			  prawd przed: ammo_2 = 5
			  prawd przed: body_1 = 1

			  dp.wystepowanie to bedzie
			  ammo_1 = 4
			  weap_1 = 6
			  ammo_2 = 11
			  body_1 = 12

			  suma_wystepowania = 12

			  Przy tworzeniu itemu losujemy a=random(suma_wystepowania) i szukamy najmniejszego item_probability[kat]>a						  

			  */
			  dp.sum_of_items_probabilities += wart_kat;
			  dp.item_probability[kat] = dp.sum_of_items_probabilities;
		  }
		  else if (line.find("PROBABILITY OF MONSTER:")!=-1)
		  {
			  int kat = get_category(line,file.line_number);
			  int wart_kat = get_value(line,file.line_number);

			  dp.sum_of_monster_probabilities += wart_kat;
			  dp.monster_probability[kat] = dp.sum_of_monster_probabilities;
		  }
		  else if (line.find(" =")!=-1)
		  {
			  fprintf(stderr,"Unknown assign %s in line #%d", line.c_str(),file.line_number);
			  keyboard.wait_for_key(); exit(27);
		  }
	  } // endof while

	file.close();
	printf("  -- Done. %d definitions loaded.\n",read);  
	return true;
}






















