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

	/// Создать контекст
	static ContextId_t createContext(const std::string &holder, const std::string &actor);
	/// Уничтожить контекст
	static void removeContext(ContextId_t);

	/// Возвращает имя актера
	static const std::string &getActor();
	/// Возвращает имя квестхолдера
	static const std::string &getHolder();
};

}