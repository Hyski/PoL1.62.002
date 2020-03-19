#pragma once

#include "VisParams.h"

class Marker;
class Spectator;

//=====================================================================================//
//                                   class Relation                                    //
//=====================================================================================//
class Relation
{
	Marker *m_marker;
	Spectator *m_spectator;

	VisParams m_flags;
	player_type m_curPlayer;

public:
	Relation(Marker *, Spectator *);
	virtual ~Relation() {}

	/// ������, ����� �� ����������� ������
	VisParams GetRelationFlags() const { return m_flags; }

	/// �������� ���������
	void UpdateRelation(VisUpdateEvent_t ev);

	Marker *GetMarker() const { return m_marker; }
	Spectator *GetSpectator() const { return m_spectator; }

private:
	/// ��������� ���������� ����������
	virtual VisParams DoUpdateRelation(VisUpdateEvent_t ev) const;
};