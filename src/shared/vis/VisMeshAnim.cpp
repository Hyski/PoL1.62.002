#include "Vis.h"
#include "VisAnimMgr.h"
#include "VisMeshAnim.h"
#include "VisAnimTrack.h"

namespace Vis
{

ML_RTTI_IMPL(MeshAnim,MlResource);

//=====================================================================================//
//                                MeshAnim::MeshAnim()                                 //
//=====================================================================================//
MeshAnim::MeshAnim(const std::string &name, std::istream &in)
:	m_name(name),
	m_anim("MAANIV01")
{
	m_anim.restore(mll::io::ibinstream(in));
	m_length = (*m_anim.begin<Vis::AnimTrack>())->getOriginAnim().length();
}

//=====================================================================================//
//                                MeshAnim::~MeshAnim()                                //
//=====================================================================================//
MeshAnim::~MeshAnim()
{
	AnimMgr::instance()->removeAnim(m_name);
}

}