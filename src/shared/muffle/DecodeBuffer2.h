#pragma once

#include <boost/noncopyable.hpp>

namespace Muffle2
{

struct Locker;

//=====================================================================================//
//                                 class DecodeBuffer                                  //
//=====================================================================================//
class DecodeBuffer : boost::noncopyable
{
	const Locker &m_locker;			///< Локер с информацией о памяти
	size_t m_sampleSize;			///< Размер сэмпла в short'ах
	size_t m_lastSampleWritten;		///< Порядковый номер последнего записанного сэмпла

public:
	DecodeBuffer(const Locker &l, size_t sampleSize);

	/// Скопировать данные в буфер
	void feed(const short *data, size_t sampleCount);
	/// Обнулить содержимое буфера
	void zero(size_t sampleCount);

private:
	typedef std::pair<short *, size_t> Section_t;

	/// Возвращает следующую секцию для записи
	Section_t getNextSection(size_t size);
};

}
