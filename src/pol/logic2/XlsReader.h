//
// ���������� ��������� ��� ������ xls
//

#ifndef _PUNCH_XLSREADER_H_
#define _PUNCH_XLSREADER_H_

#include "../Common/DataMgr/TxtFile.h"

//������� �������
struct column{
    const char* m_name;//�������� 
    int    m_index;    //����� �������
    int    m_type;     //��� �������

    bool IsOk() const { return m_index >= 0; }

    column(const char* name, unsigned type) :
        m_name(name), m_type(type), m_index(-1){}
};

//
// ������� ��� VFile
//
class VFilePtr{
public:

    explicit VFilePtr(const std::string& file_name) : m_base(file_name)
    {
        if(m_base.m_file->Size() == 0){
            std::ostringstream ostr;

            ostr << "VFilePtr: �� ���� ����� ����!" << std::endl;
            ostr << "��� �����: <" << file_name << ">" << std::endl;

            throw CASUS(ostr.str());
        }
    } 

    //�������� ��� �����
    const std::string& GetName() const { return m_base.m_name; }

    //��������� ������� � �����
    VFile& operator* () { return *m_base.m_file; }
    VFile* operator->() { return m_base.m_file;  }
    
private:

    struct file_base{
        VFile*      m_file;
        std::string m_name;

        ~file_base(){DataMgr::Release(m_name.c_str());}
        file_base(const std::string& name) : m_name(name), m_file(DataMgr::Load(name.c_str())){}
    };

    file_base m_base;    
};

//
// ������� TxtFile
//
class TxtFilePtr {
public:

    explicit TxtFilePtr(const std::string& file_name) : m_file(file_name)        
    {
        if(m_txt.Load(&*m_file)) return;

        std::ostringstream ostr;
        
        ostr << "TxtFilePtr: ������ ������� �����!" << std::endl;
        ostr << "��� �����: <" << file_name << ">" << std::endl;
        
        throw CASUS(ostr.str());       
    }

    //�������� ��� �����
    const std::string& GetName() const { return m_file.GetName(); }
    
    //��������� ������� � �����
    TxtFile& operator* () { return m_txt; }
    TxtFile* operator->() { return &m_txt; }

    //������������� ��������� �������
    void ReadColumns(column* first, column* last, size_t line = 0)
    {
        std::string str;
        
        column* itor; 
        int     counter = 0;     
        
        for(int x = 0; x < m_txt.SizeX(line); x++){
            
            m_txt.GetCell(line, x, str);
            
            for(itor = first; itor != last && itor->m_name != str; ++itor);
            if(itor != last)
			{
                itor->m_index = x;
                ++counter;
            }
        }    
        
        if(counter != last - first){
            
            std::ostringstream ostr;
            
            ostr << "TxtFilePtr: �� ������ ������� � �������!" << std::endl << std::endl;
            ostr << "����:\t<" << m_file.GetName() << '>' <<std::endl;
            
            for(itor = first; itor != last; ++itor){
                if(!itor->IsOk()) ostr << "�������:\t<" << itor->m_name << '>'<< std::endl;
            }
            
            throw CASUS(ostr.str());
        }
    }

private:
    
    TxtFile  m_txt;
    VFilePtr m_file;
};

#endif //_PUNCH_XLSREADER_H_

