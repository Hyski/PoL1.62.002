//
// —истема сохранени€ объектов
//

#ifndef _PUNCH_SAVELOAD_H_
#define _PUNCH_SAVELOAD_H_

//#include "../DataMgr/DataMgr.h" Punch: уже включен в precomp.h

//определение буфера данных
typedef std::vector<unsigned char> data_buff_t;

//
//абстракци€ файла дл€ записи Save'a
//
class AbstractFile{
public:
    //узнать текущее положение указател€ в файле
    virtual long GetPos() = 0;
    //установить указатель файла на опред позицию
    virtual void Seek(long offset, int origin) = 0;
    //запись / чтение из файла
    virtual size_t Write(const void *ptr, size_t size) = 0;
    virtual size_t Read(void *ptr, size_t size) = 0;
};

//
// обертка на стандартный файл
//
class StdFile: public AbstractFile{
public:

    StdFile(const char* name, const char* type):
        m_hfile(fopen(name, type)){}

    ~StdFile()
    {
      if(m_hfile) fclose(m_hfile);
    }

    //инициализаци€ файла была успешной?
    bool IsOk() const
    {
        return m_hfile != NULL;
    }

    long GetPos()
    {
        return ftell(m_hfile);
    }
    void Seek(long offset, int origin)
    {
        fseek(m_hfile, offset, origin);
    }
    size_t Write(const void *ptr, size_t size)
    {
        return fwrite(ptr, 1, size, m_hfile);
    }
    size_t Read(void* ptr, size_t size)
    {
        return fread(ptr, 1, size, m_hfile);
    }

private:
    FILE* m_hfile;
};

//
// обертка на файл считываемый из пака
//
class VFile;
class PackageFile: public AbstractFile
{
private:
	VFile *m_pVFile;
	std::string m_sFileName;
public:
	PackageFile(const char* name) : m_pVFile(DataMgr::Load(name)){m_sFileName = name;}
	~PackageFile(){DataMgr::Release(m_sFileName.c_str());}
public:
    //инициализаци€ файла была успешной?
    bool IsOk() const
    {
        return m_pVFile->Size() != 0;
    }
    long GetPos()
    {
        return m_pVFile->Tell();
    }
    void Seek(long offset, int origin)
    {
      switch(origin)
        {
        case SEEK_CUR:
          m_pVFile->Seek(offset,VFile::VFILE_CUR);
          break;
        case SEEK_END:
		      m_pVFile->Seek(offset,VFile::VFILE_END);
          break;
        case SEEK_SET:
		      m_pVFile->Seek(offset,VFile::VFILE_SET);
          break;
        }
    }
    size_t Write(const void *, size_t )
    {
		return 0;
    }
    size_t Read(void* ptr, size_t size)
    {
        return m_pVFile->Read(ptr,size);
    }
};


//
//абстракци€ дл€ хранилища информации
//
class GenericStorage
{
public:

    enum storage_mode
	{
        SM_LOAD,
        SM_SAVE,
    };

    GenericStorage(AbstractFile* file, storage_mode mode);
    ~GenericStorage();

    //сохранить/загрузить буфер с данными
    void SaveData(const char* cell_name, const data_buff_t& data);
    void LoadData(const char* cell_name, data_buff_t& data);

    //вид текущей операции
    bool IsSaving() const;

    //проверить нормальный ли Save
    bool IsValid();

	unsigned int GetMajor() const { return m_major; }
	unsigned int GetMinor() const { return m_minor; }
	unsigned int GetBuild() const { return m_build; }

    //получить версию текщей системы SaveLoad
    unsigned int GetVersion() const;

private:

    //запретим копирующий конструктор
    GenericStorage(const GenericStorage& store);
    
private:

    struct header
	{
        char        m_id_str[8];  //идентиф. строка
        unsigned    m_version;    //верси€ 
        unsigned    m_index_size; //количество размер индекса
        long        m_index_off;  //смещени€ до индекса
        unsigned    m_reserved;

        header();
    };

    typedef std::hash_map<std::string,long> str_index_t;

    AbstractFile* m_file;
    
    storage_mode  m_mode;    
    str_index_t   m_index;
    header        m_hdr;

	unsigned int m_major;
	unsigned int m_minor;
	unsigned int m_build;
};

//=====================================================================================//
//                                  class SavSlotBase                                  //
//=====================================================================================//
class SavSlotBase
{
public:
	virtual ~SavSlotBase() = 0 {}

    //сохранить/загрузить данные в €чейку
    virtual void Save(const void *ptr, size_t size) = 0;
    virtual void Load(void *ptr, size_t size) = 0;

    //режим работы €чейки
	virtual bool IsSaving() const = 0;
};

//=====================================================================================//
//                                    class SavSlot                                    //
//=====================================================================================//
//класс - абстракци€ дл€ €чейки сохранени€
class SavSlot : public SavSlotBase
{
public:
    SavSlot(GenericStorage* st, const char* name, size_t size = 128);
    ~SavSlot();
    
    //сохранить/загрузить данные в €чейку
    void Save(const void *ptr, size_t size);
    void Load(void *ptr, size_t size);

	void Reserve(size_t size) { assert( m_fsave ); m_data.reserve(m_data.size()+size); }
    
    //получить ссылку на GenericStorage этой €чейки
    GenericStorage* GetStore();

    //режим работы €чейки
    bool IsSaving() const;

	// ¬озвращает объем хранимых данных
	size_t GetSize() const { return m_data.size(); }

private:

    GenericStorage*     m_strg;
    std::string  m_name;
    data_buff_t  m_data; 
    bool         m_fsave;
    unsigned     m_cur;
};

//=====================================================================================//
//                        class SavSlotStr : public SavSlotBase                        //
//=====================================================================================//
class SavSlotStr : public SavSlotBase
{
	data_buff_t m_data;
	bool m_isSaving;
	size_t m_cur;

public:
	SavSlotStr();
	SavSlotStr(const std::string &str);

    //сохранить/загрузить данные в €чейку
    virtual void Save(const void *ptr, size_t size);
    virtual void Load(void *ptr, size_t size);

    //режим работы €чейки
	virtual bool IsSaving() const { return m_isSaving; }

	std::string GetData() const
	{
		std::string result;
		result.resize(m_data.size());
		std::copy((char *)&m_data[0], (char *)&m_data[0] + m_data.size(), &result[0]);
		return result;
	}
};

//
// ќператоры дл€ записи/чтени€ в Save
//

inline SavSlotBase& operator << (SavSlotBase& st, bool val)
{
    st.Save(&val, sizeof(bool));    
    return st;
}

inline SavSlotBase& operator >> (SavSlotBase& st, bool& val)
{
    st.Load(&val, sizeof(bool));
    return st;
}

inline SavSlotBase& operator << (SavSlotBase& st, unsigned val)
{
    st.Save(&val, sizeof(unsigned));    
    return st;
}

inline SavSlotBase& operator >> (SavSlotBase& st, unsigned& val)
{
    st.Load(&val, sizeof(unsigned));
    return st;
}

inline SavSlotBase& operator << (SavSlotBase& st, int val)
{
    st.Save(&val, sizeof(int));    
    return st;
}

inline SavSlotBase& operator >> (SavSlotBase& st, int& val)
{
    st.Load(&val, sizeof(int));
    return st;
}

inline SavSlotBase& operator << (SavSlotBase& st, float val)
{
    st.Save(&val, sizeof(float));
    return st;
}

inline SavSlotBase& operator >> (SavSlotBase& st, float& val)
{
    st.Load(&val, sizeof(float));
    return st;
}

inline SavSlotBase& operator << (SavSlotBase& st, char val)
{
    st.Save(&val, sizeof(char));
    return st;
}

inline SavSlotBase& operator >> (SavSlotBase& st, char& val)
{
    st.Load(&val, sizeof(char));
    return st;
}

inline SavSlotBase& operator << (SavSlotBase& st, short val)
{
    st.Save((void*)&val, sizeof(short));
    return st;
}

inline SavSlotBase& operator >> (SavSlotBase& st, short& val)
{
    st.Load((void*)&val, sizeof(short));
    return st;
}
inline SavSlotBase& operator << (SavSlotBase& st, unsigned short val)
{
    st.Save((void*)&val, sizeof(unsigned short));
    return st;
}

inline SavSlotBase& operator >> (SavSlotBase& st, unsigned short& val)
{
    st.Load((void*)&val, sizeof(unsigned short));
    return st;
}

inline SavSlotBase& operator << (SavSlotBase& st, const std::string& str)
{
    st.Save((void*)str.c_str(), str.size()+1);
    return st;
}

inline SavSlotBase& operator >> (SavSlotBase& st, std::string& str)
{
    char ch;    
    str.clear();
    do{
        st >> ch;
        if(ch) str += ch;
    }while(ch);

    return st;
}

//
// ~~~~~~~~~~~~ inline ~~~~~~~~~~~
//

inline bool GenericStorage::IsSaving() const
{
    return m_mode == SM_SAVE;
}

inline unsigned GenericStorage::GetVersion() const
{
    return m_hdr.m_version;
}

inline bool GenericStorage::IsValid()
{
    return true;
}

inline GenericStorage* SavSlot::GetStore()
{
    return m_strg;
}

inline bool SavSlot::IsSaving() const
{
    return m_fsave;
}

#endif // _PUNCH_SAVELOAD_H_
