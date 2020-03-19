/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ��������� ������
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__GRAPH_HEX_GRID_H__)
#define __GRAPH_HEX_GRID_H__

#include "../common/graphpipe/SimpleTexturedObject.h"
class GraphPipe;

//////////////////////////////////////////////////////////////////////////////////
//
// ����� ��� ��������� ������
//
//////////////////////////////////////////////////////////////////////////////////
class GraphHexGrid
{
public:
	// ���� �������������� ������
	enum hex_type
	{
		joint = 0,
		front = 1,
		path = 2,
		land = 3
	};
	// ���������� ����� ����� ��� ���������
	void SetHexes(const hex_type type, const pnt_vec_t& vec);
	// ���������� ���� ��������� ������
	void Show(const hex_type type, const bool flag);
	// ���������� �����
	void Draw();
	// �����������
	GraphHexGrid();
private:

	// ����������, ���������� ���������� ��� ������ � ������ ������������� ����
	struct hex_info_t
	{
		bool visible;
		// ������, ������� ����� ������������ �������
		TexObject tex_object;
		// ��� �������
		std::string shader;
		// ��������� ���������� ���������
		float texcoord_factor;
	};

	// ������ ����������, ���������� ���������� ��� ������ � �������
	hex_info_t hex_info[4];
	// ������ �� 6 ����� ��� ����������� ������ �����
	point3 hex_vert[6];
	// ������, �������� ������� �������������, �������� � �����
	typedef std::vector<int> IndexVector;
	// ������, �������� ������� ���� �������������
	typedef std::vector<point3> VertVector;
	typedef VertVector::iterator VertVectorIterator;

	// ��������� ��� ����������� ������� ��������� ��� �����
	struct pntcmp
	{
		bool operator ()(const point3& p1, const point3& p2)
		{
			unsigned int s1 = static_cast<int>(p1.x*10.0);
			s1 = s1 << 10;
			s1 += static_cast<int>(p1.y*10.0);
			s1 = s1 << 10;
			s1 += static_cast<int>(p1.z*2.0);
			unsigned int s2 = static_cast<int>(p2.x*10.0);
			s2 = s2 << 10;
			s2 += static_cast<int>(p2.y*10.0);
			s2 = s2 << 10;
			s2 += static_cast<int>(p2.z*2.0);

			return s1 < s2;

		}
	};

	// ����� ��� �������� ����� � ��������
	typedef std::map<point3, int, pntcmp> VertMap;
	// �������� ��� �����
	typedef VertMap::iterator VertMapIterator;

	// ���������� ������ �����, ������� ���� ������� � ������� ��� ��������� � ����
	int PutPoint(const point3& pnt, VertVector& vec, VertMap& vmap);

	// �������������
	bool initialized;
	void Init();
};

#endif // !defined(__GRAPH_HEX_GRID_H__)
