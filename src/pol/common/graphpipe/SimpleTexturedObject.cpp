// SimpleTexturedObject.cpp: implementation of the SimpleTexturedObject class.
//
//////////////////////////////////////////////////////////////////////
#include "precomp.h"
#include "SimpleTexturedObject.h"
#include "culling.h"
#include "../saveload/saveload.h"
#include "../utils/optslot.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// создадим функции для доступа к индексам - на чтение и на чтение/запись
short int * SimpleTexturedObject::GetIndexesFull()
{
	return reinterpret_cast<short int*>(&prim.Idxs[0]);
}

const short int*  const SimpleTexturedObject::GetIndexesRO()
{
	return reinterpret_cast<const short int*>(&prim.Idxs[0]);
}

void SimpleTexturedObject::FreeAll()
{
	prim.Verts.clear();
	prim.Idxs.clear();
}

//=====================================================================================//
//                      bool SimpleTexturedObject::MakeSaveLoad()                      //
//=====================================================================================//
bool SimpleTexturedObject::MakeSaveLoad( SavSlot &slot )
{
	if(slot.IsSaving())
	{
		slot << MaterialName;
		slot << prim.Verts.size();
		slot.Reserve((sizeof(point3) + sizeof(point3) + sizeof(texcoord)) * prim.Verts.size());
		for(int v = 0; v < prim.Verts.size(); v++)
		{
			slot << prim.Verts[v].pos.x << prim.Verts[v].pos.y << prim.Verts[v].pos.z;
			slot << prim.Verts[v].norm.x << prim.Verts[v].norm.y << prim.Verts[v].norm.z;
			slot << prim.Verts[v].uv.u << prim.Verts[v].uv.v;
		}
		slot << prim.Idxs.size();
		slot.Save(&prim.Idxs[0], sizeof(prim.Idxs[0]) * prim.Idxs.size());
		//for(int id=0;id<prim.Idxs.size();id++)
		//  slot<<prim.Idxs[id];
	}
	else
	{
		FreeAll();
		slot >> MaterialName;
		unsigned int pnum;
		slot >> pnum;
		if(pnum)
		{
			prim.Verts.resize(pnum);
		}
		for(int v = 0; v < pnum; v++)
		{
			slot >> prim.Verts[v].pos.x >> prim.Verts[v].pos.y >> prim.Verts[v].pos.z;
			slot >> prim.Verts[v].norm.x >> prim.Verts[v].norm.y >> prim.Verts[v].norm.z;
			prim.Verts[v].norm = Normalize(prim.Verts[v].norm);
			slot >> prim.Verts[v].uv.u >> prim.Verts[v].uv.v;
		}
		unsigned int inum;
		slot >> inum;
		if(inum)
			prim.Idxs.resize(inum);
		for(int id = 0; id < inum; id++)
			slot >> prim.Idxs[id];
	}
	return true;
}

//=====================================================================================//
//                          void SimpleTexturedObject::Save()                          //
//=====================================================================================//
void SimpleTexturedObject::Save( OptSlot &slot )
{
	slot << MaterialName;
	slot << prim.Verts.size();
	for(int v = 0; v < prim.Verts.size(); v++)
	{
		slot << prim.Verts[v].pos.x << prim.Verts[v].pos.y << prim.Verts[v].pos.z;
		slot << prim.Verts[v].norm.x << prim.Verts[v].norm.y << prim.Verts[v].norm.z;
		slot << prim.Verts[v].uv.u << prim.Verts[v].uv.v;
	}
	slot << prim.Idxs.size();
	for(int id = 0; id < prim.Idxs.size(); id++)
	{
		slot << prim.Idxs[id];
	}
}

//=====================================================================================//
//                          void SimpleTexturedObject::Load()                          //
//=====================================================================================//
void SimpleTexturedObject::Load( OptSlot &slot )
{
	FreeAll();
	slot >> MaterialName;
	unsigned int vnum;
	slot >> vnum;
	if(vnum)
	{
		prim.Verts.resize(vnum);
	}
	for(int v = 0; v < vnum; v++)
	{
		slot >> prim.Verts[v].pos.x >> prim.Verts[v].pos.y >> prim.Verts[v].pos.z;
		slot >> prim.Verts[v].norm.x >> prim.Verts[v].norm.y >> prim.Verts[v].norm.z;
		prim.Verts[v].norm = Normalize(prim.Verts[v].norm);
		slot >> prim.Verts[v].uv.u >> prim.Verts[v].uv.v;
	}
	unsigned int inum;
	slot >> inum;
	if(inum)
	{
		prim.Idxs.resize(inum);
	}
	for(int id = 0; id < prim.Idxs.size(); id++)
	{
		slot >> prim.Idxs[id];
	}
}

//=====================================================================================//
//                           BBox TexObject::GetBBox() const                           //
//=====================================================================================//
BBox TexObject::GetBBox() const
{
	BBox b;
	b.Degenerate();
	for(int i = 0; i < PartNum; i++)
	{
		for(int j = 0; j < Parts[i]->prim.Verts.size(); j++)
		{
			b.Enlarge(Parts[i]->prim.Verts[j].pos);
		}
	}
	return b;
}

//=====================================================================================//
//                          bool TexObject::TraceRay() const                           //
//=====================================================================================//
bool TexObject::TraceRay( const ray &r, float *Pos, point3 *Norm ) const
{
	const float BOGUS = 1e+6f;
	*Pos = BOGUS;
	Triangle a;
	float t, u, v;
	for(int i = 0; i < PartNum; i++)
	{
		Primi::FVFVertex *pnt = &Parts[i]->prim.Verts[0];
		short int *idxs = Parts[i]->GetIndexesFull();

		for(int j = 0; j < Parts[i]->prim.Idxs.size(); j += 3)
		{
			a.V[0] = pnt[idxs[j]].pos;
			a.V[1] = pnt[idxs[j + 1]].pos;
			a.V[2] = pnt[idxs[j + 2]].pos;
			if(!(a.RayTriangle(r.Origin, r.Direction, &t, &u, &v) && t > 0))
				continue;
			if(t >= *Pos)
				continue;
			*Pos = t;
			*Norm = Normalize((a.V[1] - a.V[0]).Cross(a.V[2] - a.V[0]));
		}
	}
	return *Pos != BOGUS;
}

//=====================================================================================//
//                           bool TexObject::MakeSaveLoad()                            //
//=====================================================================================//
bool TexObject::MakeSaveLoad( SavSlot &slot )
{
	if(slot.IsSaving())
	{
		slot << PartNum;
	}
	else
	{
		Release();
		slot >> PartNum;
		for(int i = 0; i < PartNum; i++)
			Parts[i] = new STObject;
	}
	for(int i = 0; i < PartNum; i++)
	{
		Parts[i]->MakeSaveLoad(slot);
	}  
	return true;
}

//=====================================================================================//
//                               void TexObject::Save()                                //
//=====================================================================================//
void TexObject::Save( OptSlot &slot )
{
	slot << PartNum;
	for(int i = 0; i < PartNum; i++)
	{
		Parts[i]->Save(slot);
	}
}

//=====================================================================================//
//                               void TexObject::Load()                                //
//=====================================================================================//
void TexObject::Load( OptSlot &slot )
{
	Release();
	slot >> PartNum;
	for(int i = 0; i < PartNum; i++)
	{
		Parts[i] = new STObject;
		Parts[i]->Load(slot);
	}
}

//=====================================================================================//
//                         TexObject &TexObject::operator =()                          //
//=====================================================================================//
TexObject & TexObject::operator =( const TexObject &a )
{
	if(this == &a)
	{
		return *this;
	}
	Release();
	PartNum = a.PartNum;
	for(int i = 0; i < PartNum; ++i)
	{
		Parts[i] = new STObject;
		*Parts[i] = *a.Parts[i];
	}
	return *this;
}
