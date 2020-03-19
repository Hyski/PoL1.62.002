#include "Precomp.h"
#include "VFile.h"


//---------- Лог файл ------------
#ifdef _DEBUG_SHELL
CLog vfile;
#define vfile vfile["vfile.log"]
#else*/
#define vfile /##/
#endif
//--------------------------------

//=====================================================================================//
//                                   VFile::VFile()                                    //
//=====================================================================================//
VFile::VFile()
{
	InZero();
}

//=====================================================================================//
//                                   VFile::VFile()                                    //
//=====================================================================================//
VFile::VFile(const char *pFileName)
{
	InZero();
	Load(pFileName);
}

//=====================================================================================//
//                                   VFile::VFile()                                    //
//=====================================================================================//
VFile::VFile(FILE* hFile,const char *pFileName)
{
	InZero();
	Load(hFile,pFileName);
}

//=====================================================================================//
//                                   VFile::VFile()                                    //
//=====================================================================================//
VFile::VFile(const unsigned char *pData,unsigned int iSize,const char *pFileName)
{
	InZero();
	Load(pData,iSize,pFileName);
}

//=====================================================================================//
//                                   VFile::~VFile()                                   //
//=====================================================================================//
VFile::~VFile()
{
	Close();
}

//=====================================================================================//
//                                 bool VFile::Load()                                  //
//=====================================================================================//
bool VFile::Load(const char *pFileName)
{
	FILE *hFile;

	if(pFileName && strlen(pFileName) && ((hFile = fopen(pFileName,"rb"))!=0))
	{
		if(Load(hFile,pFileName))
		{
			fclose(hFile);
			return true;
		}
		fclose(hFile);
	}
	else
	{
		Close();
	}

	return false;
}

//=====================================================================================//
//                                 bool VFile::Load()                                  //
//=====================================================================================//
bool VFile::Load(FILE* hFile,const char *pFileName)
{
	//	очищаем переменные
	Close();
	//	определяем длину файла
	fseek(hFile,0L,SEEK_END);
	m_iFileSize = ftell(hFile);
	rewind(hFile);
	//	выделяем буфер
	if(m_iFileSize)
	{
		m_pData.resize(m_iFileSize);

		if(fread(&m_pData[0], m_iFileSize, 1, hFile))
		{
			//	запоминаем имя файла
			if(pFileName) m_pFileName = pFileName;
			m_iErrorCode = VFILE_OK;

			return true;
		}
	}

	return false;
}

//=====================================================================================//
//                                 bool VFile::Load()                                  //
//=====================================================================================//
bool VFile::Load(const unsigned char *pData,unsigned int iSize,const char *pFileName)
{
	//	очищаем переменные
	Close();
	//	грузим
	if(!((iSize == 0) || (pData == 0)))
	{
		m_pData.resize(iSize);

		memcpy(&m_pData[0],pData,iSize);
		m_iFileSize = iSize;
		//	запоминаем имя файла
		if(pFileName) m_pFileName = pFileName;
		m_iErrorCode = VFILE_OK;

		return true;
	}

	return false;
}

//=====================================================================================//
//                           const char* VFile::Extension()                            //
//=====================================================================================//
const char* VFile::Extension(void)
{
	std::string::size_type pos = m_pFileName.rfind('.');
	if(pos != std::string::npos) return &m_pFileName[pos];
	return 0;
}

//=====================================================================================//
//                                 void VFile::Seek()                                  //
//=====================================================================================//
//	перемещение указателя в любое место
void VFile::Seek(int offset,int origin)
{
	if(m_iFileSize == 0)
	{
		m_iFilePos = 0;
		m_iErrorCode = VFILE_NONE;
		return;
	}
	switch(origin)
	{
	case VFILE_END:
		offset = m_iFileSize+offset;
		break;
	case VFILE_CUR:
		offset = m_iFilePos+offset;
		break;
	case VFILE_SET:
		break;
	}
	if(offset<0)
	{
		m_iFilePos = 0;
		m_iErrorCode = VFILE_SOF;
		return;
	}
	if(offset>m_iFileSize)
	{
		m_iFilePos = m_iFileSize;
		m_iErrorCode = VFILE_EOF;
		return;
	}
	m_iFilePos = offset;
	m_iErrorCode = VFILE_OK;
}

//=====================================================================================//
//                                void VFile::Rewind()                                 //
//=====================================================================================//
// перемещение указателя в начало файла
void VFile::Rewind(void)
{
	if(m_iFileSize != 0)
	{
		m_iFilePos = 0;
		m_iErrorCode = VFILE_OK;
	}
}

//=====================================================================================//
//                             unsigned int VFile::Read()                              //
//=====================================================================================//
// чтение в буфер; возвращает кол-во считанных байт
unsigned int VFile::Read(void *pBuff,unsigned int iSize)
{
	int cSize = 0;

	if(m_iFileSize != 0)
	{
		cSize = std::min(iSize,m_iFileSize-m_iFilePos);
		if(cSize == 0)
		{
			m_iErrorCode = VFILE_EOF;
		}
		else
		{
			memcpy(pBuff,&m_pData[m_iFilePos],cSize);
			m_iFilePos += cSize;
			if((m_iFilePos==m_iFileSize) && (cSize<iSize))
				m_iErrorCode = VFILE_EOF;
			else
				m_iErrorCode = VFILE_OK;
		}
	}

	return cSize;
}

//=====================================================================================//
//                           unsigned char VFile::ReadChar()                           //
//=====================================================================================//
// чтение одного символа
unsigned char VFile::ReadChar(void)
{
	if(m_iFileSize != 0)
	{
		if(m_iFilePos == m_iFileSize)
			m_iErrorCode = VFILE_EOF;
		else
		{
			m_iErrorCode = VFILE_OK;
			return m_pData[m_iFilePos++];
		}
	}

	return 0;
}

//=====================================================================================//
//                                void VFile::InZero()                                 //
//=====================================================================================//
void VFile::InZero(void)
{
	//Sleep(100);
	m_iFileSize		= 0;
	m_iFilePos		= 0;
	m_iErrorCode    = VFILE_NONE;
}

//=====================================================================================//
//                                 void VFile::Close()                                 //
//=====================================================================================//
void VFile::Close(void)
{
	vfile("VFile::Release(\"%s\");\n",m_pFileName);

	m_pFileName.clear();
	m_pData.clear();

	m_iFileSize = 0;
	m_iFilePos	= 0;
	m_iErrorCode = VFILE_NONE;
}