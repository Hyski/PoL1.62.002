#include "precomp.h"

#include "mark.h"

namespace SecurityMark
{

const std::string Mark::MARK_DEF  = " \x3e\xf9\x22\x4d\xcb\x86\x21\x96\x5a\xd9\x6c\x32\x50\xd1\x2c\x31\xd1\x1a\x36\xac\xde\xb9\x44\xfb\xc0\xe5\x85\x27\xaa\x19\x59\xa2\x9a\x67\x55\x36\x80\xf\x8e\x3c\xa2\x50\x14\xe\x3f\x7e\xbc\xbf\x72\x55\x23\x9\x5a\x8d\xef\x86\x78\xae\x58\x23\x58\x26\xe8\xd3\x74\x99\x7e\x80\x55\x78\x99\x47\xfb\xd5\xd3\xe4\x7f\x76\xac\x6b\xf7\x28\x89\x3a\x9a\x55\xfd\x2b\x25\xd9\x46\xe\x7d\x12\x1b\x56\x95\x36\xa5\x12\x92\xea\xcb\x1f\x2d\x10\xb2\x9c\x59\xe0\x84\x9f\x26\x3a\x71\x26\xe6\x3a\xf4\x51\x8e\xe\x58\xce\x5f\x4\x7a\x94\xc5\xe7\xd0\xd4\x7f\x8c\xaa\x2c\xa8\xa8\xb8\x1f\x15\xe4\xcd\xc2\xc9\x34\xe2\xb7\x86\x64\x5e\x5f\x7c\xf4\x95\x4c\x46\x25\x8e\xf5\xcb\x54\x9\xae\x63\x87\xc0\xd7\x3c\x46\xee\x53\xfb\xac\xe\x3c\xa6\xbd\xe5\xd4\xc1\xfa\xc2\xbf\xb7\x33\x5\x70\x7c\x9d\xde\xe2\x71\x24\x57\x88\x88\x2\x95\x86\xa8\x91\x5a\x21\x53\x5d\xcf\x74\x86\x7e\x81\x65\xe0\x25\xa9\xd8\x7\x74\xb0\x23\x48\x93\xf2\xc1\x7d\xa7\xc3\x4a\x34\x26\xaf\xa2\xd\x32\x6\x71\x64\x22\x99\xd4\x30\x1e\xbe\x53\x2b\xc\x9a\x12\x34\x5f\x9f\x4b\xf1\x2f\x53\xfb";
const std::string Mark::MARK_KEY  = "\xfe\x77\x6a\x2d\xfc\x75\xcb\x34\xa7\x79\xf5\x58";
const std::string Mark::CRYPT_STR = "\xf6\x50\x8e\xe0\xf5\xa\x82\xe7\x82\xc1\x80\xbd\xcc\xba\x4f\x60\xef\x72\x6\xe0\xc5\x28\x79\x9\xff\x9b\xf8\x42\x75\x15\x39\xb5\xbb\xa8\x92\xb0\xf5\xb8\x6b\x4b\xec\xba\xf4\x4c\x61\x3a\x13\x47\xa1\x23\xa1\xa\x59\x3b\xd5\x38\xac\xed\x0\x33\x83\x4b\x7b\x3f\x24\x53\xde\xe5\x7\x71\x73\x96\xe4\x46\xeb\x91\x11\xa8\x4f\x7\xb\xea\x39\x7c\x53\x5d\x43\x70\x7b\x16\xc0\x3f\x80\xf2\xaa\x46\x5a\xd8\xdc\x47\xd2\x3e\x81\xe\x94\xef\xcd\x55\x85\xb\xeb\xe6\x56\x4f\x36\xff\x5a\x96\xab\xfa\x94\xa0\x9e\x2d\x15\x13\xad\x10";

// Конструктор - созданный обьект предназначен для проверки метки в файле
Mark::Mark(std::istream & fin)
{
	// Поиск позиции метки
	if(findMarkPos(fin))
		throw CanNotFindMark_MarkException("Can not find mark.");

	// Переход на начало маркера в выходном файле(fout)
	fin.seekg(m_FileMarkPos);

	// Буфер для чтения маркера
	char rbuf[sizeof(m_Mark)];

	// Чтение шифрованного маркера
 	fin.read(rbuf,sizeof(m_Mark));

	// Прочитанный маркер
	std::string rmark(rbuf,rbuf+sizeof(m_Mark));

	// Дефолтный маркер
	std::string mark(MARK_DEF.c_str(),MARK_DEF.c_str()+sizeof(m_Mark));

	// Проверка, маркирован ли файл
	if(rmark==mark)
		throw(FileNotMarked_MarkException("File not marked."));

	// Преобразование шифрованного маркера
	m_Mark = *reinterpret_cast<const MarkData *>(rbuf);

	// Расшифровка маркера
	crypt();
}

// Конструктор - созданный обьект предназначен для инициализации маркера пользовательской и служебной информацией
Mark::Mark(const std::string & mrk)
{
	std::string str=mrk;
	DWORD comp_str_len=MAX_COMP_STR_LEN-1;

	// Если маркер больше (MAX_MARK_LEN-1) то он обрезается
	if(str.length()>(MAX_MARK_LEN-1))
	{
		str.resize(MAX_MARK_LEN-1);
		//std::cout<<"Warning: the marker was trucated.\n";
	}

	// Формирование незашифрованного маркера:
		// Строка пользователя
	std::copy_n(str.begin(),str.size()+1,m_Mark.m_label);
		// GUID
	const std::string guid = genGuidString();
	std::copy_n(guid.begin(),guid.size()+1,m_Mark.m_guid);
		// Время создания
	time(&m_Mark.m_date);
		// Пользователь
	GetComputerName(m_Mark.m_comp,&comp_str_len);

	// Запись пкс в строку пользователя
	m_Mark.m_label[mrk.length()]='\0';
	// Запись пкс в строку с GUID'ом
	m_Mark.m_guid[guid.length()] ='\0';
}

// Поиск в файле fin позиции маркера
bool Mark::findMarkPos(std::istream & fin)
{
	char	ch ;	// Буфер для посимвольного чтения файла(fin)
	int		i=0;	// Переменная для индикации "степени найденности" ключа	

	while(fin.get(ch))
	{
		// Поиск метки(ID_MARK) маркера в файле fin
		if(ch==MARK_KEY[i])
			i++;
		else
			i=0;

		// Нашли метку(ID_MARK) маркера
		if(i==12)
		{
			// Сохранение смещения маркера в переменной m_FileMarkPos
			m_FileMarkPos=fin.tellg();
			// Метка (а также смещение маркера) найдена
			return 0;
		}
	}

	// Метка (а также смещение маркера) не найдено
	return 1;	
}

// Запись маркера в выходной файл после метки
void Mark::setMark(std::istream & fin,std::ostream & fout, bool stream)
{
	char	fBuf[FILE_BLOCK];	// Буфер для записи файла блоками

	// Поиск позиции метки во входном файле(fin)
	if(findMarkPos(fin))
		throw CanNotFindPlaceForMark_MarkException("Can not find place for mark.");

	// Создание копии входного файла(fin) в файл(fout) блоками по FILE_BLOCK байт
	// если входной и выходной поток - разные файлы
	if(!stream)
	{
		// Переход на начало входного файла(fin)
		fin.seekg(0);

		while(fin.read(fBuf,FILE_BLOCK))
			fout.write(fBuf,FILE_BLOCK);
	}

	// Переход на начало маркера в выходном файле(fout)
	fout.seekp(m_FileMarkPos);

	// Шифрование маркера
	crypt();

	// Запись маркера в выходной файл(fout)
	fout.write(reinterpret_cast<const char *>(&m_Mark),sizeof(m_Mark));
}

// Шифровака/Расшифровка маркера
void Mark::crypt()
{
	int i=0;

	// Шифровака/Расшифровка (пользовательской информации, GUID'а и имени компьютера)
	for(;i<(MAX_MARK_LEN-1);i++)
		m_Mark.m_label[i]=m_Mark.m_label[i]^CRYPT_STR[i];
	for(i=0;i<(MAX_GUID_STR_LEN-1);i++)
		m_Mark.m_guid[i]=m_Mark.m_guid[i]^CRYPT_STR[i];
	for(i=0;i<(MAX_COMP_STR_LEN-1);i++)
		m_Mark.m_comp[i]=m_Mark.m_comp[i]^CRYPT_STR[i];
}

// Получение маркера
const Mark::MarkData & Mark::getMark() const
{
	return m_Mark;
}

// Получение GUID'а
std::string Mark::genGuidString()
{
	GUID tmp_win_guid;
	CoCreateGuid(&tmp_win_guid);

	wchar_t tmp_win_str[MAX_GUID_STR_LEN];
	StringFromGUID2(tmp_win_guid, tmp_win_str, MAX_GUID_STR_LEN);

	std::string tmp_str;
	tmp_str.resize(MAX_GUID_STR_LEN-1);
	sprintf(&tmp_str[0],"%S",tmp_win_str);

	return tmp_str;
}

}