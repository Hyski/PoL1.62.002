#if !defined(__STREAM_H_INCLUDED_5625780540464499__)
#define __STREAM_H_INCLUDED_5625780540464499__

namespace Muffle
{

class RealFile;

//=====================================================================================//
//                                    class Stream                                     //
//=====================================================================================//
class Stream
{
	RealFile *m_file;
	std::auto_ptr<std::istream> m_stream;
	std::auto_ptr<ibinstream> m_binstream;

public:
	Stream();
	Stream(RealFile *);
	Stream(Stream &s);
	~Stream();

	const std::string &name() const;

	Stream &operator=(Stream &);

	ibinstream &bin() { return *m_binstream; }
	std::istream &stream() { return *m_stream; }
};

}	//	namespace Muffle

#endif // !defined(__STREAM_H_INCLUDED_5625780540464499__)