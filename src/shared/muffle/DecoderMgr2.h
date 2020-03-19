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
	/// ������� ������� ������� ����. ���� �� ������� ������� �������, �� ���������� 0
	static HDecoder createDecoder(const std::string &name);
};

}

