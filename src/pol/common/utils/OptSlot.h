#ifndef _OPTSLOT_HEADER_
#define _OPTSLOT_HEADER_

class VFile;

//=====================================================================================//
//                                    class OptSlot                                    //
//=====================================================================================//
class OptSlot
{
protected:
	std::vector<unsigned char> m_Data;
	int m_CurPos;
public:
	void Save(FILE *f);
	void Save(SavSlot &sl);
	void Save(std::ostringstream &str);

	void Load(VFile *f);
	void Load(SavSlot &sl);
	void Load(std::istringstream &str);

	void Read(void *data, unsigned int size)
	{
		std::copy_n(&m_Data[m_CurPos],size,reinterpret_cast<unsigned char *>(data));
		m_CurPos += size;
	}

	void Write(void *data, unsigned int size)
	{
		std::copy_n(reinterpret_cast<const unsigned char *>(data),size,std::back_inserter(m_Data));
		m_CurPos += size;
	}

	void Rewind(){m_CurPos=0;};
	void operator =(const OptSlot &s){m_Data=s.m_Data;};
	OptSlot(const OptSlot &s):m_CurPos(0){operator=(s);};
	OptSlot():m_CurPos(0){};
	template<class T> friend OptSlot& operator <<(OptSlot &s,const T &d);
	template<class T> friend OptSlot& operator >>(OptSlot &s,T &d);
};

//=====================================================================================//
//                   template<class T> inline OptSlot& operator <<()                   //
//=====================================================================================//
template<class T> inline OptSlot& operator <<(OptSlot &s,const T &d)
{
	const unsigned char *dptr = reinterpret_cast<const unsigned char *>(&d);
	s.m_Data.insert(s.m_Data.end(),dptr,dptr+sizeof(T));
	return s;
}

//=====================================================================================//
//                   template<class T> inline OptSlot& operator >>()                   //
//=====================================================================================//
template<class T> inline OptSlot& operator >>(OptSlot &s,T &d)
{
	memcpy(&d,&s.m_Data[s.m_CurPos],sizeof(T));
	s.m_CurPos+=sizeof(d);
	return s;
}

//=====================================================================================//
//                      template<> inline OptSlot& operator <<()                       //
//=====================================================================================//
template<> inline  OptSlot& operator <<(OptSlot &s,const std::string &d)
{
	unsigned int i=d.size();
	s<<i;
	s.m_Data.insert(s.m_Data.end(),d.begin(),d.end());
	return s;
}

//=====================================================================================//
//                      template<> inline OptSlot& operator >>()                       //
//=====================================================================================//
template<> inline OptSlot& operator >>(OptSlot &s,std::string &d)
{
	int i;
	s>>i;
	d.resize(i);
	s.Read(&d[0],i);
	return s;
}


#endif