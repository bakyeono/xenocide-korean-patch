#include "mem_check.h"

/*
Name: Mutual FOV
Author: Jakub Debski 
Date: 20.03.2007
Description: Field of view that guaranties mutual visibility.
If cell[x][y] is visible from cell[a][b],
then cell[a][b] is visible from cell[x][y].


In case of linker error in Visual Studio 2005 (caused by pdcurses)
add ignoring libc.lib in linker configuration.

*/

#include "fov.h"
#include "map.h"
#include "position.h"

inline int sgn(int a)
{
	if (a<0)
		return -1;
	else if (a>0)
		return 1;
	else
		return 0;
}

void MUTUAL_FOV::ConvertCornersToVisibleCells()
{
	int x,y;
	for (y=0;y<MAPHEIGHT+1;y++)
		for (x=0;x<MAPWIDTH+1;x++)
		{
			if (m_map->getCornerState(x,y)==CORNER_VISIBLE)
			{
				// 4 cells around this corner are visible
				m_map->setSeen(x,y,true);
				m_map->setSeen(x,y-1,true);
				m_map->setSeen(x-1,y,true);
				m_map->setSeen(x-1,y-1,true);
			}
		}
}


bool MUTUAL_FOV::CornerBlocks(const int &corner_x,const int &corner_y,const int &sgn_x,const int &sgn_y)
{
	if (m_map->blockLOS(corner_x,corner_y) && m_map->blockLOS(corner_x,corner_y-1))
		return true;
	if (m_map->blockLOS(corner_x,corner_y) && m_map->blockLOS(corner_x-1,corner_y))
		return true;
	if (m_map->blockLOS(corner_x-1,corner_y-1) && m_map->blockLOS(corner_x+1,corner_y-1))
		return true;
	if (m_map->blockLOS(corner_x-1,corner_y-1) && m_map->blockLOS(corner_x-1,corner_y))
		return true;

	if (sgn_x*sgn_y==1)
	{
		if (m_map->blockLOS(corner_x,corner_y) || m_map->blockLOS(corner_x-1,corner_y-1))
			return true;
	}
	else if (sgn_x*sgn_y==-1)
	{
		if (m_map->blockLOS(corner_x,corner_y-1) || m_map->blockLOS(corner_x-1,corner_y))
			return true;
	}
	else
	{
		if (m_map->blockLOS(corner_x-1,corner_y-1) || m_map->blockLOS(corner_x-1,corner_y) ||
			m_map->blockLOS(corner_x,corner_y-1) || m_map->blockLOS(corner_x,corner_y)
			)
			return true;
	}
	return false;
}

bool MUTUAL_FOV::AnyCellBlocks(const POSITION &start,const POSITION &end)
{
	int sgn_x=sgn(end.x-start.x);
	int sgn_y=sgn(end.y-start.y);

	// nearest cells can block
	if (sgn_x==1 && sgn_y==1 && m_map->blockLOS(start.x,start.y))
		return true;
	if (sgn_x==-1 && sgn_y==1 && m_map->blockLOS(start.x-1,start.y))
		return true;
	if (sgn_x==-1 && sgn_y==-1 && m_map->blockLOS(start.x-1,start.y-1))
		return true;
	if (sgn_x==1 && sgn_y==-1 && m_map->blockLOS(start.x,start.y-1))
		return true;

	// nearest in the direction do not block, check the rest

	int dx=abs(end.x-start.x);
	int dy=abs(end.y-start.y);

	int nx,ny;
	int rx,ry;

	int cell_x,cell_y;

	int x,y;

	// calculate vertical intersections for this x
	if (dx!=0) 
	{
		for (x=1;x<dx;x++)
		{
			ny=x*dy/dx;
			ry=x*dy%dx;

			cell_y = start.y + ny*sgn_y;
			cell_x = start.x + x*sgn_x;
			if (ry!=0)
			{
				if (sgn_y==-1)
					cell_y--;
				if (m_map->blockLOS(cell_x-1,cell_y) || m_map->blockLOS(cell_x,cell_y))
					return true;
			}
			else // special case - corner
			{
				if (CornerBlocks(cell_x,cell_y,sgn_x,sgn_y))
					return true;
			}
		}
	}
	// calculate horizontal intersections for this y
	if (dy!=0) 
	{
		for (y=1;y<dy;y++)
		{
			nx=y*dx/dy;
			rx=y*dx%dy;

			cell_x = start.x + nx*sgn_x;
			cell_y = start.y + y*sgn_y;
			if (rx!=0)
			{
				if (sgn_x==-1)
					cell_x--;
				if (m_map->blockLOS(cell_x,cell_y-1) || m_map->blockLOS(cell_x,cell_y))
					return true;
			}
			else // special case - corner
			{
				if (CornerBlocks(cell_x,cell_y,sgn_x,sgn_y))
					return true;
			}
		}
	}	
	return false;
}

void MUTUAL_FOV::PropagateShadow(const POSITION &start, const POSITION &diff, const int &dist)
{
	POSITION new_pos = start;
	new_pos.x += diff.x;
	new_pos.y += diff.y;
	while(abs(start.x-new_pos.x)<dist && abs(start.y-new_pos.y)<dist)
	{
		if (new_pos.x>=0 && new_pos.x<=MAPWIDTH && new_pos.y>=0 && new_pos.y<=MAPHEIGHT)
			m_map->setCornerState(new_pos.x,new_pos.y,CORNER_INVISIBLE_CHECKED);
		else
			return;
		new_pos.x += diff.x;
		new_pos.y += diff.y;
	}
}

void MUTUAL_FOV::CheckLOS(const POSITION &start, const POSITION &end, const int &distance)
{
	POSITION start2;

	// check if end corner is visible from any corner of start
	for (int a=0;a<2;a++)
		for (int b=0;b<2;b++)
		{
			start2=start;
			start2.x+=a;
			start2.y+=b;

			if (!AnyCellBlocks(start2,end))
			{
				// corner is visible
				m_map->setCornerState(end.x,end.y,CORNER_VISIBLE);
				return;
			}
		}

		// this corner not visible
		// we can propagate shadow for speed-up
		POSITION diff;
		diff.x = end.x-start.x;
		diff.y = end.y-start.y;
		PropagateShadow(start,diff,distance);
}

void MUTUAL_FOV::CheckWayToCorner(const POSITION &start,const POSITION &end,const int &distance)
{
	if (end.x>=0 && end.x<=MAPWIDTH && end.y>=0 && end.y<=MAPHEIGHT)
		if (m_map->getCornerState(end.x,end.y)!=CORNER_INVISIBLE_CHECKED)
			CheckLOS(start,end,distance);
}

void MUTUAL_FOV::Start(MAP* map, unsigned int x, unsigned int y, int maxRadius)
{	
	m_map = map;

	// Map is not seen
	m_map->UnSeenMap();	

	POSITION start,end;
	start.x=x;
	start.y=y;

	// Check LOS in square spiral
	for (int l=1;l<maxRadius;l++)
	{
		// top
		end.y=start.y-l;
		if (end.y>=0)
		{
			for (end.x=start.x-l;end.x<start.x+l+1;++end.x)
			{
				CheckWayToCorner(start,end,maxRadius);
			}
		}
		// right
		end.x=start.x+l+1;
		if (end.x<MAPWIDTH+1)
		{
			for (end.y=start.y-l;end.y<start.y+l+1;++end.y)
			{
				CheckWayToCorner(start,end,maxRadius);
			}
		}
		// bottom
		end.y=start.y+l+1;
		if (end.y<MAPHEIGHT+1)
		{
			for (end.x=start.x+l+1;end.x>start.x-l;--end.x)
			{
				CheckWayToCorner(start,end,maxRadius);
			}
		}
		// left
		end.x=start.x-l;
		if (end.x>=0)
		{
			for (end.y=start.y+l+1;end.y>start.y-l;--end.y)
			{
				CheckWayToCorner(start,end,maxRadius);
			}
		}
	}

	ConvertCornersToVisibleCells();
	CorrectNearest(start);
};

void MUTUAL_FOV::CorrectNearest(const POSITION &start)
{
	int single_check[][4] = {
		-1,-1, -2,-2,
		1,1,	2,2,
		-1,1,  -2,2,
		1,-1,	2,-2,
		-1,0,  -2,0,
		1,0,    2,0,
		0,-1,   0,-2,
		0,1,	0,2
	};
	for (int index=0;index<8;index++)
		if ( m_map->blockLOS(start.x+single_check[index][0],start.y+single_check[index][1]))
			m_map->setSeen(start.x+single_check[index][2],start.y+single_check[index][3],false);

	int double_check[][6] = {
		-1,-1, 0,-1, -1,-2,
		1,-1,  0,-1,  1,-2,
		-1,1,  0,1,  -1,2,
		1,1,   0,1,   1,2,
		//
		-1,-1,-1,0,  -2,-1,
		-1,1, -1,0,  -2,1,
		1,-1,  1,0,   2,-1,
		1,1,   1,0,   2,1
	};

	for (int index=0;index<8;index++)
	{
		if ( m_map->blockLOS(start.x+double_check[index][0],start.y+double_check[index][1]) &&
			m_map->blockLOS(start.x+double_check[index][2],start.y+double_check[index][3]))
		{
			m_map->setSeen(start.x+double_check[index][4],start.y+double_check[index][5],false);
		}
	}

	m_map->setSeen(start.x,start.y,true);
}
