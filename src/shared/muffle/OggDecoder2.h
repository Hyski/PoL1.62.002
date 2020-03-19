#pragma once

#include "format.h"
#include "Decoder2.h"
#include <ogg/ogg.h>
#include <vorbis/vorbisfile.h>

namespace Muffle2
{

//=====================================================================================//
//                          class OggDecoder : public Decoder                          //
//=====================================================================================//
class OggDecoder : public Decoder
{
	Stream m_stream;
	ibinstream &m_in;
	Format m_format;
	size_t m_length;
	OggVorbis_File m_file;
	size_t m_currPos;
	int m_currentSection;

public:
	OggDecoder(Stream stream);
	virtual ~OggDecoder();

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
