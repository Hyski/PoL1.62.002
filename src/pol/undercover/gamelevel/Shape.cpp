#include "precomp.h"

#include "Shape.h"
#include "../IWorld.h"
#include <common/graphpipe/GraphPipe.h>
#include <common/graphpipe/SimpleTexturedObject.h>

//=====================================================================================//
//                                   Shape::Shape()                                    //
//=====================================================================================//
Shape::Shape(TexObject *GeomData)
:	m_GeomData(*GeomData),
	m_StartVertex(0xffffffff)
{
}

//=====================================================================================//
//                                   Shape::~Shape()                                   //
//=====================================================================================//
Shape::~Shape()
{
}

//=====================================================================================//
//                              Shape& Shape::operator=()                              //
//=====================================================================================//
Shape& Shape::operator=(const Shape &a)
{
	if(this != &a)
	{
		m_GeomData = a.m_GeomData;
		m_StartVertex = a.m_StartVertex;
	}

	return *this;
}

//=====================================================================================//
//                           bool Shape::operator ==() const                           //
//=====================================================================================//
bool Shape::operator ==(const Shape &a) const
{
	if(this == &a) return true;

	const TexObject *x = &m_GeomData;
	const TexObject *y = &a.m_GeomData;

	if(x->PartNum != y->PartNum) return false;

	for(int i=0; i < x->PartNum; i++)
	{
		const Primi &primi1 = x->Parts[i]->prim;
		const Primi &primi2 = y->Parts[i]->prim;

		if(primi1.Verts.size() != primi2.Verts.size()) return false;
		if(primi1.Idxs.size()  != primi2.Idxs.size()) return false;
		if(x->Parts[i]->MaterialName != y->Parts[i]->MaterialName) return false;

		for(int j = 0; j < x->Parts[i]->prim.Verts.size(); j++)
		{	
			point3 pnt_diff(primi1.Verts[j].pos - primi2.Verts[j].pos);
			point3 uv_diff(primi1.Verts[j].uv.u-primi2.Verts[j].uv.u,primi1.Verts[j].uv.v-primi2.Verts[j].uv.v,0);
			if(fabsf(pnt_diff.x)+fabsf(pnt_diff.y)+fabsf(pnt_diff.z) > 1e-2f) return false;
			if(fabsf(uv_diff.x)+fabsf(uv_diff.y) > 1e-3f) return false;
		}

		for(int j=0;j<primi1.Idxs.size();j++)
		{
			if(primi1.Idxs[j] != primi2.Idxs[j]) return false;
		}
	}
	return true;
}

//=====================================================================================//
//                           bool Shape::operator !=() const                           //
//=====================================================================================//
bool Shape::operator !=(const Shape &a) const
{
	return !(*this==a);
}

//=====================================================================================//
//                            bool Shape::TraceRay() const                             //
//=====================================================================================//
bool Shape::TraceRay(const point3 & /*From*/, const point3 & /*Dir*/, point3 * /*Res*/, point3 * /*Norm*/) const
{
	return false;
}

//=====================================================================================//
//                             BBox Shape::GetBBox() const                             //
//=====================================================================================//
BBox Shape::GetBBox() const
{
	return m_GeomData.GetBBox();
}

//=====================================================================================//
//                              void Shape::Draw() const                               //
//=====================================================================================//
void Shape::Draw(const MatSet_t &Worlds,DynamicVB *Buf,unsigned skip_flags) const
{
	if(m_StartVertex != 0xFFFFFFFF)
	{
		IWorld::Get()->GetPipe()->Chop(&m_GeomData,Worlds,Buf,m_StartVertex, skip_flags);
	}
	else
	{
		POL_LOG("Shape::Draw: Unexpected condition, m_StartVertex == 0xFFFF\n");
	}
}
