#pragma warning(disable:4786)
#include "precomp.h"
#include "oct_tree_idx.h"
#include "oct_tree_cull.h"
#include <algorithm>
#include <numeric>
#pragma warning(disable:4786)


using mll::geometry::frustum;
using mll::geometry::aabb3;
using mll::geometry::ray3;



namespace //функторы
{
	struct tri_less
	{
		bool operator()(const oct_tree::triangle *a, const oct_tree::triangle *b)const
		{return a->mat_id<b->mat_id;}
		bool operator()(const oct_tree::triangle &a, const oct_tree::triangle &b)const
		{return a.mat_id<b.mat_id;}
	};
};

//=====================================================================================//
//                              float intersectTriangle()                              //
//=====================================================================================//
//float intersectTriangle(const point3 &v1, const point3 &v2, const point3 &v3,const ray3 &ray);
//=====================================================================================//

void oct_tree::construct(const PolyList &list, const storage &vstorage, const MatList& mats,
						 float cell_size, int min_poly_count)
{
   // перенесем все данные в наш контейнер.

	m_vertices = vstorage;
	m_triangles = list;
	m_materials = mats;

	std::sort(m_triangles.begin(), m_triangles.end(), tri_less() );

	//найдем общий обрамляющий объем.
	aabb3 box;
	m_box.degenerate();
	for(vert_storage::const_iterator i=m_vertices.begin(); i!=m_vertices.end(); i++)
	{
		box.grow(i->pos);
		m_box.grow(i->pos);
	}

	//доведём его до обрамляющего куба
	mll::algebra::vector3 diag = box.diag();
	float maxl = std::max(diag.x, std::max(diag.y,diag.y));
	diag = mll::algebra::vector3(maxl,maxl,maxl);
	box = aabb3(box.min(),diag);

	IdxPolyList ilist;
	ilist.resize(list.size());
	int ii=0;
	for(PolyList::iterator it=m_triangles.begin(); it!=m_triangles.end(); ++it,++ii)
		ilist[ii]=&*it;
	//std::iota(ilist.begin(), ilist.end(),0);

	m_min_poly_count=min_poly_count;
	m_cell_size=cell_size;

	m_root.reset( rec_contruct(ilist, box) );
}

void oct_tree::set_visible_floor(int upto)
{
	m_floor_upto = upto;
}

const mll::geometry::aabb3& oct_tree::getBBox()
{
	//assert(m_root.get());
	//return m_root->bound();
	return m_box;
};

oct_tree::node *oct_tree::rec_contruct(const IdxPolyList &list, const mll::geometry::aabb3 &bound)
{
	node *Node = new node;
	
	Node->m_bound = bound;
	Node->set_trinagles(list);

	//если дальше сечь не нужно - выход
	mll::algebra::vector3 diag = bound.diag();
	float maxl = std::max(diag.x, std::max(diag.y,diag.z));
	if((int)list.size() <= m_min_poly_count || maxl <= m_cell_size)
	{
		return Node;
	}

	//найти 3 секущих плоскости
	mll::algebra::point3 center = Node->m_bound.min() + 0.5f*Node->m_bound.diag();
	plane px(center, mll::algebra::vector3(1,0,0));
	plane py(center, mll::algebra::vector3(0,1,0));
	plane pz(center, mll::algebra::vector3(0,0,1));
	//начать проводить сечения на 8 частей
	IdxPolyList quat_final[2];
	IdxPolyList half_final[4];
	IdxPolyList final[8];

	split(list, px, &quat_final[s_far], &quat_final[s_near]);

	split(quat_final[s_near], py, &half_final[s_near|s_right], &half_final[s_near|s_left]); 
	split(quat_final[s_far], py, &half_final[s_far|s_right], &half_final[s_far|s_left]);

	split(half_final[ s_near|s_left ],  pz,
		&final[s_near|s_left|s_top], &final[s_near|s_left|s_bottom]);

	split(half_final[ s_near|s_right], pz, 
		&final[s_near|s_right|s_top],&final[s_near|s_right|s_bottom]);

	split(half_final[s_far|s_right],  pz, 
		&final[s_far|s_right|s_top], &final[s_far|s_right|s_bottom]);

	split(half_final[s_far|s_left],   pz,
		&final[s_far|s_left|s_top],  &final[s_far|s_left|s_bottom]);

	int i;

	//очистим временные списки ради экономии памяти
	for(i=0; i<4; i++)
	{
		if(i<2) quat_final[i].clear();
		half_final[i].clear();
	}

	//если все дочерние списки того же размера - прекратим разбиение.
	for(i=0;i<8;i++)
	{
		if(list.size()>final[i].size()) break;
	}
	if(i==8)
		return Node;

	//вдаримся в рекурсию
	for(i=0; i<8; i++)
	{
		float x = i&1 ? bound.min().x : bound.max().x;
		float y = i&2 ? bound.min().y : bound.max().y;
		float z = i&4 ? bound.min().z : bound.max().z;

		aabb3 ch_bound(center,mll::algebra::point3(x,y,z));

		if(final[i].size())
		{
			Node->m_child[i].reset( rec_contruct(final[i],ch_bound) );
			Node->m_has_children = true;
		}
	}

	return Node;
}

void oct_tree::split(const IdxPolyList &list, const plane &p, IdxPolyList *pos, IdxPolyList *neg)
{
	for(IdxPolyList::const_iterator i=list.begin(); i!= list.end(); i++)
	{
		float d1 = p.testpoint(  m_vertices[ (*i)->v[0] ].pos  );
		float d2 = p.testpoint(  m_vertices[ (*i)->v[1] ].pos  );
		float d3 = p.testpoint(  m_vertices[ (*i)->v[2] ].pos  );

		if(d1>=0.f && d2>=0.f && d3>=0) 
		{
			pos->push_back(*i);
		}
		else if(d1<=0.f && d2<=0.f && d3<=0.f)
		{
			neg->push_back(*i);
		}
		else
		{
			pos->push_back(*i);
			neg->push_back(*i);
		}
	}

}

float oct_tree::intersect(const mll::geometry::ray3 &ray, floor_range_t floor_range, hit_info *hit, const node *root_node) const
{
	using namespace mll::algebra;

	const float BAD = numerical_traits<float>::get_bad_ray_intersection();

	if(!root_node) root_node=m_root.get();

	std::pair<float,float> rres=root_node->m_bound.full_intersect(ray);
	float res = (rres.first!=BAD) ? rres.first : rres.second;
	if(res == root_node->m_bound.bad_result())
	{
		return BAD;
	}
	else 
	{
		if(root_node->has_children())
		{
			//если есть дочерние узлы
			//обойти узлы в порядке дальности относительно луча.
			//при первом пересечении вернуть результат
			float minp=BAD;

			for(int i=0; i<8; i++)//HACK нужно обходить в правильном порядке
			{
				if(root_node->m_child[i].get())
				{
					hit_info h;
					float t=intersect(ray, floor_range, &h, root_node->m_child[i].get());
					if( (t<minp || minp==BAD) && t!=BAD)
					{
						minp=t;
						if(hit) *hit = h;
					}
				}

			}
			return minp;
		}
		else
		{
			//если это узел-лист обойти все его треугольники
			float minp=BAD;
			for(unsigned i=0; i<root_node->m_tris.size(); i++)
			{
				triangle *tri = root_node->m_tris[i];
				if(tri->floor<floor_range.first || tri->floor>floor_range.second)
					continue;
				float t = ray.intersect_triangle( m_vertices[tri->v[0] ].pos, 
					m_vertices[tri->v[1] ].pos,
					m_vertices[tri->v[2] ].pos);
				if( t!=BAD && (t<minp || minp==BAD))
				{
					minp=t;
					if(hit) 
					{
						hit->mat = m_materials[tri->mat_id];
						{
							using mll::algebra::origin;
							using mll::algebra::null_vector;

							const float epsilon = 1.0e-5f;
							const vector3 e1 = m_vertices[tri->v[0] ].pos-m_vertices[tri->v[2] ].pos;
							const vector3 e2 = m_vertices[tri->v[1] ].pos-m_vertices[tri->v[2] ].pos;
							const vector3 p = ray.direction.cross(e2);
							const float a = e1.dot(p);
							const float f = 1.0f/a;
							const vector3 s = ray.origin-m_vertices[tri->v[2] ].pos;
							const float u = f*(s.dot(p));
							const vector3 q = s.cross(e1);
							const float v = f*(ray.direction.dot(q));
							hit->uv = (u*m_vertices[tri->v[0] ].uv-mll::algebra::origin)+
								(m_vertices[tri->v[1] ].uv*v-mll::algebra::origin)+
								m_vertices[tri->v[2] ].uv*(1-u-v);
						}
					}
				}
			}
			return minp;
		}
	}
}

void oct_tree::update(const mll::geometry::frustum &f, vis_tris_observer* obs ,node *root_node)
{

	bool root_enter=false;
	if(!root_node)
	{
		root_enter = true;
		root_node=m_root.get();
	}

	frustum::classify_result_type c = f.classify(root_node->m_bound);
	if (c == frustum::outside)
	{//эта часть дерева полностью не видна
		root_node->m_visible = oct_tree::node::none;
	}
	else if(c == frustum::inside || !root_node->m_has_children)
	{  //Эта часть дерева полностью видна
		obs->tris_visible( root_node->m_tris,const_cast<oct_tree::triangle*>(&m_triangles[0]), m_floor_upto, m_pass );
		root_node->m_visible = oct_tree::node::all;
	}
	else if(c == frustum::intersect)
	{
		root_node->m_visible = oct_tree::node::partly;
		for(int i=0; i<8; i++)
		{
			if(root_node->m_child[i].get())
				update(f, obs, root_node->m_child[i].get());
		}
	}
	if(root_enter && obs)
	{
		obs->end();
		m_pass++;
	}

	//for (std::vector<mll::geometry::aabb3>::iterator i=m_boxes.begin(); i!=m_boxes.end(); ++i )
	//{
	//	MLL_DEBUG_RENDER(mll::debug::color(0xff00ff00) << (*i) );
	//}
}

unsigned oct_tree::NodeCount(const node *start) const
{
		if(!start) start=m_root.get();
		unsigned i=0;
		for(int q=0;q<8;q++)
		  if(start->m_child[q].get())
			  i+=NodeCount(start->m_child[q].get());
		return i+1;

}
unsigned oct_tree::GetSize(const node *start) const
{
		unsigned i=0;
		if(!start)
		{
			start=m_root.get();
			i += m_triangles.size()*sizeof(triangle); 
			i += m_vertices.size()*sizeof(vert); 
			i += sizeof(oct_tree);
		}

		i+=sizeof(node)+  start->m_tris.size()*sizeof(unsigned);
		
		for(int q=0;q<8;q++)
		  if(start->m_child[q].get())
			  i+=GetSize(start->m_child[q].get());
		return i;

}
unsigned oct_tree::OverAllTriIdxs(const node *start) const
{
		unsigned i=0;
		if(!start)
		{
			start=m_root.get();
		}

		i += start->m_tris.size(); 
		for(int q=0;q<8;q++)
		  if(start->m_child[q].get())
			  i+=OverAllTriIdxs(start->m_child[q].get());
		return i;

}
void oct_tree::traverse(visitor *v, node *startnode)
	{
		if(!startnode) startnode=m_root.get();

		if(v->visit(*this, startnode))
		{
			for(int i=0;i<8;i++)
				if(startnode->m_child[i].get()) traverse(v,startnode->m_child[i].get());
		}
	}
void oct_tree::traverse(const_visitor *v, const node *startnode) const
	{
		if(!startnode) startnode=m_root.get();

		if(v->visit(*this, startnode))
		{
			for(int i=0;i<8;i++)
				if(startnode->m_child[i].get()) traverse( v,startnode->m_child[i].get() );
		}
	}

mll::io::obinstream& operator <<(mll::io::obinstream &o, const _ot_triangle& t)
	{
	o<< t.v[0]<< t.v[1]<< t.v[2]; 
	o<< t.floor<< t.mat_id; 
	 return o;
	}
mll::io::ibinstream& operator >>(mll::io::ibinstream &i, _ot_triangle& t)
	{
	i>> t.v[0]>>t.v[1]>>t.v[2]; 
	i>> t.floor>>t.mat_id; 
	t.rendered = -1;
	//t.activated = false;
return i;
	}

///вспомогательный класс-визитор для сохранения дерева
class save_helper: public oct_tree::const_visitor
{
private:
	mll::io::obinstream &m_o;
public:
	save_helper(mll::io::obinstream &o):m_o(o)  {;};
	bool visit(const oct_tree &tree, const oct_tree::node *n) //если true - продолжить рекурсию
	{
	using namespace mll::io;
		unsigned children=0;
		for(int i=0; i<8; i++)
		{
		children |= (*n)[i] ?  1<<i : 0;
		}
		//сохранение маски дочерних узлов.
		m_o<<children;
		//сохранение текущего узла.
		m_o<<n->bound()<< n->m_tris.size();
		for(unsigned k=0;k<n->m_tris.size(); k++)
			m_o<<n->m_tris[k]-&tree.get_triangles()[0];

		return true;
	}
};
///вспомогательный класс-визитор для загрузки дерева
class load_helper: public oct_tree::visitor
{
private:
	mll::io::ibinstream &m_i;
public:
	load_helper(mll::io::ibinstream &i):m_i(i)  {;};
	bool visit(oct_tree &tree, oct_tree::node *n) //если true - продолжить рекурсию
	{
 	using namespace mll::io;

		unsigned children;
	    m_i>>children;

		if(children) n->m_has_children=true;

		int nn;///<размер буфера индексов

		m_i>>n->m_bound>>nn;
		n->m_tris.resize(nn);
		for(unsigned k=0; k<n->m_tris.size(); k++)
		{
			unsigned idx;
			m_i>>idx;
			n->m_tris[k] = const_cast<oct_tree::triangle*>(&tree.get_triangles()[0]) + idx;
		}

		for(int i=0; i<8; i++)
		{
			bool load = children&1;
			children >>=1;
			if(load)
			{
				n->m_child[i].reset(new oct_tree::node());
				//i>>*new_node;
			}
		}

		return true;
	}
};

mll::io::obinstream& operator <<(mll::io::obinstream &o, const oct_tree& tree)
{
	using namespace mll::io;

	o << (int)-1;	//< присутствует номер версии
	o << (int)1;	//< номер версии
	o << tree.m_materials.size() << mll::io::array(tree.m_materials.begin(),tree.m_materials.end());
	o << tree.m_triangles.size() << array(tree.m_triangles.begin(),tree.m_triangles.end());
	o << tree.m_vertices.size() << array(tree.m_vertices.begin(), tree.m_vertices.end());
	save_helper sh(o);
	tree.traverse(&sh);
	o << tree.m_box;
	return o;
}

mll::io::ibinstream& operator >>(mll::io::ibinstream &i, oct_tree& tree)
{
	using namespace mll::io;
	
	int p_size,v_size,m_size;
	int ver_flag,ver=0;
	
	i>>ver_flag;
	if (ver_flag==-1)	// если есть номер версии
	{
		i>>ver;
		i>>p_size;
	}
	else p_size=ver_flag;

	tree.m_materials.resize(p_size);
	i>>array(tree.m_materials.begin(),tree.m_materials.end());
	
	i>>v_size;
	tree.m_triangles.resize(v_size);
	i>>array(tree.m_triangles.begin(),tree.m_triangles.end());

	i>>m_size;
	tree.m_vertices.m_verts.resize(m_size);
	i>>array(tree.m_vertices.m_verts.begin(), tree.m_vertices.m_verts.end());
  
	load_helper lh(i);
	tree.m_root.reset(new oct_tree::node());
	tree.traverse(&lh);

	if (ver>1)
	{
		// считывем бокс уровня
		i>>tree.m_box;
	}
	else
	{
		// вычисляем бокс уровня
		tree.m_box.degenerate();
		const oct_tree::storage& v = tree.get_storage();
		const oct_tree::PolyList& t = tree.get_triangles();
		oct_tree::PolyList::const_iterator end = t.end();
		for (oct_tree::PolyList::const_iterator i = t.begin();i<end;++i)
		{
			tree.m_box.grow(v[(*i).v[0]].pos);
			tree.m_box.grow(v[(*i).v[1]].pos);
			tree.m_box.grow(v[(*i).v[2]].pos);
		}
	}

	return i;
}

/////////////////////////////////////////////////////////////
void oct_tree::node::set_trinagles(const oct_tree::IdxPolyList &list)
{
	m_tris = list;
}

std::vector<unsigned> oct_tree::get_indices() const
{
	std::vector<unsigned> indexes;
	indexes.reserve(m_triangles.size()*3);
	for(unsigned i=0; i<m_triangles.size(); ++i)
	{
		indexes.push_back(m_triangles[i].v[0]);
		indexes.push_back(m_triangles[i].v[1]);
		indexes.push_back(m_triangles[i].v[2]);
	}
	return indexes;
}
//------------------------------------------------------------------------------------------------------------------------------------
void oct_tree::getShadowedTriangles(std::vector<mll::algebra::point3> &vertices, node *root_node )
{
	if(!root_node) root_node=m_root.get();
	//
	aabb3::classify_result_type c = m_aabb.classify(root_node->m_bound);
	if(c == aabb3::crt_outside) return; //эта часть дерева полностью не видна
	//
	if(c == aabb3::crt_inside || !root_node->m_has_children)
	{  //Эта часть дерева полностью видна
		{
			for ( std::vector<triangle*>::iterator i = root_node->m_tris.begin(); i != root_node->m_tris.end(); ++i )
			{
				if( m_floor_upto<(*i)->floor && m_floor_upto!=-1) continue;
				if ( (*i)->rendered != m_pass )
				{
					(*i)->rendered = m_pass;
					mll::algebra::point3 &p1 = m_vertices[(*i)->v[0]].pos;
					mll::algebra::point3 &p2 = m_vertices[(*i)->v[1]].pos;
					mll::algebra::point3 &p3 = m_vertices[(*i)->v[2]].pos;
					//
					if (m_aabb.intersect_triangle(p1,p2,p3))
					{
						// эта фишка не дает рисовать selection на стенах
						mll::algebra::vector3f norm(((p1-mll::algebra::origin-(p2-mll::algebra::origin)).cross(p1-mll::algebra::origin-(p3-mll::algebra::origin))).normalize());
						if (((norm.cross(mll::algebra::vector3f(0,0,1))).length())<0.9f) // sin(a)
						{
							vertices.push_back(p1);
							vertices.push_back(p2);
							vertices.push_back(p3);
						}
					}
				}
			}
		}
	}
	else if(c == aabb3::crt_intersect)
	{  // частичная видимость, проверяем все дочерние узлы
		for(int i=0; i<8; i++)
		{
			if(root_node->m_child[i].get())
				getShadowedTriangles(vertices, root_node->m_child[i].get() );
		}
	}
}

void oct_tree::getShadowedTriangles(const mll::geometry::aabb3 &aabb, std::vector<mll::algebra::point3> &vertices,int floor)
{
	int prev_foor = m_floor_upto;
	if (floor!=-1) m_floor_upto = floor;
	m_aabb = aabb;
	getShadowedTriangles(vertices);
	m_floor_upto = prev_foor;
	++m_pass;
}
//------------------------------------------------------------------------------------------------------------------------------------
/// вырезать треугольники по боксу

void oct_tree::cut(const mll::geometry::obb3 &obb, std::vector<point3> &vertices, std::vector<unsigned short> &indices,node *root_node)
{
	if(!root_node)
	{
		std::vector<mll::algebra::point3> p;
		p.push_back(mll::algebra::point3(0.f,0.f,0.f));
		p.push_back(mll::algebra::point3(1.f,0.f,0.f));
		p.push_back(mll::algebra::point3(0.f,1.f,0.f));
		//
		std::vector<unsigned short> i;
		i.push_back(0);
		i.push_back(1);
		i.push_back(2);
		//
		oct_tree_cull a(obb);
		a.cut(p,i);
		//
		root_node=m_root.get();
	}
	/*
	aabb3::classify_result_type c;// = root_node->m_bound.classify(obb);
	if(c==aabb3::crt_outside) return;
	else if(c == aabb3::crt_intersect)
	{
		for(int i=0; i<8; i++)
		{
			if(root_node->m_child[i].get())
				cut(obb,vertices,indices,root_node->m_child[i].get());
		}
	}
	else if(c == aabb3::crt_inside || !root_node->m_has_children)
	*/
	{
		/*
		for ( std::vector<triangle*>::iterator i = root_node->m_tris.begin(); i != root_node->m_tris.end(); ++i )
		{
			if( m_floor_upto<(*i)->floor && m_floor_upto!=-1) continue;
			if ( (*i)->rendered != m_pass )
			{
				(*i)->rendered = m_pass;
				point3 &p1 = m_vertices[(*i)->v[0]].pos;
				point3 &p2 = m_vertices[(*i)->v[1]].pos;
				point3 &p3 = m_vertices[(*i)->v[2]].pos;
				//
				if (m_aabb.intersect_triangle(p1,p2,p3))
				{
					vertices.push_back(p1);
					vertices.push_back(p2);
					vertices.push_back(p3);
				}
			}
		}
		*/
	}
}
//------------------------------------------------------------------------------------------------------------------------------------
