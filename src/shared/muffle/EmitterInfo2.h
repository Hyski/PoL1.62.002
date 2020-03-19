#pragma once

#include "BufferCtrl2.h"

namespace Muffle2
{

//=====================================================================================//
//                                 struct EmitterInfo                                  //
//=====================================================================================//
struct EmitterInfo
{
	vec position;
	vec velocity;
	vec coneOrientation;
	HBufferCtrl ctrl;

	EmitterInfo()
	:	position(0.0f,0.0f,0.0f),
		velocity(0.0f,0.0f,0.0f),
		coneOrientation(0.0f,0.0f,1.0f)
	{
	}
};

}
