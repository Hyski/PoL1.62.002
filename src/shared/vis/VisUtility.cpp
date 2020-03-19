#include "Vis.h"
#include "VisUtility.h"
#include "VisBufferMgr.h"

namespace Vis
{

//=====================================================================================//
//                     static D3DPRIMITIVETYPE getPrimitiveType()                      //
//=====================================================================================//
static D3DPRIMITIVETYPE getPrimitiveType(PrimitiveType pt)
{
	switch(pt)
	{
		case ptLineList: return D3DPT_LINELIST;
		case ptLineStrip: return D3DPT_LINESTRIP;
		case ptPointList: return D3DPT_POINTLIST;
		case ptTriangleFan: return D3DPT_TRIANGLEFAN;
		case ptTriangleList: return D3DPT_TRIANGLELIST;
		case ptTriangleStrip: return D3DPT_TRIANGLESTRIP;
	}
	assert( !"Wrong primitive type supplied" );
	return D3DPT_TRIANGLELIST;
}

//=====================================================================================//
//                               void Utility::render()                                //
//=====================================================================================//
void Utility::render(VertexRangeData *vrd, IndexRangeData *ird, PrimitiveType pt,
					 unsigned int start, unsigned int pcount, ShaderFX::HShaderState shd,
					 unsigned int delta, unsigned int distance)
{
	IDirect3DDevice9 *dev = BufferMgr::instance()->getDevice();

	vrd->select();
	ird->select();

	unsigned int passcount = shd->begin();
	for(unsigned int i = 0; i < passcount; ++i)
	{
#ifdef SHADERFX_NO_END_PASS
		shd->pass(i);
#else
		shd->beginPass(i);
#endif

		HRESULT hres = dev->DrawIndexedPrimitive
		(
			getPrimitiveType(pt),
			vrd->getStart()+delta,
			0,
			distance,
			start,
			pcount
		);
		assert( SUCCEEDED(hres) );

#ifndef SHADERFX_NO_END_PASS
		shd->endPass();
#endif
	}

	shd->end();
}

//=====================================================================================//
//                               void Utility::render()                                //
//=====================================================================================//
void Utility::render(VertexRangeData *vrd, PrimitiveType pt, 
					 unsigned int start, unsigned int pcount, ShaderFX::HShaderState shd)
{
	IDirect3DDevice9 *dev = BufferMgr::instance()->getDevice();

	vrd->select();

	unsigned int passcount = shd->begin();
	for(unsigned int i = 0; i < passcount; ++i)
	{
#ifdef SHADERFX_NO_END_PASS
		shd->pass(i);
#else
		shd->beginPass(i);
#endif

		HRESULT hres = dev->DrawPrimitive
		(
			getPrimitiveType(pt),
			start,
			pcount
		);
		assert( SUCCEEDED(hres) );

#ifndef SHADERFX_NO_END_PASS
		shd->endPass();
#endif
	}

	shd->end();
}

}