#pragma once

#include "Storage.h"

namespace Storage
{

//=====================================================================================//
//                         class Registry : boost::noncopyable                         //
//=====================================================================================//
/// Класс, реестр персистентных классов
class Registry : boost::noncopyable
{
protected:
	Registry() {}
	virtual ~Registry() {}
	
public:
	/// Возвращает указатель на экземпляр реестра
	static STORAGE_API Registry *instance();
	
	/// Зарегистрировать персистентный класс
	virtual void registerClass(PersistentClass *) = 0;
	/// Возвращает персистентный класс по имени
	virtual PersistentClass *getClass(const std::string &) const = 0;
};

}