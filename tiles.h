#ifndef TILES_H
#define TILES_H

#include "loadsave.h"

#pragma warning (disable: 4786)
#pragma warning (disable: 4788)
// disable warning (compare bool with int)
#pragma warning (disable: 4805)

#include <string>
using namespace std;

class TILE : public TOSAVE {
private:
	int positionX;
	int positionY;
public:

	inline int pX() { return positionX; };
	inline int pY() { return positionY; };

  string name;
  char color;
  char tile;
  bool invisible;

  TILE();
  virtual ~TILE();
  virtual void display();
  virtual void draw_at(int x, int y);
  void rename(string);

  virtual bool ChangePosition(int x, int y);  

   /// INHERITED FROM ToSave
	virtual unsigned long SaveObject();
	virtual bool LoadObject();
};

#endif


