#if !defined(__HANDLE_H_INCLUDED_8093570542662288__)
#define __HANDLE_H_INCLUDED_8093570542662288__

#include <stdexcept>
#include <assert.h>

class MlRefCounted;
class MlObject;

//=====================================================================================//
//                                  class MlRCHandle                                   //
//=====================================================================================//
//! Базовый класс умного указателя с подсчетом ссылок
/*! Реализует функциональность умного указателя для автоматизации подсчета ссылок.
 *  Использует класс MlRefCounted. */
template<typename T>
class MlHandle
{
	T *m_res;

	void _doAddRef() const { if(m_res) m_res->addRef(); }
	void _doRelease() const { if(m_res) m_res->release(); }

	class Tester { void operator delete(void *); };

public:
	//! \name Конструкторы
	//! @{

	//! Конструктор по умолчанию
	/*! Инициализирует указатель нулем */
	MlHandle() : m_res(0) {}

	//! Конструктор копирования
	/*! Берет указатель из переданного объекта. Инкрементирует счетчик ссылок, 
	 *  если он отличен от нуля */
	MlHandle(const MlHandle &h) : m_res(h.m_res) { _doAddRef(); }

	//! Конструктор из указателя
	/*! Инициализирует указатель переданным значением. Инкрементирует счетчик
	 *  ссылок, если оно отлично от нуля */
	MlHandle(T *res) : m_res(res) { _doAddRef(); }
	//! @}

	//! Деструктор
	/*! Декрементирует счетчик ссылок, если хранимый указатель не нуль */
	~MlHandle() { _doRelease(); }

	//! Возвращает хранящийся указатель
	T *get() const { return m_res; }

	T *operator->() const { return m_res; }
	T &operator *() const { return *m_res; }

	bool operator!() const { return !m_res; }
	operator Tester *() const { return reinterpret_cast<Tester *>(m_res); }

	//! \name Операторы присваивания
	//! @{
	MlHandle &operator=(const MlHandle &h);
	MlHandle &operator=(T *res);
	bool valid() const { return m_res != 0; }
	//! @}

	//! \name Операторы сравнения
	//! @{
	bool operator==(const MlHandle &h) const { return get() == h.get(); }
	bool operator!=(const MlHandle &h) const { return !(get() == h.get()); }
	//! @}

	MlHandle &reset(T *res = 0) { return operator=(res); }

	MlHandle &swap(MlHandle &h)
	{
		std::swap(m_res,h.m_res);
		return *this;
	}
};

//=====================================================================================//
//                             MlBaseHandle::operator=();                              //
//=====================================================================================//
template<typename T>
inline MlHandle<T> &MlHandle<T>::operator=(const MlHandle &h)
{
	if(m_res != h.m_res)
	{
		_doRelease();
		m_res = h.m_res;
		_doAddRef();
	}
	return *this;
}

//=====================================================================================//
//                             MlBaseHandle::operator=();                              //
//=====================================================================================//
template<typename T>
inline MlHandle<T> &MlHandle<T>::operator=(T *res)
{
	if(m_res != res)
	{
		_doRelease();
		m_res = res;
		_doAddRef();
	}
	return *this;
}

//=====================================================================================//
//                               struct _HandleDataBase                                //
//=====================================================================================//
template<typename T>
class _HandleDataBase
{
	T *m_ptr;
public:
	void _set(T *ptr)
	{
		m_ptr = ptr;
	}

	T *_get() const
	{
		return m_ptr;
	}

	void swap(_HandleDataBase &h)
	{
		std::swap(m_ptr,h.m_ptr);
	}
};

//=====================================================================================//
//                                 struct _HandleData                                  //
//=====================================================================================//
template<typename T>
struct _HandleData : public _HandleDataBase<T>
{
	_HandleData() { _set(0); }
	_HandleData(T *ptr) { _set(ptr); doAddRef(); }
	_HandleData(const _HandleData &rhs) { _set(rhs._get()); doAddRef(); }
	~_HandleData() { doRelease(); }

	void doAddRef() const { if(_get()) _get()->addRef(); }
	void doRelease() const { if(_get()) _get()->release(); }

	_HandleData &operator=(const _HandleData &rhs)
	{
		if(rhs._get() != _get())
		{
			doRelease();
			_set(rhs._get());
			doAddRef();
		}
		return *this;
	}

	_HandleData &operator=(T *rhs)
	{
		if(rhs != _get())
		{
			doRelease();
			_set(rhs);
			doAddRef();
		}
		return *this;
	}

};

//=====================================================================================//
//                                  struct _CheckType                                  //
//=====================================================================================//
template<typename T>
struct _CheckType
{
	_CheckType() {}
	_CheckType(MlObject *obj) { _doCheckType(obj); }

	static void _doCheckType(MlObject *ptr)
	{
		assert( !ptr || can(rtti_object_info(ptr)).downcast_to(T::rtti_static()) );
	}
};

//=====================================================================================//
//                                struct _HandleHelper                                 //
//=====================================================================================//
template<typename T>
struct _HandleHelper : private _HandleData<T>
{
	_HandleHelper() {}
	_HandleHelper(T *ptr) : _HandleData<T>(ptr) {}

	T *get() const { return _get(); }
	void reset(T *ptr = 0)
	{
		if(ptr != _get())
		{
			doRelease();
			_set(ptr);
			doAddRef();
		}
	}

	T *operator->() const { return _get(); }
	T &operator*() const { return *_get(); }

	bool operator==(const _HandleHelper<T> &rhs) const { return _get() == rhs._get(); }
	bool operator==(const T *rhs) const { return _get() == rhs; }
	bool operator<(const _HandleHelper<T> &rhs) const { return _get() < rhs._get(); }

	_HandleHelper &operator=(T *ptr)
	{
		return _HandleData::operator =(ptr);
	}

	bool valid() const { return _get() != 0; }

	void swap(_HandleHelper &h)
	{
		_HandleDataBase<T>::swap(h);
	}

};

template<typename T> class MlObjHandle;

//=====================================================================================//
//            struct MlObjHandle<MlObject> : public _HandleHelper<MlObject>            //
//=====================================================================================//
//! Умный указатель на класс, производный от MlObject
template<>
class MlObjHandle<MlObject> : public _HandleHelper<MlObject>
{
	class Tester { void operator delete(void *); };

public:
	MlObjHandle() {}
	MlObjHandle(int zero) : _HandleHelper<MlObject>(0) { assert( zero == 0 ); }
	MlObjHandle(MlObject *ptr) : _HandleHelper<MlObject>(ptr) {}

	operator Tester *() const { return reinterpret_cast<Tester*>(get()); }

	MlObjHandle &swap(MlObjHandle &h)
	{
		_HandleHelper<MlObject>::swap(h);
		return *this;
	}
};

//=====================================================================================//
//         struct MlObjHandle : private _CheckType<T>, public _HandleHelper<T>         //
//=====================================================================================//
template<typename T>
class MlObjHandle : private _CheckType<T>, public _HandleHelper<T>
{
	class Tester { void operator delete(void *); };

public:
	MlObjHandle() {}
	MlObjHandle(int zero) : _HandleHelper<T>(0) { assert( zero == 0 ); }
	template<typename R> MlObjHandle(const MlObjHandle<R> &rhs)
		:	_CheckType<T>(rhs.get()), _HandleHelper<T>(static_cast<T*>(rhs.get())) {}
	template<typename R> MlObjHandle(R *ptr)
		:	_CheckType<T>(ptr), _HandleHelper<T>(static_cast<T*>(ptr)) {}
	MlObjHandle(T *ptr) : _HandleHelper<T>(ptr) {}
	MlObjHandle(const MlObjHandle &rhs) : _HandleHelper<T>(rhs) {}

	template<typename R>
	MlObjHandle &operator=(const MlObjHandle<R> &rhs)
	{
		if(rhs.get() != get())
		{
			if(rhs.get()) _doCheckType(rhs.get());
			reset(static_cast<T*>(rhs.get()));
		}
		return *this;
	}

	template<typename R>
	MlObjHandle &operator=(R *ptr)
	{
		if(ptr != get())
		{
			if(ptr) _doCheckType(ptr);
			reset(static_cast<T*>(ptr));
		}
		return *this;
	}

	MlObjHandle &operator=(const MlObjHandle &rhs)
	{
		if(rhs.get() != get())
		{
			reset(static_cast<T*>(rhs.get()));
		}
		return *this;
	}

	bool operator!() const { return !get(); }
	operator Tester *() const { return reinterpret_cast<Tester*>(get()); }

	bool operator==(const MlObjHandle<MlObject> &rhs) const { return get() == rhs.get(); }
	bool operator==(const MlObject *rhs) const { return get() == rhs; }

	operator MlObjHandle<MlObject>() const { return MlObjHandle<MlObject>(get()); }

	MlObjHandle &swap(MlObjHandle &h)
	{
		_HandleHelper<T>::swap(h);
		return *this;
	}
};

/// Для поддержки boost::bind
namespace boost
{

//=====================================================================================//
//                                  T *get_pointer()                                   //
//=====================================================================================//
template<typename T>
T *get_pointer(const MlHandle<T> &ptr)
{
	return ptr.get();
}

//=====================================================================================//
//                                  T *get_pointer()                                   //
//=====================================================================================//
template<typename T>
T *get_pointer(const MlObjHandle<T> &ptr)
{
	return ptr.get();
}

}

typedef MlObjHandle<MlObject> HObject;

#endif // !defined(__HANDLE_H_INCLUDED_8093570542662288__)