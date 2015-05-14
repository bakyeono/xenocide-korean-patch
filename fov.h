#ifndef FOV_H
#define FOV_H

class MAP;
struct POSITION;

class MUTUAL_FOV {
private:
	MAP *m_map;
	inline void ConvertCornersToVisibleCells();
	inline bool AnyCellBlocks(const POSITION &start,const POSITION &end);
	inline void CheckWayToCorner(const POSITION &start,const POSITION &end,const int &distance);
	inline bool LineBlocksHorizontally(const int &dx,const int &dy,const POSITION &start,const POSITION &end,const int &cx,const int &cy);
	inline bool LineBlocksVertically(const int &dx,const int &dy,const POSITION &start,const POSITION &end,const int &cx,const int &cy);
	inline void PropagateShadow(const POSITION &start, const POSITION &diff, const int &dist);
	inline void CheckLOS(const POSITION &start, const POSITION &end, const int &distance);
	inline void CorrectNearest(const POSITION &start);
	inline bool CornerBlocks(const int &corner_x,const int &corner_y,const int &sgn_x,const int &sgn_y);

public:
	void Start(MAP* map, unsigned int x, unsigned int y, int maxRadius);
};

#endif
