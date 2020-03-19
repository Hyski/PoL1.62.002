#pragma once

namespace Shadows
{

class BaseShadow;
class ComplexShadow;

//=====================================================================================//
//                                  class ShadowAccum                                  //
//=====================================================================================//
class ShadowAccum
{
	struct UpdateParams;

	typedef std::vector<BaseShadow*> Shadows_t;
	typedef std::vector<ComplexShadow*> ComplexShadows_t;

	Shadows_t m_nonComplexShadows;
	ComplexShadows_t m_complexShadows;
	ComplexShadows_t m_mainThreadShadows, m_secondaryThreadShadows;

private:
	ShadowAccum();
	~ShadowAccum();

public:
	/// Добавить тень в очередь
	void addShadow(BaseShadow *);
	/// Обновить все тени
	void updateShadows(float tau);
	
	static ShadowAccum *inst();

private:
	void updateFromList(float, ComplexShadows_t *);
	static DWORD WINAPI worker(LPVOID);

	void updateShadowsHT(float tau);
	void updateShadowsNoHT(float tau);
};

}
