#include "mem_check.h"
#include <assert.h>

// wylaczenie ostrzezenia o inicjalizacji this
#pragma warning (disable: 4355)

//#define DEBUG_AI

#include "monster.h"
#include "map.h"
#include "keyboard.h"
#include "actions.h"
#include "system.h"
#include "global.h"
#include "level.h"
#include "directions.h"
#include "options.h"
#include "attrib.h"
#include "parser.h"
#include <math.h>
#include "sounds.h"
#include <assert.h>
#include <functional>
#include <algorithm>
#include <stdlib.h> // dla exit(0)

#include <list>
using namespace std;

// from global.cpp
extern KEYBOARD keyboard;
extern DEFINITIONS definitions;
extern LEVEL level;
extern MYSCREEN screen;
extern OPTIONS options;
extern DESCRIPTIONS descriptions;
extern SOUNDS sounds;

int ATTRIBUTE::GetValue()
{ 
	int mod_arm=0, mod_weight=0;
	if (owner!=NULL)
	{
		if (owner->in_armor())
		{
			switch(type) {
			case ATTRIB_TYPE_STR:
				mod_arm = owner->armor->MOD_STR;
				break;
			case ATTRIB_TYPE_DEX:
				mod_arm = owner->armor->MOD_DEX;
				break;
			case ATTRIB_TYPE_SPD:
				mod_arm = owner->armor->MOD_SPD;
				break;
			}
		}
	}

	int value = val + mod + mod_arm - mod_weight;

	if (owner!=NULL && type == ATTRIB_TYPE_SPD)
	{
		INTELLIGENT *intel = owner->IsIntelligent();
		if (intel!=NULL)
		{
			if (intel->isStrained())
				value/=2;
			else if (intel->isBurdened())
				value=(value*2)/3;
		}
	}
	
	if (value<=0 && !(type==ATTRIB_TYPE_HP || type==ATTRIB_TYPE_EP))
		return 1;

	return value;
};

bool INTELLIGENT::isStrained()
{
	int total_capacity = get_monster_capacity();
	if (get_backpack_weight() > (total_capacity - total_capacity/4))
		return true;
	return false;
}

bool INTELLIGENT::isBurdened()
{
	if ( get_backpack_weight() > get_monster_capacity()/2)
		return true;
	return false;
}


MONSTER::MONSTER()
: fov_radius(this,ATTRIB_TYPE_FOV), hit_points(this,ATTRIB_TYPE_HP), energy_points(this,ATTRIB_TYPE_EP),
  strength(this,ATTRIB_TYPE_STR), dexterity(this,ATTRIB_TYPE_DEX), endurance(this,ATTRIB_TYPE_END),
  intelligence(this,ATTRIB_TYPE_INT), speed(this,ATTRIB_TYPE_SPD), metabolism(this,ATTRIB_TYPE_MET)
{
	ClassName = "MONSTER";

   category = 0;

   is_dead=false;

   direction = 0;
   special_properties = SPECIAL_PROPERTY_NONE;

   weapon=&unarmed;
   armor=&no_armor;
   no_armor.owner=this;

   fov_radius.val = 12;
   fov_radius.max = 12;

   strength.val = 1;
   strength.max = 1;
   dexterity.val = 1;
   dexterity.max = 1;
   endurance.val = 1;
   endurance.max = 1;
   intelligence.val = 1;
   intelligence.max = 1;
   speed.val = 1;
   speed.max = 1;
   metabolism.val = 1;
   metabolism.max = 1;
   self_treatment = 0;

   group_affiliation = GROUP_ENEMIES;   

   next_action_time = 0;
   
   unarmed.tile='*';
   unarmed.color=6;
   unarmed.skill_to_use = SKILL_UNARMED;
   
   seen_now=false;
   seen_last_time_in_turn=0;
   experience_for_kill=0;
   
   hit_points.val=1;
   hit_points.max=1;

   ChangePosition(-1,-1);
   
   last_pX=-1;
   last_pY=-1;

   int a;
   for (a=0;a<NUMBER_OF_SKILLS;a++)
     skill[a]=20;
   for (a=0;a<NUMBER_OF_RESISTS;a++)
     resist[a]=0;
   for (a=0;a<NUMBER_OF_STATES;a++)
     state[a]=0;
}

MONSTER::~MONSTER()
{
	level.map.UnMarkMonsterOnMap(pX(),pY());
}

ENEMY *MONSTER::IsHostile()
{
	return dynamic_cast<ENEMY *>(this);
}

INTELLIGENT *MONSTER::IsIntelligent()
{
	return dynamic_cast<INTELLIGENT *>(this);
}

ANIMAL * MONSTER::IsAnimal()
{
	return dynamic_cast<ANIMAL *>(this);
};

MADDOCTOR * MONSTER::IsMadDoctor()
{
	return dynamic_cast<MADDOCTOR *>(this);
};

ROBOT * MONSTER::IsRobot()
{
	return dynamic_cast<ROBOT *>(this);
};

HERO * MONSTER::IsPlayer()
{
	return dynamic_cast<HERO *>(this);
};

SEARCHLIGHT * MONSTER::IsSearchlight()
{
	return dynamic_cast<SEARCHLIGHT *>(this);
}

WORM * MONSTER::IsWorm()
{
	return dynamic_cast<WORM *>(this);
};


bool MONSTER::is_enemy(MONSTER *m)
{
	if (group_affiliation == GROUP_NEUTRAL)
		return false;
	if (group_affiliation == m->group_affiliation)
		return false;
	if (m->group_affiliation == GROUP_NEUTRAL)
		return false;

	return true;
}

bool MONSTER::ChangePosition(int x, int y)
{
	if (x==pX() && y==pY()) // nie trzeba zmienic
		return false;

	level.map.UnMarkMonsterOnMap(pX(),pY());
	level.map.MarkMonsterOnMap(x,y);
	TILE::ChangePosition(x,y);
	return true;
}

bool MONSTER::is_friendly(MONSTER *m)
{
	return (group_affiliation == m->group_affiliation);
}

int MONSTER::mutate(int type)
{
	if (this->seen_now)
	{
		level.player->stop_repeating();				  							
		screen.console.add(name + "은 변이를 일으킨다!",14);
	}
	
	// simple mutate
	int random_value = random(8);
	int positive;

	switch(type) {
	case MUTATION_POSITIVE:
		positive = lower_random(75,100);
		break;
	case MUTATION_NEUTRAL:
		positive = lower_random(50,100);
		break;
	case MUTATION_NEGATIVE:
		positive = lower_random(25,100);
		break;
	}

	int value;
	if (positive)
		value=1;
	else
		value=-1;

	switch(random_value) {
	case 0:
		hit_points.max+=(random(3)+1)*value;
		break;
	case 1:			
		strength.val+=value;
		strength.max+=value;
		break;
	case 2:
		dexterity.val+=value;
		dexterity.max+=value;
		break;
	case 3:
		endurance.val+=value;
		endurance.max+=value;
		break;
	case 4:
		intelligence.val+=value;
		intelligence.max+=value;
		break;
	case 5:
		speed.val+=value*3;
		speed.max+=value*3;
		break;
	case 6:
		speed.val+=value*3;
		speed.max+=value*3;
		break;
	case 7:
		metabolism.val+=value*3;
		metabolism.max+=value*3;
		break;
	}

	return true;
}

bool MONSTER::heal(int damage)
{
  bool healed=false;
  bool healed_player=false;

  if (damage<=0)
	  return false;

  if (hit_points.val != hit_points.max)
  {
	healed=true;

	if (this==level.player)
		healed_player=true;
  }
  
  hit_points.val+=damage;
  if (hit_points.val >= hit_points.max)
  {
    hit_points.val = hit_points.max;  
	if (healed_player && level.player->is_repeating_action()==REPEAT_ACTION_REST)
	{
		level.player->stop_repeating();
	}
  }
  return healed;
}

void MONSTER::self_treatment_every_turn()
{
   self_treatment += metabolism.GetValue();
   if (self_treatment >= REACHED_TREATMENT)
   {
     heal(1);
     self_treatment=0;
   }
}

void MONSTER::status_management_every_turn()
{
	int endurance_val;
	int value;

	endurance_val = endurance.GetValue();
	// trucizny

	if (state[STATE_RADIOACTIVE]>0)
	{
		if (lower_random(endurance_val,500))
			state[STATE_RADIOACTIVE]--;

		if (state[STATE_RADIOACTIVE]<0)
			state[STATE_RADIOACTIVE]=0;
		else if ( lower_random( max (state[STATE_RADIOACTIVE],100),40000) ) // 0,2% szans w turze, ale dla duzej mutacji
		{
			if (state[STATE_RADIOACTIVE]>5)
				this->mutate(MUTATION_NEGATIVE);
		}
	}

	if (state[STATE_CHEM_POISON]>0)
	{
		if (lower_random(endurance_val,200))
			state[STATE_CHEM_POISON]--;
		if (state[STATE_CHEM_POISON]<0)
			state[STATE_CHEM_POISON]=0;

		else if (lower_random(state[STATE_CHEM_POISON]*2,150))
		{
			value=random(state[STATE_CHEM_POISON])/5+1;
			if (value>0)
			{
				if (this->seen_now==true)
				{
					level.player->stop_repeating();				  					
					screen.console.add(name + "은 독에 시달린다.",2);
					if (cause_damage(value)==DAMAGE_DEATH)
						screen.console.add(name + "독이 퍼져 죽는다.",4);
				}
			}
		}
	}
	if (state[STATE_CHEM_POISON]<0)
		state[STATE_CHEM_POISON]=0;

	// temperature

	if (state[STATE_TEMPERATURE]>0)
	{
		if (lower_random(endurance_val,50))
			state[STATE_TEMPERATURE]--;

		if (state[STATE_TEMPERATURE]<0)
			state[STATE_TEMPERATURE]=0;

		if (lower_random(state[STATE_TEMPERATURE]*3,100))
		{
			value=random(state[STATE_TEMPERATURE])/2;
			if (value>0 && this->seen_now==true)
			{
				level.player->stop_repeating();				  					
				screen.console.add(name + "은 불타고 있다.",12);
				if (cause_damage(value)==DAMAGE_DEATH)
					screen.console.add(name + "은 불에 타서 죽는다.",4);
			}
		}
	}

	if (state[STATE_TEMPERATURE]<0)
	{
		if (lower_random(endurance_val,50))
			state[STATE_TEMPERATURE]++;

		if (state[STATE_TEMPERATURE]>0)
			state[STATE_TEMPERATURE]=0;

		value=random(abs(state[STATE_TEMPERATURE]))/2;
		if (value>0 && this->seen_now==true)
		{
			level.player->stop_repeating();				  					
			screen.console.add(name + "은 추위에 떨고 있다.",15);
			if (cause_damage(value)==DAMAGE_DEATH)
				screen.console.add(name + "은 얼어 죽는다.",4);
		}
	}

	// speed

	if (state[STATE_SPEED]<0)
	{
		if (lower_random(endurance_val,50))
			state[STATE_SPEED]++;
		
		if (state[STATE_SPEED]>0)
			state[STATE_SPEED]=0;
	}
	else if (state[STATE_SPEED]>0)
	{
		state[STATE_SPEED]--;
		
		if (lower_random(endurance_val,50))
			state[STATE_SPEED]--;
		
		if (state[STATE_SPEED]<0)
			state[STATE_SPEED]=0;
	}


	// power
	
	if (state[STATE_STRENGTH]<0)
	{
		if (random(5)==0)
			state[STATE_STRENGTH]++;
		
		if (lower_random(endurance_val,150))
			state[STATE_STRENGTH]++;
		
		if (state[STATE_STRENGTH]>0)
			state[STATE_STRENGTH]=0;
	}
	else if (state[STATE_STRENGTH]>0)
	{
		if (random(5)==0)
			state[STATE_STRENGTH]--;

		if (lower_random(endurance_val,150))
			state[STATE_STRENGTH]--;
		
		if (state[STATE_STRENGTH]<0)
			state[STATE_STRENGTH]=0;
	}
	
	if (endurance_val!=0)
	{
		// change speed modifier by state[STATE_SPEED]
		speed.mod = (2*state[STATE_SPEED])/endurance_val;
		if (speed.val + speed.mod < 1)
			speed.mod = 1 - speed.val;
	}
	// change strength modifier by state[STATE_STRENGTH]
	strength.mod = (state[STATE_STRENGTH])/30;
	if (strength.val + strength.mod < 1)
		strength.mod = 1 - strength.val;
	
	// stun

	if (state[STATE_STUN]>0)
		state[STATE_STUN]--;
}

void MONSTER::every_turn()
{
  // heal a bit
	self_treatment_every_turn();
	status_management_every_turn();	
	// check energy
	if (energy_points.val > energy_points.max)
		energy_points.val = energy_points.max;
}

void MONSTER::wait (TIME t)
{
  next_action_time=level.turn+t;
}

void MONSTER::cell_in_current_direction(int &x, int &y)
{
  switch (direction)
  {
      case _kn:
           x=pX();
           y=pY()-1;
      break;
      case _kne:
           x=pX()+1;
           y=pY()-1;
      break;
      case _ke:
           x=pX()+1;
           y=pY();
      break;
      case _kse:
           x=pX()+1;
           y=pY()+1;
      break;
      case _ks:
           x=pX();
           y=pY()+1;
      break;
      case _ksw:
           x=pX()-1;
           y=pY()+1;
      break;
      case _kw:
           x=pX()-1;
           y=pY();
      break;
      case _knw:
           x=pX()-1;
           y=pY()-1;
      break;
  }
}

void MONSTER::direction_to_near_cell(int x, int y) 
{
      if (x>pX() && y==pY())
        direction=_ke;
      else if (x<pX() && y==pY())
        direction=_kw;
      else if (x==pX() && y<pY())
        direction=_kn;
      else if (x==pX() && y>pY())
        direction=_ks;
      // skosy
      else if (x>pX() && y>pY())
        direction=_kse;
      else if (x<pX() && y<pY())
        direction=_knw;
      else if (x>pX() && y<pY())
        direction=_kne;
      else if (x<pX() && y>pY())
        direction=_ksw;
      else if (x==pX() && y==pY())
		  direction=0;
}



int MONSTER::cause_damage(int damage)
{
   string text;

   if (damage<=0)
   {
      if (this->seen_now==true)
      {
        text=name +"은 상처를 입지 않는다.";
        screen.console.add(text,7);
      }
      return DAMAGE_NONE;
   }
   hit_points.val-=damage;

   if (hit_points.GetValue()<1)
   {
     if (death()==true)
		return DAMAGE_DEATH;
   }
   else
   {
	   if (this->seen_now)
	   {
		   MONSTER *sound=(MONSTER *) definitions.find_monster(this->name);
		   if (sound!=NULL)
			   sounds.PlaySoundOnce(sound->sound_pain);		
	   }
   }
   return damage;
}


bool MONSTER::is_unarmed()
{
   if (weapon==&unarmed)
     return true;
   return false;
}

bool MONSTER::in_armor()
{
   if (armor==&no_armor || armor==NULL)
     return false;
   return true;
}

int MONSTER::get_h2h_attack_value()
{
	int attack;
	if (weapon==NULL)
		return 0;

	weapon->uses_energy();
	
	attack=dexterity.GetValue();
	attack+=weapon->HIT;
	 // according to skill
	 attack*=skill[ weapon->skill_to_use ];

	return attack;
}

int MONSTER::get_h2h_defense_value()
{
	int defense;
	if (weapon==NULL)
		return 0;	
	weapon->uses_energy();	
	
	defense=dexterity.GetValue();
	defense+=weapon->DEF;
	// uwzglednienie skilla
	defense*=skill[ weapon->skill_to_use ];

	return defense;
}

void MONSTER::attack(MONSTER *a_enemy)
{
   int attack, defense;
   ITEM *weapon;
   ITEM *enemy_weapon;
   string text;
   
   assert(a_enemy!=NULL);

   weapon=this->weapon;
   enemy_weapon=a_enemy->weapon;

   descriptions.zero_attack("HIT");
   descriptions.zero_attack("ROBOTSAYS");

   this->enemy = a_enemy;

   // chance, that attacked enemy will change his target to attacker
	if (a_enemy!=level.player && a_enemy->enemy != this)
	{
		if (a_enemy->is_enemy(this) && coin_toss())
		{
			a_enemy->enemy = this;
		}
	}


   if (!IsStunned())
   {
	  attack = get_h2h_attack_value();
   }
   else
		attack=0;

   if (!a_enemy->IsStunned())
   {	   
	   defense = a_enemy->get_h2h_defense_value();
   }
   else
	    defense=0;

   if (lower_random(defense,defense+attack*3))
   {
     if (this->seen_now==true) // evaded
     {
		 if (a_enemy->seen_now)
		 {
			text=name + "은 " + a_enemy->name + "을 공격하지만 맞추지 못한다.";
			sounds.PlaySoundOnce("data/sounds/other/miss.wav");		
		 }
		 else
			text=name + "은 무언가를 공격하지만 맞추지 못한다.";
        screen.console.add(text,7);
     }
   }
   else // hit
   {
	 int skill;
	 if (weapon == &unarmed)
		 skill = SKILL_UNARMED;
	 else
		 skill = SKILL_MELEE_WEAPONS;

	 int with_damage = weapon->used_by_on(this,a_enemy,skill);		 
	 
     if (with_damage!=0)
	 {
		 if (with_damage!=-1)
			descriptions.add_attack("HIT", " " + IntToStr(with_damage) + "만큼의 피해를 입는다");
		 else
			descriptions.add_attack("HIT","살해당한다");
		 
		 if (this==level.player)
		 {
			 if (is_unarmed())
				level.player->skill_used(skill);
			 else
				level.player->skill_used(skill);
		 }
	 }
	 else // no damage
	 {
		 string text = descriptions.get_attack("HIT");
		 if (text.find("피해를 입는다")==-1)			 
			descriptions.add_attack("HIT","아무 피해를 입지 않는다");
	 }
	 if (this->seen_now==true)
	 {
		 // so important, that we stop repeating
		 level.player->stop_repeating();

		 string t = descriptions.get_attack(weapon->name);
		 if (t=="")
			 t = "공격한다.";

		 if (text.size()>0 && text[text.size()-1]==',')
			 text = text.substr(0,text.size()-1);		 

		 string something = "무언가";
		 if (a_enemy->seen_now)
			 something=a_enemy->name;

		 text=name + "은 " + something + "을 " + t + " " + something + "은 " + descriptions.get_attack("HIT") + ".";
		 if (text.find("죽인다")!=-1)
			screen.console.add(text,4);
		 else
			screen.console.add(text,6);

		 text=descriptions.get_attack("ROBOTSAYS");
		 if (text.size()!=0)
		 {
			 screen.console.add(text,15);
			 descriptions.zero_attack("ROBOTSAYS");
		 }
	 }	 
   }
}

int MONSTER::move()
{
      int x,y, x1, y1;
      MONSTER *monster;
	  
      cell_in_current_direction(x,y);

      if (!level.map.blockMove(x,y))
      {
         monster=level.monster_on_cell(x,y);
         if (monster==NULL) // ruch na to pole
         {
			 ChangePosition(x,y);
         }
         else
		 {
			 if (is_friendly(monster)==true) // wymiana miejscami
			 {
				if (monster!=level.player || (monster==level.player && options.number[OPTION_DONT_LET_SWAP]==false))
				{					
					if ((monster==level.player || this==level.player) || (coin_toss() && coin_toss())) // gdy player, to zawsze wymiana, gdy other, to 25% szans na to.
					{
						x = pX();	
						y = pY();
						x1 = monster->pX();
						y1 = monster->pY();
						
						ChangePosition(-1,-1); // aby nie wlaczyl sie asert (w MarkMonsterOnMap - proba wejscia na place, gdzie jest potworek)...
						monster->ChangePosition(-1,-1);  // ...nie robimy bezposredniej zamiany places potworkow
						
						ChangePosition(x1,y1);
						monster->ChangePosition(x,y);
						
						if (this->seen_now || monster->seen_now)
							screen.console.add(name + "와 " + monster->name + "은 서로 자리를 바꾼다.",7);
					}
					else
						return false;
				}
			 }
			 else // enemyi lub neutralny
			 {
				 attack(monster);
				 return 3;
			 }
		 }

         return 1; // byl ruch

      }
      else if (level.map.isOpenable(x,y))
      {
         return 2;
      }
      return false;
}

bool MONSTER::attack_in_direction()
{
	MONSTER *monster;
	int x,y;
	cell_in_current_direction(x,y);
	if (!level.map.onMap(x,y))
		return false;

	monster=level.monster_on_cell(x,y);
	// if is monster then attack monster
	if (monster!=NULL)	
	{
		attack(monster);				
		return true;
	}
	// if no monster, attack terrain
	int power=strength.GetValue() + weapon->DMG;
	weapon->uses_energy();
	power = random(power);
	level.map.damage(x,y,power);
	if (this->seen_now)
	{
		ITEM *sound=(ITEM *) definitions.find_item(weapon->name);
		if (sound!=NULL)
			sounds.PlaySoundOnce(sound->sound_used);

		if (power>0)
			screen.console.add(name + "은 " + level.map.getCellName(x,y) + "을 공격한다.",7);	
		else
			screen.console.add(name + "은 " + level.map.getCellName(x,y) + "을 공격하지만 아무 일도 일어나지 않는다.",7);	
	}
	
	return true;
}


ANIMAL::ANIMAL()
{
	ClassName = "ANIMAL";
}

WORM::WORM()
{
	ClassName = "WORM";
	prev=NULL;
	next=NULL;
	length_left=0;
	skill[SKILL_DODGING]=0;
}

MADDOCTOR::MADDOCTOR()
{
	ClassName = "MADDOCTOR";
	tile = 'D';

	// mad doctor likes quotes from old movies;)

	histext[0] = "날 잡을 수 있을거 같나! 쓰레기 자식!";
	histext[1] = "음화하하하하!";
	histext[2] = "받아랏!";
	histext[3] = "기분 좋으냐???";
	histext[4] = "죽어!";
	histext[5] = "윽... 죽을거 같아... 하하! 넌 날 죽일 수 없어!";
	histext[6] = "망할 외계인 놈들! 날 잡아갈 수 있을거 같아?";
	histext[7] = "내 새로운 모습이 어떤가? 하하!";
	histext[8] = "넌 이제 죽었다!";
	histext[9] = "죽일 테다!";
	histext[10] = "일단 널 죽인 다음, 요리해서 먹어 주마!";
	histext[11] = "넌 이미 죽어 있다!";
	histext[12] = "날개 달린 사탄의 새끼 같으니! 죽어라!";
	histext[13] = "여긴 내 영역이다. 이곳에선 나만이 죽음을 선고할 수 있다!";
	histext[14] = "네놈들은 죽지도 않냐?";
	histext[15] = "뭐? 그 소녀를 살리겠다? 나부터 죽여야 할 걸, 찰스! 하하!";
	histext[16] = "네 새끼들이 너무 멍청한 탓에 모두 죽어버렸다고 방위사령부가 네게 알려주지 않더냐!";
	histext[17] = "그래, 조만간 널 죽여 주마!";
	histext[18] = "실버맨, 넌 이미 죽었어! 이미 죽었어!";
	histext[19] = "놈들이 망할 벽을 뚫고 나오고 있어!";
	histext[20] = "넌 죽었다! 하하! 하하하!";
	histext[21] = "주인님! 이 자 또한 주인님의 노예가 될 것입니다!";
	histext[22] = "난 도망치지 않는다. 넌 날 죽여야만 할거다!";
	histext[23] = "몬도샤완족은 인간을 신뢰한 적이 결코 없다!";
	histext[24] = "너와 대니를 죽일 날만 꿈꿔 왔지. 하지만 그냥 죽일 수는 없다. 조각조각 썰어내 주마! 하하하!";	

	yelling_chance = 0;
}

void ENEMY::monster_sees_enemy(MONSTER *enemy)
{
		if (enemy!=NULL)
		{
         last_x_of_enemy=enemy->pX();
         last_y_of_enemy=enemy->pY();
         last_direction_of_enemy=enemy->direction;
         is_at_enemy_position=false;
		 enemy_last_seen_in_turn = level.turn;
		}
}


int cell_value(int *data, int x, int y)
{
  int w;
  if (x<0 || x>=MAPWIDTH || y<0 || y>=MAPHEIGHT)
    return -1;
  else
  {
    w=data[y*MAPWIDTH+x];
    return w;
  }
}

// flood fill - change to ASTAR?
int find_shortest_path(int *data, int *cost_of_travel, int x1, int y1, int x2, int y2, bool finish_at_first_found)
{
   int x,y;
   list <POSITION>::iterator m;
   list <POSITION> positions;
   int value;
   bool found=false;
   
   POSITION pos;
   // FIFO queue - flood fill

   // start from target

         pos.x=x2;
         pos.y=y2;
         data[y2*MAPWIDTH+x2]=0;
         positions.push_back(pos);
         do
         {
  		    if (positions.size()==0)
			 break;

			m=positions.begin();
            pos=*m;
         
            x=pos.x;
            y=pos.y;
            
            positions.pop_front();
            
            value=cell_value(data,x,y);

//            set_color(value);
//            print_character(x,y,'*'); // !!!
//            myrefresh();
            

            if ( cell_value(data,x+1,y) > value + cell_value(cost_of_travel,x+1,y) )
            {
//              print_text(40,42,"positions: " + IntToStr(pozycje.size()) + " x: " + IntToStr(x+1) + " y: " + IntToStr(y));
//              myrefresh();
            
              data[x+1+y*MAPWIDTH]=value + cell_value(cost_of_travel,x+1,y);
              pos.x=x+1;
              pos.y=y;
              positions.push_back(pos);
            }

            if ( cell_value(data,x-1,y) > value+cell_value(cost_of_travel,x-1,y) )
            {
//              print_text(40,42,"positions: " + IntToStr(pozycje.size()) + " x: " + IntToStr(x-1) + " y: " + IntToStr(y));
//              myrefresh();
            
              data[x-1+y*MAPWIDTH]=value+cell_value(cost_of_travel,x-1,y);
              pos.x=x-1;
              pos.y=y;
              positions.push_back(pos);
            }

            if ( cell_value(data,x,y+1) > value+cell_value(cost_of_travel,x,y+1) )
            {
//              print_text(40,42,"positions: " + IntToStr(pozycje.size()) + " x: " + IntToStr(x) + " y: " + IntToStr(y+1));
//              myrefresh();
              
              data[x+(y+1)*MAPWIDTH]=value+cell_value(cost_of_travel,x,y+1);
              pos.x=x;
              pos.y=y+1;
              positions.push_back(pos);
             }

            if ( cell_value(data,x,y-1) > value+cell_value(cost_of_travel,x,y-1) )
            {
//              print_text(40,42,"positions: " + IntToStr(pozycje.size()) + " x: " + IntToStr(x) + " y: " + IntToStr(y-1));
//              myrefresh();
              
              data[x+(y-1)*MAPWIDTH]=value+cell_value(cost_of_travel,x,y-1);
              pos.x=x;
              pos.y=y-1;
              positions.push_back(pos);
            }

            // skosy

            if ( cell_value(data,x+1,y+1) > value+cell_value(cost_of_travel,x+1,y+1) )
            {
              data[x+1+(y+1)*MAPWIDTH]=value+cell_value(cost_of_travel,x+1,y+1);
              pos.x=x+1;
              pos.y=y+1;
              positions.push_back(pos);
            }

            if ( cell_value(data,x-1,y+1) > value+cell_value(cost_of_travel,x-1,y+1) )
            {
              data[x-1+(y+1)*MAPWIDTH]=value+cell_value(cost_of_travel,x-1,y+1);
              pos.x=x-1;
              pos.y=y+1;
              positions.push_back(pos);
            }

            if ( cell_value(data,x-1,y-1) > value+cell_value(cost_of_travel,x-1,y-1) )
            {
              data[x-1+(y-1)*MAPWIDTH]=value+cell_value(cost_of_travel,x-1,y-1);
              pos.x=x-1;
              pos.y=y-1;
              positions.push_back(pos);
            }

            if ( cell_value(data,x+1,y-1) > value+cell_value(cost_of_travel,x+1,y-1) )
            {
              data[x+1+(y-1)*MAPWIDTH]=value+cell_value(cost_of_travel,x+1,y-1);
              pos.x=x+1;
              pos.y=y-1;
              positions.push_back(pos);
            }
            if (abs(x-x1)<2 && abs(y-y1)<2)
				found=true;
         } while (! (found && finish_at_first_found) );

		 if (found)
			 return value;
		 else 
			return 0;
}

// Set direction of movement to enemy
int ENEMY::set_direction_to_cell_by_shortest_path(int cell_x,int cell_y,bool open_doors)
{
     int cells,a;
     int data[MAPWIDTH*MAPHEIGHT+1];
     int cost_of_travel[MAPWIDTH*MAPHEIGHT+1];
     int x,y;
	 bool end_on_first = true; // when we don't evaluate the route
	 int travel_distance;

	 if (cell_x==-1 || cell_y==-1)
		 return 0;

	 if (cell_x == pX() && cell_y == pY())
		 return 0;

	 // if cell is near by
	 if (abs(pX()-cell_x)<2 && abs(pY()-cell_y)<2)
	 {
		direction_to_near_cell(cell_x,cell_y);
		return 1;
	 }
     
     bool way_blocked;
     struct POSITION position[100]; // so much for the map
     travel_distance=cells=generate_bresenham_line(this->pX(),this->pY(),cell_x,cell_y, (POSITION *) position, 100);

 // check, if the line can be passed?

     way_blocked=false;
     for (a=1;a<cells;a++)
     {
	   if (level.map.isOpenable(position[a].x,position[a].y) && open_doors)
		   continue;
       if (!level.entering_on_cell_possible(position[a].x,position[a].y) || how_danger_is_cell(position[a].x,position[a].y)>0)
       {
         way_blocked=true;
         break;
       }
     }

 // set direction to cell near by bresenham line
     if (way_blocked==false)
       direction_to_near_cell(position[1].x,position[1].y);
     else
     {
       for (x=0;x<MAPWIDTH;x++)
        for (y=0;y<MAPHEIGHT;y++)
        {
		  if (open_doors && level.map.isOpenable(x,y))
			  cost_of_travel[x+y*MAPWIDTH] = 2;
		  else
			  cost_of_travel[x+y*MAPWIDTH] = 1;

		  // if cell is dangerous, then entering costs more
		  int danger_value = how_danger_is_cell(x,y);
		  if (danger_value>0)
		  {
			  cost_of_travel[x+y*MAPWIDTH] += danger_value;
			  end_on_first=false;
		  }

		  if (level.map.seen(x,y) && level.map.IsMonsterOnMap(x,y))
			  data[x+y*MAPWIDTH]=-1;

		  if ( !level.map.blockMove(x,y) || (open_doors && level.map.isOpenable(x,y)) ) 
			  data[x+y*MAPWIDTH]=10000;
		  else
			  data[x+y*MAPWIDTH]=-1;

        }
     
  //       find shortest path
		travel_distance = find_shortest_path((int*)data,(int*)cost_of_travel,this->pX(),this->pY(),cell_x,cell_y,end_on_first);
		if (travel_distance==0)
			return travel_distance;

//////////////////////////////////////////////////////////////////////////
#ifdef DEBUG_AI
		for (x=0;x<MAPWIDTH;x++)
			for (y=0;y<MAPHEIGHT;y++)
			{
				print_character(x,y,cell_value(data,x,y)+'0');
			}
		myrefresh();
		keyboard.wait_for_key();
#endif
//////////////////////////////////////////////////////////////////////////

        cells=direction;
        int minimal_value;
        minimal_value=10000;
        for (a=0;a<8;++a)
        {
          direction=a;
          x=pX();
          y=pY();
          cell_in_current_direction(x,y);
          if (cell_value(data,x,y)<minimal_value && cell_value(data,x,y)!=-1)
          {
            cells=direction;
            minimal_value=cell_value(data,x,y);
          }
        }
        direction=cells;
        // check, if the same cost by the bresenham line
        if (cell_value(data,position[1].x,position[1].y)<=minimal_value && cell_value(data,position[1].x,position[1].y)!=-1)
            direction_to_near_cell(position[1].x,position[1].y);
        
     }
     
	return travel_distance;
}


ENEMY::ENEMY()
{
	ClassName = "ENEMY";

   last_x_of_enemy=-1;
   last_y_of_enemy=-1;

   camping=0;
   last_direction_of_enemy=0;
   is_at_enemy_position=true;
   enemy_last_seen_in_turn = 0;
   enemy = NULL;

   actual_behaviour = BEHAVIOUR_CALM;
   turned_to_fight=false;
}

bool ENEMY::go_around_cell(int x,int y, bool opening) // returns if opened door
{
         int random_value;
         MONSTER *monster;

         if (level.entering_on_cell_possible(x,y)==false)
         {
			// jezeli pole mozna otworzyc, to otwarcie
			 if (opening && level.map.isOpenable(x,y))
			 {
				 chosen_action = ACTION_OPEN;					 
				 return true;
			 }

            monster=level.monster_on_cell(x,y);
            if (monster==NULL || (monster!=NULL && is_enemy(monster)==false)) // atakujemy gracza
            {
               random_value=random(3)-1;
               if (random(5)==1)
                 random_value*=2;
               direction+=random_value;
               if (direction<1)
                 direction=8;
               else if (direction>8)
                 direction=1;
            }
         }
		 return false;
}

ACTION ANIMAL::get_action() // AI of animal
{
	  if (IsStunned())
		return ACTION_WAIT_STUNNED;

      int a,x,y;
	  
	  look_around();

	  // check, czy nie uciekac

	  if (have_low_hp())
	  {
		  if (seen_friends.size()==0 && !turned_to_fight)
		  {
			  if (enemy!=NULL && !enemy->have_low_hp() && random(5)==0)
				  actual_behaviour = BEHAVIOUR_RUN_AWAY;
		  }		  
		  if (have_critically_low_hp() && !turned_to_fight)
		  {
			  if (random(5)==0)
				  actual_behaviour = BEHAVIOUR_RUN_AWAY;
		  }
	  }
	  else
	  {
		  if (actual_behaviour == BEHAVIOUR_RUN_AWAY && random(3)==0)
		  {
			  actual_behaviour=BEHAVIOUR_SEARCH_FOR_ENEMY;
			  turned_to_fight=true;
		  }
	  }

	  int danger = how_danger_is_cell(pX(),pY());
	  int most_safe_direction=direction;
	  int n;
	  
	  if (danger>0) // something is wrong with the cell we stay at
	  {
		  direction = random(8);
		  for (a=1;a<=8;a++) // find place around where it's safe
		  {
			  direction++;
			  if (direction>8)
				  direction=1;
			  cell_in_current_direction(x,y);
			  n = how_danger_is_cell(x,y);
			  if (n==0)
			  {
				  most_safe_direction = direction;
				  break;
			  }
			  else if (n<danger)
			  {
				  danger = n;
				  most_safe_direction = direction;
			  }
			  
		  }
		  cell_in_current_direction(x,y);
		  go_around_cell(x,y,false);
		  return ACTION_MOVE;
	  }	

	  if (actual_behaviour!=BEHAVIOUR_RUN_AWAY && seen_enemies.size()>0)
		  actual_behaviour = BEHAVIOUR_H2H_COMBAT;

	  // when we see the enemy
	  if (actual_behaviour==BEHAVIOUR_H2H_COMBAT)
	  {
		  if (pX()==last_x_of_enemy && pY()==last_y_of_enemy)
		  {
			  camping=0;
			  actual_behaviour=BEHAVIOUR_SEARCH_FOR_ENEMY;
		  }
		  else
		  {
			  set_direction_to_cell_by_shortest_path(last_x_of_enemy,last_y_of_enemy,false);
			  cell_in_current_direction(x,y);
			  if (!(x==last_x_of_enemy && y==last_y_of_enemy))				  
				go_around_cell(x,y,false);
		  }		  
		  return ACTION_MOVE;		  
	  }
	  if (actual_behaviour==BEHAVIOUR_CALM)
	  {
		  if (random(30)==0)
		  {
			  actual_behaviour = BEHAVIOUR_SEARCH_FOR_ENEMY;
			  camping=0;
		  }

		  return ACTION_WAIT;		  
	  }
	  if (actual_behaviour==BEHAVIOUR_SEARCH_FOR_ENEMY)
	  {
		  if (random(30)==0)
		  {
			  actual_behaviour = BEHAVIOUR_CALM;
			  turned_to_fight=false;
		  }

		  int x,y;
		  if (last_pX==pX() && last_pY==pY()) // nie lubi stac w jednym miejscu szukajac of enemy
		  {
			  camping++;
			  if (camping>2)
			  {
				  for (int index=0;index<100;index++)
				  {
					  direction = random(8);
					  cell_in_current_direction(x,y);
					  
					  if (level.entering_on_cell_possible(x,y))
					  {
						  last_direction_of_enemy = direction;
						  break;
					  }
				  };				  
			  }
		  }
		  else
		  {
			  camping=0;		  		  
			  direction=last_direction_of_enemy;
			  cell_in_current_direction(x,y);
			  go_around_cell(x,y,false);
		  }
		  	
		  return ACTION_MOVE;		  
	  }
	  if (actual_behaviour==BEHAVIOUR_RUN_AWAY) // direction jest wyznaczany powyzej przy staniu na niebezpiecznym polu
	  {
		  return ACTION_MOVE;		  
	  }
	  
	  return ACTION_WAIT;
}

ACTION WORM::get_action() // AI of giant worm
{
	// If no other segments
	if (length_left==0 && next==NULL && prev==NULL)
	{
		death();
		return ACTION_WAIT;
	}

	look_around();

	int x,y;

	// when we see the enemy
	if (actual_behaviour==BEHAVIOUR_H2H_COMBAT)
	{
		if (pX()==last_x_of_enemy && pY()==last_y_of_enemy)
		{
			camping=0;
			actual_behaviour=BEHAVIOUR_SEARCH_FOR_ENEMY;
		}
		else
		{
			set_direction_to_cell_by_shortest_path(last_x_of_enemy,last_y_of_enemy,false);
			cell_in_current_direction(x,y);
			if (!(x==last_x_of_enemy && y==last_y_of_enemy))				  
				go_around_cell(x,y,false);
		}		  
		return ACTION_MOVE;		  
	}
	if (actual_behaviour==BEHAVIOUR_CALM)
	{
		if (random(30)==0)
		{
			actual_behaviour = BEHAVIOUR_SEARCH_FOR_ENEMY;
			camping=0;
		}
		return ACTION_WAIT;		  
	}
	if (actual_behaviour==BEHAVIOUR_SEARCH_FOR_ENEMY)
	{
		if (random(30)==0)
		{
			actual_behaviour = BEHAVIOUR_CALM;
			turned_to_fight=false;
		}

		int x,y;
		if (last_pX==pX() && last_pY==pY()) // doesn't like to stay in one place
		{
			camping++;
			if (camping>2)
			{
				for (int index=0;index<100;index++)
				{
					direction = random(8);
					cell_in_current_direction(x,y);

					if (level.entering_on_cell_possible(x,y))
					{
						last_direction_of_enemy = direction;
						break;
					}
				};				  
			}
		}
		else
		{
			camping=0;		  		  
			direction=last_direction_of_enemy;
			cell_in_current_direction(x,y);
			go_around_cell(x,y,false);
		}

		return ACTION_MOVE;		  
	}
	if (actual_behaviour==BEHAVIOUR_RUN_AWAY) // direction jest wyznaczany powyzej przy staniu na niebezpiecznym polu
	{
		return ACTION_MOVE;		  
	}

	return ACTION_WAIT;
}

int ANIMAL::cause_damage(int damage)
{
	if (actual_behaviour==BEHAVIOUR_RUN_AWAY)
	{
		if (coin_toss())
		{
			actual_behaviour=BEHAVIOUR_H2H_COMBAT;
			turned_to_fight=true;
		}
	}	
	return MONSTER::cause_damage(damage);
}

bool ANIMAL::look_around()
{
	ptr_list::iterator m,_m;
	MONSTER *monster;

	bool sees_enemy_again=false;
	int a,b;
	
	// look at the map

	seen_enemies.clear();
	seen_friends.clear();

	level.fov.Start(&level.map,pX(),pY(),fov_radius.GetValue());
	MONSTER::look_around(); // to handle cameras and searchlights
	
	// friends and enemies

	for(m=level.monsters.begin(),_m=level.monsters.end(); m!=_m; m++)
	{
		monster=(MONSTER *)*m;
		if (monster==this)
			continue;
		
		if (level.map.seen_by_camera(monster->pX(),monster->pY()) || (level.map.seen(monster->pX(),monster->pY()) && distance(monster->pX(),monster->pY(),pX(),pY())<fov_radius.GetValue()))
		{
				if (is_friendly(monster)) // friend
					seen_friends.push_back(monster);
				else if (is_enemy(monster)) // enemy
				{
					seen_enemies.push_back(monster);
					
					if (monster == enemy)
						sees_enemy_again = true;
					
					// add danger
					
					for (a=0;a<3;a++)
						for (b=0;b<3;b++)
							level.map.addDanger(monster->pX() + a-1,monster->pY() + b-1,DANGER_ENEMY);						
				} 
			// neutral are ignored
		}	
	}
	
	if (seen_enemies.size()==0)
		enemy = NULL;

	if (seen_enemies.size()>0 && 
	   (actual_behaviour==BEHAVIOUR_CALM || actual_behaviour==BEHAVIOUR_SEARCH_FOR_ENEMY))
		actual_behaviour = BEHAVIOUR_H2H_COMBAT;		
	
	if (!sees_enemy_again)
	{
		if (seen_enemies.size()>0)
		{
			actual_behaviour=BEHAVIOUR_H2H_COMBAT;
			enemy = (MONSTER *) *seen_enemies.begin();
			monster_sees_enemy(enemy);
		}
	}
	else
		monster_sees_enemy(enemy);

	return true;
}

bool WORM::look_around()
{
	ptr_list::iterator m,_m;
	MONSTER *monster;

	bool sees_enemy_again=false;

	// look at the map

	seen_enemies.clear();

	level.fov.Start(&level.map,pX(),pY(),fov_radius.GetValue());
	MONSTER::look_around(); // to handle cameras and searchlights

	// friends and enemies

	for(m=level.monsters.begin(),_m=level.monsters.end(); m!=_m; m++)
	{
		monster=(MONSTER *)*m;
		if (monster==this)
			continue;

		if (level.map.seen_by_camera(monster->pX(),monster->pY()) || (level.map.seen(monster->pX(),monster->pY()) && distance(monster->pX(),monster->pY(),pX(),pY())<fov_radius.GetValue()))
		{
			if (is_enemy(monster)) // enemy
			{
				seen_enemies.push_back(monster);

				if (monster == enemy)
					sees_enemy_again = true;
			} 
		}	
	}

	if (seen_enemies.size()==0)
		enemy = NULL;

	if (seen_enemies.size()>0 && 
		(actual_behaviour==BEHAVIOUR_CALM || actual_behaviour==BEHAVIOUR_SEARCH_FOR_ENEMY))
		actual_behaviour = BEHAVIOUR_H2H_COMBAT;		

	if (!sees_enemy_again)
	{
		if (seen_enemies.size()>0)
		{
			actual_behaviour=BEHAVIOUR_H2H_COMBAT;
			enemy = (MONSTER *) *seen_enemies.begin();
			monster_sees_enemy(enemy);
		}
	}
	else
		monster_sees_enemy(enemy);

	return true;
}

TIME ANIMAL::do_action( ACTION action )
{
   last_pX = pX();
   last_pY = pY();
	
   switch (action)
   {
      case ACTION_MOVE:
		  int x,y;
		  cell_in_current_direction(x,y);
		  if (level.map.blockMove(x,y))
			  go_around_cell(x,y,false);
		  
		  move();
		  return (TIME_MOVE/speed.GetValue())+1;
      case ACTION_WAIT_STUNNED:
			return state[STATE_STUN];
      case ACTION_WAIT:
			return (TIME_MOVE/speed.GetValue())+1;
   }
   return 0;
}

TIME WORM::do_action( ACTION action )
{
	last_pX = pX();
	last_pY = pY();

	switch (action)
	{
	case ACTION_MOVE:
		int x,y;
		cell_in_current_direction(x,y);
		if (level.map.blockMove(x,y))
			go_around_cell(x,y,false);

		move();
		return (TIME_MOVE/speed.GetValue())+1;
	case ACTION_WAIT:
		return (TIME_MOVE/speed.GetValue())+1;
	}
	return 0;
}

SEARCHLIGHT::SEARCHLIGHT()
{
	ClassName = "SEARCHLIGHT";
	
	int a;
	for (a=0;a<NUMBER_OF_SKILLS;a++)
		skill[a]=0;
	for (a=0;a<NUMBER_OF_RESISTS;a++)
		resist[a]=0;
	resist[RESIST_ELECTRICITY]=-30;	

	last_x_of_enemy=-1;
	last_y_of_enemy=-1;
	destination_x=-1;
	destination_y=-1;			
	speed.val=0;
	speed.max=0;
}

bool SEARCHLIGHT::death()
{	
	return MONSTER::death();
}


ACTION SEARCHLIGHT::get_action() // AI reflektora
{
	level.map.UnSeenMap();
	look_around();
    return ACTION_WAIT;
}


TIME SEARCHLIGHT::do_action( ACTION )
{
	if (enemy==NULL)
	{
		this->color=14;
		if (last_x_of_enemy==destination_x && last_y_of_enemy==destination_y)
		{
			if (random(3)==0)
			{				
				destination_x=-1;
				while (destination_x<0 || destination_x>MAPWIDTH || destination_y>30)
				{
					destination_x=random(40)-20+pX();
					destination_y=random(28)+pY()+5;			
				}
			}
		}
		else // ruch w nowe place
		{
			struct POSITION position[100]; // tyle sie miescie na ekranie po przekatnej	
			int cells;
			cells=generate_bresenham_line(last_x_of_enemy,last_y_of_enemy, destination_x, destination_y, (POSITION *) position, 100);
			last_x_of_enemy=position[1].x;
			last_y_of_enemy=position[1].y;					
		}
	}
	else
	{
		this->color=12;		
		last_x_of_enemy=enemy->pX();
		last_y_of_enemy=enemy->pY();

		if (last_y_of_enemy<pY()+4 || abs(last_x_of_enemy-pX())/abs(last_y_of_enemy-pY())>4)
		{
			this->color=14;
			enemy=NULL;
		}
	}		
	return 2;
}

bool SEARCHLIGHT::look_around()
{

	if (destination_x==-1) // tego sie bedzie mozna pozbyc, gdy all items beda mialy w pierwszej turze na poziomie ruch
	{
		while (destination_x<0 || destination_x>MAPWIDTH || destination_y>30)
		{
			destination_x=random(40)-20+pX();
			destination_y=random(28)+pY()+5;			
		}
		while (last_x_of_enemy<0 || last_x_of_enemy>MAPWIDTH || last_y_of_enemy>30)
		{
			last_x_of_enemy=random(40)-20+pX();
			last_y_of_enemy=random(28)+pY()+5;			
		}
	}
	

	int targetx, targety;
	//

	level.map.setSeenByCamera(pX(),pY()); // searchlight seen	
	
	struct POSITION position[100]; // tyle sie miescie na ekranie po przekatnej	
	int cells, index;
	for (int a=last_x_of_enemy-5;a<last_x_of_enemy+5;a++)
	{
		targetx=last_x_of_enemy+(a-pX())*2;
		targety=last_y_of_enemy+(last_y_of_enemy-pY())*2;
		
		cells=generate_bresenham_line(this->pX(),this->pY(),targetx,targety, (POSITION *) position, 100);
		for (index=1;index<cells;index++)
		{
			if (level.map.blockLOS(position[index].x,position[index].y))
				break;
			else
			{
				if (position[index].y>last_y_of_enemy-4 && position[index].y<last_y_of_enemy+4)
				{
					if (!( (a==last_x_of_enemy-5 || a==last_x_of_enemy+4) &&
						 (position[index].y==last_y_of_enemy-3 || position[index].y==last_y_of_enemy+3)
						))
					level.map.setSeenByCamera(position[index].x,position[index].y);
					if (enemy==NULL)
					{
						MONSTER *monster1=level.monster_on_cell(position[index].x,position[index].y);
						if (monster1!=NULL)
						{
							if (is_enemy(monster1))
							{
								// if it follows it
								ptr_list::iterator m,_m;
								MONSTER *temp;
								for (m=level.monsters.begin(),_m=level.monsters.end();m!=_m;m++)
								{
									temp = (MONSTER *) *m;
									if (temp->IsSearchlight())
									{
										if (temp->enemy==monster1)
											break;
									}
								}
								if (m==_m) // other searchlight follows it
								{
									// if not too far away and not too close
									if (last_y_of_enemy>=pY()+4 && abs(last_x_of_enemy-pX())/abs(last_y_of_enemy-pY())<=4)
									  enemy=monster1;
								}
							}
						}
					}
				}					
			}
		}
	}
	return true;
}


INTELLIGENT::INTELLIGENT()
{
	ClassName = "INTELLIGENT";

  misses_in_shooting = 0;

  items_in_backpack=0;
  max_items_in_backpack=40;

  AI_restrictions = AI_NOT_LIMITED;
  turns_of_searching_for_enemy = 0;
  turns_of_calm = 0;

  run_away_to_x = MAPWIDTH/2;
  run_away_to_y = MAPHEIGHT/2;

  ready_weapon = NULL;
}

void calculate_hit_place(int who_x, int who_y, int &target_x, int &target_y, double angle_miss)
{
   double enemyx, enemyy;
   double alfa;
   double distance, newx, newy;

   enemyx=double(target_x-who_x)*100;
   enemyy=double(target_y-who_y)*100;

   distance=sqrt(enemyx*enemyx+enemyy*enemyy);
   if (distance==0)
   {
      target_x = who_x;
      target_y = who_y;
      return;
   }
   alfa=atan(enemyx/enemyy);
   alfa=RadToDeg(alfa);
   
   if (enemyy<0)
     alfa=180+alfa;
   
   alfa+=angle_miss;
   newx=distance*sin(DegToRad(alfa));
   newy=distance*cos(DegToRad(alfa));

   target_x = who_x + round_up(newx); // miejsca trafienia na linii strzalu ale duzo dalej
   target_y = who_y + round_up(newy); // aby nawet przy strzelaniu na pole obok przy pocisku lecacyc daleko byla roznica
}

bool INTELLIGENT::throw_item_to (int x, int y, ITEM *item)
{
  int accuracy;
  int target_x, target_y;
  int start_x, start_y;
  int random_value;
  int max_range, range;
  double angle_miss;
  struct Screen_copy scr_copy;  
  int result;

     // print message
     if (this->seen_now==true)
     {
        screen.console.add(name + "은 " + item->show_name() + "을 던진다.",3);
     }

  store_screen(&scr_copy);
  
  if (!IsStunned() && !IsConfused())
  {
	accuracy=dexterity.GetValue()*10; 
    // uwzglednienie procentowego skilla
    accuracy*=skill[ SKILL_THROWING ];
    accuracy/=50;
  }
  else
	accuracy=0;

  if (accuracy==0)
	  accuracy++;
  
  max_range=strength.GetValue()*2 - item->weight/1000;
  if (max_range>80)
   max_range=80;


     random_value=random(300);
     angle_miss= ((double) random_value)/accuracy;
     
  
     if (coin_toss()) // angle can be below 0
      angle_miss*=-1;
        
     target_x=x;
     target_y=y;

	 start_x = pX();
	 start_y = pY();
	 
     calculate_hit_place(start_x,start_y,target_x,target_y, angle_miss);

	 // throw there

     struct POSITION position[100]; // struktura opisujaca wyliczony tor lotu
     int cells, index;
     MONSTER *monster;
     bool zablokowany;

     // count liczby cells to target
     cells=generate_bresenham_line(start_x,start_y, x, y, (POSITION *) position,100);
     // count straight do rzeczywistego celu
     generate_bresenham_line(start_x, start_y, target_x, target_y, (POSITION *) position,100);

// count rangeu 0-100 (/100 aby w procentach
     range=100-accuracy;
     if (range<0) range=0;
     range=random(range);
     if (coin_toss())
       range=100-range;
     else
       range=100+range;

     cells = (cells*range)/100; // procentowe chybienie zalezy tez od odleglosci
     if (cells>max_range)
       cells=max_range;

     zablokowany=false;
       
     for (index=1;index<=cells;index++)
     {
        if (level.entering_on_cell_possible(position[index].x,position[index].y)==false)
        {
          // check, czy na polu jest monster
          monster=level.monster_on_cell(position[index].x,position[index].y);

			 if (monster!=NULL && monster->evade_missile(this)==false) // nie uniknal
			 {
				   result = item->used_by_on(this,monster,SKILL_THROWING); // uproszczone hit rzucanym

				   if (result>0 && monster->seen_now)
					   screen.console.add(item->show_name() + "은 " + monster->name + "을 공격해 " + IntToStr(result) + "만큼의 피해를 입힌다.",6);

				   target_x=position[index].x;
				   target_y=position[index].y;
				   zablokowany=true;
				   break;
			 }
			 // bound on walls
			 if (level.map.blockMove(position[index].x,position[index].y))
			 {
//				 print_character(position[index-1].x,position[index-1].y,'0');
//				 print_character(position[index].x,position[index].y,'1');
//				 myrefresh();
				 // jezeli leci z SE
				 if (position[index-1].x>position[index].x &&  position[index-1].y > position[index].y)
				 {
					 if (level.map.blockMove(position[index].x,position[index].y+1) && !level.map.blockMove(position[index].x+1,position[index].y))
					 {
						 target_x = position[index].x + (position[index].x - target_x) + 1;
						 start_x = position[index].x + 1;
						 start_y = position[index].y;
						 generate_bresenham_line(start_x, start_y, target_x, target_y, (POSITION *) position,100);
					 }
					 else if (level.map.blockMove(position[index].x+1,position[index].y) && !level.map.blockMove(position[index].x,position[index].y+1))
					 {
						 target_y = position[index].y + (position[index].y - target_y) + 1;
						 start_x = position[index].x;
						 start_y = position[index].y + 1;
						 generate_bresenham_line(start_x, start_y, target_x, target_y, (POSITION *) position,100);
					 }
					 else
						 break;
					 cells-=index+item->calculate_weight()/400; // hit w sciane zmniejsza range
					 index=0;
				 }
				 // jezeli leci z SW
				 else if (position[index-1].x<position[index].x && position[index-1].y > position[index].y)
				 {
					 if (level.map.blockMove(position[index].x,position[index].y+1) && !level.map.blockMove(position[index].x-1,position[index].y))
					 {
						 target_x = position[index].x + (position[index].x - target_x) - 1;
						 start_x = position[index].x - 1;
						 start_y = position[index].y;
						 generate_bresenham_line(start_x, start_y, target_x, target_y, (POSITION *) position,100);
					 }
					 else if (level.map.blockMove(position[index].x-1,position[index].y)  && !level.map.blockMove(position[index].x,position[index].y+1))
					 {
						 target_y = position[index].y + (position[index].y - target_y) + 1;
						 start_x = position[index].x;
						 start_y = position[index].y + 1;
						 generate_bresenham_line(start_x, start_y, target_x, target_y, (POSITION *) position,100);
					 }
					 else
						 break;
					 cells-=index+item->calculate_weight()/400; // hit w sciane zmniejsza range
					 index=0;
				 }
				 // jezeli leci z NW
				 else if (position[index-1].x<position[index].x && position[index-1].y < position[index].y)
				 {
					 if (level.map.blockMove(position[index].x,position[index].y-1) && !level.map.blockMove(position[index].x-1,position[index].y))
					 {
						 target_x = position[index].x + (position[index].x - target_x) - 1;
						 start_x = position[index].x - 1;
						 start_y = position[index].y;
						 generate_bresenham_line(start_x, start_y, target_x, target_y, (POSITION *) position,100);
					 }
					 else if (level.map.blockMove(position[index].x-1,position[index].y) && !level.map.blockMove(position[index].x,position[index].y-1))
					 {
						 target_y = position[index].y + (position[index].y - target_y) + 1;
						 start_x = position[index].x;
						 start_y = position[index].y - 1;
						 generate_bresenham_line(start_x, start_y, target_x, target_y, (POSITION *) position,100);
					 }
					 else
						 break;
					 cells-=index+item->calculate_weight()/400; // hit w sciane zmniejsza range
					 index=0;
				 }
				 // jezeli leci z NE
				 else if (position[index-1].x>position[index].x && position[index-1].y < position[index].y)
				 {
					 if (level.map.blockMove(position[index].x,position[index].y-1) && !level.map.blockMove(position[index].x+1,position[index].y))
					 {
						 target_x = position[index].x + (position[index].x - target_x) + 1;
						 start_x = position[index].x + 1;
						 start_y = position[index].y;
						 generate_bresenham_line(start_x, start_y, target_x, target_y, (POSITION *) position,100);
					 }
					 else if (level.map.blockMove(position[index].x+1,position[index].y) && !level.map.blockMove(position[index].x,position[index].y-1))
					 {
						 target_y = position[index].y + (position[index].y - target_y) + 1;
						 start_x = position[index].x;
						 start_y = position[index].y - 1;
						 generate_bresenham_line(start_x, start_y, target_x, target_y, (POSITION *) position,100);
					 }
					 else
						 break;
					 cells-=index+item->calculate_weight()/400; // hit w sciane zmniejsza range
					 index=0;
				 }
				 else // jezeli nie moze sie odbic, to sie zatrzymuje
					 break;				 
			 }
        }
        if (level.map.seen_by_player(position[index].x,position[index].y))
        {
		   restore_screen(&scr_copy);
		   item->draw_at(position[index].x,position[index].y);
           myrefresh();
           delay(20);
        }	
     }
     if (zablokowany==false)
     {
		 target_x=position[index-1].x;
		 target_y=position[index-1].y;
     }

	 // take item to the map
     take_out_from_backpack(item);

	 item->ChangePosition(target_x,target_y);
     
	 if (this==level.player)
		 for (int a=0;a<3;a++)
			level.player->skill_used(SKILL_THROWING);

  return true;
}




int INTELLIGENT::shoot_into (int x, int y)
{
  int accuracy;
  int target_x, target_y;
  int random_value;
  double angle_miss, angle_shoot;
  int number_of_shoots, fire_number;
  int distance_to;
  bool firing_at_enemy;

  bool hit = false;

  if (enemy==NULL)
    firing_at_enemy=false;
  else
    firing_at_enemy=true;
    
  RANGED_WEAPON *range_weapon;
  range_weapon=weapon->IsShootingWeapon();
  assert (range_weapon!=NULL);

  descriptions.zero_attack("HIT");
  descriptions.zero_attack("ROBOTSAYS");
  
  if (IsConfused())
	  accuracy = range_weapon->ACC*5 + range_weapon->ammo.ACC*5;
  else
	  accuracy=dexterity.GetValue()*10 + (range_weapon->ACC + range_weapon->ammo.ACC)*5;

  accuracy*=skill[ weapon->skill_to_use ];
  accuracy/=50;
  if (accuracy==0)
	  accuracy++;

  switch(range_weapon->fire_type)
  {
  case FIRE_SINGLE:
	  number_of_shoots=1;
	  break;
  case FIRE_DOUBLE:
	  number_of_shoots=2;
	  break;
  case FIRE_TRIPLE:
	  number_of_shoots=3;
	  break;
  case FIRE_BURST:
	  if (range_weapon->properties&TYPE_ENERGY)
		  number_of_shoots=6;
	  else
		  number_of_shoots=6+random(6);
	  break;     
  }
  if (range_weapon->properties&TYPE_ENERGY)
  {
	  // use energy
	  bool enough_energy=false;
	  if (this->energy_points.GetValue()>=number_of_shoots*range_weapon->ammo.PWR)
		  enough_energy=true;
	  for (int e=0;e<number_of_shoots*range_weapon->ammo.PWR;e++)
		  range_weapon->uses_energy(false);
	  if (!enough_energy)
		  return 0;
  }

  // play proper sound
  RANGED_WEAPON *sound;
  if (this->seen_now) // on bunker enterance we hear everything
	  sound =(RANGED_WEAPON *) definitions.find_item(range_weapon->name);
  else
	  sound=NULL;
  switch(range_weapon->fire_type)
  {
  case FIRE_SINGLE:
	  if (sound!=NULL)
		  sounds.PlaySoundOnce(sound->sound_file_single);
	  break;
  case FIRE_DOUBLE:
	  if (sound!=NULL)
		  sounds.PlaySoundOnce(sound->sound_file_double);
	  break;
  case FIRE_TRIPLE:
	  if (sound!=NULL)
		  sounds.PlaySoundOnce(sound->sound_file_triple);
	  break;
  case FIRE_BURST:
	  if (sound!=NULL)
		  sounds.PlaySoundOnce(sound->sound_file_burst);
	  break;     
  }

  if (range_weapon->properties&TYPE_ENERGY)
	  number_of_shoots=1;

  angle_shoot=0;
  
  for (fire_number=1;fire_number<=number_of_shoots;fire_number++)
  {
  if (firing_at_enemy)
    distance_to=1000;
  else
  {
    distance_to=distance(pX(),pY(),x,y);
    int range;
    range=100-accuracy;
    if (range<0) range=0;
    range=random(range);

    range = (distance_to*range)/100; 
    distance_to+=random(range*2)-range;

  }

     if (range_weapon->ammo.quantity==0)
        break;

     random_value=random(360);

     angle_miss= ((double) random_value)/accuracy;
  
     if (coin_toss()) 
      angle_miss*=-1;

        
     target_x=x;
     target_y=y;
     angle_shoot+=angle_miss;
     calculate_hit_place(this->pX(),this->pY(),target_x,target_y, angle_shoot);
     // send missile to place
     if (seen_now==true)
	 {
		 set_color(14);
		 print_character(pX(),pY(),tile);
	 }
	 
     hit = range_weapon->fire_missile(this->pX(), this->pY(), target_x,target_y, enemy, distance_to);

     if (seen_now==true)
	 {
		 level.player->stop_repeating();

		 delay(50);
		 string text;
		 text = descriptions.get_attack("HIT");
		 if (text!="")
		 {
			 if (text.size()>0 && text[text.size()-1]==',')
				 text = text.substr(0,text.size()-1);

			 if (text.find("killing it")!=-1)
				 screen.console.add(text + '.',4);
			 else
				 screen.console.add(text + '.',6);	  
		 }
		 text=descriptions.get_attack("ROBOTSAYS");
		 screen.console.add(text,15);
		 descriptions.zero_attack("ROBOTSAYS");			 		 
		 descriptions.zero_attack("HIT");		 

	 }

	 if (this==level.player)
	 {
	    level.player->skill_used(weapon->skill_to_use);
		level.player->skill_used(weapon->skill_to_use);
	 }
  }

  if (seen_now==true)
  {
	  delay(200);	  
  }
	  
  return hit; // is hit
}


int INTELLIGENT::get_backpack_weight()
{
   ptr_list::iterator m,_m;
   ITEM *item;
   int weight;
   if (backpack.size()==0)
	   return 0;

        weight=0;
        
        for(m=this->backpack.begin(),_m=this->backpack.end(); m!=_m; m++)
        {
         item=(ITEM *)*m;
         weight+=item->calculate_weight();
        }// endof for
   return weight;
}

bool INTELLIGENT::choose_ready_weapon(ITEM *item)
{
	if (item==NULL)
		return false;
	
	if (item->property_ready() && item!=weapon && item!=ready_weapon)
	{
		ready_weapon = item;
		return true;
	}

	return false;
}

bool INTELLIGENT::set_ready_weapon()
{
	ITEM *item;
	if (ready_weapon!=NULL && ready_weapon!=weapon)
	{
		item = weapon;
		set_weapon(ready_weapon);
		if (item!=&this->unarmed)
		{
			ready_weapon = item;
		}
		else
			ready_weapon = NULL;
		
		return true;
	}
	return false;
}


bool set_ready_weapon();


int INTELLIGENT::reload_weapon()
{
   ptr_list::iterator m,_m;
   ITEM *item;
   bool found;
   RANGED_WEAPON *ranged_weapon = weapon->IsShootingWeapon();
   AMMO *am;
   string other=".";

   found=false;
   if (weapon->property_load_ammo() && ranged_weapon!=NULL)
   {
      if ( !weapon->IsLoaded() ) 
      {
        for(m=this->backpack.begin(),_m=this->backpack.end(); m!=_m; m++)
        {
         item=(ITEM *)*m;
		  // match exactly the same ammo
         if ( ranged_weapon->ammo.compare_activities_with(item)==true)
         {
           found=true;
           break;
         }
        }// endof for

		// match other, also good

		if (!found)
		{
			for(m=this->backpack.begin(),_m=this->backpack.end(); m!=_m; m++)
			{
				item=(ITEM *)*m;
				am = item->IsAmmo();
				if (am!=NULL && ranged_weapon->ammo.ammo_type == am->ammo_type) 
				{
					found=true;
					if (this==level.player)
						other = " with new ammo.";
					break;
				}
			}// endof for			
		}

		if (found==true)
		{
			am = item->IsAmmo();
			assert (am!=NULL);

			if (ranged_weapon->load_ammo(am)==0)
			{
				if (this->seen_now==true)
				{
					screen.console.add(this->name + "은 무기를 재장전한다" + other,3);
					RANGED_WEAPON *sound=(RANGED_WEAPON *) definitions.find_item(ranged_weapon->name);
					if (sound!=NULL)
						sounds.PlaySoundOnce(sound->sound_reload);


				}
			}
			return true;
		}              
      } // endof if weapon nie zaladowana
   }
   return false;
}

int INTELLIGENT::pick_up_item(ITEM *item, bool see_possible)
{
   string text;

   ptr_list::iterator m,_m;
   ITEM *temp;
   bool found;
   
   assert(item->owner==NULL);

   if (item==NULL)   
     return false;

   if (!item->property_get())
	   return false;

   found=false;
   
   if (item->property_join())  // is countable
   {
 //   1. Find the same in backpack
      for(m=this->backpack.begin(),_m=this->backpack.end(); m!=_m; m++)
      {
       temp=(ITEM *)*m;

       if (item->name == temp->name)
       {
           found=true;
           break;
       }
      }
	  if (found) // the same
	  {
		  item->add_and_destroy_object(temp);
		  
          if (see_possible && this->seen_now)
          {
			  screen.console.add(this->name + "은 " + item->article_the() + item->show_name() +"을 줍는다.",3);
			  sounds.PlaySoundOnce("data/sounds/other/itemget.wav");		
          } 
		  return true;
	  }
   }
   
   // no such item
   
      level.remove_from_items_on_map(item);
      backpack.push_back(item);
      items_in_backpack++;
      item->owner=this;

      if (see_possible && this->seen_now==true)
      {
		sounds.PlaySoundOnce("data/sounds/other/itemget.wav");		
		screen.console.add(this->name + "은 " + item->article_the() + item->show_name() +"을 줍는다.",3);
      }      
   
   //////////////////////////////////////////////////////////////////////////
   // IF BATTERY THEN INCRASE ENERGY_POINTS.MAX BY CAPACITY
   //////////////////////////////////////////////////////////////////////////
	BATTERY *bat = item->IsBattery();
	if (bat!=NULL)
	{
		this->energy_points.max += bat->max_capacity;
		this->add_energy( bat->capacity );
	}
   return true;
}

int INTELLIGENT::use_item(ITEM *item)
{
	return item->use_on(this);
}
	
int INTELLIGENT::take_out_from_backpack(ITEM *item)
{
   if (item==NULL)
     return false;

   if (item->owner==NULL)
	   return false;

   item->ChangePosition(pX(),pY()); 
   // jest jeszcze jego owner, wiec samo nie doda informacji na mapie
   
   backpack.remove(item);
   level.add_to_items_on_map(item);
   
   items_in_backpack--;
   if (item==this->weapon)
      weapon=&unarmed;
   else if (item==this->armor)
   {
	   remove_armor();
   }

   if (item==ready_weapon)
	   ready_weapon=NULL;
     

   //////////////////////////////////////////////////////////////////////////
   // IF BATTERY THE DECRASE MAX BY CAPACITY
   //////////////////////////////////////////////////////////////////////////

   BATTERY *bat = item->IsBattery();
   if (bat!=NULL)
   {
	   this->energy_points.max -= bat->max_capacity;
	   if (energy_points.val > energy_points.max)
	   {
		   bat->capacity = energy_points.val - energy_points.max;
		   energy_points.val = energy_points.max;
	   }
	   else
		   bat->capacity=0;
   }   
   
   item->owner=NULL;   
   return true;
}

int INTELLIGENT::drop_item(ITEM *item, bool see_possible)
{
   string text;
   if (item==NULL)
     return false;
     
   take_out_from_backpack(item);
   
   if (see_possible)
     if (this->seen_now==true)
     {
       text=this->name + "은 " + item->article_the() + item->show_name() + "을 버린다.";
       screen.console.add(text,3,false);
     }

   return true;
}

int INTELLIGENT::get_monster_capacity()
{
   return strength.GetValue()*5000;
}

int INTELLIGENT::can_pick_up(ITEM *item)
{
   if (get_backpack_weight()+item->calculate_weight()> get_monster_capacity())
     return 1; // to heavy
   if (this->items_in_backpack+1 > this->max_items_in_backpack)
     return 2; // can't take more
   return 0; // fine
}

int INTELLIGENT::set_weapon(ITEM *item)
{
   string text;
   if (item==NULL)
     return false;
   
   if (item==ready_weapon)
	   ready_weapon=weapon;

   this->weapon = item;

   if (this->seen_now==true)
   {
     text=this->name + "은 이제 " + item->show_name() + "을 무기로 사용한다.";
     screen.console.add(text,3);
   }
   return true;
}

int HERO::set_weapon(ITEM *item)
{
	if (item==NULL)
		return false;

	if (unarmed.name=="맨손")
		return INTELLIGENT::set_weapon(item);

	screen.console.add("그것을 들 수 있는 손이 없다!",3);
	return false;
}

int INTELLIGENT::remove_armor()
{
	if (!this->in_armor())
		return false;

	armor = &no_armor;
	return true;
}
	

int INTELLIGENT::set_armor(ITEM *item)
{
   string text;
   if (item==NULL)
     return false;

   if (this->in_armor())
   {
	   remove_armor();
   }

   if (this->strength.val + armor->MOD_STR <=0 ||
	   this->dexterity.val + armor->MOD_DEX <=0 ||
	   this->speed.val + armor->MOD_SPD <=0 )
   {
	   screen.console.add(this->name + "은 " + item->show_name() + "을 입을 수 없다.",3);	   
	   return false;
   }
   

   this->armor = (ARMOR *) item;
      
   if (this->seen_now==true)
   {
     if (this->in_armor())
       text=this->name + "은 " + item->show_name() + "을 입는다.";
     else
       text=this->name + "은 이제 아무 것도 입고 있지 않다.";
     
     screen.console.add(text,3);
   }
   return true;
}

bool MONSTER::death()
{
   if (this->is_dead)
		return false;

   if (this->seen_now)
   {
	   MONSTER *sound=(MONSTER *) definitions.find_monster(this->name);
	   if (sound!=NULL)
		   sounds.PlaySoundOnce(sound->sound_dying);
   }

 // pozostawienie ciala
   if (is_enemy(level.player))
   {
     if (this->seen_last_time_in_turn + 100 > level.turn)
     {
		 int value = experience_for_kill + random(experience_for_kill);
		 if (!seen_now) // jezeli nie widzimy jak enemy umiera, to dostajemy polowe exp.
			 value/=2;
         level.player->give_experience(value);
     }
   }

	 if (special_properties&SPECIAL_PROPERTY_SPLITTER && hit_points.max>5)
	 {
		 experience_for_kill/=2;
		 weight/=2;
		 hit_points.max/=2;
		 hit_points.val=hit_points.max;
		 direction=random(8);
		 // znalezienie wolnego pola na okolo
		 for (int a=0;a<10;a++)
		 {
			 int posx=pX()+random(3)-1;
			 int posy=pY()+random(3)-1;
			 if (level.entering_on_cell_possible(posx,posy))
			 {
				 int oldx = pX();
				 int oldy = pY();
				 this->ChangePosition(-1,-1);
				 if (this->seen_now)
				 {
					 screen.console.add(this->name + "은 분열한다!",14);
				 }

				 MONSTER *new_one = this->duplicate();
				 new_one->ChangePosition(posx,posy);
				 level.monsters.push_back(new_one);
				 
				 this->ChangePosition(oldx,oldy);
				 return false;
			 } // endof if
		 }		 
	 }
	 	 
	 if ((IsAnimal() || IsIntelligent()) && !IsMadDoctor() && -hit_points.val<hit_points.max/3)
	 {
		 CORPSE *corpse;
		 corpse=(CORPSE *) level.create_base_item("CORPSE");
		 
		 corpse->color=this->color;
		 corpse->of_what=this->name;
		 corpse->rename(corpse->of_what + "의 시체");
		 corpse->weight=this->weight;
		 
		 corpse->ChangePosition(this->pX(),this->pY());
		 
		 corpse->hit_points=this->weight/4000+1;
		 corpse->max_hit_points=this->weight/4000+1;
	 }
	 
	 if (this!=level.player)
		ChangePosition(-1,-1);
	 else
		 this->tile = '%';
	 
	 this->is_dead=true;
	 level.monsters_to_delete.push_back(this);
	 
	 // in all monsters that have it as enemy, zero it
	 ptr_list::iterator m,_m;
	 MONSTER *monster;
	 for(m=level.monsters.begin(),_m=level.monsters.end(); m!=_m; m++)
	 {
		 monster=(MONSTER *)*m;
		 if (monster->enemy==this)
		 {
			 monster->enemy_died();
		 }
		 if (this->IsWorm())
		 {
			 WORM *worm = monster->IsWorm();
			 if (worm) // monster is worm
			 {
				 if (worm->prev==this)
					 worm->prev=NULL;
				 if (worm->next==this)
					 worm->next=NULL;
			 }
		 }
	 }
	 
	 return true;
}

void MONSTER::enemy_died()
{
	enemy=NULL;
}

void ENEMY::enemy_died()
{
	last_x_of_enemy = pX();
	last_y_of_enemy = pY();
	MONSTER::enemy_died();
}


bool MONSTER::evade_missile(MONSTER *thrower)
{
  int hit;
  int evade;

  if (this==level.player)
	  if (random(5)==0)
		level.player->skill_used(SKILL_DODGING);

  if (!IsStunned())
  {
	  evade = this->dexterity.GetValue();
	  evade*=skill[ SKILL_DODGING ];
	  evade/=100;
  }
  else
	  evade=0;

  if (thrower==NULL) 
  {
    if (lower_random( evade, 100))
    {
      if (this->seen_now==true)
        screen.console.add(name +"은 피한다.",7);
      return true;
    }
  }
  else
  {
    hit = thrower->dexterity.GetValue()*2;
    // uwzglednienie procentowego skilla
    hit*=thrower->skill[ SKILL_THROWING ];
    hit/=100;
	  
    if (lower_random( evade, evade + hit) )
    {
      if (this->seen_now==true)
        screen.console.add(name +"은 피한다.",7);

	  if (this==level.player)
	    level.player->skill_used(SKILL_DODGING);
      return true;
    }
  }
  return false;
}

bool INTELLIGENT::evade_missile(MONSTER *thrower)
{
   return MONSTER::evade_missile(thrower);
}

bool ANIMAL::evade_missile(MONSTER *thrower)
{
   return MONSTER::evade_missile(thrower);
}

int MONSTER::hit_by_item(int hit_power, ITEM *item)
{
	int normal_power;
	string descr;

	assert(item!=NULL);

	if (hit_power==0)
		return 0;

	normal_power = hit_power;

	// damage weapon that attacked
	if (item->owner!=NULL)
	{
		if (&(item->owner->unarmed)!=item) // if it's not his unarmed
		{
			// if it's not an ammo
			AMMO *am = item->IsAmmo();
			if (!item->property_to_load() && am==NULL)
			{
				if (item->DMG < armor->ARM)
					item->damage_it(random(armor->ARM - item->DMG)); 
			}
		}
	}
	
	// if enemy in armor
    if (this->in_armor())
	{
		// hit this armor;

		  if (item->IsAmmo() && item->properties&TYPE_ARMOR_PERCING) // when missile is AP
		  {
			 if (!(armor->properties&TYPE_ARMOR_PERCING)) // when armor is not AP
			 {
				 if (hit_power>0)
				 {
					 int jakie_obrazenia = armor->damage_it(random(armor->ARM)+1);
					 if (jakie_obrazenia>0)
						 descriptions.add_attack("HIT",", damaging his " + armor->show_name() + ",") ;
					 else if (jakie_obrazenia==DAMAGE_DEATH)
						 descriptions.add_attack("HIT",", destroing his "  + armor->show_name() + ",") ;
					 
				 }
				 normal_power+=armor->ARM;
			 }
			 else // use energy, when AP energetic
			 {
				 BASE_ARMOR *parmor = armor->IsArmor();
				 if (parmor!=NULL && parmor->energy_activated && parmor->energy_properties&TYPE_ARMOR_PERCING)
					 parmor->uses_energy();
			 }
		  }
      

		  if (item->properties&TYPE_NORMAL) // when weapon do mechanical damage
		  {
			  normal_power-=armor->ARM;
		  }
	} // endof gdy ma armor


  if (!item->IsGrenade()) 
	  if ( (item->properties&TYPE_NORMAL && normal_power>0) || !(item->properties&TYPE_NORMAL))
	  {
		  if (item->properties&TYPE_RADIOACTIVE) 
				this->hit_changes_status(TYPE_RADIOACTIVE, hit_power*3/2);
		  if (item->properties&TYPE_CHEM_POISON) 
				this->hit_changes_status(TYPE_CHEM_POISON, hit_power*3/2);
		  if (item->properties&TYPE_PARALYZE) 
				this->hit_changes_status(TYPE_PARALYZE, hit_power);
		  if (item->properties&TYPE_STUN) 
				this->hit_changes_status(TYPE_STUN, hit_power);
		  if (item->properties&TYPE_ELECTRIC) 
			  this->hit_changes_status(TYPE_ELECTRIC, hit_power);
		  if (item->properties&TYPE_INCENDIARY) 
			  this->hit_changes_status(TYPE_INCENDIARY, hit_power);
	  }

  if (item->properties&TYPE_NORMAL) // when mechanical damage
  {
	  if (!(no_armor.properties&TYPE_ARMOR_PERCING) && item->properties&TYPE_ARMOR_PERCING) // through hard skin the same
	  {
		  normal_power+=no_armor.ARM;
	  }	  
	  return cause_damage(normal_power - no_armor.ARM);
	  
  }

  return 0;
}

int MONSTER::hit_by_explosion(int hit_power)
{
	if (hit_power==0)
		return 0;

    if (this->in_armor())
	{

		if (armor->properties&TYPE_EXPLOSIVE)
			hit_power-=armor->ARM*5;
		else
		{
			hit_power-=armor->ARM;
		}

		if (hit_power>armor->ARM)
		{
			if (armor->ARM!=0)
				armor->damage_it(random(hit_power/armor->ARM));
		}
	}

    cause_damage(hit_power - no_armor.ARM);
    return 0;
}

int MONSTER::hit_changes_status(int kind, int power)
{
	if (power<=0)
		return 0;
	if (kind&TYPE_INCENDIARY)
	{

		if (this->in_armor())
		{
			if (armor->properties&TYPE_INCENDIARY) // when armor protects
			{
				power-=armor->ARM;
				if (power<0)
					power=0;
			}
			else
			{
				if (random(5)==0)
					armor->damage_it(power/2);
			}
		}
		
		power = power - power*resist[RESIST_FIRE]/100;
		if (power>state[STATE_TEMPERATURE])
			state[STATE_TEMPERATURE] = (power + state[STATE_TEMPERATURE])/2;
	}
	else if (kind&TYPE_RADIOACTIVE)
	{
		power = power - power*resist[RESIST_RADIOACTIVE]/100;
		if (power>0)
		{
			state[STATE_RADIOACTIVE]+=power;
			if (this->seen_now==true)
				screen.console.add(name + "은 방사능에 오염된다.",14);
		}
	}
	else if (kind&TYPE_CHEM_POISON)
	{
		power = power - power*resist[RESIST_CHEM_POISON]/100;
		if (power>0)
		{
			if (power>state[STATE_CHEM_POISON]/2)
				state[STATE_CHEM_POISON]+=power;
			if (this->seen_now==true)
				screen.console.add(name + "은 독에 감염된다.",2);
		}
	}
	else if (kind&TYPE_STUN)
	{
		power = power - power*resist[RESIST_STUN]/100;
		power*=3;
		power-=endurance.GetValue();
		if (power>state[STATE_STUN])
		{
			state[STATE_STUN] = power;
			if (this->seen_now==true)
				screen.console.add(name + "은 기절한다.",8);
		}
	}
	else if (kind&TYPE_ELECTRIC)
	{
		power = power - power*resist[RESIST_ELECTRICITY]/100;
		power-=endurance.GetValue()/2;
		if (power>0)
		{
			this->cause_damage(power);
			if (this->seen_now==true)
				screen.console.add(name + "은 충격을 받아 " + IntToStr(power) + "만큼의 피해를 입는다.",5);
		}
	}
	else if (kind&TYPE_PARALYZE)
	{
		power = power - power*resist[RESIST_PARALYZE]/100;
		if (power>state[STATE_SPEED] && power>0)
		{
			state[STATE_SPEED] -= power;
			if (this->seen_now==true)
				screen.console.add(name + "은 느려진다.",1);
		}
	}
	return 0;
}


bool INTELLIGENT::open_in_direction()
{
  int x,y;
  string old_one;
  cell_in_current_direction(x,y);
  old_one=level.map.getCellName(x,y);
  old_one=old_one.substr(0, old_one.find('(')-1);
  
  if (level.map.open(x,y)==true && this->seen_now==true)
  {
	  if ((this==level.player && !level.player->is_repeating_action()) || this!=level.player)
		screen.console.add(name + "은 " + old_one +"을 연다.",3);
    return true;
  }
  return false;
}

bool INTELLIGENT::close_in_direction()
{
  int x,y;
  string old_one;
  cell_in_current_direction(x,y);
  old_one=level.map.getCellName(x,y);
  old_one=old_one.substr(0, old_one.find('(')-1);
  
  if (level.map.IsMonsterOnMap(x,y)==false)
  {
    if (level.map.close(x,y)==true && this->seen_now==true)
    {
      screen.console.add(name + "은 " + old_one +"을 닫는다.",3);
      return true;
    }
  }
  return false;
}


MONSTER * INTELLIGENT::duplicate()
{
   INTELLIGENT *new_one;
   ITEM *temp, *temp2;
   ptr_list::iterator m, _m;
   
   new_one=new INTELLIGENT;
   ITEM *old_armor= this->armor;
   if (old_armor!=&this->no_armor)
   {
	   this->remove_armor();	   
   }
   *new_one=*this;
   if (old_armor!=&this->no_armor)
   {
	   this->set_armor(old_armor);
   }

   // change owner's attributes
   new_one->set_attributes_on_self();   

   new_one->energy_points.val=0;  
   new_one->energy_points.mod=0;  
   new_one->energy_points.max=0;     

   // copy backpack
  new_one->weapon=&new_one->unarmed;
  new_one->armor=&new_one->no_armor;
  new_one->weapon->owner = new_one;
  new_one->armor->owner = new_one;  
  
  new_one->backpack.clear();
  new_one->items_in_backpack=0;
  
  new_one->ChangePosition(-1,-1);

  for(m=this->backpack.begin(),_m=this->backpack.end(); m!=_m; m++)
  {
         temp=(ITEM *)*m;
         temp2=temp->duplicate();
		 new_one->ChangePosition(-1,-1);
         new_one->pick_up_item(temp2,false);
         if (temp==this->weapon)
         {
           new_one->weapon=temp2;
         }
         else if (temp==this->armor)
         {
		   ((INTELLIGENT *)new_one)->set_armor(temp2);
         }
         else if (temp==this->ready_weapon)
         {
			 new_one->ready_weapon=temp2;
         }
         
  }
  // create random items, if have
  if (definitions.lista_kategorii_losowych_itemow_potwora.find(name)!=definitions.lista_kategorii_losowych_itemow_potwora.end())
  {
	  list <int>::iterator k,l;
	  size_t list_size = definitions.lista_kategorii_losowych_itemow_potwora[name].size();
	  k = definitions.lista_kategorii_losowych_itemow_potwora[name].begin();
	  l = definitions.lista_kategorii_losowych_itemow_potwora[name].end();
	  int category_number;

	  // przejscie przez liste items (kategorii, z ktorych jest losowanie)
	  for (;k!=l;k++)
	  {
		  category_number = *k;
		  // znalezienie losowego itemu z danej kategorii
		  new_one->pick_up_item(level.CreateRandomItemFromCategory(category_number, -1, -1, 0, 0),false);			  
	  }
  }
   return (MONSTER *) new_one;
}

int MONSTER::use_energy(int energy)
{
	energy_points.val-=energy;
	if (energy_points.val<=0)
	{
		energy_points.val=0;
		return false;
	}
	return true;
}

int MONSTER::add_energy(int energy)
{
	if (energy_points.val == energy_points.max)
		return false;

	energy_points.val+=energy;
	if (energy_points.max>999)
		energy_points.max=999;

	if (level.is_player_on_level && this==level.player)
		if (energy_points.val == energy_points.max)
			level.player->stop_repeating();

	if (energy_points.val>energy_points.max)
	{
		energy_points.val=energy_points.max;
	}
	return true;
}

void MONSTER::set_attributes_on_self()
{
	fov_radius.ChangeOwner(this);
	strength.ChangeOwner(this);
	dexterity.ChangeOwner(this);
	intelligence.ChangeOwner(this);
	speed.ChangeOwner(this);
	metabolism.ChangeOwner(this);
	hit_points.ChangeOwner(this);
	energy_points.ChangeOwner(this);
	endurance.ChangeOwner(this); 	
}

MONSTER * ANIMAL::duplicate()
{
   ANIMAL *new_one;
   new_one=new ANIMAL;
   *new_one=*this;

   // change owner's of attributes
   new_one->set_attributes_on_self();
   
   new_one->weapon=&new_one->unarmed;
   new_one->armor=&new_one->no_armor;
   new_one->weapon->owner = new_one;
   new_one->armor->owner = new_one;
   return (MONSTER *) new_one;
}

MONSTER * WORM::duplicate()
{
	WORM *new_one;
	new_one=new WORM;
	*new_one=*this;

	// change owner's of attributes
	new_one->set_attributes_on_self();

	new_one->weapon=&new_one->unarmed;
	new_one->armor=&new_one->no_armor;
	new_one->weapon->owner = new_one;
	new_one->armor->owner = new_one;
	return (MONSTER *) new_one;
}

MONSTER * MADDOCTOR::duplicate()
{
	MADDOCTOR *new_one;
	new_one=new MADDOCTOR;
	*new_one=*this;
	
	// change owner's of attributes
	new_one->set_attributes_on_self();
	
	new_one->weapon=&new_one->unarmed;
	new_one->armor=&new_one->no_armor;
	new_one->weapon->owner = new_one;
	new_one->armor->owner = new_one;
	new_one->tail.owner = new_one;
	return (MONSTER *) new_one;
}


MONSTER * SEARCHLIGHT::duplicate()
{
	SEARCHLIGHT *new_one;
	new_one=new SEARCHLIGHT;
	*new_one=*this;      

	// change owner's of attributes
	new_one->set_attributes_on_self();
	
	new_one->weapon=&new_one->unarmed;
	new_one->armor=&new_one->no_armor;
	new_one->weapon->owner = new_one;
	new_one->armor->owner = new_one;

	new_one->enemy = NULL;
	return (MONSTER *) new_one;
}


MONSTER * ROBOT::duplicate()
{
	ROBOT *new_one;
	ptr_list::iterator m, _m;
	
	new_one=new ROBOT;
	*new_one=*this;

	// change owner's of attributes
	new_one->set_attributes_on_self();
	
	// duplicate shell
	assert (shell!=NULL);
	new_one->shell = (ROBOT_SHELL *) shell->duplicate();
	// shell is not on the map
	level.remove_last_item_on_map();
	
	new_one->shell->last_robot_name = this->name;
	new_one->shell->owner = new_one;

	new_one->Creator_ID=0;	
	
	new_one->shell->owner = new_one;
	new_one->rename(new_one->shell->last_robot_name);
	
	new_one->group_affiliation = new_one->shell->cpu->group_affiliation;
	
	// set attributes
	
	new_one->strength.val = new_one->shell->cpu->quality/4;
	new_one->strength.max = new_one->shell->cpu->quality/4;
	new_one->dexterity.val = new_one->shell->cpu->quality/4;
	new_one->dexterity.max = new_one->shell->cpu->quality/4;
	new_one->endurance.val = new_one->shell->cpu->quality/4;
	new_one->endurance.max = new_one->shell->cpu->quality/4;
	new_one->intelligence.val = 1;
	new_one->intelligence.max = 1;
	new_one->speed.val = 1;
	new_one->speed.max = 1;
	new_one->metabolism.val = 0;
	new_one->metabolism.max = 0;
	
	for (int a=0;a<NUMBER_OF_SKILLS;a++)
		new_one->skill[a]=new_one->shell->cpu->quality;
	
	new_one->last_x_of_enemy = new_one->pX();
	new_one->last_y_of_enemy = new_one->pY();
		
   return (MONSTER *) new_one;
}


int INTELLIGENT::cause_damage(int damage)
{
	if (actual_behaviour == BEHAVIOUR_CALM)
		actual_behaviour = BEHAVIOUR_SEARCH_FOR_ENEMY;

	if (coin_toss())
		turned_to_fight = true;

	// if have active grenade, maybe it will drop it by damage shock
	
	ptr_list::iterator m,_m;
	ITEM *temp;
	GRENADE *gr;

	for(m=this->backpack.begin(),_m=this->backpack.end(); m!=_m; m++)
	{
		temp=(ITEM *)*m;
		gr = temp->IsGrenade();
		if (gr!=NULL && gr->active)
		{
			if (lower_random(damage,hit_points.val))
			{
				drop_item(temp,false);
				if (this->seen_now)
				{
					level.player->stop_repeating();					
					screen.console.add(this->name + "은 " + temp->article_a() + temp->show_name() + "을 잡지 못하고 떨어뜨린다.",12);
				}
				break; // drop only one
			}
		}
	}
	return MONSTER::cause_damage(damage);			
}

ACTION INTELLIGENT::get_action() // AI of intelligent
{
	if (IsStunned())
	{
#ifdef DEBUG_AI
			screen.console.add(name + " says: ",7);
			screen.console.add("I'm STUNNED.",7);
#endif
		return ACTION_WAIT_STUNNED;
	}

	chosen_action = ACTION_NOTHING;

	int random_value;

//	I. FAZA WSPOLNA DLA WSZYSTKICH ZACHOWAN
//	0. Ogladaj otoczenie - listy items i potworkow widzianych

	look_around();		

#ifdef DEBUG_AI
	//	if (this->seen_now)
	{
		screen.console.add(name + " says: ",7);
		
		switch(actual_behaviour) {
		case BEHAVIOUR_CALM:
			screen.console.add("CALM",7);
			break;
		case BEHAVIOUR_H2H_COMBAT:
			screen.console.add("H2H COMBAT",7);
			break;
		case BEHAVIOUR_RANGED_COMBAT:
			screen.console.add("RANGED COMBAT",7);
			break;
		case BEHAVIOUR_RUN_AWAY:
			screen.console.add("RUN AWAY",7);
			break;
		case BEHAVIOUR_SEARCH_FOR_ENEMY:
			screen.console.add("SEARCH FOR ENEMY",7);
			break;
		case BEHAVIOUR_TURN_TO_FIGHT:
			screen.console.add("TURN TO FIGHT",7);
			break;			
		}
		screen.console.show();
	}
#endif
	

//	3. Rzu?we of enemy active granat z backpacka. Je풽li of enemy nie ma,
//		to rzu?go gdzie?daleko.

	if (!(AI_restrictions & AI_DONT_USE_GRENADES))
	{
		if (throw_active_grenade()==true)
			return ACTION_THROW;
	}		

//	1. Uzyj leczacego, gdy malo HPkow

	if (!(AI_restrictions & AI_DONT_USE_HEALING))
	{
		if (have_low_hp())
		{
#ifdef DEBUG_AI
				if (this->seen_now)
					screen.console.add("I'm trying to use healing item.",7);
#endif
				if (use_healing_item()==true)
				{
					return ACTION_INVENTORY;
				}
		} 
	}  // gdy nie jest zabronione
	
//  1.2 Uzyj leczacego z trucizny, gdy zatruty

	if (!(AI_restrictions & AI_DONT_USE_HEALING))
	{
		if (state[STATE_RADIOACTIVE] > 0 || state[STATE_CHEM_POISON] > 0)
		{
#ifdef DEBUG_AI
			if (this->seen_now)
				screen.console.add("I'm trying to neutralize poison.",7);
#endif
			if (use_remove_poison_item()==true)
			{
				return ACTION_INVENTORY;
			}
		}	
	} // gdy nie jest zabronione
	

//	2. Je풽li dalej ma쿽 HPk? to przejd?do UCIECZKI
		
	if (!(AI_restrictions & AI_DONT_RUN_AWAY))
	{
		if (have_critically_low_hp() && !turned_to_fight)
		{
			random_value = 2;
			if (enemy!=NULL && !enemy->have_low_hp()) // gdy enemy tez slaby, to moze nie uciekamy
				random_value = 6;
				
			if (random(random_value)==0)
			{
#ifdef DEBUG_AI
					if (this->seen_now)
						screen.console.add("I'm running away!",7);
#endif
					start_to_run_away();
			}
			
		}
		else if (actual_behaviour == BEHAVIOUR_RUN_AWAY)
		{
			if (random(20)==0)
			{
#ifdef DEBUG_AI
				screen.console.add("I'm turning to fight again!",7);
#endif
				actual_behaviour=BEHAVIOUR_SEARCH_FOR_ENEMY;
				turned_to_fight=true;
			}
		}
	} // gdy ucieczka dozwolona
		

//	4. Powiedz innym potworkom widzianym przez ciebie (jezeli sa tego
//		samego typu - name), gdzie ostatnio widziales gracza, jezeli
//		widziales go pozniej niz on (turn).

	if (!(AI_restrictions & AI_DONT_COOPERATE_WITH_FRIENDS))
	{
		tell_others_about_enemy(); // jezeli other nie ma of enemy, to juz ma :)
	}

// a moze uciekac???

	if (how_danger_is_cell(pX(),pY())>0) // stoimy na polu, gdzie cos jest nie tak
	{
		// sprawdz czy mozna sie jakos obronic
		if (defend_self_from_danger_on_map()==true)
			return chosen_action;
	}
	

//	II. FAZA WSPOLNA DLA WALKI
	if (actual_behaviour==BEHAVIOUR_H2H_COMBAT ||
		actual_behaviour==BEHAVIOUR_RANGED_COMBAT || turned_to_fight)
	{
		
		//	1. U퓓j booster?, gdy masz jakie?
		if (!(AI_restrictions & AI_DONT_USE_BOOSTERS))
		{
			if (seen_enemies.size()>1 || (enemy!=NULL && enemy->hit_points.GetValue() > this->hit_points.GetValue()))
			{
				random_value = 6;
				if (have_low_hp())
					random_value = 3;
				if (random(random_value)==0)
				{
					if (use_booster_item()==true)
					{
						return ACTION_INVENTORY;
					}
				}
			}
		} // gdy nie zabronione
		
		//	2. Zmie?bro?na lepsz?(tego samego typu), gdy masz (wyce?item).
		if (!(AI_restrictions & AI_DONT_USE_OTHER_WEAPONS))
		{
			if (change_weapon_to_better_one()==true) // te?gdy nie masz pociskow do tej.
			{
#ifdef DEBUG_AI
				screen.console.add("I'm using now " + weapon->show_name(),12);
#endif
				return ACTION_INVENTORY;
			}
		} // gdy nie zabronione		

		//	3. Aktywuj granat/mine, gdy masz i gdy enemy daleko. Rzuci w I fazie wspolnej.
		if (!(AI_restrictions & AI_DONT_USE_GRENADES))
		{
			random_value = 6;
			if (have_low_hp())
				random_value = random_value/2;		
			if (random(random_value)==0)
			{
				if (activate_grenade_when_enemy_is_far()==true)
				{
					if (this->seen_now==true)
						screen.console.add(name + "은 무언가를 작동시킨다!",12);												
					return ACTION_INVENTORY;
				}
			}
		} // gdy nie zabronione					
		//	4. Aktywuj weapon i armor, gdy masz do nich energie
		if (activate_weapon_and_armor_when_energy_available())
			return ACTION_INVENTORY;
	}	

//	III. WALKA NA ODLEGLOSC
	if (actual_behaviour==BEHAVIOUR_RANGED_COMBAT ||
		actual_behaviour==BEHAVIOUR_TURN_TO_FIGHT)
	{
		turns_of_searching_for_enemy = 0;

		//	2. Przeladuj weapon, gdy number pociskow = 0. Je풽li nie masz wiecej pociskow,
		//	zmie?bro?na inn?strzelaj퉏?(kt?a jest zaladowana!).
		//	Je풽li nie masz takiej broni przejd?do WALKI W ZWARCIU.
		if (!(AI_restrictions & AI_DONT_USE_RANGED_WEAPONS))
		{
			if (weapon->property_load_ammo())
			{
				RANGED_WEAPON *ranged_weapon = weapon->IsShootingWeapon();
				if (ranged_weapon!=NULL)
				{
					if (ranged_weapon->ammo.quantity == 0)
					{
						if (load_weapon()==true)
							return ACTION_INVENTORY;
						else
							actual_behaviour = BEHAVIOUR_H2H_COMBAT;
					}
				}
			}
			//	3. Strzelaj w kierunku of enemy.
			if (actual_behaviour==BEHAVIOUR_RANGED_COMBAT) // jezeli dalej tak sie zachowuje
			{
				if (misses_in_shooting>=2) 
				{
					if (go_to_direction_of_enemy()==true) // podchodzimy do of enemy
					{
						if (coin_toss()) // aby nie bylo podejscie, strzal, podejscie, strzal. Podchodzimy czasem wiecej niz o 1 pole
							misses_in_shooting--;
						return chosen_action;
					}
				}
				else if (fire_at_enemy()==true) 
					return ACTION_FIRE;
			}			
		}	// gdy nie zabronione		
	}

// 	IV. WALKA W ZWARCIU
//	1. Idz do pola, gdzie jest enemy (opening drzwi po drodze)

	if (actual_behaviour==BEHAVIOUR_H2H_COMBAT)
	{
		turns_of_searching_for_enemy = 0;

		if (go_to_direction_of_enemy()==true)
			return chosen_action;		
	}
		
//	V. SPOK?
//	1. Zaladuj niezaladowan?bro?strzelaj퉏? Je풽li pocisk? do niej w backpacku
//	jest wi?ej ni?pojemno쒏 magazynka i gdy magazynek nie pe쿻y, to roz쿪duj
//	bro?

	if (actual_behaviour==BEHAVIOUR_CALM)
	{
		turns_of_calm++;
		turns_of_searching_for_enemy = 0;
		if (turns_of_calm<5) // tylko w I turach przeladowujemy weapon i zmieniamy armor
		{
			if (!(AI_restrictions & AI_DONT_USE_RANGED_WEAPONS))
			{
				if (weapon->property_load_ammo())
				{
					RANGED_WEAPON *ranged_weapon = weapon->IsShootingWeapon();
					if (ranged_weapon!=NULL)
					{
						if (ranged_weapon->ammo.quantity != ranged_weapon->magazine_capacity) // gdy magazynek nie pelny
						{
							if (ranged_weapon->ammo.quantity == 0)
							{
								if (load_weapon()==true)
									return ACTION_INVENTORY;
							}
							else if (carried_number_of_missiles_for_weapon(ranged_weapon)>ranged_weapon->magazine_capacity)
							{
								if (unload_weapon()==true)
									return ACTION_INVENTORY;						
							}
						}
					} // ranged_weapon!=NULL
				} // endof if weapon mozna zaladowac
			} // gdy nie zabronione

			if (!(AI_restrictions & AI_DONT_USE_OTHER_WEAPONS))
			{
				if (change_weapon_to_better_one()==true)
				{
#ifdef DEBUG_AI
					screen.console.add("I'm using now " + weapon->show_name(),12);
#endif
					return ACTION_INVENTORY;
				}
			} // gdy nie zabronione
			
			if (!(AI_restrictions & AI_DONT_USE_OTHER_WEAPONS))
			{
				if (change_armor_to_better_one()==true)
				{
#ifdef DEBUG_AI
					screen.console.add("I'm wearing now " + armor->show_name(),12);
#endif
					return ACTION_INVENTORY;
				}
			} // gdy nie zabronione

		} // endof if turn sroomu

		if (turns_of_calm++>random(20)+20)
		{
			//	2. Idz do wartosciowych items i pick_up je
			if (!(AI_restrictions & AI_DONT_PICK_UP_ITEMS))
			{
				switch(pick_valuable_items()) 
				{
				case 1: // podnosimy
					return ACTION_GET; 
				case 2: // idziemy
					return ACTION_MOVE;
				}
			} // gdy nie zabronione podnoszenie
			if (!(AI_restrictions & AI_DONT_MOVE))
			{
				throw_item_to();
			}
			if (turns_of_calm++>random(20)+40 && random(5)==0) // w pewnym momencie musi odpoczac
				turns_of_calm = 0;
		}
	} // endof zachowanie sroom
		
//	VI. UCIECZKA
	if (!(AI_restrictions & AI_DONT_RUN_AWAY))
	{
		if (actual_behaviour==BEHAVIOUR_RUN_AWAY && !turned_to_fight)
		{
			// Wykorzystaja technike defensywna
			use_defensive_techniques_running_away();
			//	1. Znajdz oddalone pole (w rangeu wzroku?), do ktorego mozna dojsc.
			if (choose_run_away_direction()==true)
				return ACTION_MOVE;
		}		
	}

//	VII. POSZUKIWANIE WROGA

	if (!(AI_restrictions & AI_DONT_MOVE))
	{
		if (actual_behaviour==BEHAVIOUR_SEARCH_FOR_ENEMY)
		{
			if (turns_of_searching_for_enemy++ > 70+random(50)) // szukamy juz dlugo, enemy gdzies przepadl
			{
				actual_behaviour=BEHAVIOUR_CALM;
				turned_to_fight=false;
				turns_of_calm = 0;
				turns_of_searching_for_enemy = 0;
			}
			//	1. Jezeli enemy zniknal z oczu (and not umarl) to szukaj go:
			//	1.1 Idz do ostatniego pola, gdzie go widziales a pozniej w kierunku, gdzie on szedl
			if (!is_at_enemy_position)
			{
				if (go_to_direction_of_enemy()==true)
				{
					camping = 0;
					return chosen_action;		
				}
			}
			
			// doszlismy do tego pola, a gracza nie widac, wiec zaczynamy szukanie
			throw_item_to();
			return chosen_action;		
			
		}		
	} // gdy nie zabronione poruszanie

//  nie pobrano akcji z jakiegos powodu?
	if (chosen_action==ACTION_NOTHING)
	{
#ifdef DEBUG_AI
		if (this->seen_now)
			screen.console.add("I'm waiting.\n",7);
#endif		
		return ACTION_WAIT;	
	}
	else // wybrana action mogla zostac podjeta podczas innej (np. otwarcie drzwi podczas chodzenia)
		return chosen_action;
}	

bool INTELLIGENT ::throw_item_to()
{
		int x,y;
		chosen_action = ACTION_MOVE;					
		
		// nie lubimy stac w miejscu szukajac of enemy - idziemy przy scianie gdy idac po skosie trafiamy na nia
		bool change_direction = false;
		int old_direction = direction;
		cell_in_current_direction(x,y);
		if ((!level.entering_on_cell_possible(x,y) && !level.map.isOpenable(x,y)) || how_danger_is_cell(x,y)>0)
		{
			int to_left = direction-1;
			int to_right = direction+1;
			
			if (to_left==-1)
				direction = 7;
			if (to_right==8)
				direction = 0;
			
			direction = to_left;
			cell_in_current_direction(x,y);
			if ((level.entering_on_cell_possible(x,y) || level.map.isOpenable(x,y)) && how_danger_is_cell(x,y)>0)
				change_direction = true;
			
			if (!change_direction)
			{
				direction = to_right;
				cell_in_current_direction(x,y);
				if (level.entering_on_cell_possible(x,y) || level.map.isOpenable(x,y) && how_danger_is_cell(x,y)>0)
					change_direction = true;
			}			
		}

		if (!change_direction)
		{
			direction = old_direction;				
			// wylosowanie kierunku, gdzie ma isc
			int a,t,random_value;
			char directions[8];
			for (a=0;a<8;a++)
				directions[a]=a;

			// shuffle
			for (a=0;a<8;a++)
			{
				random_value = random(8);
				t = directions[a];
				directions[a] = directions[random_value];
				directions[random_value] = t;
			}

			// szukamy nowego, gdzie other niz przeciwny do old_direction
			int oposite_direction = old_direction + 4;
			if (oposite_direction>7)
				oposite_direction-=8;

			for (a=0;a<8;a++)
			{
				direction = directions[a];
				if (direction==oposite_direction)
					continue;

				cell_in_current_direction(x,y);
				if ((level.entering_on_cell_possible(x,y) || level.map.isOpenable(x,y)) && how_danger_is_cell(x,y)==0)
					break;
			};
			// szukamy nowego, gdzie mo풽 by?taki sam, jak przeciwny do old_direction
			for (a=0;a<8;a++)
			{
				direction = directions[a];
				cell_in_current_direction(x,y);
				if ((level.entering_on_cell_possible(x,y) || level.map.isOpenable(x,y)) && how_danger_is_cell(x,y)==0)
					break;
			};
			return true;;
		}
		else // gracze lubia wchodzi w drzwi :)
		{	 //Jezeli przechodzimy obok jakis, to czemu by w nie nie wejsc
			 // jezeli pomieszczenie jest male, to tez tak z niego wyjdzie.
			 // sprawdzamy direction, aby nie wracac przy wychodzeniu
			if (random(3)==0) // losujemy, bo czesto przechodzi 3 tury obok 1 drzwi
			{
				if ((level.map.isOpenable(pX()+1,pY()) || level.map.isClosable(pX()+1,pY()) ) && direction != _kw)
					direction_to_near_cell(pX()+1,pY());
				else if ((level.map.isOpenable(pX()-1,pY()) || level.map.isClosable(pX()-1,pY())) && direction != _ke)
					direction_to_near_cell(pX()-1,pY());
				else if ((level.map.isOpenable(pX(),pY()+1) || level.map.isClosable(pX(),pY()+1)) && direction != _kn)
					direction_to_near_cell(pX(),pY()+1);
				else if ((level.map.isOpenable(pX(),pY()-1) || level.map.isClosable(pX(),pY()-1)) && direction != _ks)
					direction_to_near_cell(pX(),pY()-1);
				else if ((level.map.isOpenable(pX()+1,pY()+1) || level.map.isClosable(pX()+1,pY()+1)) && direction != _knw)
					direction_to_near_cell(pX()+1,pY()+1);
				else if ((level.map.isOpenable(pX()-1,pY()+1) || level.map.isClosable(pX()-1,pY()+1)) && direction != _kne)
					direction_to_near_cell(pX()-1,pY()+1);
				else if ((level.map.isOpenable(pX()+1,pY()-1) || level.map.isClosable(pX()+1,pY()-1)) && direction != _ksw)
					direction_to_near_cell(pX(),pY()-1);
				else if ((level.map.isOpenable(pX()-1,pY()-1) || level.map.isClosable(pX()-1,pY()-1)) && direction != _kse)
					direction_to_near_cell(pX(),pY()-1);
			}		
		}
		// idziemy w obranym kierunku
		cell_in_current_direction(x,y);
		go_around_cell(x,y,true);		
		cell_in_current_direction(x,y);
		if (how_danger_is_cell(x,y)>0)
		{
			chosen_action = ACTION_WAIT;
			return false;			
		}
		return true;
}

TIME INTELLIGENT::do_action( ACTION action )
{
	last_pX = pX();
	last_pY = pY();
	
	switch (action)
	{
	case ACTION_MOVE:
		int x,y;
		cell_in_current_direction(x,y);
		if (level.map.blockMove(x,y))
			go_around_cell(x,y,false);
		move();
		return (TIME_MOVE/speed.GetValue())+1;
	case ACTION_WAIT:
		return (TIME_MOVE/speed.GetValue())+1;
	case ACTION_INVENTORY:
		camping=0;
		return (TIME_MOVE/speed.GetValue())+1;
	case ACTION_GET:
		camping=0;
		return (TIME_MOVE/speed.GetValue())+1;
	case ACTION_FIRE:
		camping=0;
		return (TIME_MOVE/speed.GetValue())+1;
	case ACTION_OPEN:
		camping=0;
		open_in_direction();
		return (TIME_MOVE/speed.GetValue())+1;
	}
	return 1;
}	

int INTELLIGENT::go_to_direction_of_enemy() // zwraca, czy powinien otworzyc drzwi
{
	if (AI_restrictions & AI_DONT_MOVE)
		return false;
		
	int x,y;
	x = last_x_of_enemy;
	y = last_y_of_enemy;
	if (pX() == x && pY() == y)
	{
		actual_behaviour = BEHAVIOUR_SEARCH_FOR_ENEMY; // na w razie of_what, choc i tak byc powinno
		is_at_enemy_position = true;
		direction = last_direction_of_enemy;
		return false;
	}
	else
		is_at_enemy_position = false;

	chosen_action = ACTION_MOVE;
	
	if (set_direction_to_cell_by_shortest_path(last_x_of_enemy,last_y_of_enemy,true)) // z otwieraniem drzwi
	{
		cell_in_current_direction(x,y);
		go_around_cell(x,y,true);
	}
	else // gdy nie mozna dojsc do of enemy /// zrobic cos lepszego w takim wypadku!!!!
	{
		throw_item_to();
		enemy = NULL;
		last_x_of_enemy = pX();
		last_y_of_enemy = pY();		
	}
	return true;
}

bool INTELLIGENT::look_around()
{
	ptr_list::iterator m,_m;
	ITEM *temp;
	MONSTER *temp2;

	bool sees_enemy_again=false;
	int danger;
	int a,b;
	
	// obejrzenie map

	seen_items.clear();
	seen_enemies.clear();
	seen_friends.clear();

	level.fov.Start(&level.map,pX(),pY(),fov_radius.GetValue());
	MONSTER::look_around(); // obsluga kamer i reflektorow	

	// ptr_list enemyow i przyjaciol

	for(m=level.monsters.begin(),_m=level.monsters.end(); m!=_m; m++)
	{
		temp2=(MONSTER *)*m;
		
		if (temp2==this)
			continue;
		
		if (level.map.seen_by_camera(temp2->pX(),temp2->pY()) || (level.map.seen(temp2->pX(),temp2->pY()) && distance(temp2->pX(),temp2->pY(),pX(),pY())<fov_radius.GetValue()))
		{
				if (is_friendly(temp2)) // przyjaciel
					seen_friends.push_back(temp2);
				else if (is_enemy(temp2)) // enemy
				{
					seen_enemies.push_back(temp2);
					
					if (temp2 == enemy)
						sees_enemy_again = true;
					
					// dodanie niebezpieczenstwa
					
					for (a=0;a<3;a++)
						for (b=0;b<3;b++)
							level.map.addDanger(temp2->pX() + a-1,temp2->pY() + b-1,DANGER_ENEMY);						
				} // czy enemyi
			// neutralnych ignorujemy. Kto by sie przejmowal, czy zgina np. od wybuchu :)				
		}
	}
	
	// ptr_list items
	for(m=level.items_on_map.begin(),_m=level.items_on_map.end(); m!=_m; m++)
	{
		temp=(ITEM *)*m;
		if (level.map.seen(temp->pX(),temp->pY()) && !level.map.isShield(temp->pX(),temp->pY()))
		{
			if (!temp->property_controller() && !temp->invisible) // gdy nie jest to sterujacy i gdy nie invisible
			{
				seen_items.push_back(temp);				

				// dodanie niebezpieczenstwa
				GRENADE *gr = temp->IsGrenade();
				if (gr!=NULL && gr->active)
				{					
					    level.map.addDanger(temp->pX(),temp->pY(),DANGER_GRENADE);
					
						danger=DANGER_NONE;
						if (temp->properties&TYPE_SENSOR)
							danger |= DANGER_SENSOR;
						if (temp->properties&TYPE_INCENDIARY)
							danger |= DANGER_IGNITION;
						if (temp->properties&TYPE_RADIOACTIVE)
							danger |= DANGER_RADIOACTIVE;
						if (temp->properties&TYPE_CHEM_POISON)
							danger |= DANGER_CHEM_POISON;
						if (temp->properties&TYPE_STUN)
							danger |= DANGER_STUN;
						if (temp->properties&TYPE_PARALYZE)
							danger |= DANGER_PARALYSE;											
					
						if (gr->properties&TYPE_SENSOR)
						{
							// danger tylko na sasiednich polach w kolo
							for (a=0;a<3;a++)
								for (b=0;b<3;b++)
									level.map.addDanger(temp->pX() + a-1,temp->pY() + b-1,danger | DANGER_SENSOR);
						}
						else // active i za chwile wybuchnie!
						{
							// danger dajemy, ze jest na 5 cells w kolo granatu
							for (int a=0;a<10;a++)
								for (int b=0;b<10;b++)
								{
									if (distance(temp->pX(),temp->pY(),temp->pX() + a-5,temp2->pY() + b-5)<=5)
										level.map.addDanger(temp->pX() + a-5,temp->pY() + b-5,danger);
								}
						} // endof if sensor

				} // if active
			} // if is to item widzialny
		} // jezeli na widzianym polu
	}
	
	if (seen_enemies.size()==0)
		enemy = NULL;

	if (seen_enemies.size()>0 && 
	   (actual_behaviour==BEHAVIOUR_CALM || actual_behaviour==BEHAVIOUR_SEARCH_FOR_ENEMY))
		actual_behaviour = BEHAVIOUR_RANGED_COMBAT;		
	
	if (!sees_enemy_again)
	{
		if (seen_enemies.size()>0)
		{
			actual_behaviour=BEHAVIOUR_RANGED_COMBAT;
			enemy = (MONSTER *) *seen_enemies.begin();
			monster_sees_enemy(enemy);
		}
		if (enemy==NULL)
		{
			if (actual_behaviour==BEHAVIOUR_RANGED_COMBAT ||
				actual_behaviour==BEHAVIOUR_H2H_COMBAT)
			actual_behaviour=BEHAVIOUR_SEARCH_FOR_ENEMY;
		}
	}
	else
		monster_sees_enemy(enemy);

	return true;
}

bool INTELLIGENT::load_weapon()
{
	if (AI_restrictions & AI_DONT_USE_RANGED_WEAPONS)
		return false;
	
	// znalezienie pociskow w backpacku pasujacych do tej broni
	ptr_list::iterator m,_m;
	ITEM *temp;
	bool found;
	RANGED_WEAPON *ranged_weapon = weapon->IsShootingWeapon();
	AMMO *am;
	
	found=false;
	if (weapon->property_load_ammo() && ranged_weapon!=NULL)
	{
		if ( !weapon->IsLoaded() ) // gdy weapon nie jest zaladowana
		{
			for(m=this->backpack.begin(),_m=this->backpack.end(); m!=_m; m++)
			{
				temp=(ITEM *)*m;
				am = temp->IsAmmo();
				if (am!=NULL)
				{
					if (am->ammo_type==ranged_weapon->ammo_type)
					{
						found = true;
						break;
					}
				}
			}// endof for
			if (found==true)
			{
				if (ranged_weapon->load_ammo(am)==0)
				{
					if (this->seen_now==true)
						screen.console.add(name + "은 무기에 탄을 장전한다.",3,false);
				}
				return true;
			}              
		} // endof if lp==0
	}
	return false;
}

bool INTELLIGENT::unload_weapon()
{
	if (AI_restrictions & AI_DONT_USE_RANGED_WEAPONS)
		return false;
	
	if (weapon->property_unload_ammo())
	{
		RANGED_WEAPON *ranged_weapon = weapon->IsShootingWeapon();
		if (ranged_weapon!=NULL)
		{
			if (ranged_weapon->unload_ammo(this)==true)
			{
				if (this->seen_now==true)
					screen.console.add(name + "은 " + ranged_weapon->show_name() + "의 탄을 분리한다.\n",3);
				return true;
			}
		}			
	}	
	return false;
}

bool INTELLIGENT::fire_at_enemy()
{
	if (AI_restrictions & AI_DONT_USE_RANGED_WEAPONS)
		return false;
	
	int x,y;

	// jezeli nie mamy strzelby...
	if (!weapon->property_load_ammo())
		return false;

	if (enemy==NULL)
		return false;
	x = enemy->pX();
	y = enemy->pY();

	if (x==-1 || y==-1)
		return false;

	// check, czy jest na linii strzalu
	struct POSITION position[100];
	int cells, index;
	cells=generate_bresenham_line(pX(), pY(), x, y, (POSITION *) position,100);
	bool it_blocks = false;

	for (index=1;index<cells;index++)
	{
		// HIT W POLE
		if (level.map.blockMove(position[index].x,position[index].y))
			it_blocks = true;
	}
	if (!it_blocks)
	{
		if (this->seen_now)
		{
			level.player->stop_repeating();			
			screen.console.add(name + "은 사격한다!",3);								
		}
		if (shoot_into(x,y)==false)
			misses_in_shooting++;
		else
			misses_in_shooting=0;
		
		return true;
	}		
	else
	{
		go_to_direction_of_enemy();
	}
	return false;
}

bool INTELLIGENT::use_healing_item()
{
	if (AI_restrictions & AI_DONT_USE_HEALING)
		return false;
	
	ptr_list::iterator m,_m;
	ITEM *temp;
	PILL *p;
			
	// przejrzenie backpacka i znalezienie itemu leczacego

	for(m=backpack.begin(),_m=backpack.end(); m!=_m; m++)
	{
		temp=(ITEM *)*m;

		if (temp->property_use())
		{
			p = temp->IsPill();
			if (p!=NULL && p->purpose == PURPOSE_INCRASE_HP)
			{
				if (this->seen_now)
					screen.console.add(name + "은 무언가를 먹는다.",2);				
				p->use_on(this);
				return true;
			}
		}
	}

	return false;
}

bool INTELLIGENT::use_remove_poison_item()
{
	if (AI_restrictions & AI_DONT_USE_HEALING)
		return false;

	ptr_list::iterator m,_m;
	ITEM *item;
	PILL *pill;
	
	// przejrzenie backpacka i znalezienie itemu neutralizujacego trucizne
	
	for(m=backpack.begin(),_m=backpack.end(); m!=_m; m++)
	{
		item=(ITEM *)*m;
		
		if (item->property_use())
		{
			pill = item->IsPill();
			if (pill!=NULL)
			{
				if (pill->purpose == PURPOSE_DECRASE_RADIOACTIVE && state[STATE_RADIOACTIVE]>0)
				{
					if (this->seen_now)
						screen.console.add(name + "은 무언가를 먹는다.",2);			
					pill->use_on(this);
					return true;
				}
				if (pill->purpose == PURPOSE_DECRASE_CHEM_POISON && state[STATE_CHEM_POISON]>0)
				{
					if (this->seen_now)
						screen.console.add(name + "은 무언가를 먹는다.",2);			
					pill->use_on(this);
					return true;
				}
			}
		}
	}
	
	return false;
}


bool INTELLIGENT::throw_active_grenade()
{
	if (AI_restrictions & AI_DONT_USE_GRENADES)
		return false;

	// przejrzenie backpacka i znalezienie aktywnego granatu
	ptr_list::iterator m,_m;
	ITEM *temp;
	GRENADE *grenade;

	int x,y;
	
	for(m=backpack.begin(),_m=backpack.end(); m!=_m; m++)
	{
		temp=(ITEM *)*m;
		
		grenade = temp->IsGrenade();
		if (grenade!=NULL && grenade->active)
		{
			if (!(grenade->properties&TYPE_SENSOR))
			{
				if (grenade->properties&TYPE_EXPLOSIVE ||
					grenade->properties&TYPE_RADIOACTIVE ||
					grenade->properties&TYPE_CHEM_POISON ||
					grenade->properties&TYPE_PARALYZE ||
					grenade->properties&TYPE_STUN ||
					grenade->properties&TYPE_INCENDIARY)
				{
				// throw go w odpowiednie place
				// rzuc tez, gdy ma krytycznie malo hp, aby przy trafieniu go nie upuscil
					if (choose_place_to_throw_grenade(x,y)==true || have_critically_low_hp())
#ifdef DEBUG_AI
					set_color(15);
					print_character (x,y,'X');
					wait(200);
#endif						
					if (throw_item_to(x,y,grenade)==true)
						return true;
				}
			}
		}
	}
	
	return false;
}

bool INTELLIGENT::choose_place_to_throw_grenade(int &x,int &y)
{
	list < POSITION > position_list;
	list < POSITION >::iterator m,_m;
	ptr_list::iterator k,_k;
	MONSTER *monst;
	

	// 0. Take cells, where we can throw
	int mx, my;
	POSITION pos;
	POSITION cel;
	
	const int range = 6;

    int r = min (15, fov_radius.GetValue());

	for (mx=pX()-r;mx<pX()+r;mx++)
		for (my=pY()-r;my<pY()+r;my++)
		{
			if (level.map.seen(mx,my) && !level.map.blockMove(mx,my))
			{
				pos.x = mx;
				pos.y = my;
				if (distance(pX(),pY(),mx,my)>=range)
					position_list.push_back(pos);
			}
		}

	// radomize places
	shuffle_list(position_list);

	if (position_list.size()<10) // there are no places
	{
		// ... but we have grenade to throw!
		x = pX();
		y = pY();
		while (distance(x,y,pX(),pY())<range*2)
		{
			x = random(MAPWIDTH);
			y = random(MAPHEIGHT);			
		}
#ifdef DEBUG_AI
		set_color(15);
		print_character (x,y,'X');
		myrefresh();
		wait(1300);
#endif						
		
		return true;		
	}
			
	// 1. Przede wszystkim nie ranimy siebie...

	// 1.2 Usuwamy pola, na ktorych wybuch nas zrani
	int throw_far_away=0;
	if (have_low_hp())
		throw_far_away += 2;
	if (have_critically_low_hp())
		throw_far_away += 2;
	
	bool remove_it;
	for (m=position_list.begin(),_m=position_list.end();m!=_m;)
	{
		remove_it = false;

		pos = *m;
		m++;
		if (distance(pos.x,pos.y,pX(),pY())<range + throw_far_away)
		{
			remove_it = true;
		}
		else
		{
			// 2. ... ani przyjaciol
			
			for (k=seen_friends.begin(),_k=seen_friends.end();k!=_k;k++)
			{
				monst = (MONSTER *) *k;
				if (distance(pos.x,pos.y,monst->pX(),monst->pY())< range )
					remove_it = true;
			}
		}
		if (remove_it)
		{
			position_list.remove(pos);
			continue;
		}
		
	}
	// mamy teraz niegrozne dla nas miejsca do rzucenia

	// 3. Za to rzucamy we of enemy... (wybieramy pole najblizej niego)

	int lowest_distance = 1000;
	int dist;

	struct POSITION position[100];
	int cells, i;
	bool it_blocks = false;	

	if (level.map.seen(last_x_of_enemy,last_y_of_enemy)) // jezeli moze rzucic w ostatnie pole, gdzie widzial of enemy, niech probuje (player mogl sie schowac za rog)
	{
		for (m=position_list.begin(),_m=position_list.end();m!=_m;m++)
		{
			pos = *m;
			dist=distance(pos.x,pos.y,last_x_of_enemy,last_y_of_enemy);
			if (dist<range)
			{
				if (dist<lowest_distance)
				{
					// check, czy z tego pola zostanie trafiony wybuchem
					// aby nie rzucal do innego pomieszczenia
					cells=generate_bresenham_line(pos.x, pos.y, last_x_of_enemy, last_y_of_enemy, (POSITION *) position, range);
					it_blocks = false;
					
					for (i=1;i<cells;i++)
					{
						// HIT W POLE
						if (level.map.blockMove(position[i].x,position[i].y))
							it_blocks = true;
					}
					if (!it_blocks)
					{
						lowest_distance = dist;
						cel = pos;
					}						
				}
				
			}
			
		}		
	}
	// 4. ... a jezeli not in niego, to w reszte enemyow
	
	if (lowest_distance!=1000) // wybrano cel - of enemy
	{
		x = cel.x;
		y = cel.y;
		return true;
	}
	else
	{
		lowest_distance = 1000;
		for (k=seen_enemies.begin(),_k=seen_enemies.end();k!=_k;k++)
		{
			monst = (MONSTER *) *k;
			for (m=position_list.begin(),_m=position_list.end();m!=_m;m++)
			{
				pos = *m;
				dist=distance(pos.x,pos.y,monst->pX(),monst->pY());
				if (dist<range)
				{
					// check, czy z tego pola zostanie trafiony wybuchem
					// aby nie rzucal do innego pomieszczenia
					cells=generate_bresenham_line(pos.x, pos.y, last_x_of_enemy, last_y_of_enemy, (POSITION *) position, range);
					it_blocks = false;
					
					for (i=1;i<cells;i++)
					{
						// HIT W POLE
						if (level.map.blockMove(position[i].x,position[i].y))
							it_blocks = true;
					}
					if (!it_blocks)
					{
						lowest_distance = dist;
						cel = pos;
					}						
				} // endof if
				
			}	// endof for
		} // endof for
	} // endof else
	
	if (lowest_distance!=1000) // wybrano cel - innego of enemy
	{
		x = cel.x;
		y = cel.y;

#ifdef DEBUG_AI
		set_color(15);
		print_character (x,y,'X');
		myrefresh();
		wait(1300);
#endif						
		
		return true;
	}
	
	// 5. Gdy zadne z powyzszych, to po prostu rzucamy go jak najdalej od siebie.
	lowest_distance = 0;

	for (m=position_list.begin(),_m=position_list.end();m!=_m;m++)
	{
		pos = *m;
		dist=distance(pos.x,pos.y,pX(),pY());
		if (dist>lowest_distance) // szukamy najdalszego pola
		{
			lowest_distance = dist;
			cel = pos;
		}		
	}		
	
	if (lowest_distance != 0) 
	{
		x = cel.x;
		y = cel.y;

#ifdef DEBUG_AI
		set_color(15);
		print_character (x,y,'X');
		myrefresh();
		wait(1300);
#endif						
		
		return false;
	}
	else // nie found pola, rzucamy w najdalsze widziane
	{
		int highest_distance;
		highest_distance = 0;
		for (mx=pX()-r;mx<pX()+r;mx++)
			for (my=pY()-r;my<pY()+r;my++)
			{
				if (level.map.seen(mx,my))
				{
					dist=distance(mx,my,pX(),pY());
					if (dist>highest_distance)
					{
						highest_distance = dist;
						x = mx;
						y = my;
					}
				}
			}						
	}

#ifdef DEBUG_AI
	set_color(15);
	print_character (x,y,'X');
	myrefresh();
	wait(1300);
#endif							

	return false;
}

bool INTELLIGENT::tell_others_about_enemy()
{
	ptr_list::iterator m,_m;
	MONSTER *temp;
	if (AI_restrictions & AI_DONT_COOPERATE_WITH_FRIENDS) // jezeli wspolworks_now
		return false;
	if (actual_behaviour==BEHAVIOUR_CALM)
		return false;
					
	for (m=seen_friends.begin(),_m=seen_friends.end();m!=_m;m++)
	{
		temp = (MONSTER *) *m;
		if (temp!=level.player && temp->is_intelligent()==true)
		{
			INTELLIGENT *in = dynamic_cast <INTELLIGENT *> (temp);
			if (in!=NULL)
			{
				if (!(in->AI_restrictions & AI_DONT_COOPERATE_WITH_FRIENDS)) // jezeli wspolworks_now
				{
					if (in->actual_behaviour == BEHAVIOUR_CALM ||
						(in->actual_behaviour == BEHAVIOUR_SEARCH_FOR_ENEMY && in->enemy==NULL))
					{
						if (in->enemy_last_seen_in_turn+10 < this->enemy_last_seen_in_turn)
						{
							in->actual_behaviour = BEHAVIOUR_SEARCH_FOR_ENEMY;
							in->last_x_of_enemy = this->last_x_of_enemy;
							in->last_y_of_enemy = this->last_y_of_enemy;
							in->last_direction_of_enemy=this->last_direction_of_enemy;
							in->is_at_enemy_position=false;
							in->turns_of_searching_for_enemy = 0;							
							in->enemy_last_seen_in_turn = this->enemy_last_seen_in_turn;
#ifdef DEBUG_AI
							// OR SHOUT random like "there he is!"
							screen.console.add("Follow my enemy!",7);
#endif
						}
					}
				}
			}
		}
	}

	return true;
}

bool INTELLIGENT::use_booster_item()
{
	if (AI_restrictions & AI_DONT_USE_BOOSTERS)
		return false;

	ptr_list::iterator m,_m;
	ITEM *item;
	PILL *pill;
	
	// przejrzenie backpacka i znalezienie itemu typu booster
	
	for(m=backpack.begin(),_m=backpack.end(); m!=_m; m++)
	{
		item=(ITEM *)*m;
		
		if (item->property_use())
		{
			pill = item->IsPill();
			if (pill!=NULL)
			{
				if (pill->purpose == PURPOSE_INCRASE_SPEED ||
					pill->purpose == PURPOSE_INCRASE_STRENGTH ||
					pill->purpose == PURPOSE_INCRASE_ENDURANCE ||
					pill->purpose == PURPOSE_INCRASE_METABOLISM)
				{
					if (this->seen_now)
						screen.console.add(name + "은 무언가를 먹는다.",2);			
					pill->use_on(this);
					return true;
				}
			}
		}
	}
	
	return false;
}

bool INTELLIGENT::change_armor_to_better_one()
{	
	if (AI_restrictions & AI_DONT_USE_OTHER_ARMORS)
		return false;

	// przejrzenie backpacka
	ptr_list::iterator m,_m;
	ITEM *temp, *best;
	int best_value;

	best = armor;
	best_value = armor->evaluate_item();
	
	for(m=backpack.begin(),_m=backpack.end(); m!=_m; m++)
	{
		temp=(ITEM *)*m;
		if (temp->IsArmor())
		{
			int value = temp->evaluate_item();
			if (value > best_value)
			{
				best_value = value;
				best = temp;
			}
		}
	}

	if (best != armor)
	{
		set_armor(best);
		return true;
	}
	return false;
		
}	

bool INTELLIGENT::change_weapon_to_better_one()
{	
	if (AI_restrictions & AI_DONT_USE_OTHER_WEAPONS)
		return false;

	// przejrzenie backpacka
	ptr_list::iterator m,_m;
	ITEM *best_weapon;
	ITEM *best_for_h2h;
	ITEM *best_for_ranged;

	int best_evaluation_h2h;
	int best_evaluation_ranged;

	best_weapon = weapon;
	
	find_best_weapons(best_for_h2h,best_evaluation_h2h, best_for_ranged, best_evaluation_ranged);

	// mamy najlepsze bronie, decydujemy ktora wybrac
	if (actual_behaviour == BEHAVIOUR_H2H_COMBAT) // jezeli walczymy w zwarciu, to taka jest lepsza
		best_evaluation_h2h *= 5;

	if (actual_behaviour == BEHAVIOUR_RANGED_COMBAT) // jezeli walczymy w zwarciu, to taka jest lepsza
		best_evaluation_ranged *= 5;

	if (best_evaluation_h2h > best_evaluation_ranged)
		best_weapon = best_for_h2h;
	else if (best_evaluation_h2h < best_evaluation_ranged)
		best_weapon = best_for_ranged;
	
	if (best_weapon!=weapon)
	{
		if (actual_behaviour==BEHAVIOUR_RANGED_COMBAT && best_weapon == best_for_h2h)
			actual_behaviour=BEHAVIOUR_H2H_COMBAT; // aby nie probowal z niej strzelac
		else if (actual_behaviour==BEHAVIOUR_H2H_COMBAT && best_weapon == best_for_ranged)
			actual_behaviour=BEHAVIOUR_RANGED_COMBAT; // jakims cudem ma pociski do broni - moze weapon sie zaladowala?
		
		set_weapon(best_weapon);
		return true;
	}

	if (actual_behaviour==BEHAVIOUR_RANGED_COMBAT && best_weapon == best_for_h2h)
		actual_behaviour=BEHAVIOUR_H2H_COMBAT; // aby nie probowal z niej strzelac	

	return false;
}

int INTELLIGENT::activate_weapon_and_armor_when_energy_available()
{
	ptr_list::iterator m,_m;
	ITEM *temp;
	HAND_WEAPON *hand_weapon;
	RANGED_WEAPON *ranged_weapon;
	BASE_ARMOR *arm;
	bool found=false;
	
	for(m=backpack.begin(),_m=backpack.end(); m!=_m; m++)
	{
		temp=(ITEM *)*m;
		
		hand_weapon = temp->IsHandWeapon();
		ranged_weapon = temp->IsShootingWeapon();
		arm = temp->IsArmor();
		if (hand_weapon!=NULL && hand_weapon->energy_activated==false)
		{
			if (hand_weapon->activate()==true)
				break;
		}
		if (ranged_weapon!=NULL && ranged_weapon->energy_activated==false)
		{
			if (ranged_weapon->activate()==true)
				break;
		}
		if (seen_enemies.size()>0) // turn on armor
		{
			if (arm!=NULL && arm->energy_activated==false)
			{
				if (arm->activate()==true)
					break;
			}			
		}
		else if (seen_enemies.size()==0) // turn off armor
		{
			if (arm!=NULL && arm->energy_activated==true)
			{
				if (arm->activate()==true)
					break;
			}			
		}
		
	}	
	// gdy cos aktywowal
	if (m!=_m && this->seen_now==true)
	{
		screen.console.add(name + "은 자신의" + temp->show_name() + "을 작동시킨다",12);
		return true;
	}	
	return 0;
}

bool INTELLIGENT::activate_grenade_when_enemy_is_far()
{
	if (AI_restrictions & AI_DONT_USE_GRENADES)
		return false;
	
	ptr_list::iterator m,_m;
	ITEM *item;
	GRENADE *grenade;
	bool found=false;
	
	int x,y;

	if (seen_enemies.size()==0)
		return false;

	if (enemy==NULL)
		return false;

	if (!level.map.seen(last_x_of_enemy,last_y_of_enemy))
		return false;

	if (distance(pX(),pY(),last_x_of_enemy,last_y_of_enemy)<6)
		return false;
	
	for(m=backpack.begin(),_m=backpack.end(); m!=_m; m++)
	{
		item=(ITEM *)*m;
		
		grenade = item->IsGrenade();
		if (grenade!=NULL)
		{
			if (grenade->active) // already have activated
				break;

			if (random(3)==0) // not the first one
				if (!(grenade->properties&TYPE_SENSOR)) // don't throw mines
					if (grenade->properties&TYPE_EXPLOSIVE ||
						grenade->properties&TYPE_RADIOACTIVE ||
						grenade->properties&TYPE_CHEM_POISON ||
						grenade->properties&TYPE_PARALYZE ||
						grenade->properties&TYPE_STUN ||
						grenade->properties&TYPE_INCENDIARY)
					{
						found = true;
						break;
					}
		}
	}
	
	if (found==true && choose_place_to_throw_grenade(x,y)==true)
	{
#ifdef DEBUG_AI
		set_color(15);
		print_character (x,y,'X');
		wait(200);
#endif								
		grenade->activate();
		return true;
	}

	return false;
}

int INTELLIGENT::carried_number_of_missiles_for_weapon(RANGED_WEAPON *ranged_weapon)
{
	ptr_list::iterator m,_m;
	ITEM *item;
	bool found;
	AMMO *am;

	if (ranged_weapon==NULL)
		return 0;
	
	found=false;

	int ammo_count_carried;

	ammo_count_carried = ranged_weapon->ammo.quantity; // pociski w broni
	// + in backpack
	for(m=this->backpack.begin(),_m=this->backpack.end(); m!=_m; m++)
	{
		item=(ITEM *)*m;
		am = item->IsAmmo();
		if (am!=NULL && ranged_weapon->ammo_type==am->ammo_type)
		{
			ammo_count_carried += am->quantity;
		}
	}// endof for
	return ammo_count_carried;
}

bool INTELLIGENT::death()
{
	ITEM *item;
	// drop items
	while (backpack.size()>0)
	{
		item=(ITEM *) *backpack.begin();
		take_out_from_backpack(item);
	}// endof for

#ifdef DEBUG_AI
	if (this->seen_now)
		screen.console.add("Aaaaarrrrghhhh....",15);
#endif
	
	return MONSTER::death();	
}

ITEM * INTELLIGENT::go_to_valuable_item()
{
	if (AI_restrictions & AI_DONT_MOVE ||
		AI_restrictions & AI_DONT_PICK_UP_ITEMS)
		return NULL;
	
	ptr_list::iterator m,_m;
	ITEM *item;
	ITEM *best_for_h2h;
	ITEM *best_for_ranged;
	int highest_eval_h2h, highest_eval_ranged;

	find_best_weapons(best_for_h2h,highest_eval_h2h,best_for_ranged,highest_eval_ranged);
	
	item = NULL;
	bool pick_up = false;
	
	// look seen items
	for (m=seen_items.begin(),_m=seen_items.end();m!=_m;m++)
	{
		item = (ITEM *) *m;
		if (item->IsHandWeapon()!=NULL) 
		{
			if (skill[SKILL_MELEE_WEAPONS]/2 * item->evaluate_item_for_h2h_combat() > highest_eval_h2h)
				pick_up = true;
		}
		else if (item->IsShootingWeapon()!=NULL) 
		{
			if (!(AI_restrictions & AI_DONT_USE_RANGED_WEAPONS))
				if (skill[item->skill_to_use]/2 * item->evaluate_item_for_ranged_combat() > highest_eval_ranged)
					pick_up = true;
		}
		else if (item->IsPill()!=NULL) 
		{
			if (!(AI_restrictions & AI_DONT_USE_HEALING))
				if (item->evaluate_item()>0)
					pick_up = true;
		}
		else if (item->IsArmor()!=NULL)
		{
			if (!(AI_restrictions & AI_DONT_USE_OTHER_ARMORS))
				if (item->evaluate_item()>armor->evaluate_item())
					pick_up = true;
		}
		else if (item->IsAmmo()!=NULL) 
		{
			if (!(AI_restrictions & AI_DONT_USE_RANGED_WEAPONS))
				if (item->evaluate_item()>0)
					pick_up = true;
		}
		else if (item->IsGrenade()!=NULL)
		{
			if (!(AI_restrictions & AI_DONT_USE_GRENADES))
			{
				if (item->evaluate_item()>0)
					pick_up = true;
			}
		}
		else if (item->IsBattery()!=NULL)
		{
			if (item->evaluate_item()>0)
				pick_up = true;
		}
		if (pick_up==true)
		{
			if (can_pick_up(item)==0)
			{
				if (set_direction_to_cell_by_shortest_path(item->pX(), item->pY(), true))
				{
					chosen_action = ACTION_MOVE;
					return item;
				}
			}
			else
				pick_up = false;
		}
	}
	
	return NULL;
}

int INTELLIGENT::pick_valuable_items()
{
	ITEM *to_pick_up;
	to_pick_up = go_to_valuable_item();
	if (to_pick_up!=NULL) // idzie do jakiegos
	{
					int tempx,tempy;
					
#ifdef DEBUG_AI
					screen.console.add(string("I'm going to get ") + to_pick_up->show_name(),14);																	
#endif
					turns_of_calm = 51; // aby nie przestal do niego isc
					if (to_pick_up->pX() == pX() && to_pick_up->pY() == pY())
					{
						if (pick_up_item(to_pick_up,true)==true)
						{
#ifdef DEBUG_AI
							screen.console.add(string("I'm picking up ") + to_pick_up->show_name(),13);																	
#endif
							turns_of_calm = 0; // aby go wzial do lapy/ubral
							return true;
						}
					}
					cell_in_current_direction(tempx,tempy);
					go_around_cell(tempx,tempy,true);						
					return 2;
	} // endof do podniesienia != NULL
	return false;
}

bool INTELLIGENT::find_best_weapons(ITEM *&best_for_h2h, int &best_evaluation_h2h, ITEM *&best_for_ranged, int &best_evaluation_ranged)
{
	ptr_list::iterator m,_m;
	ITEM *item;
	RANGED_WEAPON *ranged_weapon;
	HAND_WEAPON *hand_weapon;

	int evaluation_h2h=0;
	int evaluation_ranged=0;
	
	best_for_h2h  = &unarmed;
	best_evaluation_h2h = skill[unarmed.skill_to_use]/2 * unarmed.evaluate_item_for_h2h_combat();
	
	best_for_ranged = NULL;
	best_evaluation_ranged = 0;	
	
	for(m=backpack.begin(),_m=backpack.end(); m!=_m; m++)
	{
		item=(ITEM *)*m;
		// dla przyspieszenia wyceniamy tylko bronie
		ranged_weapon = item->IsShootingWeapon();
		hand_weapon = item->IsHandWeapon();
		
		if (hand_weapon!=NULL || ranged_weapon!=NULL)
			evaluation_h2h = skill[SKILL_MELEE_WEAPONS]/2 * item->evaluate_item_for_h2h_combat();
		else
			evaluation_h2h=0;
		
		if (ranged_weapon!=NULL)
			evaluation_ranged = skill[item->skill_to_use]/2  * item->evaluate_item_for_ranged_combat();
		else
			evaluation_ranged=0;
		
		if (evaluation_ranged>0)
		{
			if (ranged_weapon!=NULL && carried_number_of_missiles_for_weapon(ranged_weapon)==0)
				evaluation_ranged = 0;
		}
		
		if (evaluation_h2h>best_evaluation_h2h)
		{
			best_for_h2h = item;
			best_evaluation_h2h=evaluation_h2h;
		}
		
		if (evaluation_ranged>best_evaluation_ranged)
		{
			best_for_ranged = item;
			best_evaluation_ranged=evaluation_ranged;
		}
		
	}	// endof for	
	return true;	
}

// przed tym trzeba KONIECZNIE wywolac start_to_run_away dla ustawienia run_away_to_x,y na potworka
bool INTELLIGENT::choose_run_away_direction()
{
	int x,y;
	int a,b,angle;

 	for (int prob=0;prob<20 && ((pX()==run_away_to_x && pY()==run_away_to_y) || how_danger_is_cell(run_away_to_x,run_away_to_y)>0);prob++)
	{
			chosen_action = ACTION_MOVE;
			// znalezienie nowego pola, gdzie uciekamy
			for (a=MAPWIDTH*50;a>0;a--)
			{
				angle = random(360);
				b = (int) ((double) a * sin((float) angle))/50;
				x = pX()+b;

				b = (int) ((double) a * cos((float) angle))/50;
				if (coin_toss())
					b=-b;
				y = pY()+b;

				if (!level.map.onMap(x,y))
					continue;
				if (level.map.seen(x,y)) // idzemy tylko do pola, ktorego nie widac
					continue;
				if (level.map.blockMove(x,y)==false)
				{
					run_away_to_x = x;
					run_away_to_y = y;
					break;
				}
		}
	} // endof prob	
	if (!set_direction_to_cell_by_shortest_path(run_away_to_x,run_away_to_y,true))
	{
		run_away_to_x = pX();
		run_away_to_y = pY();
		direction_to_near_cell(last_x_of_enemy,last_y_of_enemy); // run away from enemy
		direction += 4;
		if (direction>7)
			direction-=8;
	}
	
	return true;
}

bool INTELLIGENT::use_defensive_techniques_running_away()
{
	// like mines, closing doors
	return false;
}

void INTELLIGENT::start_to_run_away()
{
	if (actual_behaviour!=BEHAVIOUR_RUN_AWAY)
	{
		run_away_to_x = pX();
		run_away_to_y = pY();
		actual_behaviour = BEHAVIOUR_RUN_AWAY;
	}
}

int INTELLIGENT::how_danger_is_cell(int x,int y)
{
	ptr_list::iterator m,_m;
	GAS *gas;
	int how_danger=0;

	// jezeli go nie widzimy, to jest bezpieczne
	if (level.map.seen(x,y)==false)
	{
#ifdef DEBUG_AI
		print_character(x,y,'?');
#endif
		return 0;
	}

	// pobranie widzianego niebezpieczenstwa
	int danger = level.map.getDanger(x,y);

	int fire_density=0, radioactive_density=0; 
	int density_chem_poison=0, density_paralyze=0, density_stun=0;
	
	// wybuchy sa zawsze niebezpieczne
	if (danger & DANGER_EXPLOSION)
	{
#ifdef DEBUG_AI
		print_character(x,y,'B');
		myrefresh();
		wait(500);
#endif		
		how_danger+=10;
	}

	if (danger & DANGER_IGNITION)
		how_danger+=10*(100-resist[RESIST_FIRE])/100;
	
	if (actual_behaviour==BEHAVIOUR_RUN_AWAY) // nie przechodzimy obok of enemy uciekajac
	{
		if (danger & DANGER_ENEMY)
		{
#ifdef DEBUG_AI
			print_character(x,y,'W');
			myrefresh();
			wait(500);
#endif					
			how_danger+=10;
		}
	}

	// pobranie stezen gazu i ognia z danego pola
	int gases = level.map.GetNumberOfGases(x,y);
	int checked=0;
	for(m=level.gases_on_map.begin(),_m=level.gases_on_map.end(); m!=_m && checked<gases; m++)
	{
		gas=(GAS *)*m;
		if (gas->pX()==x && gas->pY()==y) // jest na tym polu
		{
			checked++;
			if (gas->properties&TYPE_INCENDIARY)
				fire_density+=gas->density;
			if (gas->properties&TYPE_RADIOACTIVE)
				radioactive_density+=gas->density;
			if (gas->properties&TYPE_CHEM_POISON)
				density_chem_poison+=gas->density;
			if (gas->properties&TYPE_PARALYZE)
				density_paralyze+=gas->density;
			if (gas->properties&TYPE_STUN)
				density_stun+=gas->density;
		}
	}
			
	// check if we afraid of fire
	if (fire_density-4 > resist[RESIST_FIRE]/2)
		if (fire_density>4 && (!(armor->properties&TYPE_INCENDIARY)))
			how_danger+=(fire_density*(100-resist[RESIST_FIRE])/100);
	// check if we afraid of radioactivity
	if (radioactive_density-4 > resist[RESIST_RADIOACTIVE]/2)
		if ((!(armor->properties&TYPE_RADIOACTIVE)))
			how_danger+=(radioactive_density*(100-resist[RESIST_RADIOACTIVE])/300);
	if (density_chem_poison-4 > resist[RESIST_CHEM_POISON]/2)
		if ((!(armor->properties&TYPE_CHEM_POISON)))
			how_danger+=(density_chem_poison*(100-resist[RESIST_CHEM_POISON])/300);
	if (density_paralyze-4 > resist[RESIST_PARALYZE]/2)
		if ((!(armor->properties&TYPE_PARALYZE)))
			how_danger+=(density_paralyze*(100-resist[RESIST_PARALYZE])/300);
	if (density_stun-4 > resist[RESIST_STUN]/2)
		if ((!(armor->properties&TYPE_STUN)))
			how_danger+=(density_stun*(100-resist[RESIST_STUN])/300);
					
	return how_danger;
}

int ANIMAL::how_danger_is_cell(int x,int y)
{
	ptr_list::iterator m,_m;
	GAS *temp;

	int danger = level.map.getDanger(x,y);

	// boimi sie of enemy, gdy uciekamy
	
	if (actual_behaviour==BEHAVIOUR_RUN_AWAY) // nie przechodzimy obok of enemy uciekajac
	{
		if (danger & DANGER_ENEMY)
			return 15;
	}	
	
	// pobranie stezen gazu i ognia z danego pola

	if (level.map.GetNumberOfGases(x,y)!=0)
	{
		for(m=level.gases_on_map.begin(),_m=level.gases_on_map.end(); m!=_m; m++)
		{
			temp=(GAS *)*m;
			if (temp->pX()==x && temp->pY()==y) // jest na tym polu
			{
				// czy to taki gaz? (sprawdzamy tylko 1)
				int gases=0;
				if (temp->properties&TYPE_INCENDIARY)
					gases+=5;
				if (temp->density>5)
				{
					if (temp->properties&TYPE_RADIOACTIVE)
						gases+=5;
					if (temp->properties&TYPE_CHEM_POISON)
						gases+=5;
					if (temp->properties&TYPE_PARALYZE)
						gases+=5;
					if (temp->properties&TYPE_STUN)
						gases+=5;
				}
				if (gases>=5)
					return gases;
			}
		}	
	}
	return 0;
}


bool INTELLIGENT::defend_self_from_danger_on_map()
{
	int danger;
	ptr_list::iterator m,_m;
	ITEM *item;
	GRENADE *grenade;	

	int old_direction = direction;
	int x,y;
	bool found_run_away_direction = false;
	
	danger = level.map.getDanger(pX(),pY());

	run_away_to_x=pX();
	run_away_to_y=pY();
	if (choose_run_away_direction()==true)
	{
		found_run_away_direction = true;
		cell_in_current_direction(x,y);
		if (how_danger_is_cell(x,y)<how_danger_is_cell(pX(),pY()))
			return true;
	}

	// if the grenade in on the cell where are we, it will damage us, do don't activate shield
	if (!(danger&DANGER_GRENADE) &&
		(danger&DANGER_EXPLOSION ||
		danger&DANGER_IGNITION ||
		danger&DANGER_STUN))
	{
		// find shield
		for(m=backpack.begin(),_m=backpack.end(); m!=_m; m++)
		{
			item=(ITEM *)*m;
			grenade = item->IsGrenade();
			if (grenade!=NULL && grenade->properties&TYPE_POWER_SHIELD && grenade->DLY<2)
			{
					if (grenade->activate()==true)
					{
						chosen_action = ACTION_INVENTORY;
						return true;
					}
			}
		}// endof for
	}
	if (!found_run_away_direction)
	{
		//// jezeli nie, to uciekamy losowo!
		
		direction=random(8);
		chosen_action = ACTION_MOVE;
	}
	return true;
}

void MONSTER::ShowDescription()
{
	descriptions.show_description(name);	
}

ROBOT::ROBOT()
{
	ClassName = "ROBOT";	
	shell=NULL;
	weapon=NULL;
	armor=NULL;

	last_x_of_creator=-1;
	last_y_of_creator=-1;

	sees_creator=false; 
	Creator_ID=0;	

	resist[RESIST_RADIOACTIVE]=100;
	resist[RESIST_CHEM_POISON]=100;
	resist[RESIST_STUN]=100;
	resist[RESIST_PARALYZE]=100;
	resist[RESIST_FIRE]=100;
	resist[RESIST_BLIND]=100;

	resist[RESIST_ELECTRICITY]=0;
}

int ROBOT::get_robot_speed()
{
	int spd;

	int move_power=0;

	ptr_list::iterator m,_m;
	ITEM *item;
	ROBOT_LEG *leg;

	for (m=shell->move_slots.begin(),_m=shell->move_slots.end();m!=_m;m++)
	{
		item = (ITEM *) *m;
		leg = item->IsRobotLeg();
		if (leg!=NULL)
		{
			move_power+=leg->move_power;
		}
	}

	spd = move_power - shell->calculate_weight()/4000;
	int divider = (shell->move_slots.size());

	if (divider!=0)
		spd = spd / divider;		

	if (spd<1)
		spd=1;
	return spd;
}
////////////////////////////////////////////////////////////////////////// 

// Wykonanie pobranej akcji

TIME ROBOT::do_action( ACTION action )
{
	last_pX = pX();
	last_pY = pY();

	if (sees_creator)
	{
		if (have_critically_low_hp() && random(3)==0)
		{
			screen.console.add(name + "은 보고한다: '본 유닛의 " + shell->show_name() + "가 파괴되기 직전입니다!'",15);
		}
		else if (have_low_hp() && random(6)==0)
		{
			screen.console.add(name + "은 보고한다: '본 유닛의 " + shell->show_name() + "가 심각한 손상을 입었습니다!'",15);
		}
			
	}
				
	
	switch (action)
	{
	case ACTION_MOVE:
		int x,y;
		cell_in_current_direction(x,y);
		
		if (shell->move_slots.size()==0)
		{
			if (level.map.IsMonsterOnMap(x,y))
				attack_in_direction();

			return (TIME_MOVE/shell->cpu->frequency)+1;
		}

		if (level.map.blockMove(x,y))
			go_around_cell(x,y,false);
		
		if (move()==1) // action ruch
			return (TIME_MOVE/get_robot_speed())+1;
		else // np. attack
			return (TIME_MOVE/shell->cpu->frequency)+1;

	default: // the rest depends on CPU speed
		return (TIME_MOVE/shell->cpu->frequency)+1;

	}
	assert(0); // tak byc nie powinno
	return 0;
}

bool ROBOT::look_around()
{
	ptr_list::iterator m,_m;
	MONSTER *monster;
	ITEM *item;

	bool sees_enemy_again=false;
	
	sees_creator = false;	
	
	seen_enemies.clear();
	seen_friends.clear();
	
	level.fov.Start(&level.map,pX(),pY(),fov_radius.GetValue());
	MONSTER::look_around(); // handle cameras and searchlights

	// utility bot

	if (shell->cpu->name=="유틸리티봇 프로세서")
	{
		for(m=level.items_on_map.begin(),_m=level.items_on_map.end(); m!=_m; m++)
		{
			item = (ITEM *) *m;
		
			if (level.map.seen_by_camera(item->pX(),item->pY()) || (level.map.seen(item->pX(),item->pY()) && distance(item->pX(),item->pY(),pX(),pY())<fov_radius.GetValue()))
			{
					if (item->IsCorpse() || item->IsGarbage())
					{
						last_x_of_enemy=item->pX();
						last_y_of_enemy=item->pY();
						return true;
					}
			}
				
		} // endof for
		return true;
	}

	
	// ptr_list of enemies and friends
	
	for(m=level.monsters.begin(),_m=level.monsters.end(); m!=_m; m++)
	{
		monster=(MONSTER *)*m;
		if (monster==this)
			continue;
		
		if (level.map.seen_by_camera(monster->pX(),monster->pY()) || (level.map.seen(monster->pX(),monster->pY()) && distance(monster->pX(),monster->pY(),pX(),pY())<fov_radius.GetValue()))
		{
			assert(monster!=NULL);

			if (monster->UniqueNumber == Creator_ID)
			{
				last_x_of_creator = monster->pX();
				last_y_of_creator = monster->pY();
				sees_creator = true;
			}

			if (is_friendly(monster)) // przyjaciel
				seen_friends.push_back(monster);
			else if (is_enemy(monster)) // enemy
			{
				seen_enemies.push_back(monster);
				if (monster==enemy)
					sees_enemy_again=true;
			} // w rangeu wzroku
			// neutralnych ignorujemy.
		}
	}
	
	if (seen_enemies.size()==0)
		enemy = NULL;
	
	if (!sees_enemy_again)
	{
		if (seen_enemies.size()>0)
		{
			enemy = (MONSTER *) *seen_enemies.begin();
			monster_sees_enemy(enemy);
		}
	}
	else
		monster_sees_enemy(enemy);
	
	return true;
}

bool ROBOT::can_weapon_shoot()
{
	ptr_list::iterator m,_m;
	
	ITEM *item;
	RANGED_WEAPON *ranged_weapon;
	
	m=shell->action_slots.begin();
	_m=shell->action_slots.end();
	
	for (;m!=_m;m++)
	{
		item = (ITEM *) *m;
		ranged_weapon = item->IsShootingWeapon();
		if (ranged_weapon!=NULL)
		{
			if (ranged_weapon->ammo.quantity>0)
				return true;
		}
	}
	return false;	
}


bool ROBOT::load_ammo_to_weapon()
{
	ptr_list::iterator m,_m;
	ptr_list::iterator n,_n;
	
	ITEM *temp;
	RANGED_WEAPON *ranged_weapon;
	AMMO *ammo;

	bool zaladowano=false;

	m=shell->action_slots.begin();
	_m=shell->action_slots.end();

	while(m!=_m)
	{
		m=shell->action_slots.begin();
		_m=shell->action_slots.end();
		
		for (;m!=_m;m++)
		{
			temp = (ITEM *) *m;
			ranged_weapon = temp->IsShootingWeapon();
			if (ranged_weapon!=NULL)
			{
				if (ranged_weapon->ammo.quantity==0)
				{
					// znalezienie amunicji do tej broni
					n=shell->action_slots.begin();
					_n=shell->action_slots.end();
					
					for (;n!=_n;n++)
					{
						temp = (ITEM *) *n;
						ammo = temp->IsAmmo();
						if (ammo!=NULL)
						{
							if (ranged_weapon->load_ammo(ammo)==0)
							{
								zaladowano=true;
								break;
							}
						}
					}
				} // endof if ranged_weapon->ammo
			}
		} // endof for
	}	
	return zaladowano;
}

bool ROBOT::fire_grenades()
{
	ptr_list::iterator m,_m;

	bool grenades_activated=false;
	ITEM *item;

	ptr_list to_activate;

	m=shell->action_slots.begin();
	_m=shell->action_slots.end();
	
	for (;m!=_m;m++)
	{
		item = (ITEM *) *m;
		if (item->IsGrenade())
		{
			to_activate.push_back(item);
			grenades_activated = true;
		}
	}

	m=to_activate.begin();
	_m=to_activate.end();

	for (;m!=_m;m++)	
	{
		item = (ITEM *) *m;

		shell->uninstall_from_action_slot(item);
		item->activate();
		
		if (!item->IsDead())
		{
			item->hit_points=1;
			item->damage_it(1); // starczy, bo ma 1 hit_points
		}
	}

	return grenades_activated;
}

// CPU PROGRAMMING
// Program is a string of characters.
// Program is executed lineary every time from the beginning, on every get_action for robot.
// Executed is the first non-conditional activity
// There are two types of instructions: conditional and non-conditional
// Conditional instructions - when condition is fulfiled, CPU executes next instruction
//  	      		     when condition is not fulfiled, CPU skip the next non-conditional instruction
//			     (instruction pointer moves to the position after the next non-conditional)
// Available instructions:
//
// Conditional
// 0 - If I see the enemy
// 1 - If I stand on position where you last saw enemy
// 2 - If I stand on just by position where you last saw enemy
// 3 - If I are >4 cells from last seen enemy position
// 4 - If I are >8 cells from last seen enemy position
// 5 - If my weapon has ammo
// 6 - If I see my creator
// 7 - Coin toss (50% chance)
// 8 - If I can move
// ! - Negate next condition
//
// Non-conditional
// a - go to the last seen creator's position
// b - attack enemy by range weapon
// c - go to last seen enemy position
// d - search for an enemy
// e - go to nearest TRASH and utility it (treat trash an enemy) - for Utility Robot
// f - activate grenades by activating and damaging it
// g -
// h -
// i -
// j - //not implemented - go to the last position of friend
// k - //alarm
// l - //run away from enemy
// m - load ammo to the weapon
// n - wait

// CPU interpreter

char ROBOT::get_CPU_instruction()
{
	char instruction;
	bool is_negated=false;
	bool go_to_next_after_conditionals=false;
	bool result;

	int program_size = shell->cpu->program.size();

	ITEM *item;
	ptr_list items_on_cell;	
	ptr_list::iterator m,_m;

	look_around();

	instruction=0;
	int index;
	for (int instruction_number=0;instruction_number<program_size;instruction_number++)
	{
		instruction = shell->cpu->program[instruction_number];
		switch(instruction) {
		case '!':
			is_negated=true;
			continue;
		case '0':
			result = enemy!=NULL;
			break;
		case '1': // If I stand on position where you last saw enemy
			if (pX()==last_x_of_enemy && pY()==last_y_of_enemy)
				result = true;
			else
				result = false;
			break;
		case '2': // If I stand on just by position where you last saw enemy
			if (abs(pX()-last_x_of_enemy)<=1 && abs(pY()-last_y_of_enemy)<=1)
				result = true;
			else
				result = false;			
			break;			
		case '3': // If I are >4 cells from last seen enemy position
			if (abs(pX()-last_x_of_enemy)>4 || abs(pY()-last_y_of_enemy)>4)
				result = true;
			else
				result = false;			
			break;			
		case '4': // If I are >8 cells from last seen enemy position
			if (abs(pX()-last_x_of_enemy)>8 || abs(pY()-last_y_of_enemy)>8)
				result = true;
			else
				result = false;			
			break;
		case '5': 
			// If my weapon has ammo
			// Check any weapon
			result = can_weapon_shoot();
			break;
		case '6': // If I see my creator
			result = sees_creator;
			break;
		case '7':
			result = coin_toss();
			break;
		case '8':
			result = shell->move_slots.size()>0;
			break;
		case 'a': // go to the last seen creator's position
			if (last_x_of_creator==-1 || last_y_of_creator==-1)
				continue;

			if (this->pX()==last_x_of_creator && this->pY()==last_y_of_creator)
				continue;			

			if (this->shell->action_slots.size()>0) // with opening (here: destroying) doors
				set_direction_to_cell_by_shortest_path(last_x_of_creator, last_y_of_creator, true);
			else // don't open doors
				set_direction_to_cell_by_shortest_path(last_x_of_creator, last_y_of_creator, false);
			return 'a';
		case 'b': // attack enemy by range weapon
			if (enemy!=NULL)
			{
				if (attack_with_ranged_weapon()==true)
					return 'b'; 
			}
			continue;
		case 'c': // go to last seen enemy position
			if (last_x_of_enemy==-1 || last_x_of_enemy==-1)
				continue;
			
			if (this->pX()==last_x_of_enemy && this->pY()==last_y_of_enemy)
				continue;
			if (this->shell->action_slots.size()>0) // with opening (here: destroying) doors
				set_direction_to_cell_by_shortest_path(last_x_of_enemy, last_y_of_enemy, true);
			else // don't open doors
				set_direction_to_cell_by_shortest_path(last_x_of_enemy, last_y_of_enemy, false);
			return 'c';			
		case 'd': // search for an enemy			
				for (index=0;index<100;index++)
				{
					last_x_of_enemy = random(MAPWIDTH);
					last_y_of_enemy = random(MAPHEIGHT);
					if (level.entering_on_cell_possible(last_x_of_enemy,last_y_of_enemy))
						if (set_direction_to_cell_by_shortest_path(last_x_of_enemy, last_y_of_enemy, false))
							break;
				}
			return 'd';						
		case 'e': // go to nearest TRASH and utility it (treat trash an enemy) - for Utility Robot
			// check if can utilize
			item=NULL;
			for (m=shell->action_slots.begin(),_m=shell->action_slots.end();m!=_m;m++)
			{
				item = (ITEM *) *m;
				if (item->name == "청소기")
					break;
			}
			// if doesn't have it, then skip instruction
			if (item==NULL || item->name!="청소기")
				continue;
			
			if (this->pX()==last_x_of_enemy && this->pY()==last_y_of_enemy)
			{
				items_on_cell.clear();
				level.list_of_items_from_cell(&items_on_cell,pX(),pY());
				if (items_on_cell.size()>0)
				{
					for (m=items_on_cell.begin(),_m=items_on_cell.end();m!=_m;m++)
					{
						item = (ITEM *) *m;
						if (item->IsCorpse() || item->IsGarbage())
						{
							if (this->seen_now==true)
							{
								screen.console.add(this->name + "은 " + item->article_a() + item->show_name() + "을 수거한다.",7);
							}
							item->death();
							return 'n';	// wait
						}
					}
				} 
				// no trash here
				for (index=0;index<100;index++)
				{
					last_x_of_enemy = random(MAPWIDTH);
					last_y_of_enemy = random(MAPHEIGHT);
					if (level.entering_on_cell_possible(last_x_of_enemy,last_y_of_enemy))
						if (set_direction_to_cell_by_shortest_path(last_x_of_enemy, last_y_of_enemy, false))
							return 'c';
				}
			} // endof if 
			// not this cell
			set_direction_to_cell_by_shortest_path(last_x_of_enemy, last_y_of_enemy, false);
			return 'c';	 // go to position of enemy (trash)
		case 'f': 
			result = fire_grenades();
			if (result==true) 
				return 'f';
			else
				continue; // skip it
		case 'm': // load ammo
			result = load_ammo_to_weapon();
			if (result==true) 
				return 'm';
			else
				continue; 
		default: 
			return instruction;				
		}

		if (is_negated)
			result=!result;

		if (!result)
			go_to_next_after_conditionals = true;

		if (instruction!='!')
			is_negated=false;
		if (go_to_next_after_conditionals)
		{
			while(instruction_number<program_size)
			{
				instruction_number++;
				instruction = shell->cpu->program[instruction_number];
				if (instruction=='!' || (instruction>'0' && instruction<'9'))
					continue;
				else
				{
					go_to_next_after_conditionals=false;
					break;
				}
			}
		}
	}
	return instruction;
}

ACTION ROBOT::get_action() // AI of robot
{
	switch(get_CPU_instruction()) 
	{
		case 'a':
			// direction to creator set
			return ACTION_MOVE;
		case 'b':
			// shoot at last position of enemy
			return ACTION_FIRE;
		case 'c':
			// direction to enemy set
			return ACTION_MOVE;
		case 'd':
			// direction to search for enemy set
			return ACTION_MOVE;
		case 'f':
			// grenades activated
			return ACTION_INVENTORY;
		case 'm':
			// ammo loaded
			return ACTION_INVENTORY;
	}
	return ACTION_WAIT;
}

// attack with all hand weapons, if not available, attack also with ranged weapon
bool ROBOT::attack_in_direction()
{
	MONSTER *monster;
	int x,y;
	cell_in_current_direction(x,y);
	if (!level.map.onMap(x,y))
		return false;
	
	monster=level.monster_on_cell(x,y);
	// if is monster then attack it
	if (monster!=NULL)	
	{
		attack(monster);				
		return true;
	}
	return false;
}

// robot resists status except electicity, that damages CPU only
int ROBOT::hit_changes_status(int kind, int power)
{
	if (kind==TYPE_ELECTRIC) // uszkodzenie CPU
	{
		if (shell!=NULL && shell->cpu!=NULL)
		{
			if (power/4>0)
			{
				if (shell->cpu->damage_it(power/2)==DAMAGE_DEATH)
					return DAMAGE_DEATH;
				else
					screen.console.add(name + "은 충격을 받는다.",5);			
			}
		}
	}
	return 0;
}

/// implement it better!!!
int ROBOT::hit_by_explosion(int hit_power)
{
	return cause_damage(hit_power);
}

int ROBOT::hit_by_item(int hit_power, ITEM *item)
{
	if (item->properties&TYPE_NORMAL) 
	{
		hit_power -= shell->ARM;
	}

	if (item->properties&TYPE_ARMOR_PERCING) 
	{
		if (!(shell->properties&TYPE_ARMOR_PERCING)) 
			hit_power+=shell->ARM;
			  
			descriptions.add_attack("HIT"," through its armor");
	}
		  		
	if (hit_power>0)
	{
		if (item->properties&TYPE_ELECTRIC)
		{
			if (hit_changes_status(TYPE_ELECTRIC,hit_power)==DAMAGE_DEATH)
			{
				return DAMAGE_DEATH;
			}
		}
		if (item->properties&TYPE_NORMAL)
			return cause_damage(hit_power);
	}
	return 0;
}

int ROBOT::cause_damage(int damage)
{
	ptr_list robot_parts;
	ptr_list::iterator m,_m;

	int index;

	if (damage<=0)
	{
		if (this->seen_now==true)
		{
			string text=name +"은 상처를 입지 않는다.";
			screen.console.add(text,7);
		}
		return DAMAGE_NONE;
	}
	
	// random hit of part
	// CPU uszkadzany tylko, gdy shell zniszczona.
	
	robot_parts.push_back(shell);
	int weight_total = shell->weight;
	ITEM *item;

	m=shell->action_slots.begin();
	for (index=0;index<shell->action_slots.size();index++,m++)
	{
		item = (ITEM *)*m;
		robot_parts.push_back(item);
		weight_total += item->weight;
	}

	m=shell->move_slots.begin();
	for (index=0;index<shell->move_slots.size();index++,m++)
	{
		item = (ITEM *)*m;
		robot_parts.push_back(item);
		weight_total += item->weight;
	}
	
	// choose random element by weight (as size here)

	int random_value = random(weight_total);
	weight_total=0;

	for (m=robot_parts.begin(),_m=robot_parts.end();m!=_m;m++)
	{
		item = (ITEM *)*m;
		weight_total += item->weight;
		if (weight_total >= random_value)
			break;
	}
	assert(m!=_m);

	// we have item selected

	if (item==shell)
		damage-=shell->ARM/2; 
	
	int which_damage=0;
	int hp_before = item->hit_points;
	
	if (damage>=item->DMG) // damage only soft items
	{		
		if (item==shell && shell->hit_points<=damage) // robot dies
		{
			// damage CPU with damaging shell
			shell->cpu->damage_it(damage-shell->hit_points);
		}
		which_damage = item->damage_it(damage);
	}

	if (which_damage>0) // if shell destroyed, robot dies
	{
		if (item!=shell)
			descriptions.add_attack("HIT",", damaging " + item->show_name());
		if (!item->IsCountable() && is_friendly(level.player) && this->sees_creator)
		{
			if (hp_before - which_damage < item->max_hit_points/4 && hp_before>=item->max_hit_points/4)
			{
				descriptions.add_attack("ROBOTSAYS",name + "은 말한다: '" + item->show_name() + "이 파괴되기 직전입니다!'");
			}			
			else if (hp_before - which_damage < item->max_hit_points/2 && hp_before>=item->max_hit_points/2)
			{
				descriptions.add_attack("ROBOTSAYS",name + "은 말한다: '" + item->show_name() + "이 심각한 피해를 입었습니다!'");
			}
		}
	}
	else if (which_damage==DAMAGE_DEATH)
	{
		if (item!=shell && shell!=NULL)
		{
			descriptions.add_attack("HIT",", destroing "  + item->show_name());
			if (!item->IsCountable() && is_friendly(level.player) && this->sees_creator)
				descriptions.add_attack("ROBOTSAYS",name + "은 말한다: '" + item->show_name() + "이 파괴되었습니다!'");
		}
		else
		{
			return -1; // shell destroyed
		}
	}

	return damage;
}

void ROBOT::attack(MONSTER *enemy)
{
	ptr_list::iterator m,_m;

	ITEM *item;
	HAND_WEAPON *hand_weapon;
	RANGED_WEAPON *ranged_weapon;
	

	bool attacked=false;

	// find weapons in different slots
	for (m=shell->action_slots.begin(),_m=shell->action_slots.end();m!=_m;m++)
	{
		item = (ITEM *) *m;

		assert(item!=NULL);

		hand_weapon=item->IsHandWeapon();

		if (hand_weapon!=NULL)
		{
			attacked=true;
			weapon=hand_weapon;
			MONSTER::attack(enemy);
		}
	}

	if (!attacked)
	{
		// attack with ranged if no hand weapond found
		for (m=shell->action_slots.begin(),_m=shell->action_slots.end();m!=_m;m++)
		{
			item = (ITEM *) *m;
			
			assert(item!=NULL);
			
			ranged_weapon=item->IsShootingWeapon();
			
			if (ranged_weapon!=NULL)
			{
				weapon=ranged_weapon;
				MONSTER::attack(enemy);
			}
		}		
	}

	weapon=NULL;
	return;
}

void ROBOT::status_management_every_turn()
{
	return;
}

void ROBOT::self_treatment_every_turn()
{
	return;
}

void ROBOT::display()  // robot look like his shell
{ 
	tile=shell->tile; 
	color=shell->color; 
	
	if (level.player!=NULL)
	{
		if (level.player->enemy==this)
			color+=16;
		if (level.player->is_friendly(this))
			color+=32;
	}
		TILE::display(); 
};

void ROBOT::ShowDescription()
{
	struct Screen_copy scr_copy;
	string temp;
    store_screen(&scr_copy);
	myclear();
	int size = name.size();
    set_color(2);
    print_text(0,0, "--------------------------------------------------------------------------------");
    print_text(0,2, "--------------------------------------------------------------------------------");
    print_text(0,49,"--------------------------------------------------------------------------------");
    set_color(10);
	print_text(39-size/2,1,name);
	set_color(7);
	temp = descriptions.get_description (name);
	if (temp=="")
		temp = "이 물건에 대한 설명이 없다.";
	
	temp+=" ";
	
	char character;
	string word;
	
	int x=2;
	int y=4;
	
	for (int a=0;a<temp.size();a++)
	{
		character = temp[a];
		word += character;
		if (character==' ')
		{
			if (x+word.size()>=78)
			{
				x=2;
				y++;
			}
			print_text(x,y,word);
			x+=word.size();
			word="";
		}
	}

	ptr_list::iterator m,_m;
	ITEM *temp2;
	assert(shell!=NULL);
	y=20;

	print_text(2,y++,string("본체: ") + shell->show_name() + " <" + IntToStr(100*shell->hit_points/shell->max_hit_points) + "%>");
	y++;
		
	if (shell->action_slots.size()>0)
	{
		print_text(2,y++,string("행동 슬롯: ") + IntToStr(shell->action_slots.size()) + "/" +IntToStr(shell->max_number_action_slots) );
		y++;
		for (m=shell->action_slots.begin(),_m=shell->action_slots.end();m!=_m;m++)
		{
			temp2 = (ITEM *) *m;
			print_text(5,y++,temp2->show_name() + " <" + IntToStr(100*temp2->hit_points/temp2->max_hit_points) + "%>");
		}
		y++;
	}

	if (shell->move_slots.size()>0)
	{
			print_text(2,y++,"이동 슬롯: " + IntToStr(shell->move_slots.size()) + "/" +IntToStr(shell->max_number_move_slots) );
			y++;
			for (m=shell->move_slots.begin(),_m=shell->move_slots.end();m!=_m;m++)
			{
				temp2 = (ITEM *) *m;
				print_text(5,y++,temp2->show_name() + " <" + IntToStr(100*temp2->hit_points/temp2->max_hit_points) + "%>");
			}
			y++;
			print_text(5,y++,"(속도: " + IntToStr(get_robot_speed()) + ")");
	}
	else
		print_text(2,y++,"- 고정됨 -");
	

	myrefresh();	
	keyboard.wait_for_key();
    restore_screen(&scr_copy);
	myrefresh();	
}

bool ROBOT::attack_with_ranged_weapon()
{
	bool at_least_once_fired=false;	
	int x,y;
	
	if (enemy==NULL)
		return false;

	x = enemy->pX();
	y = enemy->pY();
	
	if (x==-1 || y==-1)
		return false;
	
	// check, czy jest na linii strzalu
	struct POSITION position[100];
	int cells, index;
	cells=generate_bresenham_line(pX(), pY(), x, y, (POSITION *) position,100);
	bool it_blocks = false;
	
	for (index=1;index<cells;index++)
	{
		// hit the cell
		if (level.map.blockMove(position[index].x,position[index].y))
			it_blocks = true;
	}
	if (!it_blocks)
	{
		if (can_weapon_shoot())
		{
			if (this->seen_now)
			{
				level.player->stop_repeating();
				screen.console.add(name + "은 사격한다!",3);				
			}
		}

		if (shoot_into(x,y)==true)
			at_least_once_fired=true;
	}		
	return at_least_once_fired;	
}

bool ROBOT::shoot_into(int x, int y)
{
  int accuracy;
  int target_x, target_y;
  int random_value;
  double angle_miss, total_angle_miss;
  int number_of_shoots, fire_number;
  int distance_to;
  bool firing_at_enemy;

  descriptions.zero_attack("HIT");
  
  bool hit = false;

  if (enemy==NULL)
    firing_at_enemy=false;
  else
    firing_at_enemy=true;

  RANGED_WEAPON *range_weapon;
  ITEM *item;
  
  // fire all available weapons

  ptr_list::iterator m,_m;
  for (m=shell->action_slots.begin(),_m=shell->action_slots.end();m!=_m;m++)
  {
	  item=(ITEM *) *m;
	  range_weapon=item->IsShootingWeapon();
	  if (range_weapon!=NULL)
	  {
		  RANGED_WEAPON *sound;
		  if (this->seen_now || level.ID=="ben")
			sound =(RANGED_WEAPON *) definitions.find_item(range_weapon->name);
		  else
			sound=NULL;
		  switch(range_weapon->fire_type)
		  {
		  case FIRE_SINGLE:
			  number_of_shoots=1;
			  if (sound!=NULL)
				  sounds.PlaySoundOnce(sound->sound_file_single);
			  break;
		  case FIRE_DOUBLE:
			  number_of_shoots=2;
			  if (sound!=NULL)
				  sounds.PlaySoundOnce(sound->sound_file_double);
			  break;
		  case FIRE_TRIPLE:
			  number_of_shoots=3;
			  if (sound!=NULL)
				  sounds.PlaySoundOnce(sound->sound_file_triple);
			  break;
		  case FIRE_BURST:
			  number_of_shoots=6+random(6);
			  if (sound!=NULL)
				  sounds.PlaySoundOnce(sound->sound_file_burst);
			  break;     
		  }
		  
		  total_angle_miss=0;
		  accuracy=shell->cpu->quality*5 + range_weapon->ACC*5 + range_weapon->ammo.ACC*5;
		  
		  accuracy*=shell->cpu->quality*3/2;
		  accuracy/=50;
		  if (accuracy==0)
			  accuracy++;  		  
		  
		  for (fire_number=1;fire_number<=number_of_shoots && range_weapon->ammo.quantity!=0;fire_number++)
		  {
			  if (firing_at_enemy)
				  distance_to=1000;
			  else
			  {
				  distance_to=distance(pX(),pY(),x,y);
				  int range;
				  range=100-accuracy;
				  if (range<0) range=0;
				  range=random(range);
				  
				  range = (distance_to*range)/100; // procentowe chybienie zalezy tez od odleglosci
				  distance_to+=random(range*2)-range;
				  
			  }
			  
			  random_value=random(360);
			  
			  angle_miss= ((double) random_value)/accuracy;
			  
			  if (coin_toss()) // kat tez moze byc ujemny
				  angle_miss*=-1;
			  
			  target_x=x;
			  target_y=y;
			  total_angle_miss+=angle_miss;
			  calculate_hit_place(this->pX(),this->pY(),target_x,target_y, total_angle_miss);
			  // wyslanie pocisku w to place
			  if (seen_now==true)
			  {
				  set_color(14);
				  print_character(pX(),pY(),tile);
			  }
			  
			  if (range_weapon->fire_missile(this->pX(), this->pY(), target_x,target_y, enemy, distance_to)==true)
				  hit=true;
			  
			  if (enemy!=NULL && hit && enemy->seen_now==true)
			  {
				  level.player->stop_repeating();				  
				  delay(10);
				  string text;
				  text = descriptions.get_attack("HIT");
				  if (text!="")
				  {
					  if (text.size()>0 && text[text.size()-1]==',')
						  text = text.substr(0,text.size()-1);
					  
					  if (text.find("killing it")!=-1)
						  screen.console.add(text + '.',4);
					  else
						  screen.console.add(text + '.',6);	  
				  }
				  descriptions.zero_attack("HIT");		 
			  }
		  } // endof for number pociskow
	  } // endof strzelania z tej broni
  }
 
  if (seen_now==true)
  {
	  delay(200);	  
  }	  
  return hit; // czy trafiono
}


int ROBOT::get_h2h_attack_value()
{
	int attack;
	if (weapon==NULL)
		return 0;

	attack=shell->cpu->quality/4;

	ptr_list::iterator m,_m;
	
	ITEM *item;
	HAND_WEAPON *hand_weapon;
	RANGED_WEAPON *ranged_weapon;
	
	m=shell->action_slots.begin();
	_m=shell->action_slots.end();
	
	for (;m!=_m;m++)
	{
		item = (ITEM *) *m;
		hand_weapon = item->IsHandWeapon();
		ranged_weapon = item->IsShootingWeapon();
		if (hand_weapon!=NULL)
		{
			hand_weapon->uses_energy();			
			attack+=hand_weapon->HIT;
		}
		else if (ranged_weapon!=NULL)
		{
			ranged_weapon->uses_energy();			
			attack+=ranged_weapon->HIT;
		}
	}

	// uwzglednienie skilla
	attack*=shell->cpu->quality*3/2;	
	
	return attack;
}

int ROBOT::get_h2h_defense_value()
{
	int defense;

	defense=shell->cpu->quality/4;

	ptr_list::iterator m,_m;
	
	ITEM *temp;
	HAND_WEAPON *hand_weapon;
	RANGED_WEAPON *ranged_weapon;
	
	m=shell->action_slots.begin();
	_m=shell->action_slots.end();
	
	for (;m!=_m;m++)
	{
		temp = (ITEM *) *m;
		hand_weapon = temp->IsHandWeapon();
		ranged_weapon = temp->IsShootingWeapon();
		if (hand_weapon!=NULL)
		{
			defense+=hand_weapon->DEF;
			hand_weapon->uses_energy();			
		}
		else if (ranged_weapon!=NULL)
		{
			defense+=ranged_weapon->DEF;
			ranged_weapon->uses_energy();			
		}
	}

	// uwzglednienie skilla
	defense*=shell->cpu->quality*3/2;

	// jezeli sie nie rusza, to duzo slabiej
	if (get_robot_speed()<5)
		defense/=10;

	return defense;
}


// returns (CHANGE TO ENUMS!!!)
// 0 not a robot
// 1 turned on
// -1 not enough skill
// -2 robot have no CPU
// -3 no place to set robot as monster

int INTELLIGENT::turn_robot_on(ITEM *to_turn_on)
{
	ROBOT_SHELL *shell;

	shell = to_turn_on->IsRobotShell();
	if (shell==NULL)
		return 0;

	int req_skill;
	req_skill = 10 + shell->action_slots.size()*10 + shell->move_slots.size()*10;
	
	if (req_skill>skill[SKILL_MECHANIC])
		return -1;	

	if (shell->cpu==NULL)
		return -2;

	// check, czy jest na robota place
	int mx,my,x,y;
	if (shell->owner!=NULL)
	{
		mx=shell->owner->pX();
		my=shell->owner->pY();
	}
	else
	{
		mx=shell->pX();
		my=shell->pY();
	}

	// is place for monster?
	x=-1;
	y=-1;

	if (level.entering_on_cell_possible(mx,my))
	{
		x=mx;
		y=my;
	}
	else if (level.entering_on_cell_possible(mx+1,my))
	{
		x=mx+1;
		y=my;
	}
	else if (level.entering_on_cell_possible(mx-1,my))
	{
		x=mx-1;
		y=my;
	}
	else if (level.entering_on_cell_possible(mx,my+1))
	{
		x=mx;
		y=my+1;
	}
	else if (level.entering_on_cell_possible(mx+1,my+1))
	{
		x=mx+1;
		y=my+1;
	}
	else if (level.entering_on_cell_possible(mx-1,my+1))
	{
		x=mx-1;
		y=my+1;
	}
	else if (level.entering_on_cell_possible(mx+1,my-1))
	{
		x=mx+1;
		y=my-1;
	}
	else if (level.entering_on_cell_possible(mx-1,my-1))
	{
		x=mx-1;
		y=my-1;
	}

	if (x==-1 || y==-1)
		return -3;

	// w x i y mamy place na robota

	if (shell->owner!=NULL) // jest u kogos w backpacku
	{
		INTELLIGENT *intel = shell->owner->IsIntelligent();
		assert(intel!=NULL);
		intel->drop_item(shell,false);
	}

	ROBOT *new_one;
	new_one = new ROBOT;

	new_one->Creator_ID=this->UniqueNumber;	

	shell->owner = new_one;
	new_one->shell = shell;
	new_one->rename(shell->last_robot_name);
	new_one->group_affiliation = shell->cpu->group_affiliation;

	// ustalenie jego atrybutow, choc wiekszosc jest nieuzywana

	new_one->strength.val = shell->cpu->quality/4;
	new_one->strength.max = shell->cpu->quality/4;
	new_one->dexterity.val = shell->cpu->quality/4;
	new_one->dexterity.max = shell->cpu->quality/4;
	new_one->endurance.val = shell->cpu->quality/4;
	new_one->endurance.max = shell->cpu->quality/4;
	new_one->intelligence.val = 1;
	new_one->intelligence.max = 1;
	new_one->speed.val = 1;
	new_one->speed.max = 1;
	new_one->metabolism.val = 0;
	new_one->metabolism.max = 0;
	
	for (int a=0;a<NUMBER_OF_SKILLS;a++)
		new_one->skill[a]=shell->cpu->quality;

	new_one->ChangePosition(x,y);	
	new_one->last_x_of_enemy = new_one->pX();
	new_one->last_y_of_enemy = new_one->pY();
	
	level.remove_from_items_on_map(shell);

	level.monsters.push_back(new_one);
			
	return true;
}

bool ROBOT::death()
{
	turn_robot_off();
	return true;
}

void ROBOT::turn_robot_off()
{
	if (shell!=NULL)
	{
		shell->ChangePosition(pX(),pY()); 
		// jest jeszcze jego owner, wiec samo nie doda informacji na mapie		
		level.add_to_items_on_map(shell);
		shell->owner=NULL;
		shell=NULL;
	}
	MONSTER::death();
}

bool ROBOT::tries_to_turn_off(MONSTER *temp)
{
	int chance=0;

	// chance - when friendly = 100
	
	if (!is_enemy(temp))
	{
		turn_robot_off();
		if (temp==level.player)
		{
			level.player->skill_used(SKILL_MECHANIC);
		}		
		return true;
	}

	chance = temp->dexterity.GetValue()*3 + temp->skill [ SKILL_MECHANIC ];

	if (get_robot_speed()<5)
		chance+=30;
	else
		chance-=10;

	if (temp->strength.GetValue() < shell->ARM*4 )
		chance-=30;

	chance -= get_h2h_defense_value()/10;
	if (chance>0 && lower_random(chance,100)) // done
	{		
		if (temp->seen_now)
			screen.console.add(temp->name + "은 " + this->name + "을 끈다.",7);
		turn_robot_off();

		if (temp==level.player)
			for (int a=0;a<5;a++) // 5 times
				level.player->skill_used(SKILL_MECHANIC);
			
		return true;
	}
	else
	{
		if (temp->seen_now)
			screen.console.add(temp->name + "은 " + this->name + "을 끄는데 실패한다.",7);		
	}
	
	return false;
}

bool MONSTER::look_around()
{
	// searchlights in bunker light up the terrain
	if (level.ID == "ben")
	{
		ptr_list::iterator m,_m;
		MONSTER *temp;
		SEARCHLIGHT *sl;
		for(m=level.monsters.begin(),_m=level.monsters.end(); m!=_m; m++)
		{
			temp=(MONSTER *)*m;
			sl = temp->IsSearchlight();
			if (sl!=NULL)
			{
				sl->look_around();
			}				
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////

ACTION MADDOCTOR::get_action() // AI MadDoctor'a
{
	if (level.is_player_on_level==false)
		hit_points.val = hit_points.max;

      int a,x,y;
	  
	  look_around();

	  if (seen_enemies.size()==0)
		  yelling_chance = random(200);

	  if (seen_enemies.size()>0 && actual_behaviour!=BEHAVIOUR_RUN_AWAY)
	  {
		  if (lower_random(yelling_chance,100))
		  {
			  yelling_chance = 0;
			  return ACTION_SHOUT;
		  }
	  }

	  if (have_low_hp())
	  {
		  if (enemy!=NULL)
		  {
			  if (have_critically_low_hp() && !enemy->have_critically_low_hp() && random(2)==0)
				  start_to_run_away();
		  }
	  }
	  else
	  {
		  if (actual_behaviour == BEHAVIOUR_RUN_AWAY && random(10)==0 )
			  actual_behaviour=BEHAVIOUR_SEARCH_FOR_ENEMY;
	  }

		  
	  int how_danger = how_danger_is_cell(pX(),pY());
	  int most_safe_direction=direction;
	  int n;
	  int random_attack;

	  if (how_danger>0) // something wrong on this cell, run away
	  {
		  yelling_chance = 0;
		  direction = random(8);
		  for (a=0;a<8;a++) // find safer place
		  {
			  direction++;
			  if (direction>8)
				  direction=1;
			  cell_in_current_direction(x,y);
			  n = how_danger_is_cell(x,y);
			  if (n==0)
			  {
				  most_safe_direction = direction;
				  break;
			  }
			  else if (n<how_danger)
			  {
				how_danger = n;
				most_safe_direction = direction;
			  }

		  }
		  cell_in_current_direction(x,y);
		  go_around_cell(x,y,false);
		  return ACTION_MOVE;
	  }	

	  if (actual_behaviour!=BEHAVIOUR_RUN_AWAY && seen_enemies.size()>0)
		  actual_behaviour = BEHAVIOUR_H2H_COMBAT;

	  // we seen enemy
	  if (actual_behaviour==BEHAVIOUR_H2H_COMBAT)
	  {
		  if (pX()==last_x_of_enemy && pY()==last_y_of_enemy)
		  {
			  camping=0;
			  actual_behaviour=BEHAVIOUR_SEARCH_FOR_ENEMY;
		  }
		  else
		  {			  
			  set_direction_to_cell_by_shortest_path(last_x_of_enemy,last_y_of_enemy,false);
			  cell_in_current_direction(x,y);
			  if (enemy!=NULL && x==enemy->pX() && y==enemy->pY()) // jezeli stoi obok of enemy
			  {
				  // kind of attack

				  // 1. normal
				  // 2. tail
				  // 3. throw enemy
				  
				  random_attack = random(100);

				  if (random_attack<50) 
				  {
					  return ACTION_ATTACK;
				  }
				  if (random_attack<70 && enemy->resist[RESIST_CHEM_POISON]<20) 
				  {
					  return ACTION_ATTACK_2;
				  }
				  if (random_attack<80 && enemy->calculate_weight()<200000)
				  {
					  return ACTION_ATTACK_3;
				  }
				  return ACTION_MOVE;
			  }
			  else if (enemy!=NULL) // if not near enemy
			  {
				  random_attack = random(100);
				  
				  if (random_attack<10 && enemy->resist[RESIST_PARALYZE]<40) // spit poison
				  {
					  return ACTION_ATTACK_4;
				  }
				  if (random_attack<20) // needle
				  {
					  return ACTION_ATTACK_5;
				  }
			  }
			  else
			  {
				  go_around_cell(x,y,false);
				  return ACTION_MOVE;		  
			  }
		  }		  
		  return ACTION_MOVE;		  
	  }
	  if (actual_behaviour==BEHAVIOUR_CALM)
	  {
		  if (random(30)==0)
		  {
			  actual_behaviour = BEHAVIOUR_SEARCH_FOR_ENEMY;
			  camping=0;
		  }

		  return ACTION_WAIT;		  
	  }
	  if (actual_behaviour==BEHAVIOUR_SEARCH_FOR_ENEMY)
	  {
		  if (random(30)==0)
		  {
			  actual_behaviour = BEHAVIOUR_CALM;
			  turned_to_fight=false;
		  }

		  int x,y;
		  if (last_pX==pX() && last_pY==pY()) // doesn't like to stay for too long
		  {
			  camping++;
			  if (camping>2)
			  {
				  for (int index=0;index<100;index++)
				  {
					  direction = random(8);
					  cell_in_current_direction(x,y);
					  
					  if (level.entering_on_cell_possible(x,y))
					  {
						  last_direction_of_enemy = direction;
						  break;
					  }
				  };				  
			  }
		  }
		  else
		  {
			  camping=0;		  		  
			  direction=last_direction_of_enemy;
			  cell_in_current_direction(x,y);
			  go_around_cell(x,y,false);
		  }
		  	
		  return ACTION_MOVE;		  
	  }
	  if (actual_behaviour==BEHAVIOUR_RUN_AWAY)
	  {
		  if (choose_run_away_direction()==true)
			  return ACTION_MOVE;
	  }
	  
	  return ACTION_WAIT;
}

int MADDOCTOR::how_danger_is_cell(int x,int y)
{
	// afraid only of fire

	ptr_list::iterator m,_m;
	GAS *temp;
	int danger_value=0;
	
	if (level.map.seen(x,y)==false)
		return 0;
	
	int danger = level.map.getDanger(x,y);
	int fire_density=0;
	
	// explosions are danger
	if (danger & DANGER_EXPLOSION)
	{
#ifdef DEBUG_AI
		print_character(x,y,'B');
		myrefresh();
		wait(500);
#endif		
		danger_value+=10;
	}
	
	if (danger & DANGER_IGNITION)
		danger_value+=10*(100-resist[RESIST_FIRE])/100;
	
	if (actual_behaviour==BEHAVIOUR_RUN_AWAY) 
	{
		if (danger & DANGER_ENEMY)
		{
#ifdef DEBUG_AI
			print_character(x,y,'W');
			myrefresh();
			wait(500);
#endif					
			danger_value+=10;
		}
	}
	
	// pobranie stezen gazu i ognia z danego pola
	int gases = level.map.GetNumberOfGases(x,y);
	int checked=0;
	for(m=level.gases_on_map.begin(),_m=level.gases_on_map.end(); m!=_m && checked<gases; m++)
	{
		temp=(GAS *)*m;
		if (temp->pX()==x && temp->pY()==y) // jest na tym polu
		{
			checked++;
			// czy to taki gaz?
			if (temp->properties&TYPE_INCENDIARY)
				fire_density+=temp->density;
		}
	}
	
	// sprawdzamy, czy boimy sie tego ognia
	if (fire_density-4 > resist[RESIST_FIRE]/2)
		if (fire_density>4 && (!(armor->properties&TYPE_INCENDIARY)))
			danger_value+=(fire_density*(100-resist[RESIST_FIRE])/100);
				
	return danger_value;
}

TIME MADDOCTOR::do_action(ACTION action)
{
	last_pX = pX();
	last_pY = pY();

	int random_value;
	string to_yell;
	int spd;
	
	switch (action)
	{
	case ACTION_SHOUT:
		random_value = random(MADDOCTOR_TEXTS);
		to_yell = histext[random_value];
		screen.console.add(name + "은 소리친다: \"" + to_yell + "\"",15);
		return 10; // czeka ze dwie tury
	case ACTION_MOVE:
		int x,y;
		cell_in_current_direction(x,y);
		if (level.map.blockMove(x,y))
			go_around_cell(x,y,false);
		
		move();
		spd = speed.GetValue();
		if (actual_behaviour == BEHAVIOUR_RUN_AWAY)
			spd += 10;
		return (TIME_MOVE/spd)+1;

	case ACTION_ATTACK:
		move();
		return (TIME_MOVE/speed.GetValue())+1;
	case ACTION_ATTACK_2: // ogonem
		set_weapon(&tail);
		move();
		set_weapon(&unarmed);
		yelling_chance+=random(20)+10;
		return (TIME_MOVE/speed.GetValue())+5;
	case ACTION_ATTACK_3: // throw enemy
		throw_enemy_at_the_wall();
		yelling_chance+=random(20)+10;
		return (TIME_MOVE/speed.GetValue())+5;
	case ACTION_ATTACK_4: // pluniecie
		spit_poison();
		return (TIME_MOVE/speed.GetValue())+1;
	case ACTION_ATTACK_5: // igla
		fire_needle();
		return (TIME_MOVE/speed.GetValue())+1;
		
	case ACTION_WAIT_STUNNED:
		//		  screen.console.add(name + " has been stunned.\n",8);
		return state[STATE_STUN];
	}		
	return 1;
}

bool MADDOCTOR::look_around()
{
	ptr_list::iterator m,_m;
	ITEM *item;
	MONSTER *monster;

	bool sees_enemy_again=false;
	int danger;
	int a,b;
	
	seen_enemies.clear();

	level.fov.Start(&level.map,pX(),pY(),fov_radius.GetValue());

	for(m=level.monsters.begin(),_m=level.monsters.end(); m!=_m; m++)
	{
		monster=(MONSTER *)*m;
		
		if (monster==this)
			continue;
		
		if (level.map.seen_by_camera(monster->pX(),monster->pY()) || (level.map.seen(monster->pX(),monster->pY()) && distance(monster->pX(),monster->pY(),pX(),pY())<fov_radius.GetValue()))
		{
				if (is_enemy(monster)) // enemy
				{
					seen_enemies.push_back(monster);
					
					if (monster == enemy)
						sees_enemy_again = true;
					
					// dodanie niebezpieczenstwa
					
					for (a=0;a<3;a++)
						for (b=0;b<3;b++)
							level.map.addDanger(monster->pX() + a-1,monster->pY() + b-1,DANGER_ENEMY);						
				} // czy enemyi
		}
	}
	
	// ptr_list items
	for(m=level.items_on_map.begin(),_m=level.items_on_map.end(); m!=_m; m++)
	{
		item=(ITEM *)*m;
		if (level.map.seen(item->pX(),item->pY()) && !level.map.isShield(item->pX(),item->pY()))
		{
			if (!item->property_controller() && !item->invisible) // gdy nie jest to sterujacy i gdy nie invisible
			{
				// dodanie niebezpieczenstwa
				GRENADE *gr = item->IsGrenade();
				if (gr!=NULL && gr->active)
				{					
					    level.map.addDanger(item->pX(),item->pY(),DANGER_GRENADE);
					
						danger=DANGER_NONE;
						if (item->properties&TYPE_SENSOR)
							danger |= DANGER_SENSOR;
						if (item->properties&TYPE_INCENDIARY)
							danger |= DANGER_IGNITION;
						if (item->properties&TYPE_RADIOACTIVE)
							danger |= DANGER_RADIOACTIVE;
						if (item->properties&TYPE_CHEM_POISON)
							danger |= DANGER_CHEM_POISON;
						if (item->properties&TYPE_STUN)
							danger |= DANGER_STUN;
						if (item->properties&TYPE_PARALYZE)
							danger |= DANGER_PARALYSE;											
					
						if (gr->properties&TYPE_SENSOR)
						{
							// danger tylko na sasiednich polach w kolo
							for (a=0;a<3;a++)
								for (b=0;b<3;b++)
									level.map.addDanger(item->pX() + a-1,item->pY() + b-1,danger | DANGER_SENSOR);
						}
						else // active i za chwile wybuchnie!
						{
							// danger dajemy, ze jest na 5 cells w kolo granatu
							for (int a=0;a<10;a++)
								for (int b=0;b<10;b++)
								{
									if (distance(item->pX(),item->pY(),item->pX() + a-5,monster->pY() + b-5)<=5)
										level.map.addDanger(item->pX() + a-5,item->pY() + b-5,danger);
								}
						} // endof if sensor

				} // if active
			} // if is to item widzialny
		} // jezeli na widzianym polu
	}
	
	if (seen_enemies.size()==0)
		enemy = NULL;

	if (seen_enemies.size()>0 && 
	   (actual_behaviour==BEHAVIOUR_CALM || actual_behaviour==BEHAVIOUR_SEARCH_FOR_ENEMY))
		actual_behaviour = BEHAVIOUR_RANGED_COMBAT;		
	
	if (!sees_enemy_again)
	{
		if (seen_enemies.size()>0)
		{
			actual_behaviour=BEHAVIOUR_RANGED_COMBAT;
			enemy = (MONSTER *) *seen_enemies.begin();
			monster_sees_enemy(enemy);
		}
		if (enemy==NULL)
		{
			if (actual_behaviour==BEHAVIOUR_RANGED_COMBAT ||
				actual_behaviour==BEHAVIOUR_H2H_COMBAT)
			actual_behaviour=BEHAVIOUR_SEARCH_FOR_ENEMY;
		}
	}
	else
		monster_sees_enemy(enemy);

	return true;
}

bool MADDOCTOR::death()
{
	seen_last_time_in_turn = level.turn; // zawsze exp. za niego
	level.create_item(pX(),pY(),"유전자 조작기 카드",0,0);
	return MONSTER::death();	
}

int MADDOCTOR::throw_enemy_at_the_wall()
{
	// 1. pick up of enemy
	// 2. throw enemy
  int target_x, target_y;
  int start_x, start_y;
  double angle_miss;
  struct Screen_copy scr_copy;  

     // print message
     if (this->seen_now==true)
     {
		 if (enemy->seen_now)
			screen.console.add(name + "은 " + enemy->name + "을 잡아 던진다.",3);
		 else
			 screen.console.add(name + "은 무언가를 잡아 던진다.",3);		 
     }

	store_screen(&scr_copy);
  
	angle_miss=random(360);
        
	 start_x = pX();
	 start_y = pY();
	 target_x=0;
	 target_y=0;
	 
     // count straight to target
	 
     calculate_hit_place(start_x,start_y,target_x,target_y, angle_miss);

     // throw w to place

     struct POSITION position[100]; // struktura opisujaca wyliczony tor lotu
     int cells, index;
     MONSTER *monster;

     // count straight 
     generate_bresenham_line(start_x, start_y, target_x, target_y, (POSITION *) position,100);

     cells = 100; 

	 // damage zaleza od masy i zrecznosci rzucanego
	 int damage = enemy->calculate_weight()/7000 - enemy->dexterity.GetValue()/2;
	 damage=damage/2+random(damage);
	        
     for (index=1;index<=cells;index++)
     {
        if (level.entering_on_cell_possible(position[index].x,position[index].y)==false)
        {
          // check, czy na polu jest monster
          monster=level.monster_on_cell(position[index].x,position[index].y);
		  if (monster!=enemy && monster!=this)
		  {
			  if (monster!=NULL && monster->evade_missile(this)==false) // nie uniknal
			  {
				  if (damage>0)
				  {
					  monster->cause_damage(damage);
					  if (monster->seen_now)
					  {
						  if (enemy->seen_now)
							  screen.console.add(enemy->name + "은 " + monster->name + "을 공격해 " + IntToStr(damage) + "만큼의 피해를 입힌다",6);				 
						  else
							  screen.console.add("무언가가 " + monster->name + "을 공격한다",6);				 

					  }
				  }
			  }
			  target_x = position[index-1].x;
			  target_y = position[index-1].y;
			  break;				 
		  }
        }
		if (enemy==level.player)
		{
			level.player->ChangePosition(position[index].x,position[index].y);
			level.player->look_around();
		}
        if (level.map.seen_by_player(position[index].x,position[index].y))
        {
		   restore_screen(&scr_copy);
		   enemy->draw_at(position[index].x,position[index].y);
           myrefresh();
           delay(20);
        }	
		if (random(3)==0)
		{
			damage--;
			if (damage<0)
				damage=0;
		}
     }     
	 enemy->ChangePosition(target_x,target_y);
	 last_x_of_enemy = target_x;
	 last_y_of_enemy = target_y;

	 if (damage>0)
	 {
		 enemy->hit_changes_status(TYPE_STUN,damage);
		 enemy->cause_damage(damage);
		 if (enemy!=NULL && enemy->seen_now)
			 screen.console.add(enemy->name + "은 벽을 공격해 " + IntToStr(damage) + "만큼의 피해를 입힌다.",6);
	 }
				 
	 
	return true;
}

int MADDOCTOR::spit_poison()
{
	if (enemy==NULL)
		return false;

	ITEM *item;
	item = level.create_item(pX(),pY(),"끈끈한 분비액",0,0);
	throw_item_to(enemy->pX(),enemy->pY(),item);
	return true;
}

int MADDOCTOR::fire_needle()
{
	if (enemy==NULL)
		return false;
	
	ITEM *item;
	item = level.create_item(pX(),pY(),"수술용 바늘",0,0);
	throw_item_to(enemy->pX(),enemy->pY(),item);
	return true;
}

int WORM::move()
{
	int x,y;
	MONSTER *monster;

	cell_in_current_direction(x,y);

	monster=level.monster_on_cell(x,y);
	if (prev!=NULL)
	{
		if (!(abs(x-prev->pX())<2 && abs(y-prev->pY())<2) && monster==NULL) // too far from prev segment
		{
			return false;
		}
	}

	if (!level.map.blockMove(x,y))
	{
		if (monster==NULL) // move all next segments
		{
			WORM *segment = this;
			int tx,ty;

			while(segment!=NULL)
			{
				tx=segment->pX();
				ty=segment->pY();
				segment->ChangePosition(x,y);
				segment->wait(TIME_MOVE/speed.val+1);
				x=tx;
				y=ty;

				if (segment->next==NULL)
				{
					if (segment->length_left>1)
					{
						WORM *new_segment = (WORM *) level.create_monster(tx,ty,segment->segment_name,0,0);
						if (new_segment!=NULL)
						{
							new_segment->length_left = segment->length_left-1;
							segment->next=new_segment;
							new_segment->prev=segment;
							segment->length_left=0;
						}
					}
					else if (segment->length_left>0)
					{
						WORM *new_tail = (WORM *) level.create_monster(tx,ty,segment->tail_name,0,0);
						if (new_tail!=NULL)
						{
							new_tail->length_left = 0;
							segment->next=new_tail;
							new_tail->prev=segment;
							segment->length_left=0;
						}
					}
				}
				segment=segment->next;
			}
			return 1; // moved
		}
		else
		{
			if (!monster->IsWorm())
				attack(monster);
			else if (prev==NULL)
			{
				// if cannot move, reverse worm
				int curr_dir=direction;
				int blocked_count=0;
				for (int a=0;a<8;++a)
				{
					int ax,ay;
					direction=a;
					cell_in_current_direction(ax,ay);
					if (!level.entering_on_cell_possible(ax,ay))
						blocked_count++;
				}
				direction=curr_dir;
				if (blocked_count==8)
				{
					reverse_worm();
				}
			}
			return 3;
		}
		return 1; // moved

	}
	return false;
}

void WORM::reverse_worm()
{
	// reverse worm
	WORM *segment=this;
	WORM *following, *temp;
	while(segment!=NULL)
	{
		following = segment->next;
		temp = segment->prev;
		segment->prev = segment->next;
		segment->next = temp;
		segment=following;
	}
}
