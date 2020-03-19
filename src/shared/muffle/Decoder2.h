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

	/// ���������� ������ �����
	virtual const Format &getFormat() const = 0;
	/// ����������� ��������� ��� ������������� � ����� ����� pos
	virtual void seekSample(size_t pos) = 0;
	/// ���������� ������ ����������� � �������
	virtual size_t getSampleCount() const = 0;
	/// ������������ � ��������� ����� ��������� ����� �����������.
	virtual void decode(size_t amount, DecodeBuffer *buffer) = 0;

	/// ���������� ���������� �������� Decoder
	static long getInstanceCount();
};

typedef Ptr<Decoder> HDecoder;

}
