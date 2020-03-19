#if !defined(__ZIP_INPUT_STREAMBUF_H_INCLUDED_8093570542662288__)
#define __ZIP_INPUT_STREAMBUF_H_INCLUDED_8093570542662288__


#include <zlib.h>

#include "zip_local_entry.h"
#include "decrypter.h"



namespace mll
{

namespace ml_encrypted_zip
{


//=====================================================================================//
//                  class zip_input_streambuf: public std::streambuf                   //
//=====================================================================================//
class zip_input_streambuf: public std::streambuf
{
public:
	typedef std::vector<char> buffer_t;
	class strategy;

private:
	decrypter m_dec;
	z_stream m_zs;
	buffer_t *m_input, *m_output;
	unsigned m_buf_size, m_remain;
	unsigned m_pos, m_unpacked, m_buf_up, m_size;
	std::streambuf *m_inbuf;
	zip_local_entry m_entry;
	
private:
	void reset();
	pos_type _underflow();
	void reset_output();
	void read_encryption_header(std::istream &is);
	void decrypt_stored_data(unsigned num_available);
	void decrypt_deflated_data(unsigned num_available);
	
private:
	class no_seek; friend no_seek;
	class store_seek; friend store_seek;
	std::auto_ptr<strategy> m_st;

public:
	explicit zip_input_streambuf(std::streambuf *buf, decrypter &dec, unsigned offset, bool seekable);
	~zip_input_streambuf();

private:
	//	buffer must be nocopyable
	zip_input_streambuf(const zip_input_streambuf &a);
	zip_input_streambuf &operator=(const zip_input_streambuf &a);
	
protected:
	virtual int_type underflow();
	virtual pos_type seekoff(off_type off, std::ios_base::seekdir way,
		std::ios_base::openmode which = std::ios_base::in | std::ios_base::out);
	virtual pos_type seekpos(pos_type sp,
		std::ios_base::openmode which = std::ios_base::in | std::ios_base::out);
};

}

}

#endif // !defined(__ZIP_INPUT_STREAMBUF_H_INCLUDED_8093570542662288__)