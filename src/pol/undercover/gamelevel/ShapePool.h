#pragma once

#include "Shape.h"

class TexObject;

//=====================================================================================//
//                                   class ShapePool                                   //
//=====================================================================================//
class ShapePool
{
public:
	typedef unsigned int Handle;

private:
	struct ItemToDraw
	{
		Handle m_handle;
		Shape::MatSet_t m_matSet;

		ItemToDraw(Handle h, const Shape::MatSet_t &ms) : m_handle(h), m_matSet(ms) {}
	};

	typedef std::vector<ItemToDraw> DrawQueue_t;
	//typedef std::map<Handle,Shape::MatSet_t> DrawQueue_t;
	typedef std::vector<Shape> shapes_set_t;

	DrawQueue_t DrawQueue;
	shapes_set_t m_Shapes;

	std::auto_ptr<DynamicVB> m_Buf;

public:
	Handle AddShape(TexObject *GeomData);
	void CreateVB();

	void PushToDraw(const Handle &h,const D3DMATRIX &World);
	void DrawAll();
	void DrawAllTransparent();
	void Clear();
	const Shape* GetShape(const Handle &h) const;
	ShapePool();
	~ShapePool();

	void OnLostDevice();
	void OnResetDevice();

private:
	int NumVertices();	
	void FillBuffer(char *Data);
};
