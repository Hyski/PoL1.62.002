#pragma once

#include "ActivityFactory.h"

class BaseEntity;

//=====================================================================================//
//                                class TracingLOFCheck                                //
//=====================================================================================//
// класс кот производит трассировку для проверки линии видимости
class TracingLOFCheck
{
	ActivityFactory::shoot_info_s m_info;
    bool  m_fprint;

public:
	TracingLOFCheck(const ActivityFactory::shoot_info_s &info, bool fprint);

    bool IsExist(BaseEntity* entity, const point3& from, const point3& to);
	void PrintMessage(BaseEntity *entity);
};
