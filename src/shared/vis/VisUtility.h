#pragma once

#include "Vis.h"
#include "VisIndexRange.h"
#include "VisVertexRange.h"
#include <ShaderFXShaderState.h>

namespace Vis
{

//=====================================================================================//
//                                 enum PrimitiveType                                  //
//=====================================================================================//
enum PrimitiveType
{
	ptLineList,
	ptLineStrip,
	ptPointList,
	ptTriangleFan,
	ptTriangleList,
	ptTriangleStrip
};

//=====================================================================================//
//                                    class Utility                                    //
//=====================================================================================//
struct VIS_BUFFER_IMPORT Utility
{
	template<typename V>
	static void render(const V &v,
				PrimitiveType pt, ShaderFX::HShaderState shd);

	template<typename V>
	static void render(const V &v,
				PrimitiveType pt, ShaderFX::HShaderState shd, const matrix3 &world);

	template<typename V>
	static void render(const V &v,
				PrimitiveType pt, unsigned int start, unsigned int pcount, ShaderFX::HShaderState shd);

	template<typename V>
	static void render(const V &v,
				PrimitiveType pt, unsigned int start, unsigned int pcount, ShaderFX::HShaderState shd,
				const matrix3 &world);

	template<typename V, typename I>
	static void render(const V &v, const I &,
				PrimitiveType pt, ShaderFX::HShaderState shd);

	template<typename V, typename I>
	static void render(const V &v, const I &,
				PrimitiveType pt, ShaderFX::HShaderState shd, const matrix3 &world);

	template<typename V, typename I>
	static void render(const V &v, const I &,
				PrimitiveType pt, unsigned int start, unsigned int pcount, ShaderFX::HShaderState shd);

	template<typename V, typename I>
	static void render(const V &v, const I &,
				PrimitiveType pt, unsigned int start, unsigned int pcount, ShaderFX::HShaderState shd,
				const matrix3 &world);

private:
	static void render(VertexRangeData *vr, IndexRangeData *ir, PrimitiveType,
						unsigned int start, unsigned int pcount, ShaderFX::HShaderState shd,
						unsigned int delta, unsigned int distance);
	static void render(VertexRangeData *vr, PrimitiveType,
						unsigned int start, unsigned int pcount, ShaderFX::HShaderState shd);

	static unsigned int getPrimitiveCount(unsigned int vtx, PrimitiveType pt)
	{
		switch(pt)
		{
			case ptLineList: return vtx/2;
			case ptLineStrip: return vtx-1;
			case ptPointList: return vtx;
			case ptTriangleFan: return vtx-2;
			case ptTriangleList: return vtx/3;
			case ptTriangleStrip: return vtx-2;
		}
		assert( !"Wrong primitive type supplied" );
		return 0;
	}
};

//=====================================================================================//
//                               void Utility::render()                                //
//=====================================================================================//
template<typename V>
void Utility::render(const V &v, PrimitiveType pt,
					 ShaderFX::HShaderState shd)
{
	VertexRangeData *vrd = v->getData();
	unsigned int pcount = v->getLockDistance() ? v->getLockDistance() : vrd->getCount();
	render(vrd,pt,vrd->getStart()+v->getPosition(),getPrimitiveCount(pcount,pt),shd);
}

//=====================================================================================//
//                               void Utility::render()                                //
//=====================================================================================//
template<typename V>
void Utility::render(const V &v, PrimitiveType pt,
					 ShaderFX::HShaderState shd, const matrix3 &world)
{
	shd->getParameter("World")->setMatrix(world);
	render(v,pt,shd);
}

//=====================================================================================//
//                               void Utility::render()                                //
//=====================================================================================//
template<typename V>
void Utility::render(const V &v, PrimitiveType pt,
					 unsigned int start, unsigned int pcount, ShaderFX::HShaderState shd)
{
	VertexRangeData *vrd = v->getData();
	render(vrd,pt,vrd->getStart()+vrd->getPosition()+start,pcount,shd);
}

//=====================================================================================//
//                               void Utility::render()                                //
//=====================================================================================//
template<typename V>
void Utility::render(const V &v, PrimitiveType pt,
					 unsigned int start, unsigned int pcount, ShaderFX::HShaderState shd,
					 const matrix3 &world)
{
	shd->getParameter("World")->setMatrix(world);
	render(v,pt,start,pcount,shd);
}

//=====================================================================================//
//                               void Utility::render()                                //
//=====================================================================================//
template<typename V, typename I>
void Utility::render(const V &v, const I &i,
					PrimitiveType pt, ShaderFX::HShaderState shd)
{
	VertexRangeData *vrd = v->getData();
	IndexRangeData *ird = i->getData();
	unsigned int pcount = i->getLockDistance() ? i->getLockDistance() : ird->getCount();
	unsigned int distance = v->getLockDistance() ? v->getLockDistance() : vrd->getCount();
	render(vrd,ird,pt,ird->getStart()+ird->getPosition(),getPrimitiveCount(pcount,pt),shd,v->getPosition(),distance);
}

//=====================================================================================//
//                               void Utility::render()                                //
//=====================================================================================//
template<typename V, typename I>
void Utility::render(const V &v, const I &i,
			PrimitiveType pt, ShaderFX::HShaderState shd, const matrix3 &world)
{
	shd->getParameter("World")->setMatrix(world);
	render(v,i,pt,shd);
}

//=====================================================================================//
//                               void Utility::render()                                //
//=====================================================================================//
template<typename V, typename I>
void Utility::render(const V &v, const I &i,
			PrimitiveType pt, unsigned int start, unsigned int pcount, ShaderFX::HShaderState shd)
{
	VertexRangeData *vrd = v->getData();
	IndexRangeData *ird = i->getData();
	unsigned int distance = v->getLockDistance() ? v->getLockDistance() : vrd->getCount();
	render(vrd,ird,pt,ird->getStart()+ird->getPosition()+start,pcount,shd,v->getPosition(),distance);
}

//=====================================================================================//
//                               void Utility::render()                                //
//=====================================================================================//
template<typename V, typename I>
void Utility::render(const V &v, const I &i,
			PrimitiveType pt, unsigned int start, unsigned int pcount, ShaderFX::HShaderState shd,
			const matrix3 &world)
{
	ShaderFX::HParameter hp_world = shd->getParameter("World");
	if (hp_world.get()) hp_world->setMatrix(world);
	render(v,i,pt,start,pcount,shd);
}


}
