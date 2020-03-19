#include "precomp.h"
#include "StatesManager.h"

StatesManager::render_types_t StatesManager::m_renderStates(256);
StatesManager::transforms_t StatesManager::m_transforms(32);
StatesManager::tex_stages_t StatesManager::m_texStages[2] = {tex_stages_t(32),tex_stages_t(32)};
IDirect3DDevice7 *StatesManager::m_device = 0;

//=====================================================================================//
//                       HRESULT StatesManager::SetRenderState()                       //
//=====================================================================================//
HRESULT StatesManager::SetRenderState(D3DRENDERSTATETYPE rst, DWORD newval)
{
	assert( rst < m_renderStates.size() );

	State<DWORD> &state = m_renderStates[rst];

	if(!state.m_valid || state.m_value != newval)
	{
		state.m_valid = true;
		state.m_value = newval;
		return m_device->SetRenderState(rst,newval);
	}

	return D3D_OK;
};

//=====================================================================================//
//                        HRESULT StatesManager::SetTransform()                        //
//=====================================================================================//
HRESULT StatesManager::SetTransform(D3DTRANSFORMSTATETYPE i, const D3DMATRIX *mtr)
{
	assert( i < m_transforms.size() );
	
	State<D3DMATRIX> &state = m_transforms[i];

	if(!state.m_valid || memcmp(&state.m_value,mtr,sizeof(D3DMATRIX)))
	{
		state.m_valid = true;
		state.m_value = *mtr;
		return m_device->SetTransform(i,(LPD3DMATRIX)mtr);
	}

	return D3D_OK;
}

//=====================================================================================//
//                    HRESULT StatesManager::SetTextureStageState()                    //
//=====================================================================================//
HRESULT StatesManager::SetTextureStageState(int stage, D3DTEXTURESTAGESTATETYPE tsst,
											DWORD newval)
{
	assert( stage < 2 );
	assert( stage >= 0 );
	assert( tsst < m_texStages[stage].size() );

	State<DWORD> &state = m_texStages[stage][tsst];

	if(!state.m_valid || state.m_value != newval)
	{
		state.m_valid = true;
		state.m_value = newval;
		return m_device->SetTextureStageState(stage,tsst,newval);
	}

	return D3D_OK;
}

//=====================================================================================//
//                         HRESULT StatesManager::SetTexture()                         //
//=====================================================================================//
HRESULT StatesManager::SetTexture(int stage, LPDIRECTDRAWSURFACE7 tex)
{
	return m_device->SetTexture(stage,tex);
}
