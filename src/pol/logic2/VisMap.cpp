#include "logicdefs.h"

#include "VisMap.h"
#include "Marker.h"
#include "Relation.h"
#include "Spectator.h"
#include "GameObserver.h"
#include "EnemyDetection.h"

namespace VisMapDetails
{

//=====================================================================================//
//                            class UnlimitedMovesRelation                             //
//=====================================================================================//
class UnlimitedMovesRelation : public Relation
{
public:
	UnlimitedMovesRelation(Spectator *s, Marker *m) : Relation(m,s) {}

private:
	/// Выполнить вычисление параметров
	virtual VisParams DoUpdateRelation(VisUpdateEvent_t ev) const
	{
		if(ev == UE_POL_UNLIM_MOVES_UPDATE)
		{
			if(ev == UE_DEL_MARKER || ev == UE_DEL_SPECTATOR) return VisParams::vpInvisible;
			return GetSpectator()->CalcVisibility(GetMarker());
		}
		else
		{
			return GetRelationFlags();
		}
	}
};

//=====================================================================================//
//                            class UnlimitedMovesSpectator                            //
//=====================================================================================//
class UnlimitedMovesSpectator : public Spectator, private GameObserver
{
public:
    DCTOR_DEF(UnlimitedMovesSpectator)

	UnlimitedMovesSpectator()
	{
		GameEvMessenger::GetInst()->Attach(this,EV_UNLIMITED_MOVES_STARTED);
		GameEvMessenger::GetInst()->Attach(this,EV_UNLIMITED_MOVES_ENDED);
		GameEvMessenger::GetInst()->Attach(this,EV_UPDATE_UNLIMITED_VISIBILITY);
	}

	~UnlimitedMovesSpectator()
	{
		GameEvMessenger::GetInst()->Detach(this);
	}

	/// Рассчитать видимость для маркера
	virtual VisParams CalcVisibility(Marker *mrk)
	{
		if(PoL::Inv::AllVisibleWhenMovesUnlimited)
		{
			if(!UnlimitedMoves())
			{
				return VisParams::vpInvisible;
			}
			else
			{
				if(mrk->GetEntity() && EnemyDetector::getInst()->isHeEnemy(mrk->GetEntity()))
				{
					return VisParams::vpInvisible;
				}

				return VisParams::vpVisible;
			}
		}
		else
		{
			return VisParams::vpInvisible;
		}
	}

	/// Рассчитать видимость для точки
	virtual VisParams CalcVisibility(const point3 &)
	{
		return VisParams::vpInvisible;
	}

	/// Создать отношение с маркером
	virtual std::auto_ptr<Relation> CreateRelationFor(Marker *mrk)
	{
		return std::auto_ptr<Relation>(new UnlimitedMovesRelation(this,mrk));
	}
	
	virtual player_type GetPlayer() const
	{
		return PT_PLAYER1;
	}

private:
	void Update(subject_t subj, event_t event, info_t info)
	{
		VisMap::GetInst()->UpdateSpectator(this,UE_POL_UNLIM_MOVES_UPDATE);
	}
};

DCTOR_IMP(UnlimitedMovesSpectator)

}

using namespace VisMapDetails;

static UnlimitedMovesSpectator g_unlimSpec;

//=====================================================================================//
//                                  VisMap::VisMap()                                   //
//=====================================================================================//
VisMap::VisMap()
{
	Insert(&g_unlimSpec);
}

//=====================================================================================//
//                                  VisMap::~VisMap()                                  //
//=====================================================================================//
VisMap::~VisMap()
{
}

//=====================================================================================//
//                                void VisMap::Reset()                                 //
//=====================================================================================//
void VisMap::Reset()
{
	m_marks.clear();
	m_specs.clear();

	for(Rels_t::iterator i = m_rels.begin(); i != m_rels.end(); ++i)
	{
		for(RelsRow_t::iterator j = i->begin(); j != i->end(); ++j)
		{
			delete *j;
		}
	}

	m_rels.clear();
	Insert(&g_unlimSpec);
}

//=====================================================================================//
//                                void VisMap::Insert()                                //
//=====================================================================================//
void VisMap::Insert(Spectator *s)
{
	Specs_t::iterator i = std::lower_bound(m_specs.begin(),m_specs.end(),s);
	if(i == m_specs.end() || *i != s)
	{
		const unsigned int index = i - m_specs.begin();

		m_specs.insert(i,s);
		m_rels.insert(m_rels.begin()+index,RelsRow_t());

		m_rels[index].resize(m_marks.size(),0);

		for(Marks_t::iterator it = m_marks.begin(); it != m_marks.end(); ++it)
		{
			Relation *& rel = m_rels[index][it-m_marks.begin()];
			rel = s->CreateRelationFor(*it).release();
		}

		for(Marks_t::iterator it = m_marks.begin(); it != m_marks.end(); ++it)
		{
			Relation *& rel = m_rels[index][it-m_marks.begin()];
			rel->UpdateRelation(UE_INS_SPECTATOR);
		}

		s->OnInsert();
		UpdateSpectator(s,UE_INS_SPECTATOR);
	}
}

//=====================================================================================//
//                                void VisMap::Insert()                                //
//=====================================================================================//
void VisMap::Insert(Marker *m)
{
	Marks_t::iterator i = std::lower_bound(m_marks.begin(),m_marks.end(),m);
	if(i == m_marks.end() || *i != m)
	{
		const unsigned int index = i - m_marks.begin();

		m_marks.insert(i,m);

		for(Rels_t::iterator it = m_rels.begin(); it != m_rels.end(); ++it)
		{
			Relation *rel = 0;
			if(m_specs[it-m_rels.begin()])
			{
				rel = m_specs[it-m_rels.begin()]->CreateRelationFor(m).release();
			}

			it->insert(it->begin()+index,rel);
		}

		for(Rels_t::iterator it = m_rels.begin(); it != m_rels.end(); ++it)
		{
			(*it)[index]->UpdateRelation(UE_INS_MARKER);
		}

		UpdateMarker(m,UE_INS_MARKER);
	}
}

//=====================================================================================//
//                                void VisMap::Remove()                                //
//=====================================================================================//
void VisMap::Remove(Spectator *s)
{
	UpdateSpectator(s,UE_DEL_SPECTATOR);
	s->OnRemove();

	Specs_t::iterator i = std::lower_bound(m_specs.begin(),m_specs.end(),s);
	if(i != m_specs.end() && *i == s)
	{
		const unsigned int index = i - m_specs.begin();

		m_specs.erase(i);
		for(RelsRow_t::iterator it = m_rels[index].begin(); it != m_rels[index].end(); ++it)
		{
			delete *it;
		}
		m_rels.erase(m_rels.begin()+index);
	}
}

//=====================================================================================//
//                                void VisMap::Remove()                                //
//=====================================================================================//
void VisMap::Remove(Marker *m)
{
	UpdateMarker(m, UE_DEL_MARKER);

	Marks_t::iterator i = std::lower_bound(m_marks.begin(),m_marks.end(),m);
	if(i != m_marks.end() && *i == m)
	{
		const unsigned int index = i - m_marks.begin();

		m_marks.erase(i);

		for(Rels_t::iterator it = m_rels.begin(); it != m_rels.end(); ++it)
		{
			delete (*it)[index];
			it->erase(it->begin()+index);
		}
	}
}

//=====================================================================================//
//                           VisParams VisMap::GetVisFlags()                           //
//=====================================================================================//
VisParams VisMap::GetVisFlags(Spectator *s, Marker *m)
{
	return GetRelation(s,m)->GetRelationFlags();
}

//=====================================================================================//
//                            void VisMap::UpdateMarker();                             //
//=====================================================================================//
void VisMap::UpdateMarker(Marker *m, VisUpdateEvent_t ev)
{
	Marks_t::iterator im = std::lower_bound(m_marks.begin(),m_marks.end(),m);
	if(im != m_marks.end() && *im == m)
	{
		const unsigned int index = im - m_marks.begin();

		for(Specs_t::iterator i = m_specs.begin(); i != m_specs.end(); ++i)
		{
			m_rels[i-m_specs.begin()][index]->UpdateRelation(ev);
		}
	}
}

//=====================================================================================//
//                           void VisMap::UpdateSpectator()                            //
//=====================================================================================//
void VisMap::UpdateSpectator(Spectator *s, VisUpdateEvent_t ev)
{
	Specs_t::iterator is = std::lower_bound(m_specs.begin(),m_specs.end(),s);
	if(is != m_specs.end() && *is == s)
	{
		const unsigned int index = is - m_specs.begin();

		for(Marks_t::iterator i = m_marks.begin(); i != m_marks.end(); ++i)
		{
			m_rels[index][i-m_marks.begin()]->UpdateRelation(ev);
		}
	}
}

//=====================================================================================//
//                  VisMap::SpecIterator_t VisMap::SpecBegin() const                   //
//=====================================================================================//
VisMap::SpecIterator_t VisMap::SpecBegin(Marker *m, VisParams f) const
{
	return SpecIterator_t(m,m_specs.begin(),m_specs.end(),f);
}

//=====================================================================================//
//                   VisMap::SpecIterator_t VisMap::SpecEnd() const                    //
//=====================================================================================//
VisMap::SpecIterator_t VisMap::SpecEnd() const
{
	return SpecIterator_t();
}

//=====================================================================================//
//                  VisMap::MarkIterator_t VisMap::MarkBegin() const                   //
//=====================================================================================//
VisMap::MarkIterator_t VisMap::MarkBegin(Spectator *s, VisParams f) const
{
	return MarkIterator_t(s,m_marks.begin(),m_marks.end(),f);
}

//=====================================================================================//
//                   VisMap::MarkIterator_t VisMap::MarkEnd() const                    //
//=====================================================================================//
VisMap::MarkIterator_t VisMap::MarkEnd() const
{
	return MarkIterator_t();
}

//=====================================================================================//
//                           Relation *VisMap::GetRelation()                           //
//=====================================================================================//
Relation *VisMap::GetRelation(Spectator *s, Marker *m)
{
	Specs_t::iterator si = std::lower_bound(m_specs.begin(),m_specs.end(),s);
	Marks_t::iterator mi = std::lower_bound(m_marks.begin(),m_marks.end(),m);
	return m_rels[si-m_specs.begin()][mi-m_marks.begin()];
}

//=====================================================================================//
//                     const Relation *VisMap::GetRelation() const                     //
//=====================================================================================//
const Relation *VisMap::GetRelation(Spectator *s, Marker *m) const
{
	Specs_t::const_iterator si = std::lower_bound(m_specs.begin(),m_specs.end(),s);
	Marks_t::const_iterator mi = std::lower_bound(m_marks.begin(),m_marks.end(),m);
	return m_rels[si-m_specs.begin()][mi-m_marks.begin()];
}

//=====================================================================================//
//                           void VisMap::ResetVisibility()                            //
//=====================================================================================//
void VisMap::ResetVisibility(player_type p)
{
	for(Marks_t::iterator i = m_marks.begin(); i != m_marks.end(); ++i)
	{
		(*i)->ResetVisibility(p);
	}
}

//=====================================================================================//
//                              VisMap *VisMap::GetInst()                              //
//=====================================================================================//
VisMap *VisMap::GetInst()
{
	static VisMap instance;
	return &instance;
}
