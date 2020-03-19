#if !defined(__VIS_MESH_H_INCLUDED_5586575688569808__)
#define __VIS_MESH_H_INCLUDED_5586575688569808__

#include "Vis.h"
#include "VisNode.h"
#include "VisPiece.h"
#include "VisMeshPiece.h"
#include "VisMeshPolyGroup.h"
#include <mutual/resource.h>

//=====================================================================================//
//                                    namespace Vis                                    //
//=====================================================================================//
namespace Vis
{

class AppFront;

#pragma warning(push)
#pragma warning(disable: 4275)

//=====================================================================================//
//                                     class Mesh                                      //
//=====================================================================================//
/// Меш
/** Класс, который структурирует информацию из хранилища с сырыми данными.
    Собирает разрозненные данные из хранилища, формирует между ними явные связи,
	создает объекты, необходимые для рендеринга. */
class VIS_MESH_IMPORT Mesh : public MlResource
{
public:
	/// Данные, хранимые в узле дерева
	struct NodeData
	{
		typedef std::vector<MeshPiece *> MeshPieces_t;

		explicit NodeData(Node *node) : m_node(node) {}
		const Node *m_node;
		MeshPieces_t m_pieces;
	};

private:
	typedef mll::utils::lcrn_tree<NodeData> NodeTree_t;

public:
	/// Контейнер групп полигонов
	typedef std::hash_map<std::string,MeshPolyGroup> PolyGroups_t;
	/// Итератор по дереву узлов
	typedef NodeTree_t::const_iterator iterator;

private:
	Storage::Repository m_storage;	///< Хранилище для частей меша
	NodeTree_t m_tree;				///< Дерево узлов
	PolyGroups_t m_polyGroups;		///< Группы полигонов

public:
	ML_RTTI_IFACE(Mesh,MlResource);

	/// \defgroup Конструкторы
	/// @{
	/// Конструктор
	/** Конструирует меш из файла
	  * \param name Имя меша
	  * \param front Интерфейс приложения
	  * \param userdata Данные, которые передаются при создании PieceBuffer
	  * \remarks Имя файла, из которого читается меш, конструируется следующим образом: "meshes/" + name + ".masce"
	  */

	Mesh(const std::string &name, AppFront *front, HObject userdata);
	/// Конструктор
	/** Конструирует меш из потока
	  * \param name Имя меша
	  * \param stream Поток
	  * \param front Интерфейс приложения
	  * \param userdata Данные, которые передаются при создании PieceBuffer
	  */

	Mesh(const std::string &name, std::istream &stream, AppFront *front, HObject userdata);
	/// @}
	/// Деструктор
	~Mesh();

	/// Возвращает итератор на первый корень дерева узлов
	iterator begin() const { return m_tree.begin(); }
	/// Возвращает итератор после последнего узла дерева
	iterator end() const { return m_tree.end(); }

	/// Возвращает контейнер групп полигонов
	const PolyGroups_t &getPolyGroups() const { return m_polyGroups; }
	/// Возвращает группу полигонов по имени
	/// \param name Имя группы полигонов
	const MeshPolyGroup *getPolyGroup(const std::string &name) const;

	/// Возвращает репозиторий, в котором хранятся части меша
	Storage::Repository *getStorage() { return &m_storage; }

private:
	void buildTree();
	void initPieces(AppFront *, HObject userdata);
	void initGroups();
};

#pragma warning(pop)

typedef MlObjHandle<Mesh> HMesh;

//=====================================================================================//
//                   const MeshPolyGroup *Mesh::getPolyGroup() const                   //
//=====================================================================================//
inline const MeshPolyGroup *Mesh::getPolyGroup(const std::string &name) const
{
	PolyGroups_t::const_iterator i = m_polyGroups.find(name);
	if(i == m_polyGroups.end()) return 0;
	return &i->second;
}

}

#endif // !defined(__VIS_MESH_H_INCLUDED_5586575688569808__)