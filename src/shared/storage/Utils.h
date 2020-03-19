#pragma once

namespace Storage
{

struct Utils
{
	static Version restoreVersion(mll::io::ibinstream &);
	static std::string restoreTag(mll::io::ibinstream &);
	static void storeVersion(mll::io::obinstream &, const Version &);
	static void storeTag(mll::io::obinstream &, const std::string &);
};

}
