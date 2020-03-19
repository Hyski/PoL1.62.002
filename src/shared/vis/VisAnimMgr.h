#pragma once

#include "Vis.h"
#include "VisMeshAnim.h"

namespace Vis
{

MLL_MAKE_EXCEPTION(AnimMgrError,Error);
	MLL_MAKE_EXCEPTION(AnimLoadFailureError,AnimMgrError);

//=====================================================================================//
//                                    class AnimMgr                                    //
//=====================================================================================//
/// Менеджер анимаций
class VIS_MESH_IMPORT AnimMgr
{
	typedef std::hash_map<std::string,MeshAnim *> Anims_t;

	AppFront *m_appFront;
	Anims_t m_anims;

public:
	AnimMgr();
	~AnimMgr();

	/// Загружена ли анимация с таким именем
	bool hasAnim(const std::string &) const;
	/// Получить анимацию по имени
	HMeshAnim getAnim(const std::string &);
	/// Получить встроенную анимацию
	HMeshAnim embedAnim(const std::string &, std::istream &);

	static AnimMgr *instance();

private:
	friend class MeshAnim;
	void removeAnim(const std::string &);
};

}