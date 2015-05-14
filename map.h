#ifndef MAP_H
#define MAP_H


#define MAPWIDTH  80
#define MAPHEIGHT 40

#pragma warning (disable: 4786)
#pragma warning (disable: 4788)
// disable warning (compare bool with int)
#pragma warning (disable: 4805)
#pragma warning (disable: 4503)

#include <string>
#include <list>
#include "loadsave.h"
using namespace std;

#define CORNER_VISIBLE 1
#define CORNER_INVISIBLE 0
#define CORNER_INVISIBLE_CHECKED -1

// schody

class STAIRS : public TOSAVE
{
public:
	bool lead_up; //
	int number; // ID of stairs
	int x,y; // position
	string to_where; // to where - ID of level
	string name; // name f.e. "Enterance to ..."

	STAIRS() { ClassName="STAIRS"; x=-1;y=-1; number=0; };
	~STAIRS();
	
   /// INHERITED FROM ToSave
	virtual unsigned long SaveObject();
	virtual bool LoadObject();
};

// Zamkniete, otwarte lub opening niedozwolone
#define OPEN_FALSE 0
#define OPEN_TRUE  1
#define OPEN_NOT_ALLOWED 2

class CELL : public TOSAVE
{
   friend class MAP;
private:
   bool seen;
   bool seen_by_player;
   char last_tile;
   char seen_at_least_once;
   bool seen_by_camera;
public: // 
   CELL();
   
   string name;
   char tile;
   int color;
   bool block_los;
   bool real_block_los;
   bool block_move;
   bool real_block_move;
   bool is_stairs;
   bool is_monster;
   int number_of_items;
   int danger_value;
   int shield; // value tarczy na tym polu
   int number_of_gases;
   int hit_points;
   int max_hit_points;
   int about_open; // OPEN_NOT_ALLOWED, OPEN_TRUE, OPEN_FALSE
   
   string destroyed_to;
//   string destroyed_to_item;
   bool operator == (CELL a);
   CELL & operator = (CELL a);
   bool operator != (CELL a);

   /// INHERITED FROM ToSave
	virtual unsigned long SaveObject();
	virtual bool LoadObject();
};

class MAP : public TOSAVE
{
private:
   CELL cells[MAPWIDTH][MAPHEIGHT];
   int corners[MAPWIDTH+1][MAPHEIGHT+1];
   int add_room(int x1,int y1, int x2, int y2, CELL *wall, CELL *door, float prawd_braku_sciany);
   bool AddDoorsInShuttle();
   
public:
   MAP(void);
   ptr_list stairs_of_map;

   STAIRS *StairsAt(int x,int y);

   void UnSeenMap(); // zeruje dla liczenia FOV
   void HideFromPlayerSight();
	   
   void display();
   void setSeen(int x, int y, bool state);
   void setSeenByPlayer(int x, int y);
   void setSeenByCamera(int x, int y);
   bool onMap(int x, int y);
   bool seen(int x, int y);
   bool seen_by_player(int x, int y);
   bool seen_by_camera(int x, int y);
   bool seen_once(int x, int y);
   bool blockLOS(int x, int y);
   bool blockMove(int x, int y);
   bool isShield(int x, int y);
   bool addDanger(int x, int y, int danger);
   int getDanger(int x, int y);

   inline int getCornerState(const int &x, const int &y);
   inline void setCornerState(const int &x, const int &y, const int &state);
   inline bool cornerOnMap(const int &x, const int &y);
   
   void setBlockLOS(int x, int y);
   void backBlockLOS(int x, int y);
   void setBlockMove(int x, int y);
   void backBlockMove(int x, int y);

   void MarkMonsterOnMap(int x, int y);
   void UnMarkMonsterOnMap(int x, int y);
   bool IsMonsterOnMap(int x, int y);

   bool AreStairs(int x, int y);
   void MarkStairs(int x, int y);
   void UnMarkStairs(int x, int y);
   
   void IncraseNumberOfItems(int x, int y);
   void DecraseNumberOfItems(int x, int y);
   int GetNumberOfItems(int x, int y);   
   void IncraseNumberOfGases(int x, int y);
   void DecraseNumberOfGases(int x, int y);   
   int GetNumberOfGases(int x, int y);
   
   void addShield(int x, int y,int value);
   void removeShield(int x, int y);
   int getShield(int x, int y);

   void setLastTile(int x, int y, int tile);
   string getCellName(int x, int y);
   int getTile(int x, int y);
   void DecraseShieldsOnMap();

   bool open(int x, int y);
   bool close(int x, int y);
   bool isOpenable(int x, int y);
   bool isClosable(int x, int y);

   bool damage(int x, int y, int value);   
   bool destroyCell(int x, int y);
   int  getPercentDamage(int x, int y);
   bool FindRectangleOfType(CELL *typ, int &px, int &py, int size_x, int size_y, int grid);
   void CollapseCeiling(int x,int y);   

   void CreateMapBunker(); // 1
   void CreateMapCaves();  // 2
   void CreateMapMines();  // 3
   void CreateMapBuilding(); // 4
   void CreateMapShuttle(); // 5
   void CreateMapCapsule(); // 6
   void CreateMapCityNW();  // 7
   void CreateMapCitySW();  // 8
   void CreateMapDoctorsCave(); // 9
   void CreateMapGeneticLab(); // 10
   void CreateMapUnderground(bool interpolation); // 11
   void CreateMapTutorial();  // 12

   bool CreateMapGeneticLabUnderground();
   
   /// INHERITED FROM ToSave
	virtual unsigned long SaveObject();
	virtual bool LoadObject();
};


#endif

