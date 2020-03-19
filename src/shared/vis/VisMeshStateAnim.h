#pragma once

#include "Vis.h"
#include "VisMeshAnim.h"
#include "VisMeshState.h"

namespace Vis
{

class AnimTrack;

#pragma warning(push)
#pragma warning(disable: 4275)

//=====================================================================================//
//                                 class MeshStateAnim                                 //
//=====================================================================================//
class VIS_MESH_IMPORT MeshStateAnim : public MlObject
{
	struct NodeData
	{
		Vis::MeshState::NodeData *m_nodeData;
		const Vis::AnimTrack *m_animTrack;
	};

	HMeshAnim m_anim;
	HMeshState m_state;

	typedef std::vector<NodeData> NodeData_t;
	NodeData_t m_nodeData;

	float m_currentTime;

public:
	ML_RTTI_IFACE(MeshStateAnim,MlObject);

	MeshStateAnim(HMeshState, HMeshAnim);

	/// ���������� ��������� �� MeshAnim
	HMeshAnim getMeshAnim() const { return m_anim; }

	/// ������������� ������� ����� �� time � ��������� �������
	void update(float time);
	/// ���������� ������������ �������� � ���
	float getDuration() const { return m_anim->getDuration(); }
	/// ���������� ������� �����
	float getCurrentTime() const { return m_currentTime; }

private:
	void makeNodeData();
};

#pragma warning(pop)

typedef MlObjHandle<MeshStateAnim> HMeshStateAnim;

}