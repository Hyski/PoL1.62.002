#pragma once

#include "Sizer.h"

namespace PoL
{

//=====================================================================================//
//                       class Persistent : public MlRefCounted                        //
//=====================================================================================//
class Persistent : public MlRefCounted
{
public:
	typedef unsigned int Id_t;

private:
	Id_t m_id;		///< Уникальный идентификатор

public:
	Persistent(Id_t id) : m_id(id) {}
	virtual ~Persistent() {}

	/// Возвращает уникальный идентификатор
	Id_t getId() const { return m_id; }
	
	/// Возвращает тип объекта
	virtual SizerTypes getType() const = 0;

	/// Сохранить состояние объекта
	virtual void store(mll::io::obinstream &) const = 0;
	/// Восстановить состояние объекта
	virtual void restore(mll::io::ibinstream &) = 0;

	/// Сбросить состояние на начальное
	virtual void reset() = 0;

	/// Вывести состояние объекта в лог
	virtual void dumpState(std::ostream &out) const = 0;

	/// Возвращает системное имя объекта
	const std::string &getName() const;
};

typedef MlHandle<Persistent> HPersistent;

/// Вспомогательные классы
template<typename T> struct RemoveRef;
template<typename T> struct RemoveRef<T &> { typedef T result; };

/// Класс, сохраняющий и восстанавливающие переданное значение в виде long
template<typename T> class AsInt
{
	T m_valueRef;

public:
	AsInt(T vref) : m_valueRef(vref) {}

	mll::io::obinstream &doOutput(mll::io::obinstream &out)
	{
		long val = static_cast<long>(m_valueRef);
		return out << val;
	}

	mll::io::ibinstream &doInput(mll::io::ibinstream &in)
	{
		long val;
		in >> val;
		m_valueRef = static_cast<typename RemoveRef<T>::result>(val);
		return in;
	}
};

/// Функции, служащие для упрощения инстанцирования шаблона AsInt
template<typename T> AsInt<T &> asInt(T &v) { return AsInt<T&>(v); }
template<typename T> AsInt<const T &> asInt(const T &v) { return AsInt<const T &>(v); }

/// Операторы ввода/вывода
template<typename T> mll::io::obinstream &operator<<(mll::io::obinstream &out, AsInt<T &> val)
{
	return val.doOutput(out);
}

template<typename T> mll::io::ibinstream &operator>>(mll::io::ibinstream &in, AsInt<T &> val)
{
	return val.doInput(in);
}

}
