// #define TEST_TELEPORT_ON_START
// #define TEST_TELEPORT_TO "csw"
#define RESCUE_CAPSULE_TO "cnw"
//#define DRAW_MAP_CREATION

#include "mem_check.h"
#include <assert.h>

#include "map.h"
#include "system.h"
#include "parser.h"
#include "level.h"
#include "types.h"
#include <math.h>
#include "sounds.h"

extern LEVEL level;
extern bool coin_toss();
extern unsigned int random(unsigned int range);
extern bool lower_random(int lower_value, int range);
extern DEFINITIONS definitions;
extern int distance(const int& x1,const int& y1,const int& x2,const int& y2);
extern int round_up(double variable);
extern MYSCREEN screen;
extern SOUNDS sounds;

bool MAP :: close(int x, int y)
{
  string name;
  string to_find;
  int place;
  CELL *found;

  if (!onMap(x,y))
    return false;
  if (cells[x][y].about_open!=OPEN_TRUE)
    return false;

  // can close, when weight of item on cell <5000
  ptr_list items_on_cell;
  ptr_list::iterator m,_m;
  level.list_of_items_from_cell(&items_on_cell,x,y);
  ITEM *temp;
  unsigned long weight=0;
  for (m=items_on_cell.begin(),_m=items_on_cell.end();m!=_m;m++)
  {
	  temp = (ITEM *) *m;
	  weight+=temp->calculate_weight();
	  if (weight>5000)
		  return false;
  }
    
  name=cells[x][y].name;
  place=name.find("(열림)");
  
  if (place==-1)
    return false;
  to_find=name.substr(0,place) + "(닫힘)";
  found=definitions.find_terrain(to_find);
  if (found!=NULL)
  {
     cells[x][y].name=found->name;
     cells[x][y].tile=found->tile;
     cells[x][y].color=found->color;
     cells[x][y].block_los=found->block_los;
     cells[x][y].real_block_los=found->real_block_los;
     cells[x][y].block_move=found->block_move;
     cells[x][y].real_block_move=found->real_block_move;
     cells[x][y].about_open=OPEN_FALSE;
     return true;
  }
  else
    return false;
}

bool MAP :: open(int x, int y)
{
  string name;
  string to_find;
  int place;
  CELL *found;

  if (!onMap(x,y))
    return false;
  if (cells[x][y].about_open!=OPEN_FALSE)
    return false;

    
  name=cells[x][y].name;
  place=name.find("(닫힘)");
  
  if (place==-1)
    return false;
  to_find=name.substr(0,place) + "(열림)";
  found=definitions.find_terrain(to_find);
  if (found!=NULL)
  {
     cells[x][y].name=found->name;
     cells[x][y].tile=found->tile;
     cells[x][y].color=found->color;
     cells[x][y].block_los=found->block_los;
     cells[x][y].real_block_los=found->real_block_los;
     cells[x][y].block_move=found->block_move;
     cells[x][y].real_block_move=found->real_block_move;
     cells[x][y].about_open=OPEN_TRUE;
     return true;
  }
  else
    return false;
}


CELL :: CELL()
{
      seen=false;
      last_tile=' ';
      seen_at_least_once=false;

   name=string("");
   destroyed_to=string("");
   tile='#';
   color=7;
   block_los=false;
   real_block_los=false;
   block_move=false;
   real_block_move=false;
   is_monster=false;
   is_stairs=false;
   number_of_items=0;
   number_of_gases=0;
   shield=0;
   hit_points=1;
   max_hit_points=1;
   about_open=OPEN_NOT_ALLOWED;
}

bool CELL::operator == (CELL a)
{
  return name==a.name;
}

CELL &  CELL::operator = (CELL a)
{
	seen = a.seen;
	seen_by_player = a.seen_by_player;
	last_tile = a.last_tile;
	seen_at_least_once = a.seen_at_least_once;

	name = a.name;
	tile = a.tile;
	color = a.color;
	block_los = a.block_los;
	real_block_los = a.real_block_los;
	block_move = a.block_move;
	real_block_move = a.real_block_move;
	hit_points = a.hit_points;
	max_hit_points = a.max_hit_points;
	about_open = a.about_open;
	destroyed_to = a.destroyed_to;
		
	return *this;
}


bool CELL::operator != (CELL a)
{
  return name!=a.name;
}

void MAP::UnSeenMap()
{
   int x,y;
   for(x=0; x<MAPWIDTH; x++)
      for(y=0; y<MAPHEIGHT; y++)
	  {
		  cells[x][y].seen=false;
		  cells[x][y].seen_by_camera=false;
		  cells[x][y].danger_value = DANGER_NONE;
	  }

  for (x=0;x<MAPWIDTH+1;x++)
	  for (y=0;y<MAPHEIGHT+1;y++)
	  {
		  corners[x][y]=CORNER_INVISIBLE;
	  }
}

void MAP::HideFromPlayerSight()
{
	for(int x=0; x<MAPWIDTH; x++)
		for(int y=0; y<MAPHEIGHT; y++)
		{
			cells[x][y].seen_by_player=false;
			cells[x][y].seen_by_camera=false;
		}
}

STAIRS *MAP::StairsAt(int x,int y)
{
   STAIRS *stairs;
   ptr_list::iterator j,_j;

   bool stairs_exist=false;
   for (j=stairs_of_map.begin(),_j=stairs_of_map.end();j!=_j;j++)
   {
	   stairs = (STAIRS *) *j;
	   if (stairs->x == x && stairs->y == y)
	   {
		   stairs_exist = true;
		   break;
	   }
   }
   if (stairs_exist)
	   return stairs;

   return NULL;
}

#define NO_WALL_IN_ROOM_PROBABILITY (float) 0.2
#define MINIMUM_CELLS_IN_ROOM 6
#define DOOR_ON_WALL_LENGTH 8

#define M_FLOOR "바닥"
#define M_WALL "강철 벽"
#define M_CLOSED_DOOR "강철 문 (닫힘)"

int MAP :: add_room(int x1,int y1, int x2, int y2, CELL *wall, CELL *door, float prawd_braku_sciany)
{
  int index;
  int place, random_value;
  int vertical;

  // count pola
  place=(x2-x1)*(y2-y1);
  if (place<MINIMUM_CELLS_IN_ROOM)
    return 0;

  
  if (rand()%2==0) // vertical sciana
    vertical=true;
  else
    vertical=false;

  // bez bardzo dlugich roomow
  
  if ((x2-x1)*5<(y2-y1))
    vertical=false;
  if ((x2-x1)>(y2-y1)*5)
    vertical=true;
    

  if (abs(x2-x1)<6)
    vertical=false;
  if (vertical==false && abs(y2-y1)<6)
    vertical=2;

    
  if (vertical==true)
  {
    place=x1+(random(x2-1-x1))+1;
    if (!lower_random ((int) (NO_WALL_IN_ROOM_PROBABILITY*100),100) || (y2-y1)>12)
    {
      // narysowanie sciany
      for (index=y1;index<=y2;index++)
      {
#ifdef DRAW_MAP_CREATION
        print_character(place,index,'#');
        myrefresh();
#endif
        cells[place][index]=*wall;        
      }
        for (index=0;index<=(y2-y1)/DOOR_ON_WALL_LENGTH;index++)
        {
          random_value=y1+(random(y2-1-y1))+1;

          cells[place][random_value]=*door;
#ifdef DRAW_MAP_CREATION
          print_character(place,random_value,'/');
          myrefresh();
#endif
        }
    }

    add_room(x1,y1,place-1,y2, wall, door, prawd_braku_sciany);
    add_room(place+1,y1,x2,y2, wall, door, prawd_braku_sciany);
  }
  else if (vertical==false) // pozioma sciana
  {
    place=y1+(random(y2-1-y1))+1;
    if (!lower_random ((int) (NO_WALL_IN_ROOM_PROBABILITY*100),100) || (x2-x1)>30)
    {
      // narysowanie sciany
      for (index=x1;index<=x2;index++)
      {
#ifdef DRAW_MAP_CREATION
        print_character(index,place,'#');
        myrefresh();
#endif
        cells[index][place]=*wall;
      }
      // dodanie drzwi gdy na koncach sciany jest sciana
        for (index=0;index<=(x2-x1)/DOOR_ON_WALL_LENGTH;index++)
        {
          random_value=x1+(random(x2-1-x1))+1;
          
          cells[random_value][place]=*door; // !!! poprawka
          
#ifdef DRAW_MAP_CREATION
          print_character(random_value,place,'/');
          myrefresh();
#endif
        }
    }
    
    add_room(x1,y1,x2,place-1, wall, door, prawd_braku_sciany);
    add_room(x1,place+1,x2,y2, wall, door, prawd_braku_sciany);
  }
  return 0;
}

void MAP :: CreateMapBuilding()
{
   int x, y, a;
   CELL *predefined_floor, *predefined_wall, *predefined_door;
   
   predefined_floor=definitions.find_terrain(M_FLOOR);
   predefined_wall=definitions.find_terrain(M_WALL);

   // fill the map with blocking cells and set all cells to not seen
   
   for(x=0; x<MAPWIDTH; x++)
      for(y=0; y<MAPHEIGHT; y++)
         this->cells[x][y]=*predefined_floor;
   
   predefined_door=definitions.find_terrain("강철 문 (열림)");
   add_room(1,1,MAPWIDTH-2,MAPHEIGHT-2, predefined_wall, predefined_door, NO_WALL_IN_ROOM_PROBABILITY);

   // kasujemy zbedne drzwi (dwoje obok siebie) i dajemy ramke
   int count;
   
   for(x=0; x<MAPWIDTH; x++)
   {
      for(y=0; y<MAPHEIGHT; y++)
      {
         count=0;
         // delete drzwi
         if (cells[x][y]==*predefined_door) // drzwi
         {
           if (onMap(x-1,y))
           {
              if (cells[x-1][y]==*predefined_door)
                cells[x-1][y]=*predefined_wall;
              if (cells[x-1][y]==*predefined_wall)
                count++;
           }
           if (onMap(x+1,y))
           {
              if (cells[x+1][y]==*predefined_door)
                cells[x+1][y]=*predefined_wall;
              else if (cells[x+1][y]==*predefined_wall)
                count++;
           }
           if (onMap(x,y+1))
           {
              if (cells[x][y+1]==*predefined_door)
                cells[x][y+1]=*predefined_wall;
              if (cells[x][y+1]==*predefined_wall)
                count++;
           }
           if (onMap(x,y-1))
           {
              if (cells[x][y-1]==*predefined_door)
                cells[x][y-1]=*predefined_wall;
              if (cells[x][y-1]==*predefined_wall)
                count++;
           }
           // delete drzwi na rogach
           if (count>=3)
              cells[x][y]=*predefined_floor;
         }
         
         // dodanie ramki
         if (x==0 || x==MAPWIDTH-1 || y==0 || y==MAPHEIGHT-1)
           this->cells[x][y]=*predefined_wall;
         this->cells[x][y].seen=false;
      }
   }

   // change niektorych drzwi na puste pole

   for(x=0; x<MAPWIDTH; x++)
      for(y=0; y<MAPHEIGHT; y++)
      {
         if (cells[x][y]==*predefined_door)
         if (random(5)==0)
		 {
            cells[x][y]=*predefined_floor;
            break;
		 }
      }

   // usuniecie zbednych drzwi w scianach poziomych i poziomych
   // (takich, gdzie sciana nie konczy sie w innej)

   for(x=0; x<MAPWIDTH; x++)
   {
      for(y=0; y<MAPHEIGHT; y++)
      {
         if (cells[x][y]==*predefined_door)
         {
#ifdef DRAW_MAP_CREATION
           print_character(x,y,'?');
           myrefresh();
#endif
           // w to_left
           for (a=x;a>=0;a--)
           {
              if (onMap(a-1,y))
              {
                if (cells[a-1][y]==*predefined_floor)
                {
                   if (onMap(a,y-1))
                   {
                     if (cells[a][y-1]==*predefined_wall || cells[a][y-1]==*predefined_door)
                       break;
                   }
                   if (onMap(a,y+1))
                   {
                     if (cells[a][y+1]==*predefined_wall || cells[a][y+1]==*predefined_door)
                       break;
                   }
                   cells[x][y]=*predefined_wall;
                   cells[a][y]=*predefined_floor;
                   break;
                }
                
              }
           } // endof for


           // w to_right
           for (a=x;a<MAPWIDTH;a++)
           {
              if (onMap(a+1,y))
              {
                   if (onMap(a,y-1))
                   {
                     if (cells[a][y-1]==*predefined_wall || cells[a][y-1]==*predefined_door)
                       break;
                   }
                   if (onMap(a,y+1))
                   {
                     if (cells[a][y+1]==*predefined_wall || cells[a][y+1]==*predefined_door)
                       break;
                   }
                   
                if (cells[a+1][y]==*predefined_floor)
                {
                   cells[x][y]=*predefined_wall;
                   cells[a][y]=*predefined_floor;
                   break;
                }
                
              }
           } // endof for


           // w gore
           for (a=y;a>=0;a--)
           {
              if (onMap(x,a-1))
              {
                if (cells[x][a-1]==*predefined_floor)
                {
                   if (onMap(x-1,a))
                   {
                     if (cells[x-1][a]==*predefined_wall || cells[x-1][a]==*predefined_door)
                       break;
                   }
                   if (onMap(x+1,a))
                   {
                     if (cells[x+1][a]==*predefined_wall || cells[x+1][a]==*predefined_door)
                       break;
                   }
                   cells[x][y]=*predefined_wall;
                   cells[x][a]=*predefined_floor;
                   break;
                }
                
              }
           } // endof for


           // w dol
           for (a=y;a<MAPHEIGHT;a++)
           {
              if (onMap(x,a+1))
              {
                if (cells[x][a+1]==*predefined_floor)
                {
                   if (onMap(x+1,a))
                   {
                     if (cells[x+1][a]==*predefined_wall || cells[x+1][a]==*predefined_door)
                       break;
                   }
                   if (onMap(x-1,a))
                   {
                     if (cells[x-1][a]==*predefined_wall || cells[x-1][a]==*predefined_door)
                       break;
                   }
                   cells[x][y]=*predefined_wall;
                   cells[x][a]=*predefined_floor;
                   break;
                }
                
              }
           } // endof for
           
         }
      }
   }


   // change niektorych drzwi na zamkniete

   for(x=0; x<MAPWIDTH; x++)
      for(y=0; y<MAPHEIGHT; y++)
      {
         if (cells[x][y]==*predefined_door)
         if (random(4)==0)
            cells[x][y]=*definitions.find_terrain(M_CLOSED_DOOR);
      }

///////////////////////////////////// TO TESTOWE
	 // create stairs down
	 CELL *floor = definitions.find_terrain("바닥");
	 CELL *elevator = definitions.find_terrain("아래로 통하는 계단");
	 list <string>::iterator j,_j;
	 for (j = level.stairs_down[level.ID].begin(),_j = level.stairs_down[level.ID].end();j!=_j;j++)
	 {
		 STAIRS *stairs = new STAIRS;
		 stairs_of_map.push_back(stairs);
		 while(1)
		 {
			 x = random(MAPWIDTH-3)+1;
			 y = random(MAPHEIGHT-3)+1;
			 if (cells[x][y]==*floor)
			 {
				 cells[x][y]=*elevator;
				 stairs->to_where = *j;
				 stairs->name = level.levels[*j].name + " 입구";
				 stairs->x = x;
				 stairs->y = y;
				 level.map.MarkStairs(stairs->x,stairs->y);					 
				 stairs->lead_up = false;
				 stairs->number = 1;
				 break;
			 }
		 }
	 }

	 // create stairs up
	 elevator = definitions.find_terrain("위로 통하는 계단");
	 for (j = level.stairs_up[level.ID].begin(),_j = level.stairs_up[level.ID].end();j!=_j;j++)
	 {
		 STAIRS *stairs = new STAIRS;
		 stairs_of_map.push_back(stairs);
		 while(1)
		 {
			 x = random(MAPWIDTH-3)+1;
			 y = random(MAPHEIGHT-3)+1;
			 if (cells[x][y]==*floor)
			 {
				 cells[x][y]=*elevator;
				 stairs->to_where = *j;
				 stairs->name = level.levels[*j].name + " 입구";
				 stairs->x = x;
				 stairs->y = y;
				 level.map.MarkStairs(stairs->x,stairs->y);					 
				 stairs->lead_up = true;
				 stairs->number = 1;
				 break;
			 }
		 }
	 }
	  
   return;
}


void MAP :: CreateMapCaves()
{
	int x,y;
	int fill;

   CELL *predefined_floor=definitions.find_terrain("바닥");
   CELL *predefined_wall=definitions.find_terrain("돌 벽");

   // fill the map with blocking cells and set all cells to not seen
   for(x=0; x<MAPWIDTH; x++)
   {
      for(y=0; y<MAPHEIGHT; y++)
      {
         this->cells[x][y]=*predefined_floor;
      }
   }

   // generate a game of life like cave
//   for(int fill=0; fill<(MAPWIDTH*MAPHEIGHT*0.55); fill++)
   for(fill=0; fill<(MAPWIDTH*MAPHEIGHT*0.65); fill++)
   {
      this->cells[(int)(((float)rand()/RAND_MAX) * MAPWIDTH)][(int)(((float)rand()/RAND_MAX) * MAPHEIGHT)]=*predefined_wall;
   }

#ifndef CONST_MAP
   for(x=20; x<40; x++)
      for(y=15; y<25; y++)
      {
        cells[x][y]=*predefined_floor;
      }
#endif

   int count=0;
   for(int apply=0; apply<1; apply++)
   {
      for(x=0; x<MAPWIDTH; x++)
      {
         for(y=0; y<MAPHEIGHT; y++)
         {
            count=0;
            if(this->blockLOS(x-1,y-1)) count++;   // NW
            if(this->blockLOS(x,y-1)) count++;     // N
            if(this->blockLOS(x+1,y-1)) count++;   // NE
            if(this->blockLOS(x+1,y)) count++;     // E
            if(this->blockLOS(x+1,y+1)) count++;   // SE
            if(this->blockLOS(x,y+1)) count++;     // S
            if(this->blockLOS(x-1,y+1)) count++;   // SW
            if(this->blockLOS(x-1,y)) count++;     // W

            // a wall is removed if there are less than four adjacent walls
            if(this->blockLOS(x,y))
            {
               if(count<4)
               {
                 this->cells[x][y]=*predefined_floor;
               }
            }
            // a wall is created if there more than four adjacent walls
            else
            {
               if(count>4)
               {
                   this->cells[x][y]=*predefined_wall;
               }
            }


            if (x==0 || x==MAPWIDTH-1 || y==0 || y==MAPHEIGHT-1)
                   this->cells[x][y]=*predefined_wall;
            

         }
      }
   }

///////////////////////////////////// TO TESTOWE
	 // create stairs down
	 CELL *floor = definitions.find_terrain("바닥");
	 CELL *elevator = definitions.find_terrain("아래로 통하는 계단");
	 list <string>::iterator j,_j;
	 for (j = level.stairs_down[level.ID].begin(),_j = level.stairs_down[level.ID].end();j!=_j;j++)
	 {
		 STAIRS *stairs = new STAIRS;
		 stairs_of_map.push_back(stairs);
		 while(1)
		 {
			 x = random(MAPWIDTH-3)+1;
			 y = random(MAPHEIGHT-3)+1;
			 if (cells[x][y]==*floor)
			 {
				 cells[x][y]=*elevator;
				 stairs->to_where = *j;
				 stairs->name = level.levels[*j].name + " 입구";
				 stairs->x = x;
				 stairs->y = y;
				 level.map.MarkStairs(stairs->x,stairs->y);					 
				 stairs->lead_up = false;
				 stairs->number = 1;
				 break;
			 }
		 }
	 }

	 // create stairs up
	 elevator = definitions.find_terrain("위로 통하는 계단");
	 for (j = level.stairs_up[level.ID].begin(),_j = level.stairs_up[level.ID].end();j!=_j;j++)
	 {
		 STAIRS *stairs = new STAIRS;
		 stairs_of_map.push_back(stairs);
		 while(1)
		 {
			 x = random(MAPWIDTH-3)+1;
			 y = random(MAPHEIGHT-3)+1;
			 if (cells[x][y]==*floor)
			 {
				 cells[x][y]=*elevator;
				 stairs->to_where = *j;
				 stairs->name = level.levels[*j].name + " 입구";
				 stairs->x = x;
				 stairs->y = y;
				 level.map.MarkStairs(x,y);					 
				 stairs->lead_up = true;
				 stairs->number = 1;
				 break;
			 }
		 }
	 }
}


MAP::MAP(void)
{
	ClassName = "MAP";
	UniqueNumber = MAP_UNIQUE_NUMBER;

	// TO FIX STRANGE LINKING ERROR LNK2019 in fov.cpp	
	setCornerState(0,0,getCornerState(0,0));
}

bool MAP::onMap(int x, int y)
{
   return (x>=0 && x<MAPWIDTH && y>=0 && y<MAPHEIGHT);
}

int MAP::getCornerState(const int &x, const int &y)
{
	if (cornerOnMap(x,y))
		return corners[x][y];
	return 0;
}

void MAP::setCornerState(const int &x, const int &y, const int &state)
{
	if (cornerOnMap(x,y))
		corners[x][y]=state;
}

bool MAP::cornerOnMap(const int &x, const int &y)
{
	if (x>=0 && x<MAPWIDTH+1 && y>=0 && y<MAPHEIGHT+1)
		return true;
	return false;
}


bool MAP::blockLOS(int x, int y)
{
   if(!onMap(x,y)) return true;

   return cells[x][y].block_los;
}

bool MAP::blockMove(int x, int y)
{
   if(!onMap(x,y)) return true;

   if (isShield(x,y))
	   return true;

   return cells[x][y].block_move;
}

void MAP::setBlockLOS(int x, int y)
{
   if(!onMap(x,y)) return;
   cells[x][y].block_los=true;
}

void MAP::backBlockLOS(int x, int y)
{
   if(!onMap(x,y)) return;
   this->cells[x][y].block_los=this->cells[x][y].real_block_los;
}


void MAP::setBlockMove(int x, int y)
{
   if(!onMap(x,y)) return;
   this->cells[x][y].block_move=true;
}

void MAP::backBlockMove(int x, int y)
{
   if(!onMap(x,y)) return;
   this->cells[x][y].block_move=this->cells[x][y].real_block_move;
}


void MAP::setSeen(int x, int y, bool state)
{
   if(!onMap(x,y)) return;

   this->cells[x][y].seen=state;
}

void MAP::setSeenByPlayer(int x, int y)
{
	if(!onMap(x,y)) return;
	
	this->cells[x][y].seen_by_player=true;
	this->cells[x][y].seen_at_least_once=true;
}

void MAP::setSeenByCamera(int x, int y)
{
	if(!onMap(x,y)) return;
	
	this->cells[x][y].seen_by_camera=true;
}

bool MAP::isShield(int x, int y)
{
   if(!onMap(x,y)) return false;

   return cells[x][y].shield>0;
}

bool MAP :: isOpenable(int x, int y)
{
   if(!onMap(x,y)) return false;

   return (this->cells[x][y].about_open==OPEN_FALSE);
}

bool MAP :: isClosable(int x, int y)
{
   if(!onMap(x,y)) return false;

   return (this->cells[x][y].about_open==OPEN_TRUE);
}


void MAP::addShield(int x, int y, int value)
{
   if(!onMap(x,y)) return;
   cells[x][y].shield+=value;
   if (cells[x][y].shield<0)
		cells[x][y].shield = 0;
}

int MAP::getShield(int x, int y)
{
	if(!onMap(x,y)) return 0;
	
	return cells[x][y].shield;
}


void MAP::removeShield(int x, int y)
{
   if(!onMap(x,y)) return;

   this->cells[x][y].shield=0;
}

void MAP::MarkMonsterOnMap(int x, int y)
{
	if(!onMap(x,y)) return;	
//	assert(this->cells[x][y].is_monster==false);
	this->cells[x][y].is_monster=true;
}

void MAP::UnMarkMonsterOnMap(int x, int y)
{
	if(!onMap(x,y)) return;	

//	assert(this->cells[x][y].is_monster==true);
	
	this->cells[x][y].is_monster=false;
}

bool MAP::IsMonsterOnMap(int x, int y)
{
	if(!onMap(x,y)) return false;	
	return this->cells[x][y].is_monster;
}


void MAP::IncraseNumberOfItems(int x, int y)
{
	if(!onMap(x,y)) return;	
	this->cells[x][y].number_of_items++;
}

void MAP::DecraseNumberOfItems(int x, int y)
{
	if(!onMap(x,y)) return;	
	this->cells[x][y].number_of_items--;

	assert(this->cells[x][y].number_of_items>=0);
}   

int MAP::GetNumberOfItems(int x, int y)
{
	if(!onMap(x,y)) return false;	
	assert(this->cells[x][y].number_of_items>=0);

	return this->cells[x][y].number_of_items;
}

void MAP::IncraseNumberOfGases(int x, int y)
{
	if(!onMap(x,y)) return;	
	this->cells[x][y].number_of_gases++;
}

void MAP::DecraseNumberOfGases(int x, int y)
{
	if(!onMap(x,y)) return;	
	this->cells[x][y].number_of_gases--;
	
	assert(this->cells[x][y].number_of_gases>=0);
}   


int MAP::GetNumberOfGases(int x, int y)
{
	if(!onMap(x,y)) return false;	
	assert(this->cells[x][y].number_of_gases>=0);
	
	return this->cells[x][y].number_of_gases;
}

bool MAP::AreStairs(int x, int y)
{
	if(!onMap(x,y)) return false;	
	return this->cells[x][y].is_stairs;
}

void MAP::MarkStairs(int x, int y)
{
	if(!onMap(x,y)) return;	
	this->cells[x][y].is_stairs=true;
}

void MAP::UnMarkStairs(int x, int y)
{
	if(!onMap(x,y)) return;	
	this->cells[x][y].is_stairs=false;
}

STAIRS::~STAIRS()
{
  level.map.UnMarkStairs(x,y);
}


// na razie proste - nie tworzymy itemu
bool MAP::destroyCell(int x, int y)
{
  CELL *found;
  if (!onMap(x,y))
    return false;
  found=definitions.find_terrain(this->cells[x][y].destroyed_to);
  if (found==NULL)
    return false;
  cells[x][y] = *found;
  return true;
}

int MAP::getPercentDamage(int x, int y)
{
   int value;
   if(!onMap(x,y)) return false;

   value=100*this->cells[x][y].hit_points/this->cells[x][y].max_hit_points;
   return 100-value; // "100- value" aby uszkodzenia
}

bool MAP::damage(int x, int y, int value)
{
   if(!onMap(x,y)) return false;
   if (this->cells[x][y].hit_points>=0)
     this->cells[x][y].hit_points-=value;
   else
     return false;
     
   if (this->cells[x][y].hit_points<0)
   {
     destroyCell(x,y);
   }
   return true;
}

bool MAP::seen(int x, int y)
{
   if (x<0 || x>=MAPWIDTH || y<0 || y>=MAPHEIGHT)
     return false;
     
   return (cells[x][y].seen);
}

bool MAP::seen_once(int x, int y)
{
	if (x<0 || x>=MAPWIDTH || y<0 || y>=MAPHEIGHT)
		return false;

	return (cells[x][y].seen_at_least_once);
}


bool MAP::seen_by_player(int x, int y)
{
	if (x<0 || x>=MAPWIDTH || y<0 || y>=MAPHEIGHT)
		return false;
	
	if (level.is_player_on_level==false)
		return false;

	if (cells[x][y].seen_by_camera==true)
		return true;
	
	return (cells[x][y].seen_by_player);
}

bool MAP::seen_by_camera(int x, int y)
{
	if (x<0 || x>=MAPWIDTH || y<0 || y>=MAPHEIGHT)
		return false;
	
	return (cells[x][y].seen_by_camera);
}


//

void MAP::setLastTile(int x, int y, int tile)
{
   if(!onMap(x,y)) return;

   this->cells[x][y].last_tile=tile;
}

string MAP :: getCellName(int x, int y)
{
   if (!onMap(x,y))
     return string("");

   // check stairs
   STAIRS *stairs = level.map.StairsAt(x,y);
   if (stairs!=NULL)
	  return stairs->name;

   return this->cells[x][y].name;
}

int MAP :: getTile (int x, int y)
{
   if (!onMap(x,y))
     return ' ';
   return this->cells[x][y].tile;
}

void MAP::display()
{
   char shield_colors[5]={1,3,9,11,15};
   int x,y;

   for(x=0; x<MAPWIDTH; x++)
   {
      for(y=0; y<MAPHEIGHT; y++)
      {
         if(seen_by_player(x,y))
         {
            this->cells[x][y].last_tile=this->cells[x][y].tile;

            if (this->cells[x][y].shield)
            {
              set_color(shield_colors[random(5)]);
              print_character(x,y,'*');
            }
            else
            {
              set_color(this->cells[x][y].color);
              print_character(x,y,this->cells[x][y].tile);
            }
         }
         else
         {
            if (this->cells[x][y].seen_at_least_once==false)
            {
              set_color(0);
              print_character(x,y,' ');
            }
            else
            {
			  if (this->cells[x][y].is_stairs==true)
				set_color(14);
			  else
				set_color(8);
              print_character(x,y,this->cells[x][y].last_tile);
            }
            this->cells[x][y].seen=false;
         }
      }
   }
}



struct Room {
	int x1,y1,x2,y2;
	int type;
	inline bool IsInRoom(int x,int y) { return (x>=x1 && x<=x2 && y>=y1 && y<=y2); };
};

bool MAP::AddDoorsInShuttle()
{
     ptr_list :: iterator m;
     ptr_list pozycje;

     CELL *wall=definitions.find_terrain("우주선 벽");
	 CELL *door;
	 door=definitions.find_terrain("자동문 (닫힘)");

     int data[MAPWIDTH+1][MAPHEIGHT+1];
     struct POSITION *pos;
	 int x,y,xm,ym;

	 int number_of_rooms = 0;

// zero 

    for(x=0; x<MAPWIDTH; x++)
         for(y=0; y<MAPHEIGHT; y++)
		 {
			if (blockMove(x,y))
				data[x][y] = -2;
			else
				data[x][y] = -1;

#ifdef DRAW_MAP_CREATION
			set_color(cells[x][y].color);
			print_character(x,y,cells[x][y].tile); // !!!
#endif
		 }
#ifdef DRAW_MAP_CREATION
 		 myrefresh();
#endif
	 // 1. stworzenie listy pomieszczen do links

		 // 1.1 - znalezienie niedodanego pomieszczenia
    for(ym=0; ym<MAPHEIGHT; ym++)
		for(xm=0; xm<MAPWIDTH; xm++)
		 {
			 if (data[xm][ym]==-1)
			 {
				 pos=new POSITION;
				 pos->x=xm;
				 pos->y=ym;
				 data[xm][ym]=number_of_rooms;
				 pozycje.push_back(pos);

					 while(1)
					 {
						if (pozycje.size()==0)
						 break;
						m=pozycje.begin();
						pos=(POSITION *)*m;
         
						x=pos->x;
						y=pos->y;
            
						pozycje.pop_front();
						delete pos;
            
#ifdef DRAW_MAP_CREATION
						int value=data[x][y];
						set_color(7);
			            print_character(x,y,'0'+value); // !!!
#endif
            

						if ( data[x+1][y] == -1 )
						{
						  data[x+1][y] = number_of_rooms;
						  pos=new POSITION;
						  pos->x=x+1;
						  pos->y=y;
						  pozycje.push_back(pos);
						}

						if ( data[x-1][y] == -1 )
						{
						  data[x-1][y] = number_of_rooms;
						  pos=new POSITION;
						  pos->x=x-1;
						  pos->y=y;
						  pozycje.push_back(pos);
						}

						if ( data[x][y+1] == -1 )
						{
						  data[x][y+1] = number_of_rooms;
						  pos=new POSITION;
						  pos->x=x;
						  pos->y=y+1;
						  pozycje.push_back(pos);
						 }

						if ( data[x][y-1] == -1 )
						{
						  data[x][y-1] = number_of_rooms;
						  pos=new POSITION;
						  pos->x=x;
						  pos->y=y-1;
						  pozycje.push_back(pos);
						 }
					 }

					 number_of_rooms++;
			 } // endof if

		 } // endof for

#ifdef DRAW_MAP_CREATION
    myrefresh(); // !!!
#endif
	if (number_of_rooms<5 || number_of_rooms>29)
		return false;

//  2. add doors
	int room1,room2,a,b;
	int links[30][30];
	bool enclosure[30][30];

    for(x=0; x<number_of_rooms; x++)
         for(y=0; y<number_of_rooms; y++)
			 links[x][y] = 0;

	int time_guard=0;
	while(1)
	{
check_again:
		if (time_guard++>1000) // to na wypadek bledu w generowaniu
			return false;

		x=random(MAPWIDTH-2)+1;
		y=random(MAPHEIGHT-2)+1;

		for (;x<MAPWIDTH-2;x++)
		{
			for (;y<MAPHEIGHT-2;y++)
			{
				if (data[x][y]==-2) // sciana
				{
					room1 = data[x+1][y];
					room2 = data[x-1][y];
					if (room1>=0 && room2>=0 && room1!=room2)
						if (links[room1][room2]<2)
						{
							if (random(10)==0 || links[room1][room2]==0)
							{
								links[room1][room2]++;
								links[room2][room1]++;
								cells[x][y] = *door;
								data[x][y]=-1;

#ifdef DRAW_MAP_CREATION
						set_color(14);
			            print_character(x,y,'/'); // !!!
			            myrefresh(); 
#endif
							}

							x=random(MAPWIDTH-2)+1;
							y=random(MAPHEIGHT-2)+1;
						}
					room1 = data[x][y-1];
					room2 = data[x][y+1];
					if (room1>=0 && room2>=0 && room1!=room2)
						if (links[room1][room2]<2)
						{
							if (random(10)==0 || links[room1][room2]==0)
							{
								links[room1][room2]++;
								links[room2][room1]++;
								cells[x][y] = *door;
								data[x][y]=-1;
#ifdef DRAW_MAP_CREATION
						set_color(14);
			            print_character(x,y,'/'); // !!!
			            myrefresh(); 
#endif
							}


							x=random(MAPWIDTH-2)+1;
							y=random(MAPHEIGHT-2)+1;
						}
				} // endof if
			} // endof for
		}// endof for

		// 2.1 zbadanie zamkniecia przechodniego

		// 2.1.1 zero
		for (a=0;a<number_of_rooms;a++)
			for (b=0;b<number_of_rooms;b++)
				if (links[a][b]!=0)
					enclosure[a][b]=true;
				else
					enclosure[a][b]=false;

                // 2.1.2 Algorytm chyba Warshalla
		for (a=0;a<number_of_rooms;a++)
		{
			for (b=0;b<number_of_rooms;b++)
			{
				if (enclosure[a][b]==true && a!=b)
				{
					for (int c=0;c<number_of_rooms;c++)
					{
						if (enclosure[b][c]==true)
						{
							enclosure[a][c]=true;
							enclosure[c][a]=true;
						}
					}
				}
			}
		}
		// 2.1.3 Sprawdzenie
		for (a=0;a<number_of_rooms;a++)
		{
			for (b=0;b<number_of_rooms;b++)
			{
				if (enclosure[a][b]==false)
				{
					goto check_again;
				}
			}
		}
		return true; // all 1 - pomieszczenia przechodnie
	} // endof while
}

#define ROOM_MIN_SIZE 3
#define ROOM_MAX_SIZE 15

typedef list < Room > rooms_list;

void MAP::CreateMapShuttle()
{
	const int max_number_of_rooms = random(5)+8;
	int number_of_rooms;
	int number_of_free;
	rooms_list rooms_list;
	rooms_list::iterator m,_m;
	Room temp;
	int x,y;

    CELL *floor, *wall , *door, *undef, *wares, *boxes;
	undef=definitions.find_terrain("우주선 벽");
	floor=definitions.find_terrain("바닥");
	wall =definitions.find_terrain("우주선 벽");
	door=definitions.find_terrain("자동문 (닫힘)");
	wares=definitions.find_terrain("화물 더미");
	boxes=definitions.find_terrain("상자 더미");

	int x1,y1,x2,y2, rx, ry;

	while(1) // tworzymy statek az nie bedzie dobry
	{
	rooms_list.clear();
#ifdef DRAW_MAP_CREATION
	myclear();
#endif
	// wypelnienie scianami
    for(x=0; x<MAPWIDTH; x++)
         for(y=0; y<MAPHEIGHT; y++)
			 cells[x][y] = *undef;


	for (number_of_rooms=0;number_of_rooms<max_number_of_rooms;)
	{
		if (number_of_rooms==0)
		{
			x1 = MAPWIDTH/2 - random(ROOM_MAX_SIZE);
			y1 = MAPHEIGHT/2 - random(ROOM_MAX_SIZE) - ROOM_MIN_SIZE;
			rx = random(ROOM_MAX_SIZE) + ROOM_MIN_SIZE;
			ry = random(ROOM_MAX_SIZE - ROOM_MIN_SIZE) + ROOM_MIN_SIZE;
			x2 = x1+rx;
			y2 = MAPHEIGHT/2;
			if (x2 >= MAPWIDTH)
				continue;
		}
		else
		{
			x1 = random(MAPWIDTH - ROOM_MIN_SIZE) + 1;
			y1 = random(MAPHEIGHT - ROOM_MIN_SIZE)/2 + 1;
			rx = random(ROOM_MAX_SIZE - ROOM_MIN_SIZE) + ROOM_MIN_SIZE;
			ry = random(ROOM_MAX_SIZE - ROOM_MIN_SIZE) + ROOM_MIN_SIZE;
			x2 = x1+rx;
			y2 = y1+ry;

			if (x2 >= MAPWIDTH-1 || y2 >= MAPHEIGHT/2 + 3)
				continue;
		}
			
			// check, czy nie jest w istniej퉏ym
			bool rand_again= false;
			for (m = rooms_list.begin(), _m = rooms_list.end();m!=_m;m++)
			{
				temp = *m;
				rand_again = true;
				if (temp.IsInRoom(x1,y1))
				{
					if (!temp.IsInRoom(x2,y2))
					{
						rand_again = false;
						temp.type++;
					}
					break;
				}
				if (temp.IsInRoom(x2,y2))
				{
					if (!temp.IsInRoom(x1,y1))
					{
						rand_again = false;
						temp.type++;
					}
					break;
				}
			}
			if (rand_again)
				continue;
			// create room
			temp.x1 = x1;
			temp.y1 = y1;
			temp.x2 = x2;
			temp.y2 = y2;
			if (number_of_rooms == 0)
				temp.type = 0;

			rooms_list.push_back(temp);
			number_of_rooms++;

	} // endof for

	// mirror
	m = rooms_list.begin();
	int index;
	for (index = 0;index<number_of_rooms;index++,m++)
	{
			temp = *m;
			temp.y1 = MAPHEIGHT - temp.y1 -1;
			temp.y2 = MAPHEIGHT - temp.y2 -1;		
			y1 = temp.y1;
			temp.y1 = temp.y2;
			temp.y2 = y1;
			rooms_list.insert(m, temp);
	};
	for (m = rooms_list.begin(), _m = rooms_list.end();m!=_m;m++)
	{
			temp = *m;
			for (x=temp.x1;x<=temp.x2;x++)
				for (y=temp.y1;y<=temp.y2;y++)
				{
					if (cells[x][y] == *undef)
						cells[x][y] = *floor;
					cells[x][y].color = temp.type;

				}			
	}

#ifdef DRAW_MAP_CREATION
	// wyrysowanie
    for(x=0; x<MAPWIDTH; x++)
         for(y=0; y<MAPHEIGHT; y++)
		 {
			if (cells[x][y] == *floor)
			{
				set_color(cells[x][y].color);
				print_character(x,y,'#'); 
			}
		 }
	myrefresh();
#endif

	// create walls on connections
	number_of_free = 0;
    for(x=0; x<MAPWIDTH-1; x++)
         for(y=0; y<MAPHEIGHT/2; y++)
		 {
			 if (cells[x][y].color!= cells[x+1][y].color && cells[x+1][y] != *wall)
				 cells[x][y]= *wall;
			 else if (cells[x][y].color != cells[x][y+1].color && cells[x][y+1] != *wall)
				 cells[x][y]= *wall;
			 else if (cells[x][y].color != cells[x+1][y+1].color && cells[x+1][y+1] != *wall)
				 cells[x][y]= *wall;
			 
			 if (cells[x][y]!= *wall)
				 number_of_free+=2; // +2 bo za lustrzane odbicie

			 cells[x][MAPHEIGHT-y-1] = cells[x][y]; // i odbicie lustrzane
		 }

//	size of ship

		 if (number_of_free<500)
			 continue;

		 if (AddDoorsInShuttle())
		 {
			 // Create Wares

			 list < POSITION > positions;
			 for(x=1; x<MAPWIDTH-2; x++)
				 for(y=1; y<MAPHEIGHT-2; y++)
				 {
					 bool ok=true;
					 for (int a=0;a<3 && ok;a++)
						 for (int b=0;b<3;b++)
							 if (cells[x+a][y+b] != *floor)
							 {
								 ok=false;
								 break;
							 }
							 if (ok)
							 {
								 POSITION pos;
								 pos.x=x+1;
								 pos.y=y+1;
								 if (random(10)!=0)
									 positions.push_back(pos);
							 }
				 }

				 list < POSITION >::iterator m,_m;
				 for (m=positions.begin(),_m=positions.end();m!=_m;m++)
				 {
					 if (random(5)!=0)
						cells[(*m).x][(*m).y]=*wares;
					 else
						cells[(*m).x][(*m).y]=*boxes;
				 }
			 break;
		 }

	} 

	// dodanie items automatycznie otwierajacych drzwi
    for(x=0; x<MAPWIDTH; x++)
         for(y=0; y<MAPHEIGHT; y++)
		 {
			 if (cells[x][y] == *door)
			 {
				 level.create_item(x,y,"Door Opener",0,0);
			 }
		 }

	 // find place for player on I level
	 if (level.player!=NULL && (level.player->pX()==-1 || level.player->pY()==-1))
	 {
		 while(1)
		 {
				 x = random(MAPWIDTH-3)+1;
				 y = random(MAPHEIGHT-3)+1;
				 if (cells[x][y]==*floor)
				 {
					 level.player->ChangePosition(x, y);
					 break;
				 }
		 }
	 }

	 // create stairs down
	 CELL *elevator = definitions.find_terrain("아래로 통하는 계단");
	 list <string>::iterator j,_j;
	 for (j = level.stairs_down[level.ID].begin(),_j = level.stairs_down[level.ID].end();j!=_j;j++)
	 {
		 STAIRS *stairs = new STAIRS;
		 stairs_of_map.push_back(stairs);
		 while(1)
		 {
			 x = random(MAPWIDTH-3)+1;
			 y = random(MAPHEIGHT-3)+1;
			 if (cells[x][y]==*floor && (cells[x-1][y]==*wall || cells[x+1][y]==*wall || cells[x][y-1]==*wall || cells[x][y+1]==*wall))
			 {
				 cells[x][y]=*elevator;
				 stairs->to_where = *j;
				 stairs->name = level.levels[*j].name + "으로";
				 stairs->x = x;
				 stairs->y = y;
				 level.map.MarkStairs(stairs->x,stairs->y);					 
				 stairs->lead_up = false;
				 stairs->number = 1;
				 break;
			 }
		 }
	 }

	 // create stairs up
	 elevator = definitions.find_terrain("위로 통하는 계단");
	 for (j = level.stairs_up[level.ID].begin(),_j = level.stairs_up[level.ID].end();j!=_j;j++)
	 {
		 if (*j == "START")
		 {
#ifdef TEST_TELEPORT_ON_START
			screen.console.add("TEST TELEPORT GENERATED TO " + string(TEST_TELEPORT_TO),15);
			 while(1)
			 {
				 x = random(MAPWIDTH-3)+1;
				 y = random(MAPHEIGHT-3)+1;
				 if (cells[x][y]==*floor && (cells[x-1][y]==*wall || cells[x+1][y]==*wall || cells[x][y-1]==*wall || cells[x][y+1]==*wall))
					break;
			 }
			 STAIRS *stairs = new STAIRS;
			 stairs_of_map.push_back(stairs);
			 cells[x][y]=*elevator;
			 stairs->to_where = TEST_TELEPORT_TO;
			 stairs->name = string("Teleport to ") + TEST_TELEPORT_TO;
			 stairs->x = x;
			 stairs->y = y;
			 level.map.MarkStairs(stairs->x,stairs->y);					 
			 stairs->lead_up = true;
			 stairs->number = 3;
#endif
			 continue;
		 }
		 STAIRS *stairs = new STAIRS;
		 stairs_of_map.push_back(stairs);
		 while(1)
		 {
			 x = random(MAPWIDTH-3)+1;
			 y = random(MAPHEIGHT-3)+1;
			 if (cells[x][y]==*floor)
			 {
				 cells[x][y]=*elevator;
				 stairs->to_where = *j;
				 stairs->name = level.levels[*j].name + "으로";
				 stairs->x = x;
				 stairs->y = y;
				 level.map.MarkStairs(stairs->x,stairs->y);					 
				 stairs->lead_up = true;
				 stairs->number = 1;
				 break;
			 }
		 }
	 }
	 return;
}

struct MazeCorridor {
	int x,y;
	int length;
	char direction;
	MazeCorridor() { length=0;direction=random(4);};
};

typedef list < MazeCorridor *> list_of_maze_corridors;

#define MC_TURN_LEFT 15
#define MC_TURN_RIGHT 30
#define MC_FORWARD 100
#define MC_CROSSROAD 3
#define MC_LIFETIME 200
#define MAX_EMPTY_COUNT MAPWIDTH*MAPHEIGHT/3

void MAP::CreateMapUnderground(bool interpolation)
{
   int x,y;
   int number_pustych = 0;

   CELL *predefined_floor=definitions.find_terrain("바닥");
   CELL *predefined_wall=definitions.find_terrain("돌 벽");

   list_of_maze_corridors list_of_corridors;
   list_of_maze_corridors::iterator m,_m;

   MazeCorridor *temp, *temp2;

   // zero map
   for(x=0; x<MAPWIDTH; x++)
      for(y=0; y<MAPHEIGHT; y++)
	  {
         cells[x][y]=*predefined_wall;
#ifdef DRAW_MAP_CREATION
			print_character(x,y,'#');
#endif
	  }
#ifdef DRAW_MAP_CREATION
			myrefresh();
#endif

	// losowy first
	temp=new MazeCorridor;
	temp->x = MAPWIDTH/2;
	temp->y = MAPHEIGHT/2;

	list_of_corridors.push_back(temp);
	
   while(list_of_corridors.size()>0)
	{
		if (number_pustych>MAX_EMPTY_COUNT)
			break;
		for (m=list_of_corridors.begin(),_m=list_of_corridors.end();m!=_m;)
		{
			temp = *m;
			m++; // aby usuniecie nie psulo
			temp->length++;
			if (temp->length>MC_LIFETIME)
			{
				list_of_corridors.remove(temp);
				delete temp;
				break;
			}

			if (cells[temp->x][temp->y]!=*predefined_floor)
			{
				cells[temp->x][temp->y]=*predefined_floor;
				number_pustych++;
#ifdef DRAW_MAP_CREATION
			print_character(temp->x,temp->y,'.');
			myrefresh();
#endif
			}


			int random_value = random(100);

			if (random_value<MC_TURN_LEFT)
			{
				temp->direction--;
				if (temp->direction<0)
					temp->direction=3;
			}
			else if (random_value<MC_TURN_RIGHT)
			{
				temp->direction++;
				if (temp->direction>3)
					temp->direction=0;
			}
			else
			{
				switch (temp->direction)
				{
				case 0:
					if (temp->y>1)
						temp->y--;
					break;
				case 1:
					if (temp->x<MAPWIDTH-2)
						temp->x++;
					break;
				case 2:
					if (temp->y<MAPHEIGHT-2)
						temp->y++;
					break;
				case 3:
					if (temp->x>1)
						temp->x--;
					break;
				} // endof switch

				// korytarze, mniej sal
				if (!blockLOS(x,y) && list_of_corridors.size()>5) // gdy corridor w tym miejscu
					if (random(3)==0) // death
					{
						temp->length+=MC_LIFETIME;
					}
			}

			if (random(100)<MC_CROSSROAD && list_of_corridors.size()<10)
			{
				temp2 = new MazeCorridor;
				temp2->x = temp->x;
				temp2->y = temp->y;
				list_of_corridors.push_back(temp2);
				break;
			}

		}
	}

   // delete pozostalych kopaczy
	for (m=list_of_corridors.begin(),_m=list_of_corridors.end();m!=_m;m++)
	{
		temp = *m;
        delete temp;
	}

//////////////////////////////////////////////////////////////////////////
	  if (interpolation)
      for(x=0; x<MAPWIDTH; x++)
      {
         for(y=0; y<MAPHEIGHT; y++)
         {
			int count=0;
            if(this->blockLOS(x-1,y-1)) count++;   // NW
            if(this->blockLOS(x,y-1)) count++;     // N
            if(this->blockLOS(x+1,y-1)) count++;   // NE
            if(this->blockLOS(x+1,y)) count++;     // E
            if(this->blockLOS(x+1,y+1)) count++;   // SE
            if(this->blockLOS(x,y+1)) count++;     // S
            if(this->blockLOS(x-1,y+1)) count++;   // SW
            if(this->blockLOS(x-1,y)) count++;     // W

            if(this->blockLOS(x,y))
            {
               if(count<4)
               {
                 this->cells[x][y]=*predefined_floor;
               }
            }
         }
      }

//////////////////////////////////////////////////////////////////////////


///////////////////////////////////// TO TESTOWE
	 // create stairs down
	 CELL *floor = definitions.find_terrain("바닥");
	 CELL *elevator = definitions.find_terrain("아래로 통하는 계단");
	 list <string>::iterator j,_j;
	 for (j = level.stairs_down[level.ID].begin(),_j = level.stairs_down[level.ID].end();j!=_j;j++)
	 {
		 STAIRS *stairs = new STAIRS;
		 stairs_of_map.push_back(stairs);
		 while(1)
		 {
			 x = random(MAPWIDTH-3)+1;
			 y = random(MAPHEIGHT-3)+1;
			 if (cells[x][y]==*floor)
			 {
				 cells[x][y]=*elevator;
				 stairs->to_where = *j;
				 stairs->name = level.levels[*j].name + " 입구";
				 stairs->x = x;
				 stairs->y = y;
				 level.map.MarkStairs(stairs->x,stairs->y);					 
				 stairs->lead_up = false;
				 stairs->number = 1;
				 break;
			 }
		 }
	 }

	 // create stairs up
	 elevator = definitions.find_terrain("위로 통하는 계단");
	 for (j = level.stairs_up[level.ID].begin(),_j = level.stairs_up[level.ID].end();j!=_j;j++)
	 {
		 STAIRS *stairs = new STAIRS;
		 stairs_of_map.push_back(stairs);
		 while(1)
		 {
			 x = random(MAPWIDTH-3)+1;
			 y = random(MAPHEIGHT-3)+1;
			 if (cells[x][y]==*floor)
			 {
				 cells[x][y]=*elevator;
				 stairs->to_where = *j;
				 stairs->name = level.levels[*j].name + " 입구";
				 stairs->x = x;
				 stairs->y = y;
				 level.map.MarkStairs(stairs->x,stairs->y);					 
				 stairs->lead_up = true;
				 stairs->number = 1;
				 break;
			 }
		 }
	 }

	return;
}

void MAP :: CreateMapCapsule()
{
	string mx[15];
	mx[0]  = "##:::::::::::::::::";
	mx[1]  = "##,,,,,,,,,,,***:::";
	mx[2]  = "##,,,,,,,,,,*****::";
	mx[3]  = "###,,,##,,,#**>**::";
	mx[4]  = "###,,,##,,,#**+**::";
	mx[5]  = "####-####-####.####";
	mx[6]  = "####.............##";
	mx[7]  = "#<................#";
	mx[8]  = "####.............##";
	mx[9]  = "####-####-####-####";
	mx[10] = "###,,,##,,,##,,,#::";
	mx[11] = "###,,,##,,,##,,,#::";
	mx[12] = "##,,,,,,,,,,,,,,,,:";
	mx[13] = "##,,,,,,,,,,,,,,,,:";
	mx[14] = "##:::::::::::::::::";
	
	int x,y;
	int px,py;
	
	CELL *wall=definitions.find_terrain("우주선 벽");
	CELL *floor = definitions.find_terrain("바닥");	
	CELL *space = definitions.find_terrain("우주");
	CELL *far_space = definitions.find_terrain("먼 우주");
	CELL *door=definitions.find_terrain("자동문 (닫힘)");
	CELL *door_bad = definitions.find_terrain("캡슐 문 (닫힘)");
	CELL *lift_up=definitions.find_terrain("위로 통하는 계단");
	CELL *lift_down=definitions.find_terrain("아래로 통하는 계단");
	CELL *capsule=definitions.find_terrain("구명 캡슐");
	CELL *type;
	STAIRS *stairs;

	// zero map
	for(x=0; x<MAPWIDTH; x++)
		for(y=0; y<MAPHEIGHT; y++)
		{
			cells[x][y]=*wall;
#ifdef DRAW_MAP_CREATION
			print_character(x,y,'#');
#endif
		}
		
	for (y=0;y<=14;y++)
	{
		for (x=0;x<mx[0].size();x++)
		{
			int character = mx[y][x];
			px = x+MAPWIDTH/2-(mx[0].size()/2);
			py = y+MAPHEIGHT/2-8;
			
			switch(character) {
			case '#':
				type = wall;
				break;
			case ':':
				type = far_space;
				break;
			case ',':
				type = space;
				break;
			case '.':
				type = floor;
				break;
			case '+':
				type = door;
				level.create_item(px,py,"Door Opener",0,0);				
				break;
			case '*':
				type = capsule;
				break;
			case '-':
				type = door_bad;
				break;
			case '<':
				level.player->ChangePosition(px,py);

				type = lift_up;

				stairs = new STAIRS;
				stairs_of_map.push_back(stairs);
				stairs->to_where = "s04";
				stairs->name = "우주 왕복선 4층으로";
				stairs->x = px;
				stairs->y = py;
			    level.map.MarkStairs(stairs->x,stairs->y);					 
				stairs->lead_up = true;
				stairs->number = 1;				
				break;
			case '>':
				type = lift_down;
				stairs = new STAIRS;
				stairs_of_map.push_back(stairs);
				stairs->to_where = RESCUE_CAPSULE_TO;
				stairs->name = "캡슐 발사대";
				stairs->x = px;
				stairs->y = py;
			    level.map.MarkStairs(stairs->x,stairs->y);					 
				stairs->lead_up = false;
				stairs->number = 10;						
				break;
			default:
				fprintf(stderr,"ERROR: Nieznany type w definicji Rescue Capsules\n");
				exit(10);				
			}
			cells[px][py] = *type;
			if (type == lift_down)
				cells[px][py].color = 13;
#ifdef DRAW_MAP_CREATION
			print_character(px,py,character);
			myrefresh();
#endif
			
		}
	}

	int ax,ay;
	level.find_empty_place(ax,ay);
	level.create_monster(ax,ay,"제노트로이드",0,0);				
				
#ifdef DRAW_MAP_CREATION
	myrefresh();
#endif
	return;	
}	


void MAP :: CreateMapCityNW()
{
	int x,y;
	int px,py;
	
	CELL *door  = definitions.find_terrain("자동문 (닫힘)");
	CELL *platform = definitions.find_terrain("이착륙 플랫폼");	
	CELL *capsule = definitions.find_terrain("구명 캡슐");

	CELL *wall=definitions.find_terrain("건물 벽");
	CELL *dome = definitions.find_terrain("돔");	
	CELL *door1 = definitions.find_terrain("건물 문 (열림)");
	CELL *door2 = definitions.find_terrain("건물 문 (닫힘)");
	CELL *floor = definitions.find_terrain("바닥");
	CELL *space = definitions.find_terrain("우주");
	CELL *enterance = definitions.find_terrain("위로 통하는 계단");
	
	// zero map

	for(x=0; x<MAPWIDTH; x++)
		for(y=0; y<MAPHEIGHT; y++)
			cells[x][y]=*floor;

	// stworzenie ramek

	for (x=0;x<MAPWIDTH;x++)
	{
		cells[x][0]=*dome;
		cells[x][MAPHEIGHT-1]=*dome;
	}
	for (y=0;y<MAPHEIGHT;y++)
	{
		cells[0][y]=*dome;
		cells[MAPWIDTH-1][y]=*dome;
	}

	// zaokraglenie Lewy Gorny
	int dist;

	for (y=0;y<MAPHEIGHT;y++)
		for (x=0;x<MAPHEIGHT;x++)
		{
			dist = distance(MAPHEIGHT,MAPHEIGHT,x,y);
			if (dist>MAPHEIGHT)
			{
				cells[x][y] = *dome;
			}
		}

	// stworzenie wyjsc

	// polnocne
	x = MAPWIDTH/2;
	y = 1;
	cells[x-1][y] = *dome;
	cells[x+1][y] = *dome;
	cells[x][y+1] = *space; // aby w wejsciu nie bylo budynku
	
	// poludniowe
	x = MAPWIDTH/2;
	y = MAPHEIGHT-2;
	cells[x-1][y] = *dome;
	cells[x+1][y] = *dome;
	cells[x][y-1] = *space; // aby w wejsciu nie bylo budynku
	
	// wschodnie
	x = MAPWIDTH-2;
	y = MAPHEIGHT/2;
	cells[x][y-1] = *dome;
	cells[x][y+1] = *dome;		
	cells[x-1][y] = *space; // aby w wejsciu nie bylo budynku

	
	// stworzenie platformy i kapsuly
	while (FindRectangleOfType(floor, px, py, 13, 13, 1)==false);

	for (x=0;x<11;x++)
		for (y=0;y<11;y++)
			cells[px + x+1][py + y+1] = *platform;

	// zrobienie kapsuly
	string mx[4];
    mx[0]  = ",***,";
	mx[1]  = "*****";
	mx[2]  = "**.**";
	mx[3]  = "**+**";
	
	CELL *chosen_one;

	for (x=0;x<5;x++)
		for (y=0;y<4;y++)
		{
			switch(mx[y][x]) {
			case '*':
				chosen_one = capsule;
				break;
			case ',':
				chosen_one = platform;
				break;
			case '.':
				chosen_one = floor;
				break;
			case '+':
				chosen_one = door;
				level.create_item(px + x + 4,py + y + 4,"Door Opener",0,0);								
				break;
			}
			cells[px + x + 4][py + y + 4] = *chosen_one;
		}
	
	// tymczasowe ustawienie gracza
	
	level.player->ChangePosition(px+6 , py+6);
				
		
	// stworzenie budynkow

	int buildings = 8 + random(5);
	int size_x,size_y;

	for (int a=0;a<buildings;a++)
	{
		size_x = random(8)+4;
		size_y = random(8)+4;

		// znalezienie miejsca na budynek

		if (FindRectangleOfType(floor, px, py, size_x, size_y, 5))
		{
				add_room(px+1,py+1,px+size_x-1,py+size_y-1, wall, door1, 0);
				// wylosowanie, gdzie drzwi
				int drzwi_pion = random(2);
				int drzwi_x, drzwi_y;

				if (drzwi_pion==true)
					drzwi_y = random(size_y-1)+1;
				else
					drzwi_x = random(size_x-1)+1;
				
				for (y=0;y<=size_y;y++)
				{
					if (drzwi_pion && drzwi_y==y && (py+y)%2==0)
						cells[px][py+y] = *door1;
					else
						cells[px][py+y] = *wall;

					if (cells[px-1][py+y]==*floor)
						cells[px-1][py+y] = *space; // ramka dla budynku, aby sie nie stykaly

					if (drzwi_pion && drzwi_y==y && (py+y)%2!=0)
						cells[px+size_x][py+y] = *door1;
					else
						cells[px+size_x][py+y] = *wall;

					if (cells[px+size_x+1][py+y]==*floor) 
						cells[px+size_x+1][py+y] = *space; // ramka dla budynku, aby sie nie stykaly
				}
				for (x=0;x<=size_x;x++)
				{
					if (!drzwi_pion && drzwi_x==x && (px+x)%2==0)
						cells[px+x][py] = *door1;
					else
						cells[px+x][py] = *wall;
					
					if (cells[px+x][py-1]==*floor)
						cells[px+x][py-1] = *space; // ramka dla budynku, aby sie nie stykaly

					if (!drzwi_pion && drzwi_x==x && (px+x)%2!=0)
						cells[px+x][py+size_y] = *door1;
					else
						cells[px+x][py+size_y] = *wall;

					if (cells[px+x][py+size_y+1] == *floor)
						cells[px+x][py+size_y+1] = *space; // ramka dla budynku, aby sie nie stykaly
				}

				// pokolorowanie wnetrza domu
				int k,l;
				k = random(size_x-1)+1;
				l = random(size_y-1)+1;

				for (y=0;y<size_y;y++)
					for (x=0;x<size_x;x++)
					{
						if (cells[x+px][y+py]==*floor)
								cells[x+px][y+py].color = 6;
					}
		}
	} // endof generacji budynkow

	// poprawienie map
	for (y=1;y<MAPHEIGHT-2;y++)
		for (x=1;x<MAPWIDTH-2;x++)
		{
			if (cells[x][y]==*space) // delete ramek budynkow
				cells[x][y] = *floor;
			else if (cells[x][y] == *door1) // change niektorych drzwi na zamkniete
			{
				if (coin_toss())
					cells[x][y] = *door2;
			}		
		}

	// dodanie wyjsc z sektora miasta

	CELL *c_exit = definitions.find_terrain("아래로 통하는 계단");
	STAIRS *stairs = new STAIRS;
	stairs_of_map.push_back(stairs);
	cells[MAPWIDTH/2][MAPHEIGHT-2]=*c_exit;
	stairs->to_where = "csw";
	stairs->name = "남서쪽 지구로";
	stairs->x = MAPWIDTH/2;
	stairs->y = MAPHEIGHT-2;
	level.map.MarkStairs(stairs->x,stairs->y);					 
	stairs->lead_up = false;
	stairs->number = 10;				


	c_exit = definitions.find_terrain("위로 통하는 계단");
	stairs = new STAIRS;
	stairs_of_map.push_back(stairs);
	cells[MAPWIDTH/2][1]=*c_exit;
	stairs->to_where = "gl0";
	stairs->name = "유전학 연구동 입구";
	stairs->x = MAPWIDTH/2;
	stairs->y = 1;
	level.map.MarkStairs(stairs->x,stairs->y);					 	
	stairs->lead_up = true;
	stairs->number = 11;	

	sounds.PlaySoundOnce("data/sounds/other/land.wav");
	
	return;
}

void MAP :: CreateMapCitySW()
{
	int x,y;

	CELL *wall=definitions.find_terrain("건물 벽");
	CELL *dome = definitions.find_terrain("돔");	
	CELL *door1 = definitions.find_terrain("건물 문 (열림)");
	CELL *door2 = definitions.find_terrain("건물 문 (닫힘)");
	CELL *floor = definitions.find_terrain("바닥");
	CELL *enterance = definitions.find_terrain("아래로 통하는 계단");
	STAIRS *stairs;

	// zero map

	for(x=0; x<MAPWIDTH; x++)
		for(y=0; y<MAPHEIGHT; y++)
			cells[x][y]=*floor;

	// stworzenie ramek

	for (x=0;x<MAPWIDTH;x++)
	{
		cells[x][0]=*dome;
		cells[x][MAPHEIGHT-1]=*dome;
	}
	for (y=0;y<MAPHEIGHT;y++)
	{
		cells[0][y]=*dome;
		cells[MAPWIDTH-1][y]=*dome;
	}

	// zaokraglenie Lewy dolny
	int dist;

	for (y=0;y<MAPHEIGHT;y++)
		for (x=0;x<MAPHEIGHT;x++)
		{
			dist = distance(MAPHEIGHT,0,x,y);
			if (dist>MAPHEIGHT)
			{
				cells[x][y] = *dome;
			}
		}

	// stworzenie wyjsc

	// polnocne
	x = MAPWIDTH/2;
	y = 1;
	cells[x-1][y] = *dome;
	cells[x+1][y] = *dome;
	
	// poludniowe
	x = MAPWIDTH/2;
	y = MAPHEIGHT-2;
	cells[x-1][y] = *dome;
	cells[x+1][y] = *dome;
	
	// wschodnie
	x = MAPWIDTH-2;
	y = MAPHEIGHT/2;
	cells[x][y-1] = *dome;
	cells[x][y+1] = *dome;		
	
	// stworzenie budynkow okraglych

	int px,py,random_value;

	for (int a=0;a<5;a++)
	{
		int r = 4 + random(4);
		if (FindRectangleOfType(floor, px, py, r*2+4, r*2+4, 1))
		{
			// nie bawimy sie w bresenhama
			for (x=0;x<=r*2+2;x++)
			{
				for (y=0;y<=r*2+2;y++)
				{
					if (distance(r+1,r+1,x,y)<r+1)
						cells[px+x+1][py+y+1].color = 8;
				}
			}
			
			for (random_value=0;random_value<360;random_value++)
			{
				x = px+r+2 + round_up((r*sin(random_value*3.14195/180)));
				y = py+r+2 + round_up((r*cos(random_value*3.14195/180)));

				cells[x][y] = *wall;
			}
			// enterance
			random_value = random(4)*90;
			x = px+r+2 + round_up((r*sin(random_value*3.14195/180)));
			y = py+r+2 + round_up((r*cos(random_value*3.14195/180)));				
			cells[x][y] = *floor;

			if (a==0) // enterance do kopalni w pierwszym
			{
				stairs = new STAIRS;
				stairs_of_map.push_back(stairs);
				cells[px+r+2][py+r+2]=*enterance;
				stairs->to_where = "m01";
				stairs->name = "광산으로";
				stairs->x = px+r+2;
				stairs->y = py+r+2;
				level.map.MarkStairs(stairs->x,stairs->y);					 
				stairs->lead_up = false;
				stairs->number = 1;								
			}
		}
	}	

	// dodanie wyjsc z sektora miasta

	CELL *c_exit = definitions.find_terrain("위로 통하는 계단");
	stairs = new STAIRS;
	stairs_of_map.push_back(stairs);
	cells[MAPWIDTH/2][1]=*c_exit;
	stairs->to_where = "cnw";
	stairs->name = "북서쪽 지구로";
	stairs->x = MAPWIDTH/2;
	stairs->y = 1;
	level.map.MarkStairs(stairs->x,stairs->y);					 
	stairs->lead_up = true;
	stairs->number = 10;			

	// tymczasowe ustawienie gracza

	level.player->ChangePosition(MAPWIDTH/2,1);
		
	return;
}



bool MAP :: FindRectangleOfType(CELL *type, int &px, int &py, int size_x, int size_y, int grid)
{
	int x,y;
	bool not_blocked;

	for (int b=0;b<1000;b++)
	{
		not_blocked=true;
		px = random((MAPWIDTH-size_x-3)/grid)*grid+2;
		py = random((MAPHEIGHT-size_y-3)/grid)*grid+2;
		
		for (y=0;y<=size_y && not_blocked;y++)
		{
			for (x=0;x<=size_x  && not_blocked;x++)
			{
				if (onMap(x+px,y+py))
				{
					if (cells[x+px][y+py]!=*type)
						not_blocked = false;
				}
			} // endof for
		} // endof for
		if (not_blocked)
			break;
	}
	
	return not_blocked;
}

void MAP::CollapseCeiling(int x,int y)
{
	CELL *zwalony=definitions.find_terrain("무너진 천장");
	for (int a=-1;a<=1;a++)
		for (int b=-1;b<=1;b++)
		{
			if (onMap(x+a,y+b))
				if (!blockMove(x+a,y+b))
					cells[x+a][y+b] = *zwalony;
		}
}

void MAP::CreateMapMines()
{
	int x,y,px,py,sx,sy;

	rooms_list :: iterator m;
	rooms_list rooms;
	Room temp;
	
	CELL *wall=definitions.find_terrain("돌 벽");
	CELL *floor = definitions.find_terrain("바닥");
	CELL *corridor = definitions.find_terrain("우주");
	CELL *cantilever = definitions.find_terrain("지지대");
	int random_value;
	int diff_x, diff_y;
	int px1,py1,px2,py2;
				
	// zero map
	
	for(x=0; x<MAPWIDTH; x++)
		for(y=0; y<MAPHEIGHT; y++)
			cells[x][y]=*wall;
		
	// Rozmieszczenie pomieszczen

	int pomieszczen = random(5)+12;
#ifdef DRAW_MAP_CREATION
	myclear();
	myrefresh();
#endif
	
	for (int a=0;a<pomieszczen;a++)
	{
		sx = random(5)+6;
		sy = random(5)+6;		
		if (FindRectangleOfType(wall, px, py, sx+4, sy+4, 1))
		{
			px+=2;
			py+=2;
			// polaczenie go z juz istniejacym

			if (rooms.size()>0)
			{
				random_value = random(rooms.size());
				m = rooms.begin();
				while(random_value--)
					m++;

				// srodek tego
				px1= px+sx/2;
				py1= py+sy/2;
				// srodek drugiego
				px2= (*m).x1 + ((*m).x2 - (*m).x1)/2;
				py2= (*m).y1 + ((*m).y2 - (*m).y1)/2;
				// znalezienie drogi laczacej te dwa pomieszczenia

				diff_x = px2-px1;
				diff_y = py2-py1;

				if (diff_x<0)
					diff_x=-diff_x;
				if (diff_y<0)
					diff_y=-diff_y;
				
				x=px1; y=py1;

//				while (x!=px2 && y!=py2)
				while (!(diff_x==0 && diff_y==0))
				{
					if (lower_random(diff_x,diff_x+diff_y)) // posuwamy sie w poziomie
					{
						diff_x--;
						if (x>px2)
							x--;
						else
							x++;
					}
					else
					{
						diff_y--;
						if (y>py2)
							y--;
						else
							y++;
					}
					// check, co na tej pozycji
#ifdef DRAW_MAP_CREATION
					print_character(x,y,'.');
					myrefresh();				
#endif
					if (cells[x][y]==*floor)
						break;
					else if (cells[x][y]==*corridor)
						if (coin_toss())
							break;

					cells[x][y]=*corridor;
				}
			}

			// dodanie do listy

			temp.x1=px;
			temp.y1=py;
			temp.x2=px+sx;
			temp.y2=py+sy;
			temp.type = a;
			rooms.push_back(temp);

			// narysowanie pomieszczenia
			
			int type = random(3);
			if (sx==sy)
				type=3;
			
			if (type!=2)
			{
				for (y=0;y<sy;y++)
					for (x=0;x<sx;x++)
					{
						switch (type)
						{
						case 0: // prostokat
						case 1: // prostokat
							cells[px+x][py+y]=*floor;
#ifdef DRAW_MAP_CREATION
							print_character(px+x,py+y,'*'); 
#endif
							break;
						case 3: // okragle
							if (distance(sx/2,sx/2,x,y)<sx/2)
							{
								cells[px+x][py+y]=*floor;
#ifdef DRAW_MAP_CREATION
								print_character(px+x,py+y,'*'); 
#endif
							}
							break;
						}
					}
			} // endof if
			else // type==2
			{
				for (y=0;y<=sy/2;y++)
					for (x=0;x<=sx/2;x++)
					{
						if (y>=x)
						{
							cells[px+x+sx/2][py+y]=*floor; // pg
							cells[px+x+sx/2][py+sy-y]=*floor; // pd
							cells[px+sx/2-x][py+y]=*floor;
							cells[px+sx/2-x][py+sy-y]=*floor;
							
#ifdef DRAW_MAP_CREATION
							print_character(px+x+sx/2,py+y,'1'); // pg
							myrefresh();
							print_character(px+x+sx/2,py+sy-y,'2'); // pd
							myrefresh();
							print_character(px+sx/2-x,py+y,'3');
							myrefresh();
							print_character(px+sx/2-x,py+sy-y,'4');
							myrefresh();							
#endif
						} // endof for
					} // endof for/for
			} // endof else
			
			
		} // endof dodawania pomieszczenia
	}

	// poprawienie map
	for (y=1;y<MAPHEIGHT-2;y++)
		for (x=1;x<MAPWIDTH-2;x++)
		{
			if (cells[x][y]==*corridor) // poprawienie korytarzy
			{
				if (random(40)==0)
				{
					cells[x][y] = *cantilever;
					level.create_item(x,y,"Weak Corridor in Mines",0,0);					
				}
				else
					cells[x][y] = *floor;
				if (random(40)==0)
				{
					level.create_item(x,y,"Smoke Generator",0,0);					
				}				
			}
		}
		
#ifdef DRAW_MAP_CREATION
	myrefresh();
#endif

	// stworzenie stairs

	///////////////////////////////////// TO TESTOWE
	// create stairs down
	CELL *elevator = definitions.find_terrain("아래로 통하는 계단");
	list <string>::iterator j,_j;
	for (j = level.stairs_down[level.ID].begin(),_j = level.stairs_down[level.ID].end();j!=_j;j++)
	{
		STAIRS *stairs = new STAIRS;
		stairs_of_map.push_back(stairs);
		while(1)
		{
			if (FindRectangleOfType(floor, x, y, 3, 3, 1))
			{
				x++;
				y++;
				cells[x][y]=*elevator;
				stairs->to_where = *j;
				stairs->name = level.levels[*j].name + " 입구";
				stairs->x = x;
				stairs->y = y;
				level.map.MarkStairs(stairs->x,stairs->y);					 
				stairs->lead_up = false;
				stairs->number = 1;
				break;
			}			
		}
	}
	
	// create stairs up
	elevator = definitions.find_terrain("위로 통하는 계단");
	for (j = level.stairs_up[level.ID].begin(),_j = level.stairs_up[level.ID].end();j!=_j;j++)
	{
		STAIRS *stairs = new STAIRS;
		stairs_of_map.push_back(stairs);
		while(1)
		{
			if (FindRectangleOfType(floor, x, y, 3, 3, 1))
			{
				x++;
				y++;
				cells[x][y]=*elevator;
				stairs->to_where = *j;
				stairs->name = level.levels[*j].name + " 입구";
				stairs->x = x;
				stairs->y = y;
				level.map.MarkStairs(stairs->x,stairs->y);					 
				stairs->lead_up = true;
				stairs->number = 1;
				break;
			}			
		}
	}

	return;
}

bool MAP::addDanger(int x, int y, int danger)
{
	if(onMap(x,y))
	{
		cells[x][y].danger_value |= danger;
		return true;
	}
	return false;
}

int MAP::getDanger(int x, int y)
{
	if(!onMap(x,y)) return DANGER_NONE;	
	return (cells[x][y].danger_value);
}

void MAP::CreateMapGeneticLab()
{
	while (CreateMapGeneticLabUnderground()==false);
}

bool MAP::CreateMapGeneticLabUnderground()
{
	rooms_list rooms;
	Room temp;
	Room temp2;
	int x,y;
	int number_of_rooms = 0;

	CELL *floor=definitions.find_terrain("바닥");
	CELL *corridor=definitions.find_terrain("복도");
	CELL *wall=definitions.find_terrain("플라스틱 벽");
	CELL *door[5];
	door[0]=definitions.find_terrain("플라스틱 문 (열림)");
	door[1]=definitions.find_terrain("플라스틱 문 (열림)");
	door[2]=definitions.find_terrain("강철 문 (열림)");
	door[3]=definitions.find_terrain("강철 문 (닫힘)");
	door[4]=definitions.find_terrain("복도");
	
#ifdef DRAW_MAP_CREATION
	myclear();
	myrefresh();
#endif	
	for(x=0; x<MAPWIDTH; x++)
		for(y=0; y<MAPHEIGHT; y++)
			cells[x][y] = *wall;
			
	
	temp.x1 = MAPWIDTH/2 - 3;
	temp.y1 = MAPHEIGHT/2 - 3;
	temp.x2 = MAPWIDTH/2 + 3;
	temp.y2 = MAPHEIGHT/2 + 3;

	// stworzenie roomu
	for (x=temp.x1;x<temp.x2;x++)
		for (y=temp.y1;y<temp.y2;y++)
		{
			cells[x][y] = *floor;		
#ifdef DRAW_MAP_CREATION
			print_character(x,y,'#');
			myrefresh();
#endif			
		}

	rooms.push_back(temp);

	while (rooms.size()>0)
	{
		temp = *rooms.begin();
		rooms.pop_front();
		number_of_rooms++;

		// dodanie pokoi
		int px,py,sizex,sizey,dl;
		bool not_blocked;

		px = (temp.x1 + temp.x2) /2;
		py = (temp.y1 + temp.y2) /2;

		// w gore
		dl = random(6) + 4;
		sizex = random(6) + 3;
		sizey = random(6) + 3;

		not_blocked = true;

		if (px - sizex < 0)
			not_blocked = false;
		if (px + sizex > MAPWIDTH)
			not_blocked = false;
		if (py - sizey < 0)
			not_blocked = false;
		if (py + sizey > MAPHEIGHT)
			not_blocked = false;
		
		for (x=px - sizex/2 -1;x<px - sizex/2 + sizex +1 && not_blocked;x++)
		{
			for (y=py - sizey/2 - dl -1;y<py - sizey/2 + sizey - dl +1 && not_blocked;y++)
			{
				if (!onMap(x,y))
				{
					not_blocked = false;
					break;
				}
				if (cells[x][y]!=*wall)
					not_blocked = false;
#ifdef DRAW_MAP_CREATION
				print_character(x,y,'?');
				myrefresh();
#endif
				
			}
		}

		if (not_blocked)
		{
			temp2.x1 = px - sizex/2;
			temp2.x2 = temp2.x1 + sizex;
			temp2.y1 = py - sizey/2 - dl;
			temp2.y2 = temp2.y1 + sizey;			

			rooms.push_back(temp2);

			for (x=temp2.x1;x<temp2.x2;x++)
				for (y=temp2.y1;y<temp2.y2;y++)
				{
					cells[x][y] = *floor;
#ifdef DRAW_MAP_CREATION
					print_character(x,y,'#');
					myrefresh();
#endif
				}
		}

		// doprowadzenie korytarza na polnoc do jakiegos pola

		for (y=temp.y1-1;y>0;y--)
		{
			if (!onMap(px,y))
				break;

#ifdef DRAW_MAP_CREATION
			print_character(px,y,'+');
			myrefresh();
#endif

			if (cells[px][y]==*floor)
			{
				for (y=temp.y1-1;y>0;y--)
				{
					if (cells[px][y]!=*floor)
					{
						cells[px][y]=*corridor;
#ifdef DRAW_MAP_CREATION
						print_character(px,y,'%');
						myrefresh();
#endif
					}
					else // drzwi
					{
						cells[px][y+1]=*door[random(5)];
						break;
					}
				}
				break;
			}

			if (onMap(px-1,y) && onMap(x+1,y))
				if (cells[px-1][y]!=*wall || cells[px+1][y]!=*wall)
					break;
				
		} // endof dodawania korytarza



		// w dol
		dl = random(6) + 4;
		sizex = random(6) + 3;
		sizey = random(6) + 3;
		
		not_blocked = true;

		if (px - sizex < 0)
			not_blocked = false;
		if (px + sizex > MAPWIDTH)
			not_blocked = false;
		if (py - sizey < 0)
			not_blocked = false;
		if (py + sizey > MAPHEIGHT)
			not_blocked = false;
		
		for (x=px - sizex/2 -1;x<px - sizex/2 + sizex+1 && not_blocked;x++)
		{
			for (y=py - sizey/2 + dl -1;y<py - sizey/2 + sizey + dl+1 && not_blocked;y++)
			{
				if (!onMap(x,y))
				{
					not_blocked = false;
					break;
				}
				if (cells[x][y]!=*wall)
					not_blocked = false;
#ifdef DRAW_MAP_CREATION
				print_character(x,y,'?');
				myrefresh();
#endif
				
			}
		}
		
		if (not_blocked)
		{
			temp2.x1 = px - sizex/2;
			temp2.x2 = temp2.x1 + sizex;
			temp2.y1 = py - sizey/2 + dl;
			temp2.y2 = temp2.y1 + sizey;			
			
			rooms.push_back(temp2);
			
			for (x=temp2.x1;x<temp2.x2;x++)
				for (y=temp2.y1;y<temp2.y2;y++)
				{
					cells[x][y] = *floor;
#ifdef DRAW_MAP_CREATION
					print_character(x,y,'#');
					myrefresh();
#endif
				}
		}
		
		// doprowadzenie korytarza na poludnie do jakiegos pola
		
		for (y=temp.y2;y<MAPHEIGHT;y++)
		{
			if (!onMap(px,y))
				break;
			
#ifdef DRAW_MAP_CREATION
			print_character(px,y,'+');
			myrefresh();
#endif
				
			if (cells[px][y]==*floor)
			{
				for (y=temp.y2;y<MAPHEIGHT;y++)
				{
					if (cells[px][y]!=*floor)
					{
						cells[px][y]=*corridor;
#ifdef DRAW_MAP_CREATION
						print_character(px,y,'%');
						myrefresh();
#endif
					}
					else // drzwi
					{
						cells[px][y-1]=*door[random(5)];
						break;
					}
				}
				break;
			}
			if ( onMap(px-1,y) && onMap(x+1,y) )
				if (cells[px-1][y]!=*wall || cells[px+1][y]!=*wall)
					break;
				
		} // endof dodawania korytarza



		// w to_left
		dl = random(6) + 4;
		sizex = random(6) + 3;
		sizey = random(6) + 3;
		
		not_blocked = true;

		if (px - sizex < 0)
			not_blocked = false;
		if (px + sizex > MAPWIDTH)
			not_blocked = false;
		if (py - sizey < 0)
			not_blocked = false;
		if (py + sizey > MAPHEIGHT)
			not_blocked = false;
		
		for (x=px - sizex/2 - dl -1;x<px - sizex/2 + sizex - dl+1 && not_blocked;x++)
		{
			for (y=py - sizey/2 -1;y<py - sizey/2 + sizey+1 && not_blocked;y++)
			{
				if (!onMap(x,y))
				{
					not_blocked = false;
					break;
				}
				if (cells[x][y]!=*wall)
					not_blocked = false;
#ifdef DRAW_MAP_CREATION
				print_character(x,y,'?');
				myrefresh();
#endif				
			}
		}
		
		if (not_blocked)
		{
			temp2.x1 = px - sizex/2 - dl;
			temp2.x2 = temp2.x1 + sizex;
			temp2.y1 = py - sizey/2;
			temp2.y2 = temp2.y1 + sizey;			
			
			rooms.push_back(temp2);
			
			for (x=temp2.x1;x<temp2.x2;x++)
				for (y=temp2.y1;y<temp2.y2;y++)
				{
					cells[x][y] = *floor;
#ifdef DRAW_MAP_CREATION
					print_character(x,y,'#');
					myrefresh();
#endif
				}
		}
		
		// doprowadzenie korytarza na wschod do jakiegos pola
		
		for (x=temp.x1-1;x>0;x--)
		{
			if (!onMap(x,py))
				break;
			
#ifdef DRAW_MAP_CREATION
			print_character(x,py,'+');
			myrefresh();
#endif
			
			if (cells[x][py]==*floor)
			{
				for (x=temp.x1-1;x>0;x--)
				{
					if (cells[x][py]!=*floor)
					{
						cells[x][py]=*corridor;
#ifdef DRAW_MAP_CREATION
						print_character(x,py,'%');
						myrefresh();
#endif
					}
					else // drzwi
					{
						cells[x+1][py]=*door[random(5)];
						break;
					}
				}
				break;
			}

			if (onMap(x,py-1) && onMap(x,py+1))
				if (cells[x][py-1]!=*wall || cells[x][py+1]!=*wall)
					break;
		} // endof dodawania korytarza



		// w to_right
		dl = random(6) + 4;
		sizex = random(6) + 3;
		sizey = random(6) + 3;
		
		not_blocked = true;

		if (px - sizex < 0)
			not_blocked = false;
		if (px + sizex > MAPWIDTH)
			not_blocked = false;
		if (py - sizey < 0)
			not_blocked = false;
		if (py + sizey > MAPHEIGHT)
			not_blocked = false;
		
		for (x=px - sizex/2 + dl-1;x<px - sizex/2 + sizex + dl+1 && not_blocked;x++)
		{
			for (y=py - sizey/2 -1;y<py - sizey/2 + sizey+1 && not_blocked;y++)
			{
				if (!onMap(x,y))
				{
					not_blocked = false;
					break;
				}
				if (cells[x][y]!=*wall)
					not_blocked = false;
#ifdef DRAW_MAP_CREATION
				print_character(x,y,'?');
				myrefresh();
#endif				
			}
		}
		
		if (not_blocked)
		{
			temp2.x1 = px - sizex/2 + dl;
			temp2.x2 = temp2.x1 + sizex;
			temp2.y1 = py - sizey/2;
			temp2.y2 = temp2.y1 + sizey;			
			
			rooms.push_back(temp2);
			
			for (x=temp2.x1;x<temp2.x2;x++)
				for (y=temp2.y1;y<temp2.y2;y++)
				{
					cells[x][y] = *floor;
#ifdef DRAW_MAP_CREATION
					print_character(x,y,'#');
					myrefresh();
#endif
				}
		}
		
		// doprowadzenie korytarza na zachod do jakiegos pola
		
		for (x=temp.x2;x<MAPWIDTH;x++)
		{
			if (!onMap(x,py))
				break;
			
#ifdef DRAW_MAP_CREATION
			print_character(x,py,'+');
			myrefresh();
#endif
				
			if (cells[x][py]==*floor)
			{
				for (x=temp.x2;x<MAPWIDTH;x++)
				{
					if (cells[x][py]!=*floor)
					{
						cells[x][py]=*corridor;
#ifdef DRAW_MAP_CREATION
						print_character(x,py,'%');
						myrefresh();
#endif
					}
					else // drzwi
					{
						cells[x-1][py]=*door[random(5)];
						break;
					}
				}
				break;
			}
			if (onMap(x,py-1) && onMap(x,py+1))
				if (cells[x][py-1]!=*wall || cells[x][py+1]!=*wall)
					break;
		} // endof dodawania korytarza	
	} // endof while

	// poziom musi byc duzy
	
	if (number_of_rooms<10)
		return false;


// stworzenie stairs

	///////////////////////////////////// TO TESTOWE
	// create stairs down
	CELL *elevator = definitions.find_terrain("아래로 통하는 계단");
	list <string>::iterator j,_j;
	for (j = level.stairs_down[level.ID].begin(),_j = level.stairs_down[level.ID].end();j!=_j;j++)
	{
		STAIRS *stairs = new STAIRS;
		stairs_of_map.push_back(stairs);
		while(1)
		{
			x = random(MAPWIDTH);
			y = random(MAPHEIGHT);
			if (cells[x][y]==*floor)
			{
				cells[x][y]=*elevator;
				stairs->to_where = *j;
				stairs->name = level.levels[*j].name + "으로";
				stairs->x = x;
				stairs->y = y;
				level.map.MarkStairs(stairs->x,stairs->y);					 
				stairs->lead_up = false;
				stairs->number = 1;
				break;
			}			
		}
	}

	// create stairs up
	if (level.ID!="gl0")
	{
		elevator = definitions.find_terrain("위로 통하는 계단");
		for (j = level.stairs_up[level.ID].begin(),_j = level.stairs_up[level.ID].end();j!=_j;j++)
		{
			STAIRS *stairs = new STAIRS;
			stairs_of_map.push_back(stairs);
			while(1)
			{
				x = random(MAPWIDTH);
				y = random(MAPHEIGHT);
				if (cells[x][y]==*floor)
				{
					cells[x][y]=*elevator;
					stairs->to_where = *j;
					stairs->name = level.levels[*j].name + "으로";
					stairs->x = x;
					stairs->y = y;
					level.map.MarkStairs(stairs->x,stairs->y);					 
					stairs->lead_up = true;
					stairs->number = 1;
					break;
				}			
			}
		}
	}
	else // stworzenie stairs w punkcie najbardziej na south
	{
		elevator = definitions.find_terrain("아래로 통하는 계단");
		STAIRS *stairs = new STAIRS;
		stairs_of_map.push_back(stairs);

		bool found=false;
		for (y=MAPHEIGHT-1;y>0 && !found;y--)
			for (x=0;x<MAPWIDTH  && !found;x++)
			{
				if (cells[x][y]==*wall && cells[x][y-1]==*floor)
					found=true;
			}
		y++;
		while(1)
		{
			x = random(MAPWIDTH);
			if (cells[x][y]==*wall && cells[x][y-1]==*floor)
				break;			
		}

		cells[x][y]=*elevator;
		stairs->to_where = "cnw";
		stairs->name = "도시 입구";
		stairs->x = x;
		stairs->y = y;
		level.map.MarkStairs(stairs->x,stairs->y);					 
		stairs->lead_up = false;
		stairs->number = 11;


		// dodanie laboratorium

		while(1)
		{
			x = random(MAPWIDTH);
			y = random(MAPHEIGHT);
			if (cells[x][y]==*floor)
			{
				cells[x][y] = *definitions.find_terrain("위로 통하는 계단");
				cells[x][y].color = 14;
				STAIRS *stairs = new STAIRS;
				stairs_of_map.push_back(stairs);
				stairs->to_where = "유전자 조작기";
				stairs->name = "유전자 조작기";
				stairs->x = x;
				stairs->y = y;
				level.map.MarkStairs(stairs->x,stairs->y);					 
				stairs->lead_up = true;
				stairs->number = 10;						

				for (int a=-1;a<2;a++)
					for (int b=-1;b<2;b++)
						if (onMap(x+a,y+b))
							if (cells[x+a][y+b]==*floor)
								cells[x+a][y+b].color = 1;

				break;
			}
				
		} // endof while
	}
	return true;
}


void MAP::CreateMapBunker()
{
	int x,y;

	CELL *mwall=definitions.find_terrain("강철 벽");
	CELL *pwall=definitions.find_terrain("기묘한 강철 벽");
	CELL *floor = definitions.find_terrain("하얀 바닥");
	CELL *swall = definitions.find_terrain("돌 벽");
	CELL *type;

	CELL *door=definitions.find_terrain("방공호 문");
	CELL *stairs_up=definitions.find_terrain("위로 통하는 계단");
	CELL *stairs_down=definitions.find_terrain("아래로 통하는 계단");
				
	// zero map
	
	for(x=0; x<MAPWIDTH; x++)
		for(y=0; y<MAPHEIGHT; y++)
		{
			if (x<2 || x>=MAPWIDTH-2 || y>=MAPHEIGHT-2)
			{
				cells[x][y]=*swall;
			}
			else
				cells[x][y]=*floor;
		}

	// dodanie reszty planszy
	int x1,y1;
	x1=63;
	y1=22;
	for(x=63; x<MAPWIDTH; x++)
		for(y=22; y<MAPHEIGHT; y++)
		{
			if (distance(x1,y1,x,y)>15)
			{
				cells[x][y]=*swall;
				if (random(4)==0)
				{
					cells[x-random(5)][y-random(3)]=*swall;
				}
			}
		}

	x1=17;
	y1=22;
	for(x=0; x<18; x++)
		for(y=22; y<MAPHEIGHT; y++)
		{
			if (distance(x1,y1,x,y)>15)
			{
				cells[x][y]=*swall;
				if (random(4)==0)
				{
					cells[x+random(5)][y+random(3)]=*swall;
				}
			}
		}


		// predefiniowana gora
		
		string mx[8];
		mx[0] = "##################.................................................#############";
		mx[1] = "##################.#############.########-.-########.#############.#############";
		mx[2] = "##################-#############-######+.....+######-#############-#############";
		mx[3] = "################..+..#%%%o%%%#..+..#####+...+#####..+..#%%%o%%%#..+..###########";
		mx[4] = "###...................................o.......o..............................###";
		mx[5] = "###..........................................................................###";
		mx[6] = "##o..........................................................................o##";
		mx[7] = "##............................................................................##";
		
		// Rozmieszczenie elementow
		for (y=0;y<8;y++)
		{
			for (x=0;x<mx[0].size();x++)
			{
				int character = mx[y][x];
				type=floor;
				
				switch(character) {
				case '#':
					type = mwall;
					break;
				case '%':
					type = pwall;
					break;
				case '-':
					type = door;
					break;
				case 'o':
					type = mwall;
					level.create_monster(x,y,"서치라이트",0,0);
					break;
				case '+':
					type = mwall;
					level.create_monster(x,y,"벙커건",0,0);
					break;
				}
				//			if (type!=floor)
				cells[x][y]=*type;				
				
#ifdef DRAW_MAP_CREATION
				print_character(x,y,character);
				myrefresh();
#endif			
			}
		}
		
	
#ifdef DRAW_MAP_CREATION
	myrefresh();
#endif

	// stworzenie stairs

	///////////////////////////////////// TO TESTOWE
	// create stairs down - do bunkra
	CELL *elevator = definitions.find_terrain("아래로 통하는 계단");
	list <string>::iterator j,_j;
	j = level.stairs_down[level.ID].begin();	
	STAIRS *stairs = new STAIRS;
	stairs_of_map.push_back(stairs);
	stairs->to_where = *j;
	stairs->name = level.levels[ *j ].name + " 입구";
	stairs->x = 42;
	stairs->y = 0;
	cells[stairs->x][stairs->y]=*elevator;
	level.map.MarkStairs(stairs->x,stairs->y);					 
	stairs->lead_up = false;
	stairs->number = 1;
	
	// create stairs up
	elevator = definitions.find_terrain("위로 통하는 계단");
	// tylko jedne schody na dole map
	j = level.stairs_up[level.ID].begin();
	stairs = new STAIRS;
	stairs_of_map.push_back(stairs);
	stairs->to_where = *j;
	stairs->name = level.levels[ *j ].name + " 입구";
	stairs->x = 40;
	stairs->y = 38;
	cells[stairs->x][stairs->y]=*elevator;
	level.map.MarkStairs(stairs->x,stairs->y);					 
	stairs->lead_up = true;
	stairs->number = 1;

	// player widzial raz cala mape
	for(x=1; x<MAPWIDTH-1; x++)
		for(y=3; y<MAPHEIGHT-1; y++)			
		{
			cells[x][y].seen_at_least_once=true;
			cells[x][y].last_tile=cells[x][y].tile;
		}
	return;
}

void MAP::CreateMapDoctorsCave()
{
	int x,y;

	CELL *type;
	CELL *floor=definitions.find_terrain("바닥");
	CELL *corridor=definitions.find_terrain("복도");
	CELL *wall=definitions.find_terrain("플라스틱 벽");
	CELL *swall = definitions.find_terrain("돌 벽");
				
	// zero map
	
	for(x=0; x<MAPWIDTH; x++)
		for(y=0; y<MAPHEIGHT; y++)
			cells[x][y]=*wall;

	// dodanie reszty planszy
	int x_of_stairs,y_of_stairs;
	int x_of_doctor, y_of_doctor;
		// predefiniowana mapa
		
		string mx[34];
		mx[0]  = "################################################################################";
		mx[1]  = "################################################################################";
		mx[2]  = "###################################################################%%%%%%%%#####";
		mx[3]  = "###################################################################%......%#####";
		mx[4]  = "###################################################################%......%%%%%%";
		mx[5]  = "############################...............########################%......%%...%";
		mx[6]  = "##########....................................................#####%......++.<.%";
		mx[7]  = "#####...................................#####..................####%......%%...%";
		mx[8]  = "####.....................................#####..................###%......%%%+%%";
		mx[9]  = "###...........................##.........#######................###%%%%+%%%#%+%%";
		mx[10] = "###...........................##...........#####.................#####%+%###%+%%";
		mx[11] = "###..........................................#.................#######%+%#%%%+%%";
		mx[12] = "###............................................................####%%%%+%%%....%";
		mx[13] = "###............................................................................%";
		mx[14] = "###............####...............................####....................%....%";
		mx[15] = "##...........########...............###.........######....................%%%%%%";
		mx[16] = "##............#####................####...........###.....................%#####";
		mx[17] = "##..............###.......D........####........................######%%%%%%#####";
		mx[18] = "##..................................####.......................#################";
		mx[19] = "##...............................................................###############";
		mx[20] = "##..................##................................##..........##############";
		mx[21] = "###................####...............................##.................#######";
		mx[22] = "###.................##....................................................######";
		mx[23] = "###...............######.......................#...........................#####";
		mx[24] = "###..............#######......................###.............####........######";
		mx[25] = "####............#########...........##.........###...........######.......######";
		mx[26] = "####............####..###..........####.......###...........#######........#####";
		mx[27] = "####.............####..#............##........###.............####........######";
		mx[28] = "######............##...........................#................##........######";
		mx[29] = "#######...................................................................######";
		mx[30] = "##########...............................................................#######";
		mx[31] = "#############...................................................################";
		mx[32] = "###########################.............########################################";
		mx[33] = "################################################################################";
		
		// Rozmieszczenie elementow
		for (y=0;y<34;y++)
		{
			for (x=0;x<mx[0].size();x++)
			{
				int character = mx[y][x];
				type=floor;
				
				switch(character) {
				case '#':
					type = swall;
					break;
				case '%':
					type = wall;
					break;
				case '+':
					type = corridor;
					break;
				case '<':
					x_of_stairs = x;
					y_of_stairs = y;
					break;
				case 'D':
					x_of_doctor = x;
					y_of_doctor = y;
					break;
				}
				cells[x][y]=*type;				
				
#ifdef DRAW_MAP_CREATION
				print_character(x,y,character);
				myrefresh();
#endif			
			}
		}
		
	
#ifdef DRAW_MAP_CREATION
	myrefresh();
#endif

	level.create_monster(x_of_doctor,y_of_doctor,"미친 박사",0,0);
	
	// stworzenie stairs

	///////////////////////////////////// TO TESTOWE
	// create stairs up
	CELL *elevator = definitions.find_terrain("위로 통하는 계단");
	// tylko jedne schody na dole map
	list <string>::iterator j,_j;
	j = level.stairs_up[level.ID].begin();
	STAIRS *stairs = new STAIRS;
	stairs_of_map.push_back(stairs);
	stairs->to_where = *j;
	stairs->name = level.levels[ *j ].name + " 입구";
	stairs->x = x_of_stairs;
	stairs->y = y_of_stairs;
	cells[stairs->x][stairs->y]=*elevator;
	level.map.MarkStairs(stairs->x,stairs->y);					 
	stairs->lead_up = true;
	stairs->number = 1;
	return;
}


void MAP :: CreateMapTutorial()
{
	string mx[30];
	mx[0]   = "#########################################################";
	mx[1]   = "#.....#####.....####....#################################";
	mx[2]   = "#..@!...!..!.......!....#################################";
	mx[3]   = "#.....#####.....####....#################################";
	mx[4]   = "#.....###############.###################################";
	mx[5]   = "###########....+...##]###################################";
	mx[6]   = "###########.###$##.##!################...################";
	mx[7]   = "##########$-######....################...################";
	mx[8]   = "##########...###################$$####...######......####";
	mx[9]   = "##########...####################>..+...!....!+......####";
	mx[10]  = "##########...#########################...######......####";
	mx[11]  = "##########.M.#########################...######......####";
	mx[12]  = "###########+##########################...######.....E####";
	mx[13]  = "###########!#...O###################################!####";
	mx[14]  = "#########........###################################.####";
	mx[15]  = "########.../|....O..##########################.....#.####";
	mx[16]  = "########....!....##|$#########################.###.#.####";
	mx[17]  = "#########.......O##.########.................#.###.#.####";
	mx[18]  = "###################.########.###############.#.###.#.####";
	mx[19]  = "###################O########.###############.#.###.#.####";
	mx[20]  = "##..........##.BB!..########.##...........##.#.###.#.####";
	mx[21]  = "##...........g##############.##.#########.##.#.###...####";
	mx[22]  = "##............$#############....#########!##.#.##########";
	mx[23]  = "##..............##################...####.##.#.##########";
	mx[24]  = "##..............##################.#.##...##.#.##########";
	mx[25]  = "##OO............############.M.M.+.#....####.#.##########";
	mx[26]  = "##OO.....##+################....############.#.....######";
	mx[27]  = "###########.#####$#.....!+....M.########.....#####.######";
	mx[28]  = "###########p....+C-...U.################.#########.######";
	mx[29]  = "###########$############################...........######";

	// 1. Wst?, poruszanie, help, message buffer
	// 1.1 Otwieranie drzwi
	// 2. Podnoszenie przedmiot?, inwentarz
	// 3. Otwieranie drzwi
	// 4. Look, walka
	// 5. Bro? amunicja
	// 6. Tryby strzelania
	// 7. Atak na teren
	// 8. Granaty i pigulki
	// 9. Budowa robota
	// 10. Explore i travel
	// 11. Energetyczne bronie i pancerze

	int x,y;
	int px,py;

	CELL *wall=definitions.find_terrain("강철 벽");
	CELL *floor = definitions.find_terrain("바닥");	
	CELL *door=definitions.find_terrain("플라스틱 문 (닫힘)");
	CELL *door2=definitions.find_terrain("강철 문 (닫힘)");
	CELL *boxes = definitions.find_terrain("상자 더미");	
	CELL *lift_down=definitions.find_terrain("아래로 통하는 계단");
	CELL *type;

	// zero map
	for(x=0; x<MAPWIDTH; x++)
	{
		for(y=0; y<MAPHEIGHT; y++)
		{
			cells[x][y]=*wall;
		}
	}
	PROCESSOR *cpu;
	for (y=0;y<30;y++)
	{
		for (x=0;x<mx[0].size();x++)
		{
			int character = mx[y][x];
			px = x+MAPWIDTH/2-(mx[0].size()/2);
			py = y+1;

			switch(character) {
			case '#':
				type = wall;
			break;
			case '.':
				type = floor;
			break;
			case '+':
				type = door2;
			break;
			case 'B':
				type = boxes;
				break;
			case '-':
				type = door;
				break;
			case '!':
				level.create_item(px,py,"Tutorial Advancer",0,0);
				type = floor;
			break;
			case '$':
				level.create_item(px,py,"Tutorial Advancer",0,0);
				type = wall;
				break;
			case 'g':
				level.create_item(px,py,"수류탄",0,0);
				level.create_item(px,py,"수류탄",0,0);
				level.create_item(px,py,"연막 수류탄",0,0);
				type = floor;
				break;
			case 'p':
				level.create_item(px,py,"나노테크 치료 알약",0,0);
				level.create_item(px,py,"나노테크 치료 알약",0,0);
				type = floor;
				break;
			case 'E':
				level.create_item(px,py,"파워 아머",0,0);
				level.create_item(px,py,"자가발전 응축 배터리",0,0);
				level.create_item(px,py,"전기톱",0,0);
				type = floor;
				break;
			case 'U':
				cells[px][py]=*type;
				level.create_monster(px,py,"유틸리티봇",0,0);
				type = floor;
				break;
			case 'C':
				cpu = (PROCESSOR *) level.create_item(px,py,"허술한 탐색 프로세서",0,0);
				if (cpu!=NULL)
					cpu->group_affiliation = GROUP_HERO;
				type = floor;
				level.create_item(px,py,"고무제 로봇 바퀴",0,0);
				break;
			case 'M':
				type = floor;
				cells[px][py]=*type;
				level.create_monster(px,py,"튜토리얼 괴물",0,0);
				break;
			case 'O':
				type = floor;
				cells[px][py]=*type;
				level.create_monster(px,py,"튜토리얼 타겟",0,0);
				break;
			case '@':
				level.player->ChangePosition(px,py);
			break;
			case ']':
				level.create_item(px,py,"군용 갑옷",0,0);
				type = floor;
				break;
			case '/':
				level.create_item(px,py,"침투용 라이플 LR-30",0,0);
				type = floor;
				break;
			case '|':
				level.create_item(px,py,"6.25mm 라이플탄",50,50);
				type = floor;
				break;
			case '>':
				type = lift_down;
				break;
			default:
				type = floor;
			}
			cells[px][py]=*type;
		}
	}					
}	
