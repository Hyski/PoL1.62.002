//
// ������� ���������� ��������
//

#ifndef _PUNCH_SAVELOAD_H_
#define _PUNCH_SAVELOAD_H_

//#include "../DataMgr/DataMgr.h" Punch: ��� ������� � precomp.h

//����������� ������ ������
typedef std::vector<unsigned char> data_buff_t;

//
//���������� ����� ��� ������ Save'a
//
class AbstractFile{
public:
    //������ ������� ��������� ��������� � �����
    virtual long GetPos() = 0;
    //���������� ��������� ����� �� ����� �������
    virtual void Seek(long offset, int origin) = 0;
    //������ / ������ �� �����
    virtual size_t Write(const void *ptr, size_t size) = 0;
    virtual size_t Read(void *ptr, size_t size) = 0;
};

//
// ������� �� ����������� ����
//
class StdFile: public AbstractFile{
public:

    StdFile(const char* name, const char* type):
        m_hfile(fopen(name, type)){}

    ~StdFile()
    {
      if(m_hfile) fclose(m_hfile);
    }

    //������������� ����� ���� ��������?
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
// ������� �� ���� ����������� �� ����
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
    //������������� ����� ���� ��������?
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
//���������� ��� ��������� ����������
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

    //���������/��������� ����� � �������
    void SaveData(const char* cell_name, const data_buff_t& data);
    void LoadData(const char* cell_name, data_buff_t& data);

    //��� ������� ��������
    bool IsSaving() const;

    //��������� ���������� �� Save
    bool IsValid();

	unsigned int GetMajor() const { return m_major; }
	unsigned int GetMinor() const { return m_minor; }
	unsigned int GetBuild() const { return m_build; }

    //�������� ������ ������ ������� SaveLoad
    unsigned int GetVersion() const;

private:

    //�������� ���������� �����������
    GenericStorage(const GenericStorage& store);
    
private:

    struct header
	{
        char        m_id_str[8];  //�������. ������
        unsigned    m_version;    //������ 
        unsigned    m_index_size; //���������� ������ �������
        long        m_index_off;  //�������� �� �������
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

    //���������/��������� ������ � ������
    virtual void Save(const void *ptr, size_t size) = 0;
    virtual void Load(void *ptr, size_t size) = 0;

    //����� ������ ������
	virtual bool IsSaving() const = 0;
};

//=====================================================================================//
//                                    class SavSlot                                    //
//=====================================================================================//
//����� - ���������� ��� ������ ����������
class SavSlot : public SavSlotBase
{
public:
    SavSlot(GenericStorage* st, const char* name, size_t size = 128);
    ~SavSlot();
    
    //���������/��������� ������ � ������
    void Save(const void *ptr, size_t size);
    void Load(void *ptr, size_t size);

	void Reserve(size_t size) { assert( m_fsave ); m_data.reserve(m_data.size()+size); }
    
    //�������� ������ �� GenericStorage ���� ������
    GenericStorage* GetStore();

    //����� ������ ������
    bool IsSaving() const;

	// ���������� ����� �������� ������
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

    //���������/��������� ������ � ������
    virtual void Save(const void *ptr, size_t size);
    virtual void Load(void *ptr, size_t size);

    //����� ������ ������
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
// ��������� ��� ������/������ � Save
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
