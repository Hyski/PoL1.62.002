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
	/// Возвращает формат буфера
	virtual const Format &getFormat() const = 0;
	/// Лочит буфер. Размер и положение лочащегося куска определяется автоматически
	//virtual SoundBufferLock lock() = 0;
};

typedef Ptr<Buffer> HBuffer;

}
