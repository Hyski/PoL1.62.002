#if !defined(__OBJECT_H_INCLUDED_7552370787048739__)
#define __OBJECT_H_INCLUDED_7552370787048739__

#include <mutual/novtable.h>
#include <mutual/rtti.h>

//=====================================================================================//
//                                 class MlRefCounted                                  //
//=====================================================================================//
//! Базовый класс для объектов со счетчиком ссылок
/*! Класс предоставляет своим наследникам систему подсчета ссылок.
 *  Этот класс используется в системе менеджмента объектов.
 */
class MlRefCounted
{
protected:
	virtual ~MlRefCounted() {}

private:
	mutable unsigned m_refCount;

	// Функции объявлены приватными, чтобы производный класс не мог их вызывать,
	//   но он может переопределить их (www.cuj.com, Conversations: Virtually Yours)
	virtual void load() const { }
	//! Уничтожает объект
	/*! При достижении счетчиком ссылок нуля вызывается эта функция.
	 *  Поведение по-умолчанию заключается в вызове <tt>delete this;</tt>.
	 *  Производные классы могут переопределить поведение этой функции, но вызвать
	 *  ее они не могут. */
	virtual void unload() const { delete this; }

public:
	MlRefCounted() : m_refCount(0) {}
	MlRefCounted(const MlRefCounted &) : m_refCount(0) {}

	//! \brief Инкрементирует счетчик ссылок
	/*! \return новое значение счетчика ссылок */
	inline void addRef() const { if(!m_refCount++) load(); }

	//! \brief Декрементирует счетчик ссылок
	/*! В случае, если счетчик ссылок достигнет нуля, то будет вызвана
	 *  функция \link MlObject::unload() \endlink
	 *  \return новое значение счетчика ссылок */
	inline void release() const { if(!--m_refCount) unload(); }

	//! \brief Возвращает счетчик ссылок
	inline unsigned int getRefCount() const { return m_refCount; }

	MlRefCounted &operator=(const MlRefCounted &rc) { return *this; }
};

//=====================================================================================//
//                                   class MlObject                                    //
//=====================================================================================//
//! Общий интерфейс для классов с информацией о типе и подсчетом ссылок
class ML_NOVTABLE MlObject : public MlRefCounted
{
public:
	ML_RTTI_ROOT(MlObject);

protected:
	MlObject() {}
	virtual ~MlObject() {}
};

#endif // !defined(__OBJECT_H_INCLUDED_7552370787048739__)