#pragma once

#include "../character.h"

class point3;
class KeyAnimation;

void TraceGrenade(const point3 &, const point3 &, KeyAnimation *, CharacterPool::Handle, float = 10.0f);
void PolTraceGrenade(const point3 &origin, const point3 &target, KeyAnimation *anim, CharacterPool::Handle, float maxTime);