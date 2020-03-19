#include "precomp.h"
#include "poly.h"

//=====================================================================================//
//                              const float poly::epsilon                              //
//=====================================================================================//
const float plane::epsilon = 0.001f;

//=====================================================================================//
//                                     class poly                                      //
//=====================================================================================//
poly::poly(vert_storage &vs, const poly::matid_t &matid, int floor)
:m_verts(vs),m_matid(matid),m_floor(floor)
{
}
poly::poly(const poly &p)
:m_verts(p.m_verts), m_matid(p.m_matid), m_idxs(p.m_idxs), m_floor(p.m_floor)
{

}
poly& poly::operator=(const poly&p)
{
m_verts=p.m_verts;
m_matid=p.m_matid;
m_idxs=p.m_idxs;
m_floor = p.m_floor;
 return *this;
}

poly::~poly(void)
{
}

plane poly::get_plane()	const
{
	return plane(v(0).pos, norm());
}
mll::algebra::vector3 poly::norm() const
{
	return (  v(1).pos-v(0).pos  ).cross(  v(2).pos-v(0).pos  ).normalize();
}

bool poly::adjoint(const poly &b) const
  {
	  
  int lastvert=static_cast<int>(m_idxs.size()-1);
  for(unsigned i=0;i<m_idxs.size();i++)
    {
    int lvertb=static_cast<int>(b.m_idxs.size()-1);
    for(unsigned j=0;j<b.m_idxs.size();j++)
      {
      if(m_idxs[lastvert]==b.m_idxs[j])
        if(m_idxs[i]==b.m_idxs[lvertb])
          return true;
      lvertb=j;
      }
    lastvert=i;
    }
  return false;
  }

split::split_type poly::classify(const plane &p) const
  {
	  int pos=0;
	  int neg=0;
	  for(idxs::const_iterator i=m_idxs.begin();  i!=m_idxs.end();  i++)
	  {
		  float d=p.testpoint(m_verts[*i].pos);
		  if(d<0) neg++;
		  else pos++;
	  }
	  if(neg==0) return split::positive;
	  if(pos==0) return split::negative;
	  return split::divided;

  }

mll::io::obinstream& operator<<(mll::io::obinstream &o, const vert_storage_vert &v)
{
	return o<<v.pos<<v.norm<<v.color<<v.uv<<v.lm_uv;
}

mll::io::ibinstream& operator>>(mll::io::ibinstream &i, vert_storage_vert &v)
{
	return i>>v.pos>>v.norm>>v.color>>v.uv>>v.lm_uv;
}
