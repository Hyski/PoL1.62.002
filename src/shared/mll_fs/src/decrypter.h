#if !defined(__DECRYPTER_H_INCLUDED_7552370787048739__)
#define __DECRYPTER_H_INCLUDED_7552370787048739__


#include <zlib.h>


namespace mll
{

namespace ml_encrypted_zip
{

//=====================================================================================//
//                                   class decrypter                                   //
//=====================================================================================//
class decrypter
{
private:
	unsigned long key0, key1, key2;
	bool active;

public:
	decrypter() {}
	decrypter(const std::string &pass);
	~decrypter() {}

	decrypter(const decrypter &a)
		:	key0(a.key0), key1(a.key1), key2(a.key2), active(a.active) {}

public:
	void update(char c);
	int decrypt_byte();
	bool encrypted() { return active; }
};

}

}




#endif // !defined(__DECRYPTER_H_INCLUDED_7552370787048739__)