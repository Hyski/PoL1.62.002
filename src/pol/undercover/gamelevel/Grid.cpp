// Grid.cpp: implementation of the Grid class.
//
//////////////////////////////////////////////////////////////////////
#include "precomp.h"

#include <common/graphpipe/culling.h>
#include <logic2/hexutils.h>

#include "Grid.h"

struct HexStep {int x,y;};
const  HexStep Steps[2][6]=
{
    { { 1, 0},{ 0, 1},{-1, 1},{-1, 0},{-1,-1},{ 0,-1}},
    { { 1, 0},{ 1, 1},{ 0, 1},{-1, 0},{ 0,-1},{ 1,-1}}
};

Grid::Grid(const std::string& name):
    HexGrid(name.c_str())
{
}

Grid::~Grid()
{
}

//=====================================================================================//
//                            static float dist_to_weight()                            //
//=====================================================================================//
__forceinline static float dist_to_weight(float d)
{
	if(d < 0.6f)
	{
		const float dist = 1.01f-(d/0.6f);
		return std::min(1.0f,dist*dist);
	}
	else
	{
		return 0.0001f;
	}
}

//=====================================================================================//
//                               static float dist_2d()                                //
//=====================================================================================//
__forceinline static float dist_2d(const point3 &pt1, const point3 &pt2)
{
	const float dx = pt1.x-pt2.x, dy = pt1.y - pt2.y;
	return sqrtf(dx*dx+dy*dy);
}

//=====================================================================================//
//                          float Grid::SimpleHeight() const                           //
//=====================================================================================//
float Grid::SimpleHeight(const point3 &Pos) const
{
	ipnt2_t spnt = HexUtils::scr2hex(Pos);
	if(HexGrid::GetInst()->IsOutOfRange(spnt)) return 0;
	return HexGrid::GetInst()->Get(spnt).z;
}

//=====================================================================================//
//                                    enum HexZone                                     //
//=====================================================================================//
/// Не трогать порядок значений!!!
enum HexZone
{
	zoneTop,
	zoneBottom,
	zoneLeftTop,
	zoneLeftBottom,
	zoneRightTop,
	zoneRightBottom
};

const float ctg60 = 0.57735027f;

//=====================================================================================//
//                   __forceinline static int getTriangleFromPoint()                   //
//=====================================================================================//
__forceinline static HexZone getTriangleFromPoint(const point3 &pos)
{
	if(pos.y == 0.0f)
	{
		if(pos.x == 0.0f)
		{
			return zoneTop;
		}
		else
		{
			return pos.x < 0.0f ? zoneLeftTop : zoneRightTop;
		}
	}

	if(pos.x == 0.0f)
	{
		return pos.y > 0.0f ? zoneTop : zoneBottom;
	}

	if(pos.y > 0.0f)
	{
		if(pos.x > 0.0f)
		{
			return pos.x/pos.y <= ctg60 ? zoneTop : zoneRightTop;
		}
		else
		{
			return pos.x/pos.y > -ctg60 ? zoneTop : zoneLeftTop;
		}
	}
	else
	{
		if(pos.x > 0.0f)
		{
			return pos.x/pos.y > -ctg60 ? zoneBottom : zoneRightBottom;
		}
		else
		{
			return pos.x/pos.y <= ctg60 ? zoneBottom : zoneLeftBottom;
		}
	}
}

static const int neighbours[6][2] =
{
	{1,2},	// zoneTop
	{4,5},	// zoneBottom
	{2,3},	// zoneLeftTop
	{3,4},	// zoneLeftBottom
	{0,1},	// zoneRightTop
	{0,5}	// zoneRightBottom
};

static const point3 directions[6] = 
{
	point3(1.0f,0.0f,0.0f),
	point3(COS60,SIN60,0.0f),
	point3(-COS60,SIN60,0.0f),
	point3(-1.0f,0.0f,0.0f),
	point3(-COS60,-SIN60,0.0f),
	point3(COS60,-SIN60,0.0f)
};

//=====================================================================================//
//                         __forceinline static float dot2d()                          //
//=====================================================================================//
__forceinline static float dot2d(const point3 &a, const point3 &b)
{
	return a.x*b.x + a.y*b.y;
}

//=====================================================================================//
//                             float Grid::Height() const                              //
//=====================================================================================//
float Grid::Height(const point3 &pos) const
{
	ipnt2_t spnt[3];
	//float weight[3];

	spnt[2] = HexUtils::scr2hex(pos);
	const point3 locpos = pos - HexUtils::hex2scr(spnt[2]);

	// Определим, в каком треугольнике находится точка
	HexZone triangle = getTriangleFromPoint(locpos);
	const int nbrs[2] = {neighbours[triangle][0],neighbours[triangle][1]};
	HexUtils::GetFrontPnts0(spnt[2],nbrs[0],&spnt[0]);
	HexUtils::GetFrontPnts0(spnt[2],nbrs[1],&spnt[1]);

	const point3 &e1 = directions[nbrs[0]];
	const point3 &e2 = directions[nbrs[1]];
	const float inv_det = 1.0f/(e1.y * e2.x - e1.x * e2.y);

	const bool outofrange0 = IsOutOfRange(spnt[0]);
	const bool outofrange1 = IsOutOfRange(spnt[1]);
	const bool outofrange2 = IsOutOfRange(spnt[2]);
	const float height2 = outofrange2 ? 0.0f : Get(spnt[2]).z;

	const bool passable0 = outofrange2 ? false : (!outofrange0 && Get(spnt[2]).IsPassable(nbrs[0]));
	const bool passable1 = outofrange2 ? false : (!outofrange1 && Get(spnt[2]).IsPassable(nbrs[1]));

	const float weight[3] =
	{
		(locpos.y*e2.x - locpos.x*e2.y)*inv_det,
		(locpos.x*e1.y - locpos.y*e1.x)*inv_det,
		1.0f - weight[0] - weight[1]
	};

	const float epsilon = 1e-5f;

	assert( weight[0] > -epsilon );
	assert( weight[1] > -epsilon );
	assert( weight[2] > -epsilon );
	assert( weight[0]-1.0f < epsilon );
	assert( weight[1]-1.0f < epsilon );
	assert( weight[2]-1.0f < epsilon );

	const float height[3] =
	{
		passable0 ? Get(spnt[0]).z : height2,
		passable1 ? Get(spnt[1]).z : height2,
		height2
	};

	return height[0] * weight[0] + height[1] * weight[1] + height[2] * weight[2];
}

/*
//=====================================================================================//
//                             float Grid::Height() const                              //
//=====================================================================================//
float Grid::Height(const point3 &Pos) const
{
	float d;
	int i;
	float h = 0.0f;
	float summ = 0.0f;
	float w;
	ipnt2_t spnt = HexUtils::scr2hex(Pos);
	if(HexGrid::GetInst()->IsOutOfRange(spnt)) return 0.0f;

	point3 r = HexGrid::GetInst()->Get(spnt);

	d = dist_2d(Pos,r);
	w = dist_to_weight(d);
	h += r.z*w;
	summ += w;

	for(i = 0; i < 6; i++)
	{
		ipnt2_t hpnt;
		HexUtils::GetFrontPnts0(spnt, i, &hpnt);
		if(HexGrid::GetInst()->IsOutOfRange(hpnt)) continue;
		r = HexGrid::GetInst()->Get(hpnt);
		d = dist_2d(Pos,r);
		w = dist_to_weight(d);
		h += r.z*w;
		summ += w;
	}

	return h/summ;
}
*/

/*
float Grid::Height(const point3 &_Pos) const
{
    float HEXSize = GetHexSize();
    float HEXSizeY=HEXSize*cos(3.14159265*30/180);

    int y1,y2,x1,x2,x3,x4,x5;
    point3 Pos(_Pos);
 //   Pos.y=-Pos.y;

    y1=floor(Pos.y/HEXSizeY);
    y2=y1+1;

    x1=(Pos.x-((y1&1)?HEXSize/2:0))/HEXSize;   x2=x1+1;
    x3=x1+Steps[y1&1][4].x;
    x4=x3+1;
    x5=x4+1;
    Triangle a;
    float t,u,v;
    static point3 dir(0,0,-1);
    point3  Origin(Pos+point3(0,0,10000));

    int SizeX = GetSizeX(), 
        SizeY = GetSizeY();

    HexGrid::const_cell_iterator cells = first_cell();
    HexGrid::hg_slice hs = GetSlice();

    if(y1>=0&&y1<SizeY&&y2>=0&&y2<SizeY)
    {
        if(x1>=0&&x1<SizeX&&x2>=0&&x2<SizeX&& x4>=0&&x4<SizeX)
        {
            a.V[0]= *(cells + hs(x1, y1));   //a.V[0].y = -a.V[0].y;
            a.V[1]= *(cells + hs(x2, y1));   //a.V[1].y = -a.V[1].y;
            a.V[2]= *(cells + hs(x4, y2));   //a.V[2].y = -a.V[2].y;

            if(a.RayTriangle(Origin,dir,&t,&u,&v))
            {
                return Origin.z-t;
            }
        }

        if(x1>=0&&x1<SizeX&&x4>=0&&x4<SizeX&& x3>=0&&x3<SizeX)
        {
            a.V[0]= *(cells + hs(x1, y1));   //a.V[0].y = -a.V[0].y;
            a.V[1]= *(cells + hs(x4, y2));   //a.V[1].y = -a.V[1].y;
            a.V[2]= *(cells + hs(x3, y2));   //a.V[2].y = -a.V[2].y;

            if(a.RayTriangle(Origin,dir,&t,&u,&v))
            {
                return Origin.z-t;
            }
        }
        
        if(x5>=0&&x5<SizeX&&x2>=0&&x2<SizeX&& x4>=0&&x4<SizeX)
        {
            a.V[0]= *(cells + hs(x2, y1));   //a.V[0].y = -a.V[0].y;
            a.V[1]= *(cells + hs(x5, y2));   //a.V[1].y = -a.V[1].y;
            a.V[2]= *(cells + hs(x4, y2));   //a.V[2].y = -a.V[2].y;

            if(a.RayTriangle(Origin,dir,&t,&u,&v))
            {
                return Origin.z-t;
            }
        }
    }
    return 0;
}
*/