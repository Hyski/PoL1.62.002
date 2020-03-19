#pragma once

///тип разбиения плоскостью
struct split
{
///тип разбиения плоскостью
	enum split_type
	{
		positive,///<объект находится в положительном полупространстве
		negative,///<объект находится в отрицательном полупространстве
		divided  ///<объект рассекается плоскостью
	};
};


///плоскость
struct plane
{

  static const float epsilon;

  enum {X=0x01,Y=0x02,Z=0x04};///<для оптимизации
  mll::algebra::vector3 normal; 
  float d;       
  unsigned flag; 
  int octant;    ///<в каком октанте находится нормаль
  plane(){};
  void update()
    {
    flag=X|Y|Z;
    if(fabs(normal.x)<epsilon)
      flag&=~X;
    if(fabs(normal.y)<epsilon)
      flag&=~Y;
    if(fabs(normal.z)<epsilon)
      flag&=~Z;

    octant=0;
    if(normal.x<0) octant|=0x01;
    if(normal.y<0) octant|=0x02;
    if(normal.z<0) octant|=0x04;
    }
  plane(const mll::algebra::vector3 &norm)
    {
    normal=norm;d=0;
	normal.normalize();
    update();
    }
  plane(const mll::algebra::point3 &dot,const mll::algebra::vector3 &norm)
    {
		normal=mll::algebra::vector3(norm);
    normal.normalize();
	d=-normal.dot(dot-mll::algebra::origin);
    update();
    }
  plane(const mll::algebra::point3 &a1,const mll::algebra::point3 &a2,const mll::algebra::point3 &a3)
    {
    normal=(a3-a1).cross(a2-a1);  
	normal.normalize();
	d=-normal.dot(a1-mll::algebra::origin);
    update();
    }
  float testpoint(const mll::algebra::point3 &dot) const
    { 
    float ret=d;
    if(flag&X)
      ret+=normal.x*dot.x;
    if(flag&Y)
      ret+=normal.y*dot.y;
    if(flag&Z)
      ret+=normal.z*dot.z;
    return ret;
    }
  float testpointeps(const mll::algebra::point3 &dot) const
    { 
    float ret;
    ret=normal.dot(dot-mll::algebra::origin)+d;
    if(fabs(ret)<epsilon) ret=0.f;
        return ret;
    }

  bool operator ==(const plane &p) const
    {
    if(fabs(d-p.d)>epsilon)return false;
    if((normal-p.normal).sqr_length()>epsilon) return false;
    return true;
    }
  plane operator -() const
    {
     plane p;
     p.normal=-normal; p.d=-d;
     return p;
    }
	bool rayintersect(const mll::geometry::ray3 &r,float *dist)
    {
    float e=r.direction.dot(normal);
    if(e==0) return false;
    *dist=(-d-normal.dot(r.origin-mll::algebra::origin))/e;
    return true;
    }

  };

//	вершина
struct vert_storage_vert 
{
	mll::algebra::point3 pos;
	mll::algebra::vector3 norm;
	unsigned color;
	mll::algebra::point2  uv;
	mll::algebra::point2  lm_uv;

	bool operator ==(const vert_storage_vert &a)const 
	{
		return (pos-a.pos).sqr_length()<0.00001f && (norm-a.norm).sqr_length()<0.00001f 
			&& uv==a.uv
			&& lm_uv==a.lm_uv;
	}
	bool operator <(const vert_storage_vert &a)const 
	{
		if(pos<a.pos) return true;
		else if(a.pos<pos) return false;
		else
		{
			if(norm<a.norm) return true;
			else if(a.norm<norm) return false;
			else
			{
				if(uv<a.uv) return true;
				else if(a.uv<uv) return false;
				else
				{
					if(lm_uv<a.lm_uv) return true;
					else return false;

				}

			}

		}
	}
};
class vertex_key
{
public:
	vertex_key(const vert_storage_vert &v){ val =  static_cast<int>(v.pos.x); };

	bool operator < (const vertex_key &k)const {return val<k.val;};
	bool operator == (const vertex_key &k)const {return val==k.val;};
private:
	int val;
};

///хранилище вершин для индексации
class vert_storage
{
public:

	typedef vert_storage_vert vert;
	typedef std::vector<vert> cont;
	typedef cont::const_iterator const_iterator;
	typedef std::multimap<vertex_key, unsigned> vert_map;
public:
	//возвращает индекс добавленной или уже существующей вершины
	int add(const vert &v)
	{
		std::pair<vert_map::iterator, vert_map::iterator> p=v_hash.equal_range(vertex_key(v));
		if(p.first!=p.second)
		{
			for(vert_map::iterator it=p.first; it!=p.second; it++)
			{
				if(m_verts[it->second] == v)
					return it->second;
			}
		}
		/*for(cont::iterator i=m_verts.begin();i!=m_verts.end();i++)
		{
			if(*i==v) return static_cast<int>(std::distance(m_verts.begin(),i));
		}  */
		v_hash.insert(std::pair<vertex_key, unsigned>(vertex_key(v), m_verts.size()));
		m_verts.push_back(v);
		return static_cast<int>(m_verts.size()-1);
	}
	vert& operator[](int i){return m_verts[i];}
	const vert& operator[](int i)const {return m_verts.at(i);}
	int size()const {return m_verts.size();};
	const_iterator begin()const {return m_verts.begin();}
	const_iterator end()const {return m_verts.end();}

public: //HACK неправильная видимость, сделано для написания operator>>
	cont m_verts;
	vert_map v_hash;
};

mll::io::obinstream& operator<<(mll::io::obinstream &o, const vert_storage_vert &v);
mll::io::ibinstream& operator>>(mll::io::ibinstream &i, vert_storage_vert &v);

///полигон (индексированный)
class poly
{
private:
	typedef std::vector<int> idxs;
	typedef std::string matid_t;
	idxs m_idxs;
	matid_t m_matid;
	int m_floor;
public:
	typedef vert_storage::vert vert;
	vert_storage &m_verts;


	poly(vert_storage &vs, const matid_t &matid, int floor);
	poly(const poly&p);
	poly& operator=(const poly&p);
	~poly(void);

	int size() const {return m_idxs.size();}
	const vert& v(int i)const {return m_verts[m_idxs[i]];}
	vert& v(int i) {return m_verts[m_idxs[i]];}
	unsigned    i(int i)const {return m_idxs[i];}
	int&    i(int i) {return m_idxs[i];}
	const matid_t mat()const {return m_matid;};
	plane get_plane() const;
	int get_floor() const {return m_floor;};
	mll::algebra::vector3 norm() const;


	void add_vert(const vert &v)
	{
		m_idxs.push_back(m_verts.add(v));
	};
    bool adjoint(const poly &b) const;
	split::split_type classify(const plane &p) const;
	
};
