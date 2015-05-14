#pragma once
#ifndef POSITION_H
#define POSITION_H

struct POSITION {
	int x;
	int y;
	POSITION()
	{
	};
	POSITION(const int &a_x, const int &a_y)
		: x(a_x), y(a_y)
	{
	};


	bool operator<(const POSITION &r) const
	{
		if (x<r.x)
			return true;
		else if (x==r.x && y<r.y)
			return true;
		return false;
	}

	bool operator== ( const POSITION &r) const
	{ 
		return (x==r.x && y == r.y); 
	};
	bool operator!= ( const POSITION &r) const
	{ 
		return !operator==(r); 
	};

	const POSITION & operator= ( const POSITION &r) 
	{ 
		x=r.x;
		y=r.y;
		return *this;
	};
};


#endif

