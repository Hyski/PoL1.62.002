#pragma once

namespace PoL
{

//=====================================================================================//
//                                   class EntityRef                                   //
//=====================================================================================//
template<typename T>
class EntityRef : EntityObserver
{
	T *m_entity;

public:
	EntityRef() : m_entity(0) {}
	EntityRef(T *entity) : m_entity(entity) { doAttach(); }
	EntityRef(const EntityRef &eref) : m_entity(eref.m_entity) { doAttach(); }
	~EntityRef()
	{
		doDetach();
	}

	EntityRef &operator=(const EntityRef &eref)
	{
		reset(eref.get());
		return *this;
	}

	void reset(T *entity = 0)
	{
		if(entity != m_entity)
		{
			doDetach();
			m_entity = entity;
			doAttach();
		}
	}

	bool valid() const
	{
		return m_entity != 0;
	}

	T *operator->() const
	{
		return m_entity;
	}

	T &operator*() const
	{
		return *m_entity;
	}

	T *get() const
	{
		return m_entity;
	}

private:
	void doAttach()
	{
		if(m_entity)
		{
			m_entity->Attach(this,EV_DESTROY);
		}
	}

	void doDetach()
	{
		if(m_entity)
		{
			m_entity->Detach(this);
		}
	}

	void Update(subject_t, event_t, info_t)
	{
		doDetach();
		m_entity = 0;
	}
};

typedef EntityRef<BaseEntity> BaseEntityRef;
typedef EntityRef<HumanEntity> HumanEntityRef;
typedef EntityRef<VehicleEntity> VehicleEntityRef;

}
