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
/// Класс, занимающийся созданием и хранением мешей. Он не допускает создание нескольких одинаковых мешей.
class VIS_MESH_IMPORT MeshMgr
{
	typedef std::hash_map<std::string,Mesh *> Meshes_t;

	AppFront *m_appFront;
	Meshes_t m_meshes;

public:
	/// Конструктор
	/// \param front Интерфейс приложения
	MeshMgr(AppFront *front);
	/// Деструктор
	~MeshMgr();

	/// Возвращает true, если меш с таким именем есть в менеджере
	/// \param name Имя меша
	bool hasMesh(const std::string &name) const;

	/// Возвращает меш по имени
	/*! \param name Имя меша
	 *  \remarks Если меш с таким именем уже есть в менеджере (если hasMesh(name) == true),
	 *           то возвращает его, иначе, пытается загрузить меш с диска 
	 *  \return Указатель на меш */
	HMesh getMesh(const std::string &name);
	/// Возвращает меш, который содержится в потоке и регистрирует его в менеджере
	/*! \param name Имя меша
	 *  \param name Имя меша
	 *  \remarks Если меш с таким именем уже есть в менеджере (если hasMesh(name) == true),
	 *           то возвращает его, иначе, пытается загрузить меш с диска 
	 *  \return Указатель на меш */
	HMesh embedMesh(const std::string &name, std::istream &stream, HObject userdata);

	/// Возвращает указатель на AppFront, переданный в конструктор
	AppFront *getAppFront() const { return m_appFront; }

	/// Возвращает экземпляр MeshMgr
	/// \remarks Если экземпляр не существует, то возвращает 0 
	static MeshMgr *instance();

private:
	friend class Mesh;
	void removeMesh(const std::string &);
};

}

#endif // !defined(__VIS_MESH_MGR_H_INCLUDED_2909414204844424__)