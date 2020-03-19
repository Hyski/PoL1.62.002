#pragma once

#include "Buffer2.h"

namespace Muffle2
{

//=====================================================================================//
//                         class StaticBuffer : public Buffer                          //
//=====================================================================================//
class StaticBuffer : public Buffer
{
	SoundBuffer m_buffer;

public:
	StaticBuffer();

	/// ���������� ����� ������, ��������� ��� ������
	virtual unsigned int getAvailableSpace() const;
	/// �������� ������ � �����
	virtual void feed(const short *data, unsigned int count);
};

}
