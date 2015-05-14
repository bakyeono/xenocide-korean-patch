#ifndef GAME_H
#define GAME_H

#pragma warning (disable: 4786)
#pragma warning (disable: 4788)
// disable warning (compare bool with int)
#pragma warning (disable: 4805)
#pragma warning (disable: 4503)

#include <string>
using namespace std;

class GAME {
private:
	  bool CheckToLoad(); // check if player file exists

public:
	  string name_of_player;
      GAME();
	  void Begin();   
      void NewGame(); 
      void Main();    
	  void Quit();	  
	  bool SaveGame(); 
	  bool LoadGame(); 
	  void DeleteSaveGame(); 
};

#endif
