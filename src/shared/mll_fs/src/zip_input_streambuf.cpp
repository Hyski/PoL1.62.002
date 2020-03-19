
#include "precomp.h"

#include "zip_input_streambuf.h"
#include "zip_file.h"

#include <ios>



using namespace mll::ml_encrypted_zip;



//=====================================================================================//
//                         class zip_input_streambuf::strategy                         //
//=====================================================================================//
class zip_input_streambuf::strategy
{
public:
	virtual ~strategy() {}

	virtual void transfer() = 0;	//	activate next saved buffer
	virtual void save_output() = 0;	//	create new [and save old] output buffer
	virtual void seek(unsigned start_offset) = 0;
};





//=====================================================================================//
//     class zip_input_streambuf::store_seek: public zip_input_streambuf::strategy     //
//=====================================================================================//
class zip_input_streambuf::store_seek: public zip_input_streambuf::strategy
{
private:
	typedef std::vector<buffer_t *> cont_t;
	cont_t m_cont;
	cont_t::iterator m_it;
	zip_input_streambuf *m_fb;
	
public:
	store_seek(zip_input_streambuf *fb): m_fb(fb) {}
	virtual ~store_seek();

	virtual void transfer();
	virtual void save_output();
	virtual void seek(unsigned start_offset);
};

///////////////////////////////////////////////////////
//	~store_seek()
zip_input_streambuf::store_seek::~store_seek()
{
	unsigned tmp = m_cont.size();
	for (m_it = m_cont.begin(); m_it != m_cont.end(); ++m_it)
		delete *m_it;
}

///////////////////////////////////////////////////////
//	transfer()
void zip_input_streambuf::store_seek::transfer()
{
	buffer_t &old_cont = *(*m_it);
	buffer_t &cont = *(*(++m_it));
	m_fb->setg(&cont[0], &cont[0], &cont[0] + cont.size());
	m_fb->m_pos += old_cont.size();
	m_fb->m_buf_up += cont.size();
}

///////////////////////////////////////////////////////
//	save_output()
void zip_input_streambuf::store_seek::save_output()
{
	if (m_fb->m_output->size())
		m_cont.push_back(m_fb->m_output);
	else
		delete m_fb->m_output;
}

///////////////////////////////////////////////////////
//	seek()
void zip_input_streambuf::store_seek::seek(unsigned start_offset)
{
	if (start_offset > m_fb->m_size)
	{
		assert(0);
		throw seek_error("ml_encrypted_zip: seek offset is bigger than file size");
	}
	
	while (start_offset > m_fb->m_unpacked)
		m_fb->_underflow();
	
	unsigned block_offset = 0;
	unsigned rel_offset = 0;
	for (m_it = m_cont.begin(); m_it != m_cont.end(); ++m_it)
	{
		if (block_offset + (*m_it)->size() > start_offset)
			break;
		else
			block_offset += (*m_it)->size();
	}
	rel_offset = start_offset - block_offset;
	if (m_it == m_cont.end())
	{
		m_fb->setg(0, 0, 0);
		m_fb->m_pos = block_offset;
		m_fb->m_buf_up = block_offset;
	}
	else
	{
		buffer_t &cont = *(*m_it);
		m_fb->setg(&cont[0], &cont[rel_offset],
			&cont[0] + cont.size());

		m_fb->m_pos = block_offset;// + rel_offset;
		m_fb->m_buf_up = block_offset + cont.size();
	}
}






//=====================================================================================//
//      class zip_input_streambuf::no_seek: public zip_input_streambuf::strategy       //
//=====================================================================================//
class zip_input_streambuf::no_seek: public zip_input_streambuf::strategy
{	
public:
	no_seek() {}
	virtual ~no_seek() {}
	
	virtual void transfer() {}
	virtual void save_output() {}
	virtual void seek(unsigned start_offset);
};

///////////////////////////////////////////////////////
//	seek()
void zip_input_streambuf::no_seek::seek(unsigned start_offset)
{
	assert(0);
	throw no_seekable("ml_encrypted_zip: this sub_system mounted as not seekable");
}


































//=====================================================================================//
//                     zip_input_streambuf::zip_input_streambuf()                      //
//=====================================================================================//
zip_input_streambuf::zip_input_streambuf(std::streambuf *buf, decrypter &dec,
										 unsigned offset, bool seekable)
:	m_inbuf(buf), m_buf_size(1000), m_dec(dec), m_unpacked(0), m_pos(0), m_buf_up(0)
{
	//	strategy initialization
	if (seekable == true)
		m_st = std::auto_ptr<strategy>(new store_seek(this));
	else
		m_st = std::auto_ptr<strategy>(new no_seek());

	//	zlib initialization
	m_zs.zalloc = Z_NULL;
	m_zs.zfree  = Z_NULL;
	m_zs.opaque = Z_NULL;

	//	data structures initialization
	m_input = new buffer_t();
	m_input->resize(m_buf_size);

	//	read file header
	m_inbuf->pubseekoff(offset, std::ios::beg, std::ios::in);
	std::istream is(m_inbuf); // istream does not destroy the streambuf.
	m_entry = zip_local_entry(is);
	
	read_encryption_header(is);
	reset();

	//	force underflow on first read
	buffer_t &cont = *m_output;
	setg(0, 0, 0);
}


//=====================================================================================//
//                     zip_input_streambuf::~zip_input_streambuf()                     //
//=====================================================================================//
zip_input_streambuf::~zip_input_streambuf()
{
	delete m_input;
	if (m_entry.compression_method() == 8)
	{
		int err = inflateEnd(&m_zs);
		if (err != Z_OK)
		{
			assert(0);
			throw zlib_problem("ml_encrypted_zip: unknown zlib error while closing");
		}
	}
}

/*
//=====================================================================================//
//                     zip_input_streambuf::zip_input_streambuf()                      //
//=====================================================================================//
zip_input_streambuf::zip_input_streambuf(const zip_input_streambuf &a)
{		}


//=====================================================================================//
//                zip_input_streambuf &zip_input_streambuf::operator=()                //
//=====================================================================================//
zip_input_streambuf &zip_input_streambuf::operator=(const zip_input_streambuf &a)
{	return *this;	}
*/






//=====================================================================================//
//           zip_input_streambuf::int_type zip_input_streambuf::underflow()            //
//=====================================================================================//
zip_input_streambuf::int_type zip_input_streambuf::underflow()
{
	//	if not underflow don't fill buffer
	if (gptr() < egptr())
		return static_cast<unsigned char>(*gptr());

	//	if we are in "unpacked area", call transfer()
	if (m_buf_up < m_unpacked)
	{
		m_st->transfer();
		return static_cast<unsigned char>(*gptr());
	}
	
	//	otherwise, call main algorithm
	return _underflow();
}


//=====================================================================================//
//            zip_input_streambuf::pos_type zip_input_streambuf::seekoff()             //
//=====================================================================================//
zip_input_streambuf::pos_type zip_input_streambuf::seekoff(off_type off,
		std::ios_base::seekdir way, std::ios_base::openmode which)
{
	unsigned new_offset = 0;
	switch (way)
	{
	case std::ios::beg:
		new_offset = 0;
		break;
	case std::ios::cur:
		{
			int a = (gptr() - eback());
			new_offset = m_pos + gptr() - eback();
		}
		break;
	case std::ios::end:
		new_offset = m_size;
		break;
	}
	m_st->seek(new_offset + off);
	return new_offset + off;
}


//=====================================================================================//
//            zip_input_streambuf::pos_type zip_input_streambuf::seekpos()             //
//=====================================================================================//
zip_input_streambuf::pos_type zip_input_streambuf::seekpos(pos_type sp,
		std::ios_base::openmode which)
{
	m_st->seek(sp);
	return sp;
}








//=====================================================================================//
//                          void zip_input_streambuf::reset()                          //
//=====================================================================================//
void zip_input_streambuf::reset()
{
	int ok;
	switch (m_entry.compression_method())
	{
	case 8:	//	deflated
		m_zs.next_in  = reinterpret_cast<unsigned char *>(&(*m_input)[0]);
		m_zs.avail_in = 0;
		ok = inflateInit2(&m_zs, -15);
		ok = inflateReset(&m_zs);
		break;
	case 0:	//	stored
		m_remain = m_entry.uncompressed_size();
		break;
	default:
		assert(0);
		throw unknown_method("ml_encrypted_zip: unknown compression method!");
	}
	m_size = m_entry.uncompressed_size();
}


//=====================================================================================//
//           zip_input_streambuf::pos_type zip_input_streambuf::_underflow()           //
//=====================================================================================//
zip_input_streambuf::pos_type zip_input_streambuf::_underflow()
{
	unsigned bytes_num, num_available;
	int err;
	reset_output();
	buffer_t &in_cont = *m_input;
	buffer_t &out_cont = *m_output;
	
	switch (m_entry.compression_method())
	{
	case 0:	//	stored
		//	read and decrypt stored data
		bytes_num = std::min(m_remain, m_buf_size);
		num_available = m_inbuf->sgetn(&out_cont[0], bytes_num);
		decrypt_stored_data(num_available);
		//	push data out
		setg(&out_cont[0], &out_cont[0], &out_cont[0] + num_available);
		//	count remain bytes
		m_remain -= num_available;
		break;
	case 8:	//	deflated
		//	read and decrypt deflated data, prepare z_stream input
		m_zs.avail_out = m_buf_size; 
		m_zs.next_out = reinterpret_cast<unsigned char *>(&out_cont[0]);
		bytes_num = m_inbuf->sgetn(&in_cont[0], m_buf_size);
		decrypt_deflated_data(bytes_num);
		m_zs.next_in = reinterpret_cast<unsigned char *>(&in_cont[0]);
		m_zs.avail_in = bytes_num;
		//	inflate and push out data
		err = inflate(&m_zs, Z_NO_FLUSH);
		num_available = m_buf_size - m_zs.avail_out;
		setg(&out_cont[0], &out_cont[0], &out_cont[0] + num_available);
		//	handle zlib problems
		if(err != Z_OK && err != Z_STREAM_END)
		{
			assert(0);
			throw zlib_problem("ml_encrypted_zip: unknown zlib error while inflating");
		}
		break;
	default:
		assert(0);
		throw unknown_method("ml_encrypted_zip: unknown compression method!");
	}

	m_pos = m_unpacked;
	m_unpacked += num_available;
	m_buf_up = m_unpacked;
	m_output->resize(num_available);
	m_st->save_output();

	//	detect eof
	if (num_available)
		return static_cast<unsigned char>(*gptr());
	else
		return traits_type::eof();

	return traits_type::eof();
}


//=====================================================================================//
//                      void zip_input_streambuf::reset_output()                       //
//=====================================================================================//
void zip_input_streambuf::reset_output()
{
	m_output = new buffer_t();
	m_output->resize(m_buf_size);
}


//=====================================================================================//
//                 void zip_input_streambuf::read_encryption_header()                  //
//=====================================================================================//
void zip_input_streambuf::read_encryption_header(std::istream &is)
{
	if (m_dec.encrypted())
	{
		for (unsigned i = 0; i <12; ++i)
			m_dec.update(is.get() ^ m_dec.decrypt_byte());
	}
}


//=====================================================================================//
//                  void zip_input_streambuf::decrypt_deflated_data()                  //
//=====================================================================================//
void zip_input_streambuf::decrypt_deflated_data(unsigned num_available)
{
	if (m_dec.encrypted())
	{
		unsigned char tmp;
		for (unsigned i = 0; i < num_available; ++i)
		{
			tmp = (*m_input)[i] ^ m_dec.decrypt_byte();
			m_dec.update(tmp);
			(*m_input)[i] = tmp;
		}
	}
}


//=====================================================================================//
//                   void zip_input_streambuf::decrypt_stored_data()                   //
//=====================================================================================//
void zip_input_streambuf::decrypt_stored_data(unsigned num_available)
{
	if (m_dec.encrypted())
	{
		unsigned char tmp;
		for (unsigned i = 0; i < num_available; ++i)
		{
			tmp = (*m_output)[i] ^ m_dec.decrypt_byte();
			m_dec.update(tmp);
			(*m_output)[i] = tmp;
		}
	}
}