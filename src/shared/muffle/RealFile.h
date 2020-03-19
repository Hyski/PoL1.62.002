#if !defined(__REAL_FILE_H_INCLUDED_7552370787048739__)
#define __REAL_FILE_H_INCLUDED_7552370787048739__

namespace Muffle
{

class FileMgmt;

//=====================================================================================//
//                                   class RealFile                                    //
//=====================================================================================//
class RealFile : private noncopyable
{
	std::string m_name;
	long m_refCtr;
	unsigned m_length;
	unsigned m_lastUseTime;
	ci_VFile *m_file;

public:
	RealFile(ci_VFileSystem *, const std::string &);
	~RealFile();

	void addRef();
	void release();

	const char *getData();
	unsigned getLength();
	const std::string &name() const { return m_name; }

	std::auto_ptr<std::istream> getStream();

	unsigned getRefCounter() const { return m_refCtr; }
	unsigned getLastUseTime() const { return m_lastUseTime; }

	static bool canCreateFile(ci_VFileSystem *, const std::string &);
};

}	//	namespace Muffle

#endif // !defined(__REAL_FILE_H_INCLUDED_7552370787048739__)