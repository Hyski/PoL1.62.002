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

	/// Возвращает объем буфера, доступный для записи
	virtual unsigned int getAvailableSpace() const;
	/// Записать данные в буфер
	virtual void feed(const short *data, unsigned int count);
};

}
