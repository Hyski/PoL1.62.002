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
/// Группа полигонов в меше
class VIS_MESH_IMPORT MeshPolyGroup
{
	PolyGroup *m_polyGroup;

public:
	/// \defgroup Конструкторы. Используются мешем. Не рекомендуется использовать их в приложении напрямую
	/// @{
	/// Конструктор по умолчанию
	MeshPolyGroup();
	/// Конструктор, в который передается группа, загруженная из хранилища
	MeshPolyGroup(PolyGroup *);
	/// @}

	/// Возвращает группу полигонов из хранилища
	PolyGroup *getPolyGroup() const { return m_polyGroup; }
	/// Возвращает список треугольников группы
	PolyGroup::PolyRange_t getPolyIdsOfPiece(MeshPiece *piece) const;
};

}

#endif // !defined(__VIS_MESH_POLY_GROUP_H_INCLUDED_7010462348260013__)