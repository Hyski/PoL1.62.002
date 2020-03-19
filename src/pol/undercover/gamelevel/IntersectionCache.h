#pragma once

class oct_tree;

namespace PoL
{

//=====================================================================================//
//                             struct IntersectionTriangle                             //
//=====================================================================================//
struct IntersectionTriangle
{
	typedef mll::algebra::point3 point3;
	typedef mll::algebra::vector3 vector3;

	point3 m_v1;				///< ������� ������������
	vector3 m_edge1, m_edge2;	///< ����� ������������

	unsigned char m_transMask;	///< ����� ������������ ������������
};

//=====================================================================================//
//                               struct IntersectionNode                               //
//=====================================================================================//
struct IntersectionNode
{
	typedef mll::geometry::aabb3 aabb3;
	
	unsigned char m_parentIndex;	///< ������ ������������ ������������� ����
	aabb3 m_box;					///< �������������� ���� ��� ����

	unsigned char m_childrenMask;	///< ����� �� ������������� �������� �����
	IntersectionNode *m_children;	///< �������� ����

	int m_triangleCount;			///< ���������� ������������� ����
	int m_firstTriangle;			///< ��������� �� ������ ����������� ����

	/// ���������� ���������� �������� �����
	int getChildrenCount() const
	{
		return getBitCount(m_childrenMask);
	}

	/// ������������ ������ ������� � ���������
	int getChildFromIndex(int n) const
	{
		return getBitCount(m_childrenMask & ~(0xFF << n));
	}

	/// ������������ ������ ������� � ���������
	int getIndexFromChild(int n) const
	{
		assert( n < getChildrenCount() );
		int i, mask = 0xFF;
		for(i = 0; getBitCount(m_childrenMask & ~mask) <= n; ++i, mask <<= 1);
		return i-1;
	}

private:
	static int getBitCount(int c)
	{
		static const char bitcounttbl[256] =
		{
			0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
			1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
			1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
			2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
			1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
			2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
			2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
			3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8
		};
		return bitcounttbl[c];
	}
};

//=====================================================================================//
//                               class IntersectionCache                               //
//=====================================================================================//
class IntersectionCache
{
	typedef std::vector<IntersectionTriangle> Trias_t;
	typedef std::vector<IntersectionNode> Nodes_t;

	Trias_t m_trias;
	Nodes_t m_nodes;

public:
	IntersectionCache(oct_tree *, const char *triDesc);

	/// ���������� ���� ��� �����������
	const IntersectionNode *getIntersectionNodes() const { return &m_nodes[0]; }
	/// ���������� ������������ ��� ���������� �����������
	const IntersectionTriangle *getIntersectionTriangles() const { return &m_trias[0]; }
};

}
