#pragma once

#include <common/graphpipe/SimpleTexturedObject.h>

class DynamicVB;
class ShapePool;

//=====================================================================================//
//                                     class Shape                                     //
//=====================================================================================//
class Shape
{
public:
	friend class ShapePool;
	typedef std::vector<const D3DMATRIX*> MatSet_t;

public:
	Shape(const Shape &a) : m_GeomData(a.m_GeomData), m_StartVertex(a.m_StartVertex) {}
	~Shape();

	BBox GetBBox() const;
	const TexObject* GetGeometry() const { return &m_GeomData; }
	bool TraceRay(const point3 &From, const point3 &Dir, point3 *Res, point3 *Norm) const;

	Shape& operator=(const Shape &a);
	bool operator ==(const Shape &a) const;
	bool operator !=(const Shape &a) const;

protected:
	Shape(TexObject *GeomData);

private:
	enum SKIP_FLAGS { SF_TRANSPARENT=0x01, SF_SOLID=0x20 };
	void Draw(const MatSet_t &, DynamicVB *Buf, unsigned skip_flags=0) const;

private:
	unsigned int m_StartVertex; //Ќачальна€ вершина в VB, который создастс€
	TexObject m_GeomData;
};
