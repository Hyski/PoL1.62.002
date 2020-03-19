#pragma once

#include <string>

namespace SecurityMark
{

/// Класс singelton'а
class CryptMark
{
/// Конструктор
	CryptMark();

/// Деструктор
	~CryptMark(){};

/// Расшифровка маркера
	void decrypt();

/// Пользовательсккая информация
	std::string m_label;

/// GUID
	std::string m_guid;

/// Имя компьютера
	std::string m_comp;

/// Время
	time_t m_time;

public:
/// Проверка файла на наличие маркировки
	/** Выходные параметры:
	  * - true  - файл помечен
	  * - false - файл не помечен */
	bool isMarked() const;

/// Получение метки
	const std::string &getLabel() const {return m_label;}

/// Получение GUID'а
	const std::string &getGuid() const {return m_guid;}

/// Получение ииени компьютера на котором производилась маркировка
	const std::string &getComp() const {return m_comp;}

/// Получение времени маркировки
	const time_t &getTime() const {return m_time;}

/// Создание/возврат обьекта CryptMark (singelton'а)
	static const CryptMark *getInstance();

/// Метод предназначен для форсирования включения строки в exe'ик
	static int forceInstantiate();
};

/// Форсирование включения строки в exe'ик
static const int __instantiator_of_mark_for_security__ = CryptMark::forceInstantiate();

}