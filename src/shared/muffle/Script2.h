#pragma once

namespace Muffle2
{

//=====================================================================================//
//                                    class Script                                     //
//=====================================================================================//
class Script : public Muffle::IScript
{
	Muffle::ScriptInfo m_info;

public:
	Script(const Muffle::ScriptInfo &info) : m_info(info) {}

	/// ���������� ���������� � �������
	virtual const Muffle::ScriptInfo &getInfo() const { return m_info; }
};

}

