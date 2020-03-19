#ifndef _Future_geometry
#define _Future_geometry
#include "quat.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ������ �� 2-�������� �����
template<class T>
struct point2{
	T x, y;

	point2(){}
	point2(T _x, T _y): x(_x),y(_y){}    
	//��������� �����������
	//point2(const point2<T>& p): x(p.x), y(p.y){}

	//���������� (+,-,+=,-=)
	point2<T> operator+=(const point2<T>& p)
	{
		x += p.x; y += p.y;
		return *this;
	}
	point2<T> operator-=(const point2<T>& p)
	{
		x -= p.x; y-= p.y;
		return *this;
	}

	friend point2<T> operator+(const point2<T>& p1, const point2<T>& p2) 
	{ 
		return point2<T>(p1.x + p2.x, p1.y + p2.y);
	}
	friend point2<T> operator-(const point2<T>& p1, const point2<T>& p2)
	{ 
		return point2<T>(p1.x - p2.x, p1.y - p2.y);
	}

	friend bool operator == (const point2<T>& p1, const point2<T>& p2)
	{
		return p1.x == p2.x && p1.y == p2.y;
	}

	friend bool operator != (const point2<T>& p1, const point2<T>& p2)
	{
		return p1.x != p2.x || p1.y != p2.y;
	}

	friend T dist(const point2<T>& p1, const point2<T>& p2)
	{
		point2<T> tmp = p1 - p2;
		return sqrtf(tmp.x*tmp.x + tmp.y*tmp.y);
	}
	friend bool operator<(const point2<T> &a,const point2<T> &b);
};
template<class T> bool operator<(const point2<T> &a,const point2<T> &b)
{
	if(a.x<b.x) return true;
	else if(a.x==b.x&&a.y<b.y) return true;
	return false;
}

typedef point2<int> ipnt2_t;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

typedef float matrix3[3][3];
struct Transformation
{
	matrix3 rot;//������� ��������
	point3 trans; //������ �����������
	friend point3 Transform(const Transformation &t, const point3 &a);
};
inline point3 TransformVector(const Transformation &t, const point3 &a)
{
	return point3(a.x*t.rot[0][0]+a.y*t.rot[1][0]+a.z*t.rot[2][0],
		a.x*t.rot[0][1]+a.y*t.rot[1][1]+a.z*t.rot[2][1],
		a.x*t.rot[0][2]+a.y*t.rot[1][2]+a.z*t.rot[2][2]    );
}
inline point3 TransformPoint(const Transformation &t, const point3 &a)
{
	return point3(t.trans.x+a.x*t.rot[0][0]+a.y*t.rot[1][0]+a.z*t.rot[2][0],
		t.trans.y+a.x*t.rot[0][1]+a.y*t.rot[1][1]+a.z*t.rot[2][1],
		t.trans.z+a.x*t.rot[0][2]+a.y*t.rot[1][2]+a.z*t.rot[2][2]    );
}
struct texcoord//���������� ����������
{ 
	float u,v;
	friend inline bool operator <(const texcoord& a,const texcoord& t)
	{
		if(a.u<t.u)return true;
		else if(a.u==t.u && a.v<t.v) return true;
		return false;
	}
	texcoord &operator=(const texcoord &a)
	{
		if(this == &a) return *this;
		u=a.u; v=a.v;
		return *this;
	}
	texcoord &operator=(const mll::algebra::point2 &a)
	{
		texcoord tmp(a);
		std::swap(tmp.u,u);
		std::swap(tmp.v,v);
		return *this;
	}
	texcoord()
	{
		//u=0; v=0;
	}
	texcoord(float _u,float _v)
	{
		u=_u; v=_v;
	}
	texcoord(const mll::algebra::point2 &pt)
	:	u(pt.x), v(pt.y)
	{

	}

};

struct LevelVertex
{
	point3 Pos;
	texcoord uv;
	texcoord lightmapuv;
	point3 Normal;
	unsigned long Color;
};

struct Primi
{
	struct FVFVertex
	{
		point3 pos;
		point3 norm;
		unsigned int diffuse;
		texcoord uv;
		texcoord lm;
	};
	
	enum PrimType{TRIANGLE, LINES, TRIANGLEFAN, TRIANGLESTRIP, LINESSTRIP, POINTS};
	enum PrimContents{NORMAL=1,DIFFUSE=2,NEEDTRANSFORM=4};

	PrimType Prim;  //��� ���������
	unsigned int Contents; //����������

	std::vector<unsigned short> Idxs;   //���� �������
	std::vector<FVFVertex> Verts;

	Primi() {}
	Primi(unsigned int vcount) : Verts(vcount) {}
	Primi(unsigned int vcount, unsigned int icount) : Verts(vcount), Idxs(icount) {}
	
	unsigned int GetVertexCount() const { return Verts.size(); }
	unsigned int GetIndexCount() const { return Idxs.size(); }
};

namespace std
{
//=====================================================================================//
//                                  void std::copy()                                   //
//=====================================================================================//
inline Primi::FVFVertex *copy(const Primi::FVFVertex *begin,
								   const Primi::FVFVertex *end,
								   Primi::FVFVertex *out)
{
	const unsigned int count = end-begin;
	memcpy(out,begin,count*sizeof(Primi::FVFVertex));
	return out+count;
}

//=====================================================================================//
//                       inline Primi::FVFVertex *std::copy_n()                        //
//=====================================================================================//
inline Primi::FVFVertex *copy_n(const Primi::FVFVertex *begin,
									 unsigned int count,
									 Primi::FVFVertex *out)
{
	memcpy(out,begin,count*sizeof(Primi::FVFVertex));
	return out+count;
}

}

struct ray
{
	point3 Origin;
	point3 Direction;
	int Octant; //������, � ������� ��������� ���
	void  Update()
	{
		Octant=0;
		if(Direction.x<0) Octant|=0x01;
		if(Direction.y<0) Octant|=0x02;
		if(Direction.z<0) Octant|=0x04;
	}
	ray(){Octant=-1;}
	ray(const point3 &pos, const point3 &dir)
	{
		Origin=pos;
		Direction=dir;
		Update();
	}
};
struct BBox
{
	float minx, miny, minz;
	float maxx, maxy, maxz;
	point3 GetCenter() const {return 0.5*point3(minx+maxx,miny+maxy,minz+maxz);};
	void Enlarge(const BBox &b)
	{
		if(b.minx<minx) minx=b.minx;
		if(b.maxx>maxx) maxx=b.maxx;

		if(b.miny<miny) miny=b.miny;
		if(b.maxy>maxy) maxy=b.maxy;

		if(b.minz<minz) minz=b.minz;
		if(b.maxz>maxz) maxz=b.maxz;
	}
	void Enlarge(const point3 &p)
	{
		if(p.x<minx) minx=p.x;
		if(p.x>maxx) maxx=p.x;
		if(p.y<miny) miny=p.y;
		if(p.y>maxy) maxy=p.y;
		if(p.z<minz) minz=p.z;
		if(p.z>maxz) maxz=p.z;
	}
	float GetBigRadius() const
	{
		point3 t(maxx-minx,maxy-miny,maxz-minz);
		return t.Length()/2.f;
	}
	int GetBigAxis() const
	{
		float dx=maxx-minx,dy=maxy-miny,dz=maxz-minz;
		if(dx>dy)  //x>y
		{
			if(dx>dz) return 0;//&X>z   
			else return 2;//
		}
		else
		{
			if(dy>dz) return 1;//X
			else  return 2;
		}
	}
	void Degenerate(){minz=miny=minx=100000;maxz=maxy=maxx=-100000;};
	void Box(const point3 &c,float a)
	{
		minx=c.x-a;miny=c.y-a;minz=c.z-a;
		maxx=c.x+a;maxy=c.y+a;maxz=c.z+a;
	}
	bool IntersectRay(const ray &r,point3 *p)
	{
		//������� �������� ��������� � �����
		/*point3 c(GetCenter());
		point3 cdir=(c-r.Origin);
		float dist=(cdir-cdir.Dot(r.Direction)*r.Direction).Length();
		if(dist>GetBigRadius())
		return false;*/
		//������ ��������� � ��������������
		switch(r.Octant)
		{
			//case -1:throw CASUS("������� �������������������� ���!");break;
		case 0:  // x>0 y>0 z>0
			{
				if (r.Origin.x>maxx||r.Origin.y>maxy||r.Origin.z>maxz) return false; //��� ������ � ������ �������
				*p=r.Origin+((minx-r.Origin.x)/r.Direction.x)*r.Direction;
				if(p->y>miny&&p->y<maxy&&p->z>minz&&p->z<maxz)   return true; //������ � ���������
				*p=r.Origin+((miny-r.Origin.y)/r.Direction.y)*r.Direction;
				if(p->x>minx&&p->x<maxx&&p->z>minz&&p->z<maxz)   return true; //������ � ���������
				*p=r.Origin+((minz-r.Origin.z)/r.Direction.z)*r.Direction;
				if(p->x>minx&&p->x<maxx&&p->y>miny&&p->y<maxy)   return true; //������ � ���������
			}
			break;
		case 1:  // x<0 y>0 z>0
			if (r.Origin.x<minx||r.Origin.y>maxy||r.Origin.z>maxz) return false; //��� ������ � ������ �������
			*p=r.Origin+((maxx-r.Origin.x)/r.Direction.x)*r.Direction;
			if(p->y>miny&&p->y<maxy&&p->z>minz&&p->z<maxz)   return true; //������ � ���������
			*p=r.Origin+((miny-r.Origin.y)/r.Direction.y)*r.Direction;
			if(p->x>minx&&p->x<maxx&&p->z>minz&&p->z<maxz)   return true; //������ � ���������
			*p=r.Origin+((minz-r.Origin.z)/r.Direction.z)*r.Direction;
			if(p->x>minx&&p->x<maxx&&p->y>miny&&p->y<maxy)   return true; //������ � ���������
			break;
		case 2:  // x>0 y<0 z>0
			if (r.Origin.x>maxx||r.Origin.y<miny||r.Origin.z>maxz) return false; //��� ������ � ������ �������
			*p=r.Origin+((minx-r.Origin.x)/r.Direction.x)*r.Direction;
			if(p->y>miny&&p->y<maxy&&p->z>minz&&p->z<maxz)   return true; //������ � ���������
			*p=r.Origin+((maxy-r.Origin.y)/r.Direction.y)*r.Direction;
			if(p->x>minx&&p->x<maxx&&p->z>minz&&p->z<maxz)   return true; //������ � ���������
			*p=r.Origin+((minz-r.Origin.z)/r.Direction.z)*r.Direction;
			if(p->x>minx&&p->x<maxx&&p->y>miny&&p->y<maxy)   return true; //������ � ���������
			break;
		case 3:  // x<0 y<0 z>0
			if (r.Origin.x<minx||r.Origin.y<miny||r.Origin.z>maxz) return false; //��� ������ � ������ �������
			*p=r.Origin+((maxx-r.Origin.x)/r.Direction.x)*r.Direction;
			if(p->y>miny&&p->y<maxy&&p->z>minz&&p->z<maxz)   return true; //������ � ���������
			*p=r.Origin+((maxy-r.Origin.y)/r.Direction.y)*r.Direction;
			if(p->x>minx&&p->x<maxx&&p->z>minz&&p->z<maxz)   return true; //������ � ���������
			*p=r.Origin+((minz-r.Origin.z)/r.Direction.z)*r.Direction;
			if(p->x>minx&&p->x<maxx&&p->y>miny&&p->y<maxy)   return true; //������ � ���������
			break;
		case 4:  // x>0 y>0 z<0
			if (r.Origin.x>maxx||r.Origin.y>maxy||r.Origin.z<minz) return false; //��� ������ � ������ �������
			*p=r.Origin+((minx-r.Origin.x)/r.Direction.x)*r.Direction;
			if(p->y>miny&&p->y<maxy&&p->z>minz&&p->z<maxz)   return true; //������ � ���������
			*p=r.Origin+((miny-r.Origin.y)/r.Direction.y)*r.Direction;
			if(p->x>minx&&p->x<maxx&&p->z>minz&&p->z<maxz)   return true; //������ � ���������
			*p=r.Origin+((maxz-r.Origin.z)/r.Direction.z)*r.Direction;
			if(p->x>minx&&p->x<maxx&&p->y>miny&&p->y<maxy)   return true; //������ � ���������
			break;
		case 5:  // x<0 y>0 z<0
			if (r.Origin.x<minx||r.Origin.y>maxy||r.Origin.z<minz) return false; //��� ������ � ������ �������
			*p=r.Origin+((maxx-r.Origin.x)/r.Direction.x)*r.Direction;
			if(p->y>miny&&p->y<maxy&&p->z>minz&&p->z<maxz)   return true; //������ � ���������
			*p=r.Origin+((miny-r.Origin.y)/r.Direction.y)*r.Direction;
			if(p->x>minx&&p->x<maxx&&p->z>minz&&p->z<maxz)   return true; //������ � ���������
			*p=r.Origin+((maxz-r.Origin.z)/r.Direction.z)*r.Direction;
			if(p->x>minx&&p->x<maxx&&p->y>miny&&p->y<maxy)   return true; //������ � ���������
			break;
		case 6:  // x>0 y<0 z<0
			if (r.Origin.x>maxx||r.Origin.y<miny||r.Origin.z<minz) return false; //��� ������ � ������ �������
			*p=r.Origin+((minx-r.Origin.x)/r.Direction.x)*r.Direction;
			if(p->y>miny&&p->y<maxy&&p->z>minz&&p->z<maxz)   return true; //������ � ���������
			*p=r.Origin+((maxy-r.Origin.y)/r.Direction.y)*r.Direction;
			if(p->x>minx&&p->x<maxx&&p->z>minz&&p->z<maxz)   return true; //������ � ���������
			*p=r.Origin+((maxz-r.Origin.z)/r.Direction.z)*r.Direction;
			if(p->x>minx&&p->x<maxx&&p->y>miny&&p->y<maxy)   return true; //������ � ���������
			break;
		case 7:  // x<0 y<0 z<0
			if (r.Origin.x<minx||r.Origin.y<miny||r.Origin.z<minz) return false; //��� ������ � ������ �������
			*p=r.Origin+((maxx-r.Origin.x)/r.Direction.x)*r.Direction;
			if(p->y>miny&&p->y<maxy&&p->z>minz&&p->z<maxz)   return true; //������ � ���������
			*p=r.Origin+((maxy-r.Origin.y)/r.Direction.y)*r.Direction;
			if(p->x>minx&&p->x<maxx&&p->z>minz&&p->z<maxz)   return true; //������ � ���������
			*p=r.Origin+((maxz-r.Origin.z)/r.Direction.z)*r.Direction;
			if(p->x>minx&&p->x<maxx&&p->y>miny&&p->y<maxy)   return true; //������ � ���������
			break;

		}
		return false;
	}
	float DistToPoint(const point3 &pnt) const
	{//���������� �� ����� �� ��������� ����� � BBox (���� ����� ������, �� 0)
		float dx,dy,dz;
		if(pnt.x<minx) dx=minx-pnt.x;
		else if(pnt.x>maxx) dx=pnt.x-maxx;
		else dx=0;
		if(pnt.y<miny) dy=miny-pnt.y;
		else if(pnt.y>maxy) dy=pnt.y-maxy;
		else dy=0;
		if(pnt.z<minz) dz=minz-pnt.z;
		else if(pnt.z>maxz) dz=pnt.z-maxz;
		else dz=0;
		return sqrt(dx*dx+dy*dy+dz*dz);

	}
	point3 operator[](int i) const
	{
		switch(i)
		{
		default:
		case 0:		return point3(minx,miny,minz);
		case 1:		return point3(minx,miny,maxz);
		case 2:		return point3(minx,maxy,minz);
		case 3:		return point3(minx,maxy,maxz);
		case 4:		return point3(maxx,miny,minz);
		case 5:		return point3(maxx,miny,maxz);
		case 6:		return point3(maxx,maxy,minz);
		case 7:		return point3(maxx,maxy,maxz);
		}
	}
};

#endif