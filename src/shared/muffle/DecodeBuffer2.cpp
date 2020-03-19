#include "precomp.h"
#include "DecodeBuffer2.h"
#include "Locker2.h"
#include "SoundBuffer2.h"

namespace Muffle2
{

//=====================================================================================//
//                            DecodeBuffer::DecodeBuffer()                             //
//=====================================================================================//
DecodeBuffer::DecodeBuffer(const Locker &l, size_t sampleSize)
:	m_locker(l),
	m_sampleSize(sampleSize/2),
	m_lastSampleWritten(0)
{
}

//=====================================================================================//
//                              void DecodeBuffer::feed()                              //
//=====================================================================================//
void DecodeBuffer::feed(const short *data, size_t sampleCount)
{
	size_t samplesToCopy = sampleCount;

	while(samplesToCopy)
	{
		assert( samplesToCopy < 0x7FFFFFFF );
		Section_t sect = getNextSection(samplesToCopy);
		memcpy(sect.first,data,sizeof(short)*m_sampleSize*sect.second);
		data += sect.second*m_sampleSize;
		samplesToCopy -= sect.second;
	}
}

//=====================================================================================//
//                              void DecodeBuffer::zero()                              //
//=====================================================================================//
void DecodeBuffer::zero(size_t sampleCount)
{
	size_t samplesToCopy = sampleCount;

	while(samplesToCopy)
	{
		Section_t sect = getNextSection(samplesToCopy);
		memset(sect.first,0,sizeof(short)*m_sampleSize*sect.second);
		samplesToCopy -= sect.second;
	}
}

//=====================================================================================//
//                                static bool inside()                                 //
//=====================================================================================//
static bool inside(const short *ptr1, size_t size1, const short *ptr2, size_t size2, size_t f)
{
	return ptr2 >= ptr1 && ptr2 + size2*f <= ptr1 + size1*f;
}

//=====================================================================================//
//               DecodeBuffer::Section_t DecodeBuffer::getNextSection()                //
//=====================================================================================//
DecodeBuffer::Section_t DecodeBuffer::getNextSection(size_t size)
{
	assert( m_lastSampleWritten + size <= m_locker.size1 + m_locker.size2 );

	Section_t result;

	if(m_lastSampleWritten >= m_locker.size1)
	{
		result.first = m_locker.data2 + (m_lastSampleWritten-m_locker.size1)*m_sampleSize;
		result.second = size;
	}
	else
	{
		result.first = m_locker.data1 + m_lastSampleWritten*m_sampleSize;
		result.second = std::min(size, m_locker.size1 - m_lastSampleWritten);
	}

	assert( inside(m_locker.data1,m_locker.size1,result.first,result.second,m_sampleSize) ||
			inside(m_locker.data2,m_locker.size2,result.first,result.second,m_sampleSize) );

	m_lastSampleWritten += result.second;

	assert( m_lastSampleWritten <= m_locker.size1 + m_locker.size2 );

	return result;
}

}

