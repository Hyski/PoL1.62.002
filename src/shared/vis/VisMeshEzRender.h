#pragma once

#include "Vis.h"
#include "ShaderFXFront.h"

struct IDirect3DDevice9;
namespace ShaderFX { struct DirLight; }

//=====================================================================================//
//                                    namespace Vis                                    //
//=====================================================================================//
namespace Vis
{

#pragma warning(push)
#pragma warning(disable: 4275)

//=====================================================================================//
//                                 class MeshEzRender                                  //
//=====================================================================================//
class VIS_IMPORT MeshEzRender : boost::noncopyable
{
	HMeshState m_state;

public:
	MeshEzRender();
	MeshEzRender(HMeshState);
	~MeshEzRender();

	void reset(HMeshState);
	void render(IDirect3DDevice9 *);
	void render(IDirect3DDevice9 *, ShaderFX::HShaderState);
	void render(IDirect3DDevice9 *, const MeshPolyGroup *, ShaderFX::HShaderState);
	void applyDirectionalLight(const ShaderFX::DirLight &);

	void render(HMeshState, IDirect3DDevice9 *);
	void render(HMeshState, IDirect3DDevice9 *, ShaderFX::HShaderState);
	void render(HMeshState, IDirect3DDevice9 *, const MeshPolyGroup *, ShaderFX::HShaderState);
	void applyDirectionalLight(HMeshState, const ShaderFX::DirLight &);

private:
	template<typename Fn>
	void apply(Fn);
};

#pragma warning(pop)

}
