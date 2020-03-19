#pragma once

//=====================================================================================//
//                                 class StatesManager                                 //
//=====================================================================================//
class StatesManager
{
	template<typename T> struct State
	{
		bool m_valid;
		T m_value;
		State() : m_valid(false) {}
	};

	typedef std::vector< State<DWORD> > render_types_t;
	typedef std::vector< State<D3DMATRIX> > transforms_t;
	typedef std::vector< State<DWORD> > tex_stages_t;

	static render_types_t m_renderStates;
	static transforms_t m_transforms;
	static tex_stages_t m_texStages[2];
	static IDirect3DDevice7 *m_device;

public:
	static void Clear()
	{
		std::fill(m_renderStates.begin(),m_renderStates.end(),State<DWORD>());
		std::fill(m_transforms.begin(),m_transforms.end(),State<D3DMATRIX>());
		std::fill(m_texStages[0].begin(),m_texStages[0].end(),State<DWORD>());
		std::fill(m_texStages[1].begin(),m_texStages[1].end(),State<DWORD>());
		m_device = D3DKernel::GetD3DDevice();
	};

	static HRESULT SetRenderState(D3DRENDERSTATETYPE, DWORD);
	static HRESULT SetTransform(D3DTRANSFORMSTATETYPE, const D3DMATRIX *);
	static HRESULT SetTextureStageState(int, D3DTEXTURESTAGESTATETYPE, DWORD);
	static HRESULT SetTexture(int, LPDIRECTDRAWSURFACE7);
};
