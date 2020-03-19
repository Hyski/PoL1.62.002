#pragma once

#include "StoreBox2.h"

namespace PoL
{

//=====================================================================================//
//                                    class Bureau2                                    //
//=====================================================================================//
class Bureau2
{
	typedef std::map<std::string,HStoreBox2> Boxes_t;
	Boxes_t m_boxes;

public:
	Bureau2();
	~Bureau2();

	/// Добавить бокс в бюро
	void addBox(HStoreBox2);
	/// Возвращает бокс с полным системным именем fullId
	HStoreBox2 getBox(const std::string &fullId);
	/// Возвращает есть ли такой ящик в бюро
	bool hasBox(const std::string &fullId) const;
	/// Возвращает бокс с системным именем id
	HStoreBox2 getBoxEx(const std::string &id);

	/// Сохранить состояние объекта в поток
	void store(mll::io::obinstream &) const;
	/// Восстановить состояние объекта из потока
	void restore(mll::io::ibinstream &);

	/// Возвращает экземпляр Bureau2
	static Bureau2 &inst();
};

}
