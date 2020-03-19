#pragma once

namespace PoL
{

//=====================================================================================//
//                                      class Box                                      //
//=====================================================================================//
class StoreBox2
{
	typedef BaseThing *HThing;
	typedef std::list<HThing> Things_t;

	std::string m_fullId;
	Things_t m_things;
	float m_maxWeight;
	float m_curWeight;

public:
	typedef Things_t::const_iterator Iterator_t;

public:
	StoreBox2();
	StoreBox2(const std::string &fullId, float maxWeight);
	~StoreBox2();

	/// Возвращает полное системное имя
	const std::string &getFullId() const { return m_fullId; }
	/// Возвращает максимальный вес предметов в ящике
	float getMaxWeight() const { return m_maxWeight; }
	/// Возвращает текущий вес предметов в ящике
	float getCurWeight() const { return m_curWeight; }

	Iterator_t thingsBegin() const;
	Iterator_t thingsEnd() const;

	/// Добавить предмет в ящик
	void add(BaseThing *);
	/// Убрать предмет из ящика
	void remove(BaseThing *);

	/// Сохранить состояние объекта в поток
	void store(mll::io::obinstream &) const;
	/// Восстановить состояние объекта из потока
	void restore(mll::io::ibinstream &);

	bool operator<(const StoreBox2 &box) const { return m_fullId < box.m_fullId; }

private:
	void updateCurWeight();
};

typedef boost::shared_ptr<StoreBox2> HStoreBox2;

}
