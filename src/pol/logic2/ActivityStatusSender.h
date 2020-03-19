#pragma once

#include "EntityAux.h"
#include "EntityRef.h"

//=====================================================================================//
//                 class ActivityStatusSender : private EntityObserver                 //
//=====================================================================================//
// �������� ��������� ��������
class ActivityStatusSender : private EntityObserver
{
	PoL::BaseEntityRef m_actor;

public:
	ActivityStatusSender(BaseEntity* actor);
	~ActivityStatusSender();

private:
	void Update(subject_t subj, event_t event, info_t info);
};
