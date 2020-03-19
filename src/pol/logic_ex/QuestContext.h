#pragma once

namespace PoL
{

//=====================================================================================//
//                                 class QuestContext                                  //
//=====================================================================================//
class QuestContext
{
public:
	typedef int ContextId_t;

	/// ������� ��������
	static ContextId_t createContext(const std::string &holder, const std::string &actor);
	/// ���������� ��������
	static void removeContext(ContextId_t);

	/// ���������� ��� ������
	static const std::string &getActor();
	/// ���������� ��� ������������
	static const std::string &getHolder();
};

}