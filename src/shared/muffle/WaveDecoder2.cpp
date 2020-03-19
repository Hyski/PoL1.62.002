#include "precomp.h"
#include "SafeAlloc2.h"
#include "WaveDecoder2.h"
#include "DecodeBuffer2.h"

namespace Muffle2
{

namespace WaveDecoder2Details
{
#define MKCHUNKNAME(D,C,B,A)	((unsigned int(A)<<24)+(unsigned int(B)<<16)+(unsigned int(C)<<8)+(unsigned int(D)))
	const unsigned int RIFFChunkName = MKCHUNKNAME('R','I','F','F');
	const unsigned int WAVEChunkName = MKCHUNKNAME('W','A','V','E');
	const unsigned int fmtChunkName  = MKCHUNKNAME('f','m','t',' ');
	const unsigned int dataChunkName = MKCHUNKNAME('d','a','t','a');
#undef MKCHUNKNAME
}

using namespace WaveDecoder2Details;

static const size_t DecodeTempBufferSize = 12*1024;

//=====================================================================================//
//                           void WaveDecoder::readChunkId()                           //
//=====================================================================================//
unsigned int readChunkId(mll::io::ibinstream &in)
{
	unsigned int ckId;
	in >> ckId;
	return ckId;
}

//=====================================================================================//
//                          unsigned WaveDecoder::readChunk()                          //
//=====================================================================================//
unsigned int readChunk(mll::io::ibinstream &in, unsigned int mustBe)
{
	unsigned int ckId = readChunkId(in);

	unsigned int size;
	in >> size;

	if(ckId == mustBe)
	{
		return size;
	}
	else
	{
		in.stream().seekg(size,std::ios::cur);
		return 0;
	}
}

//=====================================================================================//
//                             WaveDecoder::WaveDecoder()                              //
//=====================================================================================//
WaveDecoder::WaveDecoder(Stream stream)
:	m_stream(stream),
	m_in(m_stream.bin())
{
	if(!readChunk(m_in,RIFFChunkName)) throw mll::debug::exception("No riff");
	if(readChunkId(m_in) != WAVEChunkName) throw mll::debug::exception("No wave");

	{
		unsigned int size = readChunk(m_in,fmtChunkName);
		if(!size) throw mll::debug::exception("No fmt");

		if(sizeof(WAVEFORMAT) > size) throw mll::debug::exception("Unknown fmt chunk");

		std::vector<char> buffer(size);
		m_in >> mll::io::array(buffer.begin(),buffer.end());

		WAVEFORMAT *wfmt = reinterpret_cast<WAVEFORMAT*>(&buffer[0]);
		if(wfmt->wFormatTag != WAVE_FORMAT_PCM)
		{
			throw mll::debug::exception("Unknown RIFF data format");
		}
		if(wfmt->nBlockAlign/wfmt->nChannels != sizeof(short))
		{
			throw mll::debug::exception("Unsupported bits per sample count");
		}

		m_format.rate = wfmt->nSamplesPerSec;
		m_format.channels = wfmt->nChannels;
	}

	{
		size_t size = 0;
		while(!size) size = readChunk(m_in,dataChunkName);
		m_startPos = m_in.stream().tellg();
		m_length = size / ( sizeof(short) * m_format.channels );
	}

	m_currentPos = 0;
}

//=====================================================================================//
//                           long WaveDecoder::seekSample()                            //
//=====================================================================================//
void WaveDecoder::seekSample(size_t pos)
{
	m_currentPos = pos;
	const size_t block = m_format.channels * sizeof(short);
	std::ios::pos_type rpos = m_startPos;
	rpos += pos*block;
	m_in.stream().seekg(rpos);
}

//=====================================================================================//
//                             void WaveDecoder::decode()                              //
//=====================================================================================//
void WaveDecoder::decode(size_t amount, DecodeBuffer *buffer)
{
	MUFFLE_SYNCHRONIZED_THIS;

	assert( amount <= m_length - m_currentPos );
	if(!amount) return;

	const size_t block = m_format.channels * sizeof(short);
	short tbuffer[DecodeTempBufferSize]; // Временный буфер для декодирования (на 24Кб)

	while(amount)
	{
		const size_t size = std::min(amount, DecodeTempBufferSize/m_format.channels);
		m_in.stream().read(reinterpret_cast<char *>(tbuffer),size*block);
		m_currentPos += size;
		amount -= size;
		buffer->feed(tbuffer,size);
	}
}


}
