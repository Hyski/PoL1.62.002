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
	const Locker &m_locker;			///< ����� � ����������� � ������
	size_t m_sampleSize;			///< ������ ������ � short'��
	size_t m_lastSampleWritten;		///< ���������� ����� ���������� ����������� ������

public:
	DecodeBuffer(const Locker &l, size_t sampleSize);

	/// ����������� ������ � �����
	void feed(const short *data, size_t sampleCount);
	/// �������� ���������� ������
	void zero(size_t sampleCount);

private:
	typedef std::pair<short *, size_t> Section_t;

	/// ���������� ��������� ������ ��� ������
	Section_t getNextSection(size_t size);
};

}
