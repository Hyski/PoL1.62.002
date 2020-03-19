#include "precomp.h"
#include "Kernel2.h"
#include "FileMgmt.h"
#include "DecoderMgr2.h"
#include "OggDecoder2.h"
#include "WaveDecoder2.h"

namespace Muffle2
{

//=====================================================================================//
//                        static std::string checkExtensions()                         //
//=====================================================================================//
static std::string checkExtensions(const std::string &wave)
{
	Muffle::HFileMgmt fm = Muffle::FileMgmt::inst();

	if(fm->canCreateFile(wave.c_str()))
	{
		return wave;
	}
	else if(fm->canCreateFile((wave + ".ogg").c_str()))
	{
		return wave + ".ogg";
	}
	else if(fm->canCreateFile((wave + ".wav").c_str()))
	{
		return wave + ".wav";
	}
	else
	{
		return std::string();
	}
}

//=====================================================================================//
//                        static std::string getOpenFileName()                         //
//=====================================================================================//
static std::string getOpenFileName(const std::string &wave)
{
	std::string result = checkExtensions(Kernel::inst()->getSoundPathPrefix() + wave);

	if(result.empty())
	{
		result = checkExtensions(wave);
	}

	if(result.empty())
	{
		MLL_REPORT_WARNING("Muffle","Отсутствует звуковой файл",wave);
	}

	return result;
}

//=====================================================================================//
//                        HDecoder DecoderMgr::createDecoder()                         //
//=====================================================================================//
HDecoder DecoderMgr::createDecoder(const std::string &name)
{
	std::string realname = getOpenFileName(name);
	if(realname.empty()) return HDecoder();

	try
	{
		Stream stream = Muffle::FileMgmt::inst()->createFile(realname);

		long mark;
		stream.bin() >> mark;
		stream.stream().seekg(0);

		if(mark == 0x5367674F) // "OggS"
		{
			return Ptr<OggDecoder>::create(stream);
		}
		else if(mark == 0x46464952) // "RIFF"
		{
			return Ptr<WaveDecoder>::create(stream);
		}
	}
	catch(mll::debug::exception &e)
	{
		MLL_REPORT_WARNING("Muffle","Ошибка [" + std::string(e.what()) +  "] при открытии файла",realname);
		return HDecoder();
	}

	MLL_REPORT_WARNING("Muffle","Неизвестный формат файла",realname);

	return HDecoder();
}

}
