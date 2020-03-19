#pragma once

#include "ThrowManager.h"

class BaseThing;

//=====================================================================================//
//                class CameraShieldThrowManager : public ThrowManager                 //
//=====================================================================================//
// менеджер для броска камеры или shield'a
class CameraShieldThrowManager  : public ThrowManager
{
    point3 m_to;
    float  m_end_time;

    BaseThing*  m_thing;
    player_type m_player;

public:
    CameraShieldThrowManager();
    ~CameraShieldThrowManager();

    void Init(HumanEntity* human, const point3& to);
    bool Run();
};
