#include "Vis.h"

namespace Vis
{

ML_PERSISTENT_IMPL2(AnimTrack,VIS_BASIC_PTAG);

static const Storage::Version g_animTrackVersion(0,1);

//=====================================================================================//
//                               AnimTrack::AnimTrack()                                //
//=====================================================================================//
AnimTrack::AnimTrack()
:	m_nodeName("absolutely_wrong_node_name#@$%^")
{
}

//=====================================================================================//
//                               AnimTrack::AnimTrack()                                //
//=====================================================================================//
AnimTrack::AnimTrack(const AnimTrack &at)
:	m_nodeName(at.getNodeName()),
	m_originAnim(at.getOriginAnim()),
	m_orientAnim(at.getOrientAnim())
{
}

//=====================================================================================//
//                               AnimTrack::AnimTrack()                                //
//=====================================================================================//
AnimTrack::AnimTrack(Storage::ReadSlot &slot)
:	Storage::PersistentBase(slot)
{
	Storage::Version ver;
	slot.getStream() >> ver;
	
	if(!ver.isCompatible(g_animTrackVersion))
	{
		std::ostringstream sstr;
		sstr << "Unable to read Vis::AnimTrack from slot: wrong version, got ["
			<< ver.getMajor() << "." << ver.getMinor() << "]"
			<< " but current version is ["
			<< g_animTrackVersion.getMajor() << "." << g_animTrackVersion.getMinor()
			<< "]";
		throw AnimError(sstr.str());
	}

	slot.getStream() >> m_nodeName;
	slot.getStream() >> m_originAnim;
	slot.getStream() >> m_orientAnim;
}

//=====================================================================================//
//                            void AnimTrack::store() const                            //
//=====================================================================================//
void AnimTrack::store(Storage::WriteSlot &slot) const
{
	slot.getStream() << g_animTrackVersion;
	slot.getStream() << m_nodeName;
	slot.getStream() << m_originAnim;
	slot.getStream() << m_orientAnim;
}

}