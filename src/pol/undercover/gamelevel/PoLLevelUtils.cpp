#include "precomp.h"

#include "Chop.h"
#include "oct_tree_idx.h"
#include "PoLLevelUtils.h"
#include "../Skin/Shadow.h"
#include "IntersectionCache.h"

namespace PoLLevelUtilsDetail
{

enum IntersectResult { irOutside, irInside, irIntersects };

//=====================================================================================//
//                                   struct PoLPlane                                   //
//=====================================================================================//
struct PoLPlane
{
	point3 m_normal;
	float m_d;

	PoLPlane() {}

	PoLPlane(const point3 &on, const point3 &dir)
	:	m_normal(Normalize(on-dir)),
		m_d(-m_normal.Dot(on))
	{
	}
};

//=====================================================================================//
//                           inline bool pointInsidePlane()                            //
//=====================================================================================//
inline bool pointInsidePlane(const point3 &pt, const PoLPlane &p)
{
	return pt.Dot(p.m_normal) + p.m_d <= 0.0f;
}

//=====================================================================================//
//                        inline point3 intersectSectionPlane()                        //
//=====================================================================================//
inline point3 intersectSectionPlane(const point3 &pt1, const point3 &pt2, const PoLPlane &pl)
{
	const float epsilon = 1e-5f;
	const point3 dir = Normalize(pt2-pt1);
	const float ndotx0 = pt1.Dot(pl.m_normal);
	const float ndotr = pl.m_normal.Dot(dir);
	if(fabsf(ndotr) < epsilon) return pt1;
	const float t = - (pl.m_d+ndotx0) / ndotr;
	return pt1 + t*dir;
}

//=====================================================================================//
//                             inline int sliceTriangle()                              //
//=====================================================================================//
inline int sliceTriangle(const point3 *tri, point3 *res, const PoLPlane *planes, int planesCount = 6)
{
	int curcount = 3;
	std::copy_n(tri,3,res);

	point3 tres[64];
	int outcount = 0;

	for(int j = 0; j < planesCount; ++j)
	{
		bool prevPointInside = pointInsidePlane(res[curcount-1],planes[j]);

		for(int i = 0; i < curcount; ++i)
		{
			const bool curPointInside = pointInsidePlane(res[i],planes[j]);

			if(curPointInside != prevPointInside)
			{
				point3 isect = intersectSectionPlane(res[i==0?curcount-1:i-1],res[i],planes[j]);
				tres[outcount++] = isect;
			}
			
			if(curPointInside)
			{
				tres[outcount++] = res[i];
			}

			prevPointInside = curPointInside;
		}

		std::copy_n(tres,outcount,res);
		curcount = outcount;
		outcount = 0;
	}

	return curcount;
}

//=====================================================================================//
//                               inline void addVertex()                               //
//=====================================================================================//
inline void addVertex(point3 *outPolygon, unsigned &outVertices, bool lastVertex, point3 pt)
{
	const float epsilon = 1e-6f;
	if (outVertices == 0 ||
		(
			fabs(pt.x - outPolygon[outVertices-1].x) > epsilon ||
			fabs(pt.y - outPolygon[outVertices-1].y) > epsilon ||
			fabs(pt.z - outPolygon[outVertices-1].z) > epsilon
		) && (
			!lastVertex ||
			fabs (pt.x - outPolygon[0].x) > epsilon ||
			fabs (pt.y - outPolygon[0].y) > epsilon ||
			fabs (pt.z - outPolygon[0].z) > epsilon
		)
	) 
	{
		outPolygon[outVertices] = pt;
		outVertices++;
	}
}

//=====================================================================================//
//                             inline void chopTriangle()                              //
//=====================================================================================//
// ќбрезать треугольник плоскостью
inline void chopTriangle(const point3 *inPolygon, unsigned inVertices, const point3 &n,
						 const point3 &Pos, point3 *outPolygon, unsigned &outVertices)
{
	outVertices = 0;
	bool lastVertex = false;
	point3 prev = inPolygon[0];
	bool prevVertexInside = (n.Dot(prev - Pos) > 0);
	unsigned intersectionCount = 0;

	for (int i = 0; i <= inVertices; ++i)
	{
		point3 cur;

		if (i < inVertices)
		{
			cur = inPolygon[i];
		} else
		{
			cur = *inPolygon;
			lastVertex = true;
		}

		if(prevVertexInside) addVertex (outPolygon, outVertices, lastVertex, prev);

		bool curVertexInside = (n.Dot(cur - Pos) > 0);

		if (prevVertexInside != curVertexInside)
		{
			float denominator = n.Dot(cur - prev);

			if (denominator != 0.0)
			{
				float t = n.Dot (Pos - prev) / denominator;
				point3 temp;

				if (t <= 0.0f)
				{
					temp = prev;
				} else if (t >= 1.0f)
				{
					temp = cur;
				} else
				{
					temp = prev + t*(cur - prev);
				}

				addVertex (outPolygon, outVertices, lastVertex, temp);
			}

			if (++intersectionCount >= 2)
			{
				if (fabs(denominator) < 1) intersectionCount = 0;
				else
				{
					if (curVertexInside)
					{
						memcpy (outPolygon + outVertices, inPolygon + i, (inVertices-i)*sizeof(point3));
						outVertices += inVertices - i;
					}

					break;
				}
			}
		}

		prev = cur;
		prevVertexInside = curVertexInside;

	}

	if (outVertices < 3) outVertices = 0;
}

//=====================================================================================//
//                             inline void chopTriangle()                              //
//=====================================================================================//
inline void chopTriangle(const point3 *inPolygon, const point3 &n,
						 const point3 &Pos, point3 *outPolygon, unsigned &outVertices)
{
	chopTriangle(inPolygon,3,n,Pos,outPolygon,outVertices);
}

//=====================================================================================//
//                        IntersectResult intersectSphereAabb()                        //
//=====================================================================================//
IntersectResult intersectSphereAabb(const mll::algebra::point3 &origin,
									float rad, const mll::geometry::aabb3 &box)
{
	const float halflen = box.diag().length() * 0.5f;
	if((origin - box.center()).length() > rad + halflen)
	{
		return irOutside;
	}
	return irIntersects;
}

//=====================================================================================//
//                               class TriangleCollector                               //
//=====================================================================================//
class TriangleCollector
{
	friend struct Functor;

	struct Functor
	{
		TriangleCollector *m_coll;
		Functor(TriangleCollector *c) : m_coll(c) {}

		bool operator()(oct_tree::node *node) const { return m_coll->collect(node); }
	};

private:
	oct_tree *m_tree;
	mll::algebra::point3 m_origin;
	Primi *m_mark;
	const oct_tree::storage *m_verts;
	float m_rad;
	unsigned int m_pass;
	unsigned int m_col;

	PoLPlane m_planes[6];

public:
	TriangleCollector(oct_tree *tree, Primi *mark, unsigned int col,
					  const mll::algebra::point3 &pos, float rad)
	:	m_origin(pos),
		m_tree(tree),
		m_mark(mark),
		m_rad(rad),
		m_pass(tree->m_pass),
		m_verts(&m_tree->get_storage()),
		m_col(col)
	{
		++m_tree->m_pass;

		point3 orig = point3(m_origin);

		m_rad *= 1.1f;
		m_planes[0] = PoLPlane(orig + point3(-m_rad,   0.0f,   0.0f), orig + point3( m_rad,   0.0f,   0.0f));
		m_planes[1] = PoLPlane(orig + point3( m_rad,   0.0f,   0.0f), orig + point3(-m_rad,   0.0f,   0.0f));
		m_planes[2] = PoLPlane(orig + point3(  0.0f, -m_rad,   0.0f), orig + point3(  0.0f,  m_rad,   0.0f));
		m_planes[3] = PoLPlane(orig + point3(  0.0f,  m_rad,   0.0f), orig + point3(  0.0f, -m_rad,   0.0f));
		m_planes[4] = PoLPlane(orig + point3(  0.0f,   0.0f, -m_rad), orig + point3(  0.0f,   0.0f,  m_rad));
		m_planes[5] = PoLPlane(orig + point3(  0.0f,   0.0f,  m_rad), orig + point3(  0.0f,   0.0f, -m_rad));
		m_rad /= 1.1f;
	}

	void collect()
	{
		m_mark->Verts.reserve(1024*3);
		m_mark->Idxs.reserve(1024*3);
		m_tree->traverse2(Functor(this));
	}

private:
	bool collect(oct_tree::node *node)
	{
		bool result = false;
		switch(intersectSphereAabb(m_origin,m_rad,node->m_bound))
		{
			case irInside:
				addTriangles(node);
			case irOutside:
				result = false;
				break;
			case irIntersects:
				result = node->m_has_children;
				if(!result) addTriangles(node);
				break;
		}
		return result;
	}

	void addTriangle(oct_tree::triangle * const tri)
	{
		point3 v[3];
		float tu[3],tv[3]; //текстурные координаты
		point3 ax,ay; //оси в плоскости
		float sx,sy; //дл€ оптимизации при просчитывании текстурных координат

		v[0] = (*m_verts)[tri->v[0]].pos;
		v[1] = (*m_verts)[tri->v[1]].pos;
		v[2] = (*m_verts)[tri->v[2]].pos;

		if((v[0] - v[1]).Length() < 1e-5f) return;
		if((v[1] - v[2]).Length() < 1e-5f) return;
		if((v[0] - v[2]).Length() < 1e-5f) return;

		point3 tnormal = (v[0]-v[1]).Cross(v[2]-v[1]);
		if(tnormal.Length() < 1e-5f) return;
		point3 normal = Normalize(tnormal);
		//if(normal.z < 0.0f) normal *= -1.0f;
		//if(normal.Dot(point3((*m_verts)[tri->v[0]].norm)) > 0.0f) normal *= -1.0f;
		//if(normal.Dot(v[0] - point3(m_origin)) > 0.0f) return;
		point3 bias = normal*0.005f;

		float d = normal.Dot(point3(m_origin)-v[0]);
		if(fabsf(d) >= m_rad) return;

		float _r = (m_col>>16)&0xff;  //рассчитаем цвет на удалении
		float _g = (m_col>>8)&0xff;
		float _b = (m_col)&0xff;
		float alpha = 1.0f - fabsf(d/m_rad);
		_r*=alpha;_g*=alpha;_b*=alpha;
		unsigned long nCol = RGB_MAKE((int)_r,(int)_g,(int)_b) + (static_cast<unsigned int>(alpha*255.0f) << 24);

		const point3 axis(0.1f,0.124314f,0.1f); //абстрактна€ ось
		float r = 0.5f/sqrtf(m_rad*m_rad - d*d); //обратный радиус п€тна на этой плоскости (0,5 дл€ масштабировани€ uv в [-0.5 0.5])
		ax = r*Normalize(normal.Cross(axis));
		ay = r*Normalize(normal.Cross(ax));

		point3 Center = point3(m_origin) - normal*d;
		assert( normal.Dot(Center - v[0]) < 1e-3f );
		sx = 0.5f - Center.Dot(ax);
		sy = 0.5f - Center.Dot(ay);

		for(int i = 0; i < 3; ++i) v[i] += bias;

		point3 endPoly[128];
		point3 tmpPoly[128];
		int endVertCount;

		// ќбрежем треугольник кубом, ограничивающим отметину
		endVertCount = sliceTriangle(v,endPoly,m_planes);
		
		// ќбрезали - остаток добавл€ем в массив
		v[0] = endPoly[0];
		for (int trs = 0; trs < endVertCount-2; trs++)
		{
			// Ќадо казус сюда добавить на вс€кий случай
			//if (m_mark->GetTriangleCount() >= BaseMark::MAXTRIS-4) return;

			v[1] = endPoly[trs+1];
			v[2] = endPoly[trs+2];

			tu[0]=v[0].Dot(ax)+sx;   tv[0]=v[0].Dot(ay)+sy;
			tu[1]=v[1].Dot(ax)+sx;   tv[1]=v[1].Dot(ay)+sy;
			tu[2]=v[2].Dot(ax)+sx;   tv[2]=v[2].Dot(ay)+sy;
			//vmin=tv[0]<tv[1]?tv[0]:tv[1];  vmin=vmin<tv[2]?vmin:tv[2];
			//umin=tu[0]<tu[1]?tu[0]:tu[1];  umin=umin<tu[2]?umin:tu[2];
			//vmax=tv[0]<tv[1]?tv[1]:tv[0];  vmax=vmax>tv[2]?vmax:tv[2];
			//umax=tu[0]<tu[1]?tu[1]:tu[0];  umax=umax>tu[2]?umax:tu[2];
			//if(vmin>1||vmax<0||umax<0||umin>1) continue;
			
			Primi::FVFVertex vert1, vert2, vert3;
			vert1.uv = texcoord(tu[0],tv[0]);
			vert2.uv = texcoord(tu[1],tv[1]);
			vert3.uv = texcoord(tu[2],tv[2]);
			vert1.pos = v[0];
			vert2.pos = v[1];
			vert3.pos = v[2];
			vert1.diffuse = nCol;
			vert2.diffuse = nCol;
			vert3.diffuse = nCol;

			m_mark->Verts.push_back(vert1);
			m_mark->Verts.push_back(vert2);
			m_mark->Verts.push_back(vert3);
		}
	}

	void addTriangles(oct_tree::node *node)
	{
		oct_tree::IdxPolyList::iterator ie = node->m_tris.end();
		for(oct_tree::IdxPolyList::iterator i = node->m_tris.begin(); i != ie; ++i)
		{
			oct_tree::triangle * const tri = *i;
			if(tri->rendered != m_pass)
			{
				addTriangle(tri);
				tri->rendered = m_pass;
			}
		}
	}
};

//=====================================================================================//
//                                bool PCRayTriangle()                                 //
//=====================================================================================//
bool PCRayTriangle(const mll::algebra::point3 &orig, const mll::algebra::vector3 &dir,
				   const mll::algebra::point3 &v1, const mll::algebra::point3 &v2,
				   const mll::algebra::point3 &v3, float *t, float *u, float *v)
{
	mll::algebra::vector3 edge1, edge2, tvec, pvec, qvec;
	float det,inv_det;

	/* find vectors for two edges sharing vert0 */
	edge1 = v2-v1;
	edge2 = v3-v1;

	/* begin calculating determinant - also used to calculate U parameter */
	pvec = dir.cross(edge2);
	/* if determinant is near zero, ray lies in plane of triangle */
	det = edge1.dot(pvec);

	if (fabsf(det) < epsilon)
		return false;
	inv_det = 1.0f / det;

	/* calculate distance from vert0 to ray origin */
	tvec = orig - v1;

	/* calculate U parameter and test bounds */
	*u = tvec.dot(pvec) * inv_det;
	if (*u < 0.0 || *u > 1.0)
		return false;

	/* prepare to test V parameter */
	qvec=tvec.cross(edge1);

	/* calculate V parameter and test bounds */
	*v = dir.dot(qvec) * inv_det;
	if (*v < 0.0 || *u + *v > 1.0)
		return false;

	/* calculate t, ray intersects triangle */
	*t = edge2.dot(qvec) * inv_det;
	return *t >= 0.0f;
	//return true;
}

//=====================================================================================//
//                                 class CommonTracer                                  //
//=====================================================================================//
class CommonTracer
{
	typedef mll::geometry::ray3 ray3;
	typedef mll::algebra::point3 point3;
	typedef mll::algebra::vector3 vector3;
	typedef mll::geometry::aabb3 aabb3;

	ray3 m_ray;
	oct_tree *m_tree;
	const oct_tree::storage *m_verts;
	vector3 m_dir;
	int m_pass;
	int m_index;
	int m_indexOrder[8];
	const oct_tree::triangle *m_startTri;
	float m_dist;
	point3 m_hitPoint;
	unsigned int m_passThru;
	const char *m_triDesc;
	float m_limit;

	struct SortPred
	{
		const float *m_x;
		SortPred(const float *x) : m_x(x) {}

		bool operator()(int a, int b) const
		{
			return fabsf(m_x[a]) > fabsf(m_x[b]);
		}
	};

public:
	CommonTracer(const PoLLevelUtils::TraceContext &c, const ray3 &r, unsigned int passThru, float limit)
	:	m_ray(r),
		m_tree(c.m_tree),
		m_verts(&m_tree->get_storage()),
		m_dir(m_ray.direction),
		m_startTri(&m_tree->get_triangles()[0]),
		m_dist(-1.0f),
		m_passThru(passThru),
		m_triDesc(c.m_triDesc),
		m_limit(limit)
	{
		//m_pass = m_tree->m_pass++;

		float x[3] = { m_dir.x, m_dir.y, m_dir.z };
		int index[3] = { 0, 1, 2 };

		std::sort(index,index+3,SortPred(x));

		int ordersc[3][2] = {{oct_tree::s_far,oct_tree::s_near},{oct_tree::s_right,oct_tree::s_left},{oct_tree::s_top,oct_tree::s_bottom}};
		int orders[3][2] = {{0,0},{0,0},{0,0}};

		for(int i = 0; i < 3; ++i)
		{
			orders[i][0] = ordersc[index[i]][0];
			orders[i][1] = ordersc[index[i]][1];
			if(x[index[i]] >= 0.0f) std::swap(orders[i][0],orders[i][1]);
		}

		m_indexOrder[0] = orders[0][0] | orders[1][0] | orders[2][0];
		m_indexOrder[1] = orders[0][0] | orders[1][0] | orders[2][1];
		m_indexOrder[2] = orders[0][0] | orders[1][1] | orders[2][0];
		m_indexOrder[3] = orders[0][0] | orders[1][1] | orders[2][1];
		m_indexOrder[4] = orders[0][1] | orders[1][0] | orders[2][0];
		m_indexOrder[5] = orders[0][1] | orders[1][0] | orders[2][1];
		m_indexOrder[6] = orders[0][1] | orders[1][1] | orders[2][0];
		m_indexOrder[7] = orders[0][1] | orders[1][1] | orders[2][1];
	}

	bool trace()
	{
		if(handleNode(m_tree->get_root(), m_tree->get_root()->m_bound))
		{
			m_hitPoint = m_ray.at(m_dist);
			return true;
		}
		else
		{
			return false;
		}
	}

	const float getDist() const { return m_dist; }
	const mll::algebra::point3 &getHitPoint() const { return m_hitPoint; }
	const unsigned int getTriangleIndex() const { return m_index; }

private:
	bool hitBox(const std::pair<float,float> &isect) const
	{
		return (isect.first != -1.0f || isect.second != -1.0f) && isect.first <= m_limit;
	}

	bool hitTriangle(float t, float dist) const
	{
		return t <= m_limit && (t < dist || dist == -1.0f);
	} 

	bool handleNode(oct_tree::node *node, const aabb3 &box)
	{
		const float bad_ray_isect = mll::algebra::numerical_traits<float>::get_bad_ray_intersection();

		// проверка на попадание в бокс
		if(!hitBox(box.full_intersect(m_ray))) return false; // node->m_bound.full_intersect(m_ray))) return false;

		if(node->has_children())
		{
			for(int i = 0; i < 8; ++i)
			{
				if(oct_tree::node *child = node->m_child[m_indexOrder[i]].get())
				{
					float x = m_indexOrder[i]&1 ? box.min().x : box.max().x;
					float y = m_indexOrder[i]&2 ? box.min().y : box.max().y;
					float z = m_indexOrder[i]&4 ? box.min().z : box.max().z;

					aabb3 cbox(box.center(),mll::algebra::point3(x,y,z));

					if(handleNode(child,cbox)) return true;
				}
			}

			return false;
		}
		else
		{
			return handleLeaf(node,box);
		}
	}

	//bool testInBox(const aabb3 &box, const point3 &pt) const
	//{
	//	aabb3 box2 = box;
	//	const vector3 diag1 = box2.diag();
	//	box2.grow(pt);
	//	return (box2.diag() - diag1).length() < 1e-2f;
	//}

	bool handleLeaf(oct_tree::node *node, const aabb3 &box)
	{
		for(int i = 0; i < node->m_tris.size(); ++i)
		{
			oct_tree::triangle *tri = node->m_tris[i];
			//if(tri->rendered == m_pass) continue;

			int index = tri - m_startTri;
			//tri->rendered = m_pass;

			const unsigned int triDesc = m_triDesc[index];
			if(triDesc && (m_passThru & triDesc))
			{
				continue;
			}

			const mll::algebra::point3 &v1 = (*m_verts)[node->m_tris[i]->v[0]].pos;
			const mll::algebra::point3 &v2 = (*m_verts)[node->m_tris[i]->v[1]].pos;
			const mll::algebra::point3 &v3 = (*m_verts)[node->m_tris[i]->v[2]].pos;
			float t,u,v;

			if(PCRayTriangle(m_ray.origin,m_ray.direction,v1,v2,v3,&t,&u,&v))
			{
				if(hitTriangle(t,m_dist) && box.inside(m_ray.at(t))) // testInBox(box,m_ray.at(t)))
				{
					m_dist = t;
					m_index = node->m_tris[i] - m_startTri;
				}
			}
		}

		return m_dist != -1.0f;
	}
};

//=====================================================================================//
//                                bool PCRayTriangle()                                 //
//=====================================================================================//
bool PCRayTriangle2(const mll::geometry::ray3 &ray,
					const PoL::IntersectionTriangle *tria,
					float *t)
{
	mll::algebra::vector3 tvec, pvec, qvec;
	float det,inv_det;

	/* begin calculating determinant - also used to calculate U parameter */
	pvec = ray.direction.cross(tria->m_edge2);
	/* if determinant is near zero, ray lies in plane of triangle */
	det = tria->m_edge1.dot(pvec);

	if (fabsf(det) < epsilon)
		return false;
	inv_det = 1.0f / det;

	/* calculate distance from vert0 to ray origin */
	tvec = ray.origin - tria->m_v1;

	/* calculate U parameter and test bounds */
	const float u = tvec.dot(pvec) * inv_det;
	if (u < 0.0f || u > 1.0f)
		return false;

	/* prepare to test V parameter */
	qvec = tvec.cross(tria->m_edge1);

	/* calculate V parameter and test bounds */
	const float v = ray.direction.dot(qvec) * inv_det;
	if (v < 0.0 || u + v > 1.0)
		return false;

	/* calculate t, ray intersects triangle */
	*t = tria->m_edge2.dot(qvec) * inv_det;
	return *t >= 0.0f;
	//return true;
}

//=====================================================================================//
//                                 class CommonTracer2                                 //
//=====================================================================================//
class CommonTracer2
{
	typedef mll::geometry::ray3 ray3;
	typedef mll::algebra::point3 point3;
	typedef mll::algebra::vector3 vector3;
	typedef mll::geometry::aabb3 aabb3;

	ray3 m_ray;
	vector3 m_dir;
	int m_pass;
	int m_index;
	int m_indexOrder[8];
	float m_dist;
	point3 m_hitPoint;
	unsigned int m_passThru;
	float m_limit;
	const PoL::IntersectionCache *m_cache;

	struct SortPred
	{
		const float *m_x;
		SortPred(const float *x) : m_x(x) {}

		bool operator()(int a, int b) const
		{
			return fabsf(m_x[a]) > fabsf(m_x[b]);
		}
	};

public:
	CommonTracer2(const PoL::IntersectionCache *c, const ray3 &r, unsigned int passThru, float limit)
	:	m_ray(r),
		m_dir(m_ray.direction),
		m_dist(-1.0f),
		m_passThru(passThru),
		m_limit(limit),
		m_cache(c)
	{
		//m_pass = m_tree->m_pass++;

		float x[3] = { m_dir.x, m_dir.y, m_dir.z };
		int index[3] = { 0, 1, 2 };

		std::sort(index,index+3,SortPred(x));

		int ordersc[3][2] = {{oct_tree::s_far,oct_tree::s_near},{oct_tree::s_right,oct_tree::s_left},{oct_tree::s_top,oct_tree::s_bottom}};
		int orders[3][2] = {{0,0},{0,0},{0,0}};

		for(int i = 0; i < 3; ++i)
		{
			orders[i][0] = ordersc[index[i]][0];
			orders[i][1] = ordersc[index[i]][1];
			if(x[index[i]] >= 0.0f) std::swap(orders[i][0],orders[i][1]);
		}

		m_indexOrder[0] = orders[0][0] | orders[1][0] | orders[2][0];
		m_indexOrder[1] = orders[0][0] | orders[1][0] | orders[2][1];
		m_indexOrder[2] = orders[0][0] | orders[1][1] | orders[2][0];
		m_indexOrder[3] = orders[0][0] | orders[1][1] | orders[2][1];
		m_indexOrder[4] = orders[0][1] | orders[1][0] | orders[2][0];
		m_indexOrder[5] = orders[0][1] | orders[1][0] | orders[2][1];
		m_indexOrder[6] = orders[0][1] | orders[1][1] | orders[2][0];
		m_indexOrder[7] = orders[0][1] | orders[1][1] | orders[2][1];
	}

	bool trace()
	{
		if(handleNode(m_cache->getIntersectionNodes()))
		{
			m_hitPoint = m_ray.at(m_dist);
			return true;
		}
		else
		{
			return false;
		}
	}

	const float getDist() const { return m_dist; }
	const mll::algebra::point3 &getHitPoint() const { return m_hitPoint; }
	const unsigned int getTriangleIndex() const { return m_index; }

private:
	bool hitBox(const std::pair<float,float> &isect) const
	{
		return (isect.first != -1.0f || isect.second != -1.0f) && (isect.first <= m_limit || isect.second == -1.0f);
	}

	bool hitTriangle(float t, float dist) const
	{
		return t <= m_limit && (t < dist || dist == -1.0f);
	} 

	bool handleNode(const PoL::IntersectionNode *node)
	{
		const float bad_ray_isect = mll::algebra::numerical_traits<float>::get_bad_ray_intersection();

		// проверка на попадание в бокс
		std::pair<float,float> hit = node->m_box.full_intersect(m_ray);
		if(hit.first != -1.0f)
		{
			point3 p1 = m_ray.at(hit.first);
		}

		if(!hitBox(hit)) return false; // node->m_bound.full_intersect(m_ray))) return false;

		if(node->m_children)
		{
			for(int i = 0; i < 8; ++i)
			{
				const int ci = m_indexOrder[i];
				if(node->m_childrenMask & (1 << ci))
				{
					const int cidx = node->getChildFromIndex(ci);
					const PoL::IntersectionNode *child = node->m_children + cidx;
					if(handleNode(child)) return true;
				}
			}

			return false;
		}
		else
		{
			return handleLeaf(node);
		}
	}

	//bool testInBox(const aabb3 &box, const point3 &pt) const
	//{
	//	aabb3 box2 = box;
	//	const vector3 diag1 = box2.diag();
	//	box2.grow(pt);
	//	return (box2.diag() - diag1).length() < 1e-2f;
	//}

	bool handleLeaf(const PoL::IntersectionNode *node)
	{
		for(int i = 0; i < node->m_triangleCount; ++i)
		{
			const PoL::IntersectionTriangle *tri = m_cache->getIntersectionTriangles() + node->m_firstTriangle + i;
			//if(tri->rendered == m_pass) continue;

			//int index = tri - m_startTri;
			//tri->rendered = m_pass;

			const unsigned int triDesc = tri->m_transMask; //node->m_triangles[i]
			if(triDesc && (m_passThru & triDesc))
			{
				continue;
			}

			float t;
			if(PCRayTriangle2(m_ray,tri,&t))
			{
				if(hitTriangle(t,m_dist) && node->m_box.inside(m_ray.at(t))) // testInBox(box,m_ray.at(t)))
				{
					m_dist = t;
					m_index = node->m_firstTriangle + i;
				}
			}

			//const mll::algebra::point3 &v1 = (*m_verts)[node->m_tris[i]->v[0]].pos;
			//const mll::algebra::point3 &v2 = (*m_verts)[node->m_tris[i]->v[1]].pos;
			//const mll::algebra::point3 &v3 = (*m_verts)[node->m_tris[i]->v[2]].pos;
			//float t,u,v;

			//if(PCRayTriangle(m_ray.origin,m_ray.direction,v1,v2,v3,&t,&u,&v))
			//{
			//	if(hitTriangle(t,m_dist) && box.inside(m_ray.at(t))) // testInBox(box,m_ray.at(t)))
			//	{
			//		m_dist = t;
			//		m_index = node->m_tris[i] - m_startTri;
			//	}
			//}
		}

		return m_dist != -1.0f;
	}
};

//=====================================================================================//
//                          IntersectResult intersectPoints()                          //
//=====================================================================================//
IntersectResult intersectPoints(const point3 *pts, unsigned int cnt, const PoLPlane *planes)
{
	bool outs = false;
	for(int j = 0; j < 6; ++j)
	{
		bool outside = true;
		for(int i = 0; i < cnt; ++i)
		{
			if(planes[j].m_normal.Dot(pts[i])+planes[j].m_d <= 0.0f)
			{
				outside = false;
				break;
			}
		}
		if(outside) { outs = true; break; }
	}

	if(outs) return irOutside;

	return irIntersects;
}

//=====================================================================================//
//                         IntersectResult intersectAabbObb()                          //
//=====================================================================================//
IntersectResult intersectAabbObb(const mll::geometry::aabb3 &box, const PoLPlane *planes)
{
	point3 pts[8] = {point3(box[0]),point3(box[1]),point3(box[2]),point3(box[3]),
					 point3(box[4]),point3(box[5]),point3(box[6]),point3(box[7])};

	return intersectPoints(pts,8,planes);
}

//=====================================================================================//
//                       IntersectResult intersectTriangleObb()                        //
//=====================================================================================//
IntersectResult intersectTriangleObb(const point3 *pts, const PoLPlane *planes)
{
	return intersectPoints(pts,3,planes);
}

//=====================================================================================//
//                             inline bool cullTriangle()                              //
//=====================================================================================//
// Ќаходитс€ ли треугольник в параллелепипеде тени
inline bool cullTriangle(const point3 *triangle, const point3 &x, const point3 &y,
						 const point3 &z, const point3 *originz, float *Rad)
{
	float dot1, dot2, dot3, tdot;

	tdot = x.Dot (*originz);
	dot1 = x.Dot (triangle[0]) - tdot;
	dot2 = x.Dot (triangle[1]) - tdot;
	dot3 = x.Dot (triangle[2]) - tdot;

	if ((dot1 < 0) && (dot2 < 0) && (dot3 < 0)) return true;
	if ((-dot1 + Rad[0] < 0) && (-dot2 + Rad[0] < 0) && (-dot3 + Rad[0] < 0)) return true;

	tdot = y.Dot (*originz);
	dot1 = y.Dot (triangle[0]) - tdot;
	dot2 = y.Dot (triangle[1]) - tdot;
	dot3 = y.Dot (triangle[2]) - tdot;

	if ((dot1 < 0) && (dot2 < 0) && (dot3 < 0)) return true;
	if ((-dot1 + Rad[1] < 0) && (-dot2 + Rad[1] < 0) && (-dot3 + Rad[1] < 0)) return true;

	tdot = z.Dot (*originz);
	dot1 = z.Dot (triangle[0]) - tdot;
	dot2 = z.Dot (triangle[1]) - tdot;
	dot3 = z.Dot (triangle[2]) - tdot;

	if ((dot1 < 0) && (dot2 < 0) && (dot3 < 0)) return true;
	if ((-dot1 + Rad[2] < 0) && (-dot2 + Rad[2] < 0) && (-dot3 + Rad[2] < 0)) return true;

	return false;
}

//=====================================================================================//
//                                inline void polyAdd()                                //
//=====================================================================================//
inline void polyAdd(gPoint *outPolygon, unsigned &outVertices, bool lastVertex, float x, float y)
{
	const float epsilon = 1e-6f;
	if (outVertices == 0 ||
		(
			fabs(x - outPolygon[outVertices-1].pX) > epsilon ||
			fabs(y - outPolygon[outVertices-1].pY) > epsilon
		) && (
			!lastVertex ||
			fabs (x - outPolygon[0].pX) > epsilon ||
			fabs (y - outPolygon[0].pY) > epsilon
		)
	) 
	{
		outPolygon[outVertices].pX = x;
		outPolygon[outVertices].pY = y;
		outVertices++;
	}
}

//=====================================================================================//
//                                   void cutPoly()                                    //
//=====================================================================================//
void cutPoly(gPoly &triangle, gPoly &result, float minx, float miny, float maxx, float maxy)
{
	gPoint outPoly1[gPoly::MAXVERTS], outPoly2[gPoly::MAXVERTS];
	gPoint *inPolygon = triangle.verts;
	gPoint *outPolygon = outPoly1;
	unsigned inVertices = triangle.vCount;
	unsigned outVertices = 0;

//	const float minx = shadow->minx;
//	const float maxx = shadow->maxx;
//	const float miny = shadow->miny;
//	const float maxy = shadow->maxy;

	do {
		// edge1 - лева€ вертикальна€ граница
		// dx = 0; dy = 1;
		{
			outVertices = 0;
			bool lastVertex = false;
			float px = inPolygon[0].pX;
			float py = inPolygon[0].pY;
			bool prevVertexInside = (px > minx);
			unsigned intersectionCount = 0;

			for (int i = 1; i <= inVertices; ++i)
			{
				float cx, cy; // - текуща€ вершина треугольника

				// инициализаци€ cx, cy
				if (i < inVertices)
				{
					cx = inPolygon[i].pX;
					cy = inPolygon[i].pY;
				} else
				{
					cx = inPolygon->pX;
					cy = inPolygon->pY;
					lastVertex = true;
				}

				// if starting vertex is visible then put it into the output array
				if (prevVertexInside) polyAdd(outPolygon, outVertices, lastVertex, px, py);

				bool curVertexInside = (cx > minx);

				// if vertices are on different sides of the edge then look where we're intersecting
				if (prevVertexInside != curVertexInside)
				{
					float denominator = cx - px;

					if (denominator != 0.0)
					{
						float t = (minx - px)/denominator;
						float tx, ty;

						if (t <= 0.0)
						{
							tx = px;
							ty = py;
						} else if (t >= 1.0)
						{
							tx = cx;
							ty = cy;
						} else
						{
							tx = px+t*(cx-px);
							ty = py+t*(cy-py);
						}

						polyAdd (outPolygon, outVertices, lastVertex, tx, ty);
					}

					if (++intersectionCount >= 2)
					{
						if (fabs(denominator) < 1) intersectionCount = 0; 
						else
						{
							// drop out, after adding all vertices left in input polygon
							if (curVertexInside)
							{
								memcpy (outPolygon + outVertices, inPolygon + i, (inVertices-i)*sizeof(gPoint));
								outVertices += inVertices - i;
							}

							break;
						}
					}
				}

				px = cx;
				py = cy;

				prevVertexInside = curVertexInside;
			}

			// if polygon is wiped out, break

			if (outVertices < 3) break;

			// switch input/output polygons
			inVertices = outVertices;
			inPolygon = outPolygon;
			if (outPolygon == outPoly2) outPolygon = outPoly1; else outPolygon = outPoly2;
		}

		// edge2 - верхн€€ горизонтальна€ граница
		// dx = 1; dy = 0;
		{
			outVertices = 0;
			bool lastVertex = false;
			float px = inPolygon[0].pX;
			float py = inPolygon[0].pY;
			bool prevVertexInside = (py < maxy);
			unsigned intersectionCount = 0;

			for (int i = 1; i <= inVertices; ++i)
			{
				float cx, cy; // - текуща€ вершина треугольника

				// инициализаци€ cx, cy
				if (i < inVertices)
				{
					cx = inPolygon[i].pX;
					cy = inPolygon[i].pY;
				} else
				{
					cx = inPolygon->pX;
					cy = inPolygon->pY;
					lastVertex = true;
				}

				// if starting vertex is visible then put it into the output array
				if (prevVertexInside) polyAdd (outPolygon, outVertices, lastVertex, px, py);

				bool curVertexInside = (cy < maxy);

				// if vertices are on different sides of the edge then look where we're intersecting
				if (prevVertexInside != curVertexInside)
				{
					float denominator = py - cy;

					if (denominator != 0.0)
					{
						float t = (py - maxy)/denominator;
						float tx, ty;

						if (t <= 0.0)
						{
							tx = px;
							ty = py;
						} else if (t >= 1.0)
						{
							tx = cx;
							ty = cy;
						} else
						{
							tx = px+t*(cx-px);
							ty = py+t*(cy-py);
						}

						polyAdd (outPolygon, outVertices, lastVertex, tx, ty);
					}

					if (++intersectionCount >= 2)
					{
						if (fabs(denominator) < 1) intersectionCount = 0; 
						else
						{
							// drop out, after adding all vertices left in input polygon
							if (curVertexInside)
							{
								memcpy (outPolygon + outVertices, inPolygon + i, (inVertices-i)*sizeof(gPoint));
								outVertices += inVertices - i;
							}

							break;
						}
					}
				}

				px = cx;
				py = cy;

				prevVertexInside = curVertexInside;
			}

			// if polygon is wiped out, break

			if (outVertices < 3) break;

			// switch input/output polygons
			inVertices = outVertices;
			inPolygon = outPolygon;
			if (outPolygon == outPoly2) outPolygon = outPoly1; else outPolygon = outPoly2;
		}

		// edge3 - права€ вертикальна€ граница
		// dx = 0; dy = -1;
		{
			outVertices = 0;
			bool lastVertex = false;
			float px = inPolygon[0].pX;
			float py = inPolygon[0].pY;
			bool prevVertexInside = (px < maxx);
			unsigned intersectionCount = 0;

			for (int i = 1; i <= inVertices; ++i)
			{
				float cx, cy; // - текуща€ вершина треугольника

				// инициализаци€ cx, cy
				if (i < inVertices)
				{
					cx = inPolygon[i].pX;
					cy = inPolygon[i].pY;
				} else
				{
					cx = inPolygon->pX;
					cy = inPolygon->pY;
					lastVertex = true;
				}

				// if starting vertex is visible then put it into the output array
				if (prevVertexInside) polyAdd (outPolygon, outVertices, lastVertex, px, py);

				bool curVertexInside = (cx < maxx);

				// if vertices are on different sides of the edge then look where we're intersecting
				if (prevVertexInside != curVertexInside)
				{
					float denominator = px - cx;

					if (denominator != 0.0)
					{
						float t = (px - maxx)/denominator;
						float tx, ty;

						if (t <= 0.0)
						{
							tx = px;
							ty = py;
						} else if (t >= 1.0)
						{
							tx = cx;
							ty = cy;
						} else
						{
							tx = px+t*(cx-px);
							ty = py+t*(cy-py);
						}

						polyAdd (outPolygon, outVertices, lastVertex, tx, ty);
					}

					if (++intersectionCount >= 2)
					{
						if (fabs(denominator) < 1) intersectionCount = 0; 
						else
						{
							// drop out, after adding all vertices left in input polygon
							if (curVertexInside)
							{
								memcpy (outPolygon + outVertices, inPolygon + i, (inVertices-i)*sizeof(gPoint));
								outVertices += inVertices - i;
							}

							break;
						}
					}
				}

				px = cx;
				py = cy;

				prevVertexInside = curVertexInside;
			}

			// if polygon is wiped out, break

			if (outVertices < 3) break;

			// switch input/output polygons
			inVertices = outVertices;
			inPolygon = outPolygon;
			if (outPolygon == outPoly2) outPolygon = outPoly1; else outPolygon = outPoly2;
		}

		// edge4 - нижн€€ горизонтальна€ граница
		// dx = -1; dy = 0;
		{
			outVertices = 0;
			bool lastVertex = false;
			float px = inPolygon[0].pX;
			float py = inPolygon[0].pY;
			bool prevVertexInside = (py > miny);
			unsigned intersectionCount = 0;

			for (int i = 1; i <= inVertices; ++i)
			{
				float cx, cy; // - текуща€ вершина треугольника

				// инициализаци€ cx, cy
				if (i < inVertices)
				{
					cx = inPolygon[i].pX;
					cy = inPolygon[i].pY;
				} else
				{
					cx = inPolygon->pX;
					cy = inPolygon->pY;
					lastVertex = true;
				}

				// if starting vertex is visible then put it into the output array
				if (prevVertexInside) polyAdd (outPolygon, outVertices, lastVertex, px, py);

				bool curVertexInside = (cy > miny);

				// if vertices are on different sides of the edge then look where we're intersecting
				if (prevVertexInside != curVertexInside)
				{
					float denominator = cy - py;

					if (denominator != 0.0)
					{
						float t = (miny - py)/denominator;
						float tx, ty;

						if (t <= 0.0)
						{
							tx = px;
							ty = py;
						} else if (t >= 1.0)
						{
							tx = cx;
							ty = cy;
						} else
						{
							tx = px+t*(cx-px);
							ty = py+t*(cy-py);
						}

						polyAdd (outPolygon, outVertices, lastVertex, tx, ty);
					}

					if (++intersectionCount >= 2)
					{
						if (fabs(denominator) < 1) intersectionCount = 0; 
						else
						{
							// drop out, after adding all vertices left in input polygon
							if (curVertexInside)
							{
								memcpy (outPolygon + outVertices, inPolygon + i, (inVertices-i)*sizeof(gPoint));
								outVertices += inVertices - i;
							}

							break;
						}
					}
				}

				px = cx;
				py = cy;

				prevVertexInside = curVertexInside;
			}
		}
	} while (0);

	// вывод результата
	if (outVertices < 3) result.vCount = 0;
	else
	{
		result.vCount = outVertices;
		memcpy (result.verts, outPolygon, outVertices*sizeof(gPoint));
	}
}

//=====================================================================================//
//                          inline void projPoint2Surface ()                           //
//=====================================================================================//
inline void projPoint2Surface (point3 &pt, point3 &lt)//, point3 &n, float D, float invltDotn)
{
	const float DIST = 0.01f;
	pt -= lt*DIST;//*(D + n.Dot(pt))*invltDotn;
	//pt += DIST*n;
}

//=====================================================================================//
//                             class ShdTriangleCollector                              //
//=====================================================================================//
class ShdTriangleCollector
{
	friend struct Functor;

	struct Functor
	{
		ShdTriangleCollector *m_coll;
		Functor(ShdTriangleCollector *c) : m_coll(c) {}

		bool operator()(oct_tree::node *node) const { return m_coll->collect(node); }
	};

	D3DCOLOR m_intensity;

	// “очка прив€зки
	point3 m_pos;

	point3 m_bx, m_by;
	point3 m_tpos;
	point3 m_originz[8];
	float m_rad;
	float m_radz[3];
	float m_invDeltax, m_invDeltay;
	PoLPlane m_planes[6];

	oct_tree *m_tree;
	Shadows::BaseShadow *m_shd;
	Primi *m_prim;

	unsigned int m_pass;
	const oct_tree::storage *m_verts;

public:
	ShdTriangleCollector(oct_tree *t, Shadows::BaseShadow *shd, Primi *prim, float rad)
	:	m_shd(shd),
		m_prim(prim),
		m_rad(rad),
		m_tree(t),
		m_verts(&m_tree->get_storage())
	{
		m_pass = t->m_pass++;

		m_prim->Verts.clear();
		point3 bind = *m_shd->bindPoint;// - shadow->lt*10.0f;

		if(m_shd->parentWorld)
		{
			// Ќайдем мировые координаты точки прив€зки
			m_pos.x = bind.x * m_shd->parentWorld->_11 +
					bind.y * m_shd->parentWorld->_21 + 
					bind.z * m_shd->parentWorld->_31 + 
										   m_shd->parentWorld->_41;
			m_pos.y = bind.x * m_shd->parentWorld->_12 +
					bind.y * m_shd->parentWorld->_22 + 
					bind.z * m_shd->parentWorld->_32 + 
										   m_shd->parentWorld->_42;
			m_pos.z = bind.x * m_shd->parentWorld->_13 +
					bind.y * m_shd->parentWorld->_23 +
					bind.z * m_shd->parentWorld->_33 +
										   m_shd->parentWorld->_43;
		} else 
		{
			m_pos.x = bind.x;
			m_pos.y = bind.y;
			m_pos.z = bind.z;
		}

		m_intensity = int(float(0xFF)*Shadows::ShadowUtility::getLightIntensity(m_pos)) << 24;

		m_bx = point3(m_shd->axis._11, m_shd->axis._21, m_shd->axis._31);
		m_by = point3(m_shd->axis._12, m_shd->axis._22, m_shd->axis._32);

		m_tpos = m_shd->lt * m_shd->lt.Dot(m_pos);

		m_originz[0] = m_shd->minx*m_bx + m_shd->miny*m_by + m_tpos;
		m_originz[1] = m_shd->minx*m_bx + m_shd->maxy*m_by + m_tpos;
		m_originz[2] = m_shd->maxx*m_bx + m_shd->maxy*m_by + m_tpos;
		m_originz[3] = m_shd->maxx*m_bx + m_shd->miny*m_by + m_tpos;

		m_originz[4] = m_shd->minx*m_bx + m_shd->miny*m_by + m_tpos + m_shd->lt*m_rad;
		m_originz[5] = m_shd->minx*m_bx + m_shd->maxy*m_by + m_tpos + m_shd->lt*m_rad;
		m_originz[6] = m_shd->maxx*m_bx + m_shd->maxy*m_by + m_tpos + m_shd->lt*m_rad;
		m_originz[7] = m_shd->maxx*m_bx + m_shd->miny*m_by + m_tpos + m_shd->lt*m_rad;

		m_radz[0] = m_shd->maxx - m_shd->minx;
		m_radz[1] = m_shd->maxy - m_shd->miny;
		m_radz[2] = m_rad;

		m_invDeltax = 1.0f/(m_shd->maxx - m_shd->minx);
		m_invDeltay = 1.0f/(m_shd->maxy - m_shd->miny);

		m_planes[0] = PoLPlane(m_originz[0],m_originz[4]);
		m_planes[1] = PoLPlane(m_originz[4],m_originz[0]);
		m_planes[2] = PoLPlane(m_originz[0],m_originz[1]);
		m_planes[3] = PoLPlane(m_originz[1],m_originz[0]);
		m_planes[4] = PoLPlane(m_originz[0],m_originz[3]);
		m_planes[5] = PoLPlane(m_originz[3],m_originz[0]);
	}

	void collect()
	{
		m_tree->traverse2(Functor(this));
	}

private:
	Primi::FVFVertex makeVertex(const point3 &pt) const
	{
		Primi::FVFVertex vert;
		const float u = (pt-m_originz[0]).Dot(m_bx)*m_invDeltax;
		const float v = (pt-m_originz[0]).Dot(m_by)*m_invDeltay;
		//vert.pos = pt;
		vert.pos = pt - m_shd->lt*1e-2f;
		vert.norm = point3(0.0f,0.0f,1.0f);
		vert.uv = texcoord(u,1.0f-v);
		vert.diffuse = m_intensity;
		return vert;
	}

	void addTriangles(oct_tree::node *node)
	{
		for(oct_tree::IdxPolyList::iterator i = node->m_tris.begin(); i != node->m_tris.end(); ++i)
		{
			if((*i)->rendered == m_pass) continue;
			(*i)->rendered = m_pass;

			point3 pts[3] = {
								point3((*m_verts)[(*i)->v[0]].pos),
								point3((*m_verts)[(*i)->v[1]].pos),
								point3((*m_verts)[(*i)->v[2]].pos)};
			if(intersectTriangleObb(pts,m_planes) == irOutside) continue;

			point3 ptsr[64];
			int pcount = sliceTriangle(pts,ptsr,m_planes);
			if(!pcount) continue;

			Primi::FVFVertex vert0 = makeVertex(ptsr[0]);
			Primi::FVFVertex vertp = makeVertex(ptsr[1]);

			for(int j = 2; j < pcount; ++j)
			{
				Primi::FVFVertex vert = makeVertex(ptsr[j]);
				m_prim->Verts.push_back(vert0);
				m_prim->Verts.push_back(vertp);
				m_prim->Verts.push_back(vert);
				vertp = vert;
			}
		}
	}

	bool collect(oct_tree::node *node)
	{
		bool result;

 		switch(intersectAabbObb(node->m_bound,m_planes))
		{
		case irInside:
			addTriangles(node);
		case irOutside:
			result = false;
			break;
		case irIntersects:
			if(!node->m_has_children)
			{
				addTriangles(node);
			}
			result = node->m_has_children;
			break;
		}

		return result;
	}
};

}

using namespace PoLLevelUtilsDetail;

//=====================================================================================//
//                           bool PoLLevelUtils::TraceRay()                            //
//=====================================================================================//
bool PoLLevelUtils::TraceRay(const TraceContext &c, const ray3 &r,
							 unsigned int passThru, TraceResult *tr)
{
	const float limit = (tr->m_dist == -1.0f ? 1e+10f : tr->m_dist);
	CommonTracer tracer(c,r,passThru,limit);

	if(tracer.trace())
	{
		tr->m_dist = tracer.getDist();
		tr->m_pos = tracer.getHitPoint();
		tr->m_norm = c.m_normals[tracer.getTriangleIndex()];
		return true;
	}

	return false;
}

//=====================================================================================//
//                         bool PoLLevelUtils::TraceSegment()                          //
//=====================================================================================//
bool PoLLevelUtils::TraceSegment(const TraceContext &c, const ray3 &r,
								 unsigned int passThru, TraceResult *tr)
{
	const float limit = (tr->m_dist == -1.0f ? 1.0f : tr->m_dist);
	CommonTracer tracer(c,r,passThru,limit);

	if(tracer.trace())
	{
		tr->m_dist = tracer.getDist();
		tr->m_pos = tracer.getHitPoint();
		tr->m_norm = c.m_normals[tracer.getTriangleIndex()];
		return true;
	}

	return false;
}

//=====================================================================================//
//                           bool PoLLevelUtils::TraceRay()                            //
//=====================================================================================//
bool PoLLevelUtils::TraceRay2(const PoL::IntersectionCache *c, const ray3 &r,
							 unsigned int passThru, TraceResult *tr)
{
	const float limit = (tr->m_dist == -1.0f ? 1e+10f : tr->m_dist);
	CommonTracer2 tracer(c,r,passThru,limit);

	if(tracer.trace())
	{
		const PoL::IntersectionTriangle *tri = c->getIntersectionTriangles() + tracer.getTriangleIndex();
		tr->m_dist = tracer.getDist();
		tr->m_pos = tracer.getHitPoint();
		tr->m_norm = tri->m_edge1.cross(tri->m_edge2);
		return true;
	}

	return false;
}

//=====================================================================================//
//                         bool PoLLevelUtils::TraceSegment()                          //
//=====================================================================================//
bool PoLLevelUtils::TraceSegment2(const PoL::IntersectionCache *c, const ray3 &r,
								 unsigned int passThru, TraceResult *tr)
{
	const float limit = (tr->m_dist == -1.0f ? 1.0f : tr->m_dist);
	CommonTracer2 tracer(c,r,passThru,limit);

	if(tracer.trace())
	{
		const PoL::IntersectionTriangle *tri = c->getIntersectionTriangles() + tracer.getTriangleIndex();
		tr->m_dist = tracer.getDist();
		tr->m_pos = tracer.getHitPoint();
		tr->m_norm = tri->m_edge1.cross(tri->m_edge2);
		return true;
	}

	return false;
}

//=====================================================================================//
//                   void PoLLevelUtils::CollectTrianglesInSphere()                    //
//=====================================================================================//
void PoLLevelUtils::CollectTrianglesInSphere(oct_tree *t, Primi &prim, unsigned int col,
											 const mll::algebra::point3 &origin, float rad)
{
	TriangleCollector coll(t,&prim,col,origin,rad);
	coll.collect();
}


//=====================================================================================//
//                                   struct SectInit                                   //
//=====================================================================================//
struct SectInit
{
	CRITICAL_SECTION g_csect;

	SectInit() { InitializeCriticalSection(&g_csect); }
	~SectInit() { DeleteCriticalSection(&g_csect); }
};

//=====================================================================================//
//                   void PoLLevelUtils::CollectTrianglesForShadow()                   //
//=====================================================================================//
void PoLLevelUtils::CollectTrianglesForShadow(oct_tree *t, Shadows::BaseShadow &shd, Primi &prim, float rad)
{
	static SectInit sinit;

	EnterCriticalSection(&sinit.g_csect);

	ShdTriangleCollector coll(t,&shd,&prim,rad);
	coll.collect();

	LeaveCriticalSection(&sinit.g_csect);
}
