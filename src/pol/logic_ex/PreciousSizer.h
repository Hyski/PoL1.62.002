#pragma once

#include "Sizer.h"

namespace PoL
{

//=====================================================================================//
//                         class PreciousSizer : public Sizer                          //
//=====================================================================================//
class PreciousSizer : public Sizer
{
public:
	/// ���������� ��� ������
	virtual SizerTypes getEntryType(mll::io::ibinstream &in, const std::string &)
	{
		SizerTypes st;
		in >> asInt(st);
		return st;
	}

	/// ���������� ������
	virtual void skipEntry(mll::io::ibinstream &in, const std::string &)
	{
		size_t result;
		in >> asInt(result);
		in.stream().seekg(result,std::ios::cur);
	}
};

}
