#pragma once

#include "Vis.h"
#include "VisMesh.h"

namespace Vis
{

#pragma warning(push)
#pragma warning(disable: 4275)

//=====================================================================================//
//                                   class MeshAnim                                    //
//=====================================================================================//
class VIS_MESH_IMPORT MeshAnim : public MlResource
{
	std::string m_name;
	Storage::Repository m_anim;
	float m_length;

public:
	ML_RTTI_IFACE(MeshAnim,MlResource);

	MeshAnim(const std::string &name, std::istream &);
	~MeshAnim();

	/// ���������� �����������, � ������� �������� ��������
	const Storage::Repository &getRepository() const { return m_anim; }
	/// ���������� �������� ��������
	const std::string &getName() const { return m_name; }
	/// ���������� ������������ �������� � ���
	float getDuration() const { return m_length; }
};

#pragma warning(pop)

typedef MlObjHandle<MeshAnim> HMeshAnim;

}