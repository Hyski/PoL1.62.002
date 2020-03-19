#if !defined(__VIS_MESH_MGR_H_INCLUDED_2909414204844424__)
#define __VIS_MESH_MGR_H_INCLUDED_2909414204844424__

#include "Vis.h"
#include "VisMesh.h"
#include "VisAppFront.h"

//=====================================================================================//
//                                    namespace Vis                                    //
//=====================================================================================//
namespace Vis
{

MLL_MAKE_EXCEPTION(MeshMgrError,Error);
	MLL_MAKE_EXCEPTION(MeshLoadFailureError,MeshMgrError);

//=====================================================================================//
//                                    class MeshMgr                                    //
//=====================================================================================//
/// �����, ������������ ��������� � ��������� �����. �� �� ��������� �������� ���������� ���������� �����.
class VIS_MESH_IMPORT MeshMgr
{
	typedef std::hash_map<std::string,Mesh *> Meshes_t;

	AppFront *m_appFront;
	Meshes_t m_meshes;

public:
	/// �����������
	/// \param front ��������� ����������
	MeshMgr(AppFront *front);
	/// ����������
	~MeshMgr();

	/// ���������� true, ���� ��� � ����� ������ ���� � ���������
	/// \param name ��� ����
	bool hasMesh(const std::string &name) const;

	/// ���������� ��� �� �����
	/*! \param name ��� ����
	 *  \remarks ���� ��� � ����� ������ ��� ���� � ��������� (���� hasMesh(name) == true),
	 *           �� ���������� ���, �����, �������� ��������� ��� � ����� 
	 *  \return ��������� �� ��� */
	HMesh getMesh(const std::string &name);
	/// ���������� ���, ������� ���������� � ������ � ������������ ��� � ���������
	/*! \param name ��� ����
	 *  \param name ��� ����
	 *  \remarks ���� ��� � ����� ������ ��� ���� � ��������� (���� hasMesh(name) == true),
	 *           �� ���������� ���, �����, �������� ��������� ��� � ����� 
	 *  \return ��������� �� ��� */
	HMesh embedMesh(const std::string &name, std::istream &stream, HObject userdata);

	/// ���������� ��������� �� AppFront, ���������� � �����������
	AppFront *getAppFront() const { return m_appFront; }

	/// ���������� ��������� MeshMgr
	/// \remarks ���� ��������� �� ����������, �� ���������� 0 
	static MeshMgr *instance();

private:
	friend class Mesh;
	void removeMesh(const std::string &);
};

}

#endif // !defined(__VIS_MESH_MGR_H_INCLUDED_2909414204844424__)