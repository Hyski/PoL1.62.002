#if !defined(__VIS_MESH_POLY_GROUP_H_INCLUDED_7010462348260013__)
#define __VIS_MESH_POLY_GROUP_H_INCLUDED_7010462348260013__

#include "Vis.h"

//=====================================================================================//
//                                    namespace Vis                                    //
//=====================================================================================//
namespace Vis
{

class MeshPiece;

//=====================================================================================//
//                                 class MeshPolyGroup                                 //
//=====================================================================================//
/// ������ ��������� � ����
class VIS_MESH_IMPORT MeshPolyGroup
{
	PolyGroup *m_polyGroup;

public:
	/// \defgroup ������������. ������������ �����. �� ������������� ������������ �� � ���������� ��������
	/// @{
	/// ����������� �� ���������
	MeshPolyGroup();
	/// �����������, � ������� ���������� ������, ����������� �� ���������
	MeshPolyGroup(PolyGroup *);
	/// @}

	/// ���������� ������ ��������� �� ���������
	PolyGroup *getPolyGroup() const { return m_polyGroup; }
	/// ���������� ������ ������������� ������
	PolyGroup::PolyRange_t getPolyIdsOfPiece(MeshPiece *piece) const;
};

}

#endif // !defined(__VIS_MESH_POLY_GROUP_H_INCLUDED_7010462348260013__)