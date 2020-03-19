#pragma once

//=====================================================================================//
//                                   class ProgrBar                                    //
//=====================================================================================//
class ProgrBar
{
public:
	static void SetMofN(int m, int n);
	static void SetRange(float a, float b);
	static void SetTitle(const std::string &form, const char *n);
	static void SetTitle(const std::string &form, const std::string &n);
	static void SetPercent(float p);
};
