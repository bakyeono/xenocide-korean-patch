#include "mem_check.h"

#include "tiles.h"
#include "system.h"
#include <stdlib.h>
#include <string>


TILE :: TILE()
{
	ClassName="TILE";
	ChangePosition(-1,-1);
	color = 0;
	invisible = false;
	positionX=-1;
	positionY=-1;
}

TILE :: ~TILE()
{
}

bool TILE::ChangePosition(int x, int y)
{
	positionX = x;
	positionY = y;
	return true;
}

void TILE :: display()
{
  draw_at(pX(),pY());
}

void TILE :: draw_at(int x, int y)
{
	set_color(color);
	print_character(x,y,tile);
}


void TILE :: rename(string text)
{
   name=text;
}

