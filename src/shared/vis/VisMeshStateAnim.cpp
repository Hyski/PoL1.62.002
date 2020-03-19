#include "Vis.h"
#include "VisAnimTrack.h"
#include "VisMeshStateAnim.h"

namespace Vis
{

ML_RTTI_IMPL(MeshStateAnim,MlObject);

namespace MeshStateAnimDetail
{

struct AnimTrackByNodePred
{
	std::string m_name;
	AnimTrackByNodePred(const std::string &name) : m_name(name) {}

	bool operator()(const Vis::AnimTrack *t) const
	{
		const std::string &nodeName = t->getNodeName();
		return m_name == nodeName;
	}
};

}

using namespace MeshStateAnimDetail;

//=====================================================================================//
//                           MeshStateAnim::MeshStateAnim()                            //
//=====================================================================================//
MeshStateAnim::MeshStateAnim(HMeshState state, HMeshAnim anim)
:	m_anim(anim),
	m_state(state),
	m_currentTime(0.0f)
{
	makeNodeData();
}

//=====================================================================================//
//                         void MeshStateAnim::makeNodeData()                          //
//=====================================================================================//
void MeshStateAnim::makeNodeData()
{
	MeshState::NodeTree_t::iterator i = m_state->getNodeTreeNonConst().begin();
	m_nodeData.resize(m_state->getNodeTree().size());

	for(unsigned int j = 0; j < m_nodeData.size(); ++j, ++i)
	{
		typedef Storage::Repository::ConstRange<Vis::AnimTrack>::type Range_t;
		typedef Storage::Repository::ConstIterator<Vis::AnimTrack>::type Itor_t;

		const std::string name = i->m_node->m_node->getName();
		Range_t range = m_anim->getRepository().getObjects<Vis::AnimTrack>();
		Itor_t k = std::find_if(range.first,range.second,AnimTrackByNodePred(name));
		assert( k != range.second );

		m_nodeData[j].m_nodeData = &*i;
		m_nodeData[j].m_animTrack = *k;
	}
}

//=====================================================================================//
//                            void MeshStateAnim::update()                             //
//=====================================================================================//
void MeshStateAnim::update(float time)
{
	if(m_currentTime == time) return;

	for(NodeData_t::iterator i = m_nodeData.begin(); i != m_nodeData.end(); ++i)
	{
		i->m_nodeData->m_innodeTranslation = i->m_animTrack->getOriginAnim().get(time) - mll::algebra::origin;
		i->m_nodeData->m_innodeRotation = i->m_animTrack->getOrientAnim().get(time);
	}

	m_state->updateFromNode(m_state->getNodeTreeNonConst().begin());
	m_currentTime = time;
}

}