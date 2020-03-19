#include "precomp.h"

#include "ShapePool.h"
#include <Common/GraphPipe/DynamicVB.h>
#include <Common/GraphPipe/SimpleTexturedObject.h>

//=====================================================================================//
//                               ShapePool::ShapePool()                                //
//=====================================================================================//
ShapePool::ShapePool()
:	m_Buf(NULL)
{
}

//=====================================================================================//
//                       ShapePool::Handle ShapePool::AddShape()                       //
//=====================================================================================//
ShapePool::Handle ShapePool::AddShape(TexObject *GeomData)
{
	Shape s(GeomData);
	for(int i=0;i<m_Shapes.size();i++)
	{
		if(s==m_Shapes[i]) 
			return i;
	}
	m_Shapes.push_back(s);
	return m_Shapes.size()-1;
}

//=====================================================================================//
//                            int ShapePool::NumVertices()                             //
//=====================================================================================//
int ShapePool::NumVertices()
{
	int num=0;
	for(int i=0;i<m_Shapes.size();i++)
	{
		for(int j = 0; j < m_Shapes[i].m_GeomData.PartNum; j++)
		{
			num += m_Shapes[i].m_GeomData.Parts[j]->prim.Verts.size();
		}
	}
	return num;
}

//=====================================================================================//
//                            void ShapePool::FillBuffer()                             //
//=====================================================================================//
void ShapePool::FillBuffer(char *VBData)
{
	static const int vert_size=12+12+8+4;
	char *data=VBData;

	for(int i=0;i<m_Shapes.size();i++)
	{
		m_Shapes[i].m_StartVertex=(data-VBData)/vert_size;
		for(int j = 0; j < m_Shapes[i].m_GeomData.PartNum; j++)
		{
			SimpleTexturedObject *sto = m_Shapes[i].m_GeomData.Parts[j];

			for(int k = 0; k < m_Shapes[i].m_GeomData.Parts[j]->prim.Verts.size(); k++)
			{
				*(point3*)data				= sto->prim.Verts[k].pos;
				*(point3*)(data+12)			= sto->prim.Verts[k].norm;
				*(texcoord*)(data+2*12+4)	= sto->prim.Verts[k].uv;
				data += vert_size;
			}
		}
	}
}

//=====================================================================================//
//                             void ShapePool::CreateVB()                              //
//=====================================================================================//
void ShapePool::CreateVB()
{	
	unsigned FVFflags = D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1|D3DFVF_DIFFUSE ;
	int num_vert = NumVertices();
	if(!num_vert) return;

	m_Buf.reset(new DynamicVB( D3DKernel::GetD3D(),FVFflags,num_vert,2*sizeof(point3)+sizeof(texcoord)+4));
	unsigned int startv;
	char *data=(char*)m_Buf->Lock(num_vert,&startv);
	FillBuffer(data);
	m_Buf->Unlock();
//	m_Buf->GetInterface()->Optimize(D3DKernel::GetD3DDevice(),0);
}

//=====================================================================================//
//                            void ShapePool::PushToDraw()                             //
//=====================================================================================//
void ShapePool::PushToDraw(const Handle &h,const D3DMATRIX &World)
{
	//сюда напихается куча ссылок на объекты, которые надо отрисовать
	DrawQueue.push_back(ItemToDraw(h,Shape::MatSet_t(1,&World)));
}

//=====================================================================================//
//                              void ShapePool::DrawAll()                              //
//=====================================================================================//
void ShapePool::DrawAll()
{
	if(!m_Buf.get()) return;

	for(DrawQueue_t::iterator i = DrawQueue.begin(); i != DrawQueue.end(); ++i)
	{
		m_Shapes[i->m_handle].Draw(i->m_matSet,m_Buf.get(),Shape::SF_TRANSPARENT);
	}

	DrawQueue.clear();
}

//=====================================================================================//
//                              void ShapePool::DrawAll()                              //
//=====================================================================================//
void ShapePool::DrawAllTransparent()
{
	if(!m_Buf.get()) return;

	for(DrawQueue_t::iterator i = DrawQueue.begin(); i != DrawQueue.end(); ++i)
	{
		m_Shapes[i->m_handle].Draw(i->m_matSet,m_Buf.get(),Shape::SF_TRANSPARENT);
	}

	for(DrawQueue_t::iterator i = DrawQueue.begin(); i != DrawQueue.end(); ++i)
	{
		m_Shapes[i->m_handle].Draw(i->m_matSet,m_Buf.get(),Shape::SF_SOLID);
	}

	DrawQueue.clear();
}

//=====================================================================================//
//                               void ShapePool::Clear()                               //
//=====================================================================================//
void ShapePool::Clear()
{
	m_Buf.reset();
	m_Shapes.clear();
}

//=====================================================================================//
//                      const Shape* ShapePool::GetShape() const                       //
//=====================================================================================//
const Shape* ShapePool::GetShape(const Handle &h) const
{
	return &m_Shapes[h];
}

//=====================================================================================//
//                               ShapePool::~ShapePool()                               //
//=====================================================================================//
ShapePool::~ShapePool()
{
}

//=====================================================================================//
//                           void ShapePool::OnLostDevice()                            //
//=====================================================================================//
void ShapePool::OnLostDevice()
{
	m_Buf.reset();
}

//=====================================================================================//
//                           void ShapePool::OnResetDevice()                           //
//=====================================================================================//
void ShapePool::OnResetDevice()
{
	CreateVB();
}
