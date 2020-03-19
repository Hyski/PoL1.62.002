#if !defined(__VIS_MESH_STATE_H_INCLUDED_5722214559174769__)
#define __VIS_MESH_STATE_H_INCLUDED_5722214559174769__

#include "Vis.h"
#include "VisMesh.h"

//=====================================================================================//
//                                    namespace Vis                                    //
//=====================================================================================//
namespace Vis
{

//=====================================================================================//
//                                 struct TraceResult                                  //
//=====================================================================================//
struct TraceResult
{
	float m_coord;
	const MeshPolyGroup *m_group;
};

#pragma warning(push)
#pragma warning(disable: 4275)

//=====================================================================================//
//                                   class MeshState                                   //
//=====================================================================================//
class VIS_MESH_IMPORT MeshState : public MlObject
{
	friend class MeshStateAnim;

public:
	struct NodeData;
	class NodeHandler;

	/// Дерево матриц
	typedef mll::utils::lcrn_tree<NodeData> NodeTree_t;
	/// Итератор по дереву матриц
	typedef NodeTree_t::const_iterator iterator;

private:
	HMesh m_mesh;
	NodeTree_t m_tree;
	matrix3 m_world;
	obb3 m_obb;

public:
	ML_RTTI_IFACE(MeshState,MlObject);

	/// Создает пустой объект MeshState
	MeshState();
	/// Конструктор. Принимает меш в качестве параметра.
	MeshState(HMesh);
	/// Конструктор копирования
	MeshState(const MeshState &);

	/// Оператор присваивания
	MeshState &operator=(const MeshState &s);

	/// Возвращает хранимый меш
	HMesh getMesh() const { return m_mesh; }

	/// Возвращает класс для управления узлом
	/// \param name Имя узла
	NodeHandler getHandler(const std::string &name);

	/// Возвращает дерево матриц
	const NodeTree_t &getNodeTree() const { return m_tree; }

	/// Возвращает мировую матрицу
	const matrix3 &getWorld() const { return m_world; }

	/// Устанавливает мировую матрицу
	void setWorld(const matrix3 &);

	/// Трассирует луч
	/// \return Точку в координатах луча
	/// \remarks Если пересечение не найдено, то результат равен -1.0f
	float traceRay(const ray3 &) const;

	/// Трассирует луч. Возвращает информацию о группе полигонов, в которую попал луч
	/// \return Информацию о пересечении
	/// \remarks Если пересечение не найдено, то координата равна -1.0f
	TraceResult traceRayWithGroup(const ray3 &) const;

	/// Трассирует луч с учетом свойств кусков
	/// \return Точку в координатах луча
	/// \remarks Если пересечение не найдено, то результат равен -1.0f
	float traceRay(const ray3 &, TraceType) const;

	/// Трассирует луч с учетом свойств кусков. Возвращает информацию о группе полигонов, в которую попал луч
	/// \return Информацию о пересечении
	/// \remarks Если пересечение не найдено, то координата равна -1.0f
	TraceResult traceRayWithGroup(const ray3 &, TraceType) const;

	/// Пересчитать матрицы, начиная с заданного узла
	void updateFromNode(NodeTree_t::iterator);

	/// Возвращает ограничивающий прямоугольный параллелепипед
	const obb3 &getObb() const { return m_obb; }

	/// Меняет местами содержимое this и state
	MeshState &swap(MeshState &state);

	/// Заменить меш
	void reset(HMesh mesh) { swap(MeshState(mesh)); }

private:
	NodeTree_t &getNodeTreeNonConst() { return m_tree; }

	void constructTreeFromMesh();
	void constructTreeBranch(NodeTree_t::iterator);
	void initMatrices();
	void initMatrix(NodeTree_t::iterator);
	void calcObb();
};

#pragma warning(pop)

//=====================================================================================//
//                             struct MeshState::NodeData                              //
//=====================================================================================//
/// Данные для конкретного узла конкретной модели на уровне
struct MeshState::NodeData
{
	explicit NodeData(Mesh::iterator node)
	:	m_node(node),
		m_rotation(quaternion::identity()),
		m_innodeRotation(node->m_node->getRotation()),
		m_translation(mll::algebra::null_vector),
		m_innodeTranslation(node->m_node->getTranslation()),
		m_computedMatrix(matrix3::identity())
	{
	}

	Mesh::iterator m_node;
	quaternion m_innodeRotation;
	quaternion m_rotation;
	vector3 m_innodeTranslation;
	vector3 m_translation;
	mutable matrix3 m_computedMatrix;

	const matrix3 &getMatrix() const
	{
		return m_computedMatrix;
	}

	void update(const matrix3 &parentMatrix) const
	{
		m_computedMatrix = m_node->m_node->getTransformationPure(m_innodeRotation*m_rotation,m_innodeTranslation+m_translation);
		m_computedMatrix *= parentMatrix;
	}
};

//=====================================================================================//
//                            class MeshState::NodeHandler                             //
//=====================================================================================//
/// Класс для управления матрицей узла
class MeshState::NodeHandler
{
	HMeshState m_state;
	NodeTree_t::iterator m_node;

	void fn() {}
	typedef void (NodeHandler::*safe_bool)();

public:
	NodeHandler() {}
	NodeHandler(HMeshState state, NodeTree_t::iterator itor) : m_state(state), m_node(itor) {}

	HMeshState getMeshState() const { return m_state; }

	const quaternion &getRotation() const { return m_node->m_rotation; }
	const vector3 &getTranslation() const { return m_node->m_translation; }
	void setRotation(const quaternion &t) { m_node->m_rotation = t; m_state->updateFromNode(m_node); }
	void setTranslation(const vector3 &t) { m_node->m_translation= t; m_state->updateFromNode(m_node); }
	NodeTree_t::iterator getNodeIterator() const { return m_node; }

	operator safe_bool() const { return m_state.get()?fn:0; }
	bool operator!() const { return m_state.get() == 0; }
};

typedef MlObjHandle<MeshState> HMeshState;

}

#endif // !defined(__VIS_MESH_STATE_H_INCLUDED_5722214559174769__)