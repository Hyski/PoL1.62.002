#pragma once

#include "Ptr2.h"
#include "Synchronized2.h"

namespace Muffle2
{

//=====================================================================================//
//                         class Buffer : public Synchronized                          //
//=====================================================================================//
class Buffer : public Synchronized
{
public:
	/// ���������� ������ ������
	virtual const Format &getFormat() const = 0;
	/// ����� �����. ������ � ��������� ���������� ����� ������������ �������������
	//virtual SoundBufferLock lock() = 0;
};

typedef Ptr<Buffer> HBuffer;

}
