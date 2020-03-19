#pragma once

#include "format.h"
#include "Stream.h"
#include "Decoder2.h"

namespace Muffle2
{

//=====================================================================================//
//                         class WaveDecoder : public Decoder                          //
//=====================================================================================//
class WaveDecoder : public Decoder
{
	Stream m_stream;
	ibinstream &m_in;
	Format m_format;
	size_t m_length;
	std::ios::pos_type m_startPos;
	size_t m_currentPos;

public:
	WaveDecoder(Stream stream);

	/// ���������� ������ �����
	virtual const Format &getFormat() const { return m_format; }
	/// ����������� ��������� ��� ������������� � ����� ����� pos
	virtual void seekSample(size_t pos);
	/// ���������� ������ ����������� � �������
	virtual size_t getSampleCount() const { return m_length; }
	/// ������������ � ��������� ����� ��������� ����� �����������.
	virtual void decode(size_t amount, DecodeBuffer *buffer);
};

}
