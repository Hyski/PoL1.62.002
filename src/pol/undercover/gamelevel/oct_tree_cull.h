//=====================================================================================//
//                                                                                     //
//                                    Alfa Project                                     //
//                                                                                     //
//                             Copyright by MiST land 2003                             //
//                                                                                     //
//   -------------------------------------------------------------------------------   //
//   Author: Leo                                                                       //
//   Date:   26.11.2003                                                                //
//                                                                                     //
//=====================================================================================//
#pragma once

//=====================================================================================//
//                                 class oct_tree_cull                                 //
//=====================================================================================//
class oct_tree_cull
{
private:
	enum coll_type_t
	{
		inside,
		outside,
		intersec
	};
	mll::algebra::point3 m_points[8];
	mll::geometry::plane m_planes[6];

public:
	oct_tree_cull(const mll::geometry::obb3& obb)
	{
		for (unsigned i=0;i<8;++i) m_points[i] = obb[i];

		m_planes[0] = mll::geometry::plane( m_points[0], m_points[1], m_points[2] ); // Near
		m_planes[1] = mll::geometry::plane( m_points[6], m_points[7], m_points[5] ); // Far
		m_planes[2] = mll::geometry::plane( m_points[2], m_points[6], m_points[4] ); // Left
		m_planes[3] = mll::geometry::plane( m_points[7], m_points[3], m_points[5] ); // Right
		m_planes[4] = mll::geometry::plane( m_points[2], m_points[3], m_points[6] ); // Top
		m_planes[5] = mll::geometry::plane( m_points[1], m_points[0], m_points[4] ); // Bottom
	}

	~oct_tree_cull() {}

public:
	void cut(std::vector<mll::algebra::point3> &pnt,std::vector<unsigned short> &idx);

private:
	coll_type_t check(const mll::geometry::aabb3& aabb);
	void cut(std::vector<mll::algebra::point3> edges);
	bool EdgeIntersectsFace( mll::algebra::point3 *pEdges, mll::algebra::point3 *pFacePoints, mll::geometry::plane &Plane );
};
