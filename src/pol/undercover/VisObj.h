#pragma once

class GraphPipe;

namespace PoL
{

//=====================================================================================//
//                                   enum VisObjKind                                   //
//=====================================================================================//
enum VisObjKind
{
	vokDynObject,
	vokCharacter,
	vokShield,
	vokNonTraceable
};

//=====================================================================================//
//                                    class VisObj                                     //
//=====================================================================================//
class VisObj
{
	typedef mll::geometry::ray3 ray3;

	long m_id;

	VisObj(const VisObj &);
	VisObj &operator=(const VisObj &);

public:
	static const long NullId = -1l;

protected:
	VisObj() : m_id(NullId) {}

protected:
	virtual ~VisObj()
	{
		assert( m_id == NullId );
	}

public:
	/// Устанавливает идентификатор
	void assignId(long id) { m_id = id; }
	/// Возвращает идентификатор
	long getId() const { return m_id; }

	/// Отрисовать
	virtual void render(GraphPipe *, bool);
	/// Трассировать луч
	virtual float traceRay(const ray3 &, unsigned int passThru) = 0;
	/// Трассировать отрезок
	virtual float traceSegment(const ray3 &, unsigned int passThru) = 0;

	/// Возвращает обрамляющий параллелепипед
	virtual const mll::geometry::aabb3 &getBbox() const = 0;
};

}
