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
/// ���
/** �����, ������� ������������� ���������� �� ��������� � ������ �������.
    �������� ������������ ������ �� ���������, ��������� ����� ���� ����� �����,
	������� �������, ����������� ��� ����������. */
class VIS_MESH_IMPORT Mesh : public MlResource
{
public:
	/// ������, �������� � ���� ������
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
	/// ��������� ����� ���������
	typedef std::hash_map<std::string,MeshPolyGroup> PolyGroups_t;
	/// �������� �� ������ �����
	typedef NodeTree_t::const_iterator iterator;

private:
	Storage::Repository m_storage;	///< ��������� ��� ������ ����
	NodeTree_t m_tree;				///< ������ �����
	PolyGroups_t m_polyGroups;		///< ������ ���������

public:
	ML_RTTI_IFACE(Mesh,MlResource);

	/// \defgroup ������������
	/// @{
	/// �����������
	/** ������������ ��� �� �����
	  * \param name ��� ����
	  * \param front ��������� ����������
	  * \param userdata ������, ������� ���������� ��� �������� PieceBuffer
	  * \remarks ��� �����, �� �������� �������� ���, �������������� ��������� �������: "meshes/" + name + ".masce"
	  */

	Mesh(const std::string &name, AppFront *front, HObject userdata);
	/// �����������
	/** ������������ ��� �� ������
	  * \param name ��� ����
	  * \param stream �����
	  * \param front ��������� ����������
	  * \param userdata ������, ������� ���������� ��� �������� PieceBuffer
	  */

	Mesh(const std::string &name, std::istream &stream, AppFront *front, HObject userdata);
	/// @}
	/// ����������
	~Mesh();

	/// ���������� �������� �� ������ ������ ������ �����
	iterator begin() const { return m_tree.begin(); }
	/// ���������� �������� ����� ���������� ���� ������
	iterator end() const { return m_tree.end(); }

	/// ���������� ��������� ����� ���������
	const PolyGroups_t &getPolyGroups() const { return m_polyGroups; }
	/// ���������� ������ ��������� �� �����
	/// \param name ��� ������ ���������
	const MeshPolyGroup *getPolyGroup(const std::string &name) const;

	/// ���������� �����������, � ������� �������� ����� ����
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