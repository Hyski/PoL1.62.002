#pragma once

#include "VisParams.h"

class Marker;
class Relation;
class Spectator;
class VisMapMarkerIterator;
class VisMapSpectatorIterator;

//=====================================================================================//
//                                    class VisMap                                     //
//=====================================================================================//
class VisMap
{
	friend class VisMapMarkerIterator;
	friend class VisMapSpectatorIterator;

	typedef std::vector<Marker *> Marks_t;
	typedef std::vector<Spectator *> Specs_t;
	typedef std::vector<Relation *> RelsRow_t;
	typedef std::vector<RelsRow_t> Rels_t;

	Rels_t m_rels;
	Marks_t m_marks;
	Specs_t m_specs;

	// m_specs.size() == m_rels.size();
	// m_marks.size() == m_rels[n].size();

public:
	void Reset();

public:
	static VisMap *GetInst();

	void Insert(Spectator *);
	void Insert(Marker *);
	void Remove(Spectator *);
	void Remove(Marker *);

	VisParams GetVisFlags(Spectator *, Marker *);

	/// ќбновить состо€ние дл€ маркера
	void UpdateMarker(Marker *, VisUpdateEvent_t);
	/// ќбновить состо€ние дл€ наблюдател€
	void UpdateSpectator(Spectator *, VisUpdateEvent_t);

	/// ќбновить видимость (дл€ начала хода)
	void ResetVisibility(player_type p);

	typedef VisMapMarkerIterator MarkIterator_t;
	typedef VisMapSpectatorIterator SpecIterator_t;
	typedef Marks_t::const_iterator PlainMarkIterator_t;

	/// ѕеребор наблюдателей дл€ маркера
	SpecIterator_t SpecBegin(Marker *, VisParams) const;
	SpecIterator_t SpecEnd() const;

	/// ѕеребор маркеров дл€ наблюдател€
	MarkIterator_t MarkBegin(Spectator *, VisParams) const;
	MarkIterator_t MarkEnd() const;

	PlainMarkIterator_t PlainMarkBegin() const { return m_marks.begin(); }
	PlainMarkIterator_t PlainMarkEnd() const { return m_marks.end(); }

	/// ¬озвращает отношение между наблюдателем и маркером
	Relation *GetRelation(Spectator *, Marker *);
	const Relation *GetRelation(Spectator *, Marker *) const;
	
private:
	VisMap();
	~VisMap();
};

//=====================================================================================//
//                             class VisMapMarkerIterator                              //
//=====================================================================================//
class VisMapMarkerIterator
{
	typedef VisMap::Marks_t::const_iterator RawIt_t;

	Spectator *m_spec;
	RawIt_t m_itor, m_end;
	VisParams m_flags;

public:
	VisMapMarkerIterator();
	VisMapMarkerIterator(Spectator *, RawIt_t ib, RawIt_t ie, VisParams p);

	VisMapMarkerIterator &operator++();
	VisMapMarkerIterator operator++(int);

	bool operator==(const VisMapMarkerIterator &) const;
	bool operator!=(const VisMapMarkerIterator &m) const { return !(*this == m); }

	Marker *operator->() const { return *m_itor; }
	Marker &operator*() const { return **m_itor; }

private:
	bool IsEnd() const { return !m_spec || m_itor == m_end; }
	bool GoodRelation() const { return VisMap::GetInst()->GetVisFlags(m_spec,*m_itor) == m_flags; }
};

//=====================================================================================//
//                            class VisMapSpectatorIterator                            //
//=====================================================================================//
class VisMapSpectatorIterator
{
	typedef VisMap::Specs_t::const_iterator RawIt_t;

	Marker *m_marker;
	RawIt_t m_itor, m_end;
	VisParams m_flags;

public:
	VisMapSpectatorIterator();
	VisMapSpectatorIterator(Marker *, RawIt_t ib, RawIt_t ie, VisParams p);

	VisMapSpectatorIterator &operator++();
	VisMapSpectatorIterator operator++(int);

	bool operator==(const VisMapSpectatorIterator &) const;
	bool operator!=(const VisMapSpectatorIterator &m) const { return !(*this == m); }

	Spectator *operator->() const { return *m_itor; }
	Spectator &operator*() const { return **m_itor; }

private:
	bool IsEnd() const { return !m_marker || m_itor == m_end; }
	bool GoodRelation() const { return VisMap::GetInst()->GetVisFlags(*m_itor,m_marker) == m_flags; }
};

//=====================================================================================//
//                 inline VisMapMarkerIterator::VisMapMarkerIterator()                 //
//=====================================================================================//
inline VisMapMarkerIterator::VisMapMarkerIterator()
:	m_spec(0)
{
}

//=====================================================================================//
//                 inline VisMapMarkerIterator::VisMapMarkerIterator()                 //
//=====================================================================================//
inline VisMapMarkerIterator::VisMapMarkerIterator(Spectator *spec, RawIt_t ib, RawIt_t ie, VisParams p)
:	m_spec(spec),
	m_itor(ib),
	m_end(ie),
	m_flags(p)
{
	if(!IsEnd() && !GoodRelation())
	{
		++(*this);
	}
}

//=====================================================================================//
//           inline VisMapMarkerIterator &VisMapMarkerIterator::operator++()           //
//=====================================================================================//
inline VisMapMarkerIterator &VisMapMarkerIterator::operator++()
{
	if(!IsEnd())
	{
		do ++m_itor; while(!IsEnd() && !GoodRelation());
	}
	return *this;
}

//=====================================================================================//
//           inline VisMapMarkerIterator VisMapMarkerIterator::operator++()            //
//=====================================================================================//
inline VisMapMarkerIterator VisMapMarkerIterator::operator++(int)
{
	VisMapMarkerIterator tmp(*this);
	++(*this);
	return tmp;
}

//=====================================================================================//
//                inline bool VisMapMarkerIterator::operator==() const                 //
//=====================================================================================//
inline bool VisMapMarkerIterator::operator==(const VisMapMarkerIterator &i) const
{
	if(IsEnd() && i.IsEnd()) return true;
	return m_itor == i.m_itor;
}

//=====================================================================================//
//              inline VisMapSpectatorIterator::VisMapSpectatorIterator()              //
//=====================================================================================//
inline VisMapSpectatorIterator::VisMapSpectatorIterator()
:	m_marker(0)
{
}

//=====================================================================================//
//              inline VisMapSpectatorIterator::VisMapSpectatorIterator()              //
//=====================================================================================//
inline VisMapSpectatorIterator::VisMapSpectatorIterator(Marker *m, RawIt_t ib, RawIt_t ie, VisParams p)
:	m_marker(m),
	m_itor(ib),
	m_end(ie),
	m_flags(p)
{
	if(!IsEnd() && !GoodRelation())
	{
		++(*this);
	}
}

//=====================================================================================//
//        inline VisMapSpectatorIterator &VisMapSpectatorIterator::operator++()        //
//=====================================================================================//
inline VisMapSpectatorIterator &VisMapSpectatorIterator::operator++()
{
	if(!IsEnd())
	{
		do ++m_itor; while(!IsEnd() && !GoodRelation());
	}
	return *this;
}

//=====================================================================================//
//        inline VisMapSpectatorIterator VisMapSpectatorIterator::operator++()         //
//=====================================================================================//
inline VisMapSpectatorIterator VisMapSpectatorIterator::operator++(int)
{
	VisMapSpectatorIterator tmp(*this);
	++(*this);
	return tmp;
}

//=====================================================================================//
//               inline bool VisMapSpectatorIterator::operator==() const               //
//=====================================================================================//
inline bool VisMapSpectatorIterator::operator==(const VisMapSpectatorIterator &i) const
{
	if(IsEnd() && i.IsEnd()) return true;
	return m_itor == i.m_itor;
}

