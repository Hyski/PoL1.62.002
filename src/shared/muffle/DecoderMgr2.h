#pragma once

#include "Decoder2.h"

namespace Muffle2
{

//=====================================================================================//
//                       class DecoderMgr : public Synchronized                        //
//=====================================================================================//
class DecoderMgr
{
public:
	/// —оздает декодер нужного типа. ≈сли не удаетс€ создать декодер, то возвращает 0
	static HDecoder createDecoder(const std::string &name);
};

}

