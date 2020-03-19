#pragma once

#include "Ptr2.h"
#include "Buffer2.h"
#include "Synchronized2.h"

namespace Muffle2
{

class DecodeBuffer;

//=====================================================================================//
//                         class Decoder : public Synchronized                         //
//=====================================================================================//
class Decoder : public Synchronized
{
public:
	Decoder();
	~Decoder();

	/// Возвращает формат звука
	virtual const Format &getFormat() const = 0;
	/// Переместить указатель для декодирования в сэмпл номер pos
	virtual void seekSample(size_t pos) = 0;
	/// Возвращает размер аудиоданных в сэмплах
	virtual size_t getSampleCount() const = 0;
	/// Декодировать в указанный буфер указанный кусок аудиоданных.
	virtual void decode(size_t amount, DecodeBuffer *buffer) = 0;

	/// Возвращает количество объектов Decoder
	static long getInstanceCount();
};

typedef Ptr<Decoder> HDecoder;

}
