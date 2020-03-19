#include "precomp.h"
#include "DDS.h"

static const unsigned int DDSMagicValue = 0x20534444;	// "DDS "

//=====================================================================================//
//                                     DDS::DDS()                                      //
//=====================================================================================//
DDS::DDS(VFile *f)
:	m_image(f)
{
	unsigned int magic = 0;
	f->Read(&magic,sizeof(magic));
	assert( magic == DDSMagicValue );
	
	f->Read(&m_ddesc.dwSize,sizeof(m_ddesc.dwSize));
	f->Read(reinterpret_cast<char*>(&m_ddesc)+sizeof(m_ddesc.dwSize),m_ddesc.dwSize - sizeof(m_ddesc.dwSize));
}