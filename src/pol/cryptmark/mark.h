#pragma once

namespace SecurityMark
{
/// Основной класс исключений для класса Mark
MLL_MAKE_EXCEPTION(MarkException,mll::debug::exception);
	/// Исключение: не найден маркер (в файле)
	MLL_MAKE_EXCEPTION( CanNotFindMark_MarkException, MarkException);
	/// Исключение: не найдено место для маркера (в файле)
	MLL_MAKE_EXCEPTION( CanNotFindPlaceForMark_MarkException, MarkException);
	/// Исключение: файл не маркирован
	MLL_MAKE_EXCEPTION( FileNotMarked_MarkException, MarkException);

/// Класс - предназначен для маркировки файлов
class Mark
{
private:
/// Размер блока, для поблочного чтения/записи файлов
	static const int FILE_BLOCK  = 512;

public:
/// Максимальная длина маркера без служебной информации (со служебной - 256)
	static const int MAX_MARK_LEN = 128;

/// Максимальная длина строки с GUID'ом
	static const int MAX_GUID_STR_LEN = 39;

/// Максимальная длина строки с именем компьютера
	static const int MAX_COMP_STR_LEN = MAX_COMPUTERNAME_LENGTH+1;

/// Метка(ключ) - идентификатор маркера в файле
	static const std::string MARK_KEY;

/// Дефолтный маркер
	static const std::string MARK_DEF;

/// Строка для щифрования маркера xor'ом - 128 байт
	static const std::string CRYPT_STR;

///@{ Структура для хранения маркера
	struct MarkData
	{
		/// Пользовательская информамция
		/** максимальная длина строки - MAX_MARK_LEN */
		char m_label [MAX_MARK_LEN    ];

		/// Строка с GUID'ом
		/** максимальная длина строки - MAX_GUID_STR_LEN */
		char m_guid  [MAX_GUID_STR_LEN];

		/// Имя компьютера на котором файл отмечен
		/** максимальная длина строки - MAX_COMP_STR_LEN */
		char m_comp  [MAX_COMP_STR_LEN];

		/// Дата создания маркера
		time_t m_date;
	};
/*@}*/

/// Конструктор - созданный обьект предназначен для проверки метки в файле
	Mark(std::istream & fin);

/// Конструктор - созданный обьект предназначен для инициализации маркера пользовательской и служебной информацией
	Mark(const std::string & mrk);

/// Функция записи маркера в выходной файл(fout)
	/** Входные параметры:
	  * - fin - входной поток
	  * - fout - выходной поток
	  * - stream - <true> если входной и выходной потоки - один и тотже файл */
	void setMark(std::istream & fin,std::ostream & fout,bool stream);

/// Функция возврата структуры с маркером
	const MarkData & getMark() const;

private:
/// Шифровака/Расшифровка маркера
	void crypt();

/// Поиск в файле (fin) смещения маркера
	/** Возвращаемые значения:
	    - 0 - Метка найдена
	    - 1 - Метка не найдена */
	bool findMarkPos(std::istream & fin);

/// Получение строки с GUID'ом
	static std::string genGuidString();

/// Найденное смещение маркера в файле
	std::streampos	m_FileMarkPos;

/// Переменная с маркером
	MarkData	m_Mark;	
};

}