#include "precomp.h"
#include "OggDecoder2.h"
#include "DecodeBuffer2.h"

namespace Muffle2
{

static const size_t DecodeTempBufferSize = 12*1024;

namespace OggDecoder2Details
{
//=====================================================================================//
//                                 size_t streamRead()                                 //
//=====================================================================================//
	size_t __cdecl streamRead(void *ptr, size_t size, size_t nmemb, void *vdatasource)
	{
		Stream *datasource = reinterpret_cast<Stream *>(vdatasource);

		datasource->stream().read(reinterpret_cast<char *>(ptr),size*nmemb);
		if(!datasource->stream().good())
		{
			datasource->stream().clear();
		}
		return datasource->stream().gcount()/size;
	}

//=====================================================================================//
//                                  int streamSeek()                                   //
//=====================================================================================//
	int __cdecl streamSeek(void *vdatasource, __int64 offset, int whence)
	{
		Stream *datasource = reinterpret_cast<Stream *>(vdatasource);

		std::ios::seekdir way;
		switch(whence)
		{
			case SEEK_CUR: way = std::ios::cur; break;
			case SEEK_END: way = std::ios::end; break;
			case SEEK_SET: way = std::ios::beg; break;
		}
		datasource->stream().seekg((int)offset,way);
		assert( datasource->stream().good() );
		if(datasource->stream().good()) return 0;
		return 1;
	}

//=====================================================================================//
//                                  int streamClose()                                  //
//=====================================================================================//
	int __cdecl streamClose(void *vdatasource)
	{
		Stream *datasource = reinterpret_cast<Stream *>(vdatasource);

		assert( datasource->stream().good() );
		return 0;
	}

//=====================================================================================//
//                                  long streamTell()                                  //
//=====================================================================================//
	long __cdecl streamTell(void *vdatasource)
	{
		Stream *datasource = reinterpret_cast<Stream *>(vdatasource);

		assert( datasource->stream().good() );
		return datasource->stream().tellg();
	}

	ov_callbacks mycbks = 
	{
		streamRead,
		streamSeek,
		streamClose,
		streamTell
	};
}

using namespace OggDecoder2Details;

//=====================================================================================//
//                              OggDecoder::OggDecoder()                               //
//=====================================================================================//
OggDecoder::OggDecoder(Stream stream)
:	m_stream(stream),
	m_in(stream.bin()),
	m_currentSection(0)
{
	switch(ov_open_callbacks(&m_stream,&m_file,0,0,mycbks))
	{
		case OV_EREAD : throw mll::debug::exception("- A read from media returned an error. ");
		case OV_ENOTVORBIS : throw mll::debug::exception("- Bitstream is not Vorbis data. ");
		case OV_EVERSION : throw mll::debug::exception("- Vorbis version mismatch. ");
		case OV_EBADHEADER : throw mll::debug::exception("- Invalid Vorbis bitstream header. ");
		case OV_EFAULT : throw mll::debug::exception("- Internal logic fault; indicates a bug or heap/stack corruption. ");
	}
	vorbis_info *vi = ov_info(&m_file,-1);
	m_format.rate = vi->rate;
	m_format.channels = vi->channels;
	m_length = (unsigned int)ov_pcm_total(&m_file,-1);
	m_currPos = 0;
}

//=====================================================================================//
//                              OggDecoder::~OggDecoder()                              //
//=====================================================================================//
OggDecoder::~OggDecoder()
{
	ov_clear(&m_file);
}

//=====================================================================================//
//                            unsigned OggDecoder::decode()                            //
//=====================================================================================//
void OggDecoder::decode(size_t amount, DecodeBuffer *buff)
{
	if(!amount) return;

	size_t block = m_format.channels*sizeof(short);

	short tbuff[DecodeTempBufferSize];
	//std::vector<char> tmp(amount*block);

	size_t rema = amount;

	while(rema > 0)
	{
		size_t tocopy = std::min(rema,DecodeTempBufferSize/m_format.channels);
		size_t count = 0;

		while(count < tocopy*block)
		{
			long result = ov_read(&m_file, reinterpret_cast<char *>(tbuff) + count, tocopy*block - count, 0, 2, 1, &m_currentSection);
			if(result > 0) count += result;
		}

		rema -= tocopy;
		buff->feed(tbuff,tocopy);
	}

	m_currPos += amount;
}

//=====================================================================================//
//                              void OggDecoder::reset()                               //
//=====================================================================================//
void OggDecoder::seekSample(size_t pos)
{
	ov_pcm_seek(&m_file,pos);
	m_currPos = pos;
}

}

