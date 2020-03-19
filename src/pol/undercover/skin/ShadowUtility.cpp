#include "precomp.h"

#include <map>
#include "Shadow.h"
#include "../IWorld.h"
#include <undercover/options/options.h>
#include "ComplexShadow.h"
#include <common/TextureMgr/DIBData.h>

#define DEFAULT_LIGHT	(point3(0.7f, 0.7f, -1.0f)/point3(0.7f, 0.7f, -1.0f).Length())

namespace Shadows
{

extern bool g_useHT;

//=====================================================================================//
//                                    Sync::Sync()                                     //
//=====================================================================================//
Sync::Sync()
{
	m_event = CreateEvent(NULL,TRUE,TRUE,NULL);
}

//=====================================================================================//
//                                    Sync::~Sync()                                    //
//=====================================================================================//
Sync::~Sync()
{
	CloseHandle(m_event);
}

//=====================================================================================//
//                                 Sync &Sync::inst()                                  //
//=====================================================================================//
Sync &Sync::inst()
{
	static Sync instance;
	return instance;
}

//=====================================================================================//
//                                  void Sync::wait()                                  //
//=====================================================================================//
void Sync::wait()
{
	WaitForSingleObject(Sync::inst().m_event,INFINITE);
}

//=====================================================================================//
//                                void Sync::acquire()                                 //
//=====================================================================================//
void Sync::acquire()
{
	wait();
	ResetEvent(m_event);
}

//=====================================================================================//
//                                 void Sync::leave()                                  //
//=====================================================================================//
void Sync::leave()
{
	SetEvent(m_event);
}

HtData HtData::g_inst;

#if defined(GVZ_SHADOW_LOG)
CLog gvzLog;
#endif

//=====================================================================================//
//                                     class light                                     //
//=====================================================================================//
class light
{
public:
	point3 origin;
	float brightness;
	float sqr_max_distance;

	light (const point3 &_origin, float _brightness, float _sqr_max_distance)
		: origin(_origin), brightness(_brightness), sqr_max_distance(_sqr_max_distance) {};
	light (const light &l)
		: origin(l.origin), brightness(l.brightness), sqr_max_distance(l.sqr_max_distance) {};

	float getIntensity (const point3 & /*pt*/, float dist)
	{
//		float t = 2 - brightness/((sqr_max_distance-dist)/(2*sqr_max_distance) + 0.5);
//		float t = brightness/( dist/(2*sqr_max_distance) + 0.5 ) - 1;
		float t = brightness*(1 - sqrt(dist/sqr_max_distance));
		if (t < 0) return 0;
//		if (t > 1) return 1;
		return t;
	}

	bool isInRadius (const point3 &pt, float &distance);
};

//=====================================================================================//
//                              bool light::isInRadius()                               //
//=====================================================================================//
bool light::isInRadius(const point3 &pt, float &distance)
{
	if (sqr_max_distance < (distance = (pt.x - origin.x)*(pt.x - origin.x))) return false;
	if (sqr_max_distance < (distance += (pt.y - origin.y)*(pt.y - origin.y))) return false;
	if (sqr_max_distance < (distance += (pt.z - origin.z)*(pt.z - origin.z))) return false;

	return true;
}


std::map<std::string, BaseShadow *> shdwMap;
std::list<light> lights;

float ShadowUtility::timeCalc = 0;
float ShadowUtility::timeInProcess = 0;
float ShadowUtility::timeBegin = 0;

float ShadowUtility::timeCalcD = 0;
float ShadowUtility::timeInProcessD = 0;
float ShadowUtility::timeBeginD = 0;

bool ShadowUtility::bNoKill = false;

//=====================================================================================//
//                            void ShadowUtility::AddTime()                            //
//=====================================================================================//
void ShadowUtility::AddTime(float curTime, float relTime)
{
	if (curTime - timeBegin > 1)
	{
		timeBegin = curTime;
		timeCalc = timeInProcess;
		timeInProcess = relTime;
	} else 
	{
		timeInProcess += relTime;
	}
}

//=====================================================================================//
//                           float ShadowUtility::GetTime()                            //
//=====================================================================================//
float ShadowUtility::GetTime()
{
	return timeCalc;
}

//=====================================================================================//
//                           void ShadowUtility::AddTimeD()                            //
//=====================================================================================//
void ShadowUtility::AddTimeD(float curTime, float relTime)
{
	if (curTime - timeBeginD > 1)
	{
		timeBeginD = curTime;
		timeCalcD = timeInProcessD;
		timeInProcessD = relTime;
	} else 
	{
		timeInProcessD += relTime;
	}
}

//=====================================================================================//
//                           float ShadowUtility::GetTimeD()                           //
//=====================================================================================//
float ShadowUtility::GetTimeD()
{
	return timeCalcD;
}

//=====================================================================================//
//                                   void AddLight()                                   //
//=====================================================================================//
void AddLight(const point3 &origin, float brt, float dist)
{
	lights.push_back(light(origin, brt/256.0f, dist*dist));
	GVZ_LOG("AddLight: Got light: origin(%f, %f, %f), brightness(%f), distance(%f);\n", origin.x, origin.y, origin.z, lights.back().brightness, dist);
}

//=====================================================================================//
//                                 void ClearLights()                                  //
//=====================================================================================//
void ClearLights()
{
	lights.clear ();
	GVZ_LOG("ClearLights() called;\n");
}

//=====================================================================================//
//                                 void DrawShadows()                                  //
//=====================================================================================//
void DrawShadows(GraphPipe *Pipe)
{
	Timer::Update();
	float time = Timer::GetSeconds();
	Pipe->SwitchRenderMode(GraphPipe::RM_3D);
	std::map<std::string, BaseShadow *>::iterator i;

	//if(g_useHT) Sync::inst().wait();
	for (i = shdwMap.begin(); i != shdwMap.end(); i++)
	{
		i->second->Draw(Pipe);
	}

	Timer::Update();
	time = Timer::GetSeconds() - time;

	ShadowUtility::AddTimeD(Timer::GetSeconds(), time);
}

//=====================================================================================//
//                          void ShadowUtility::DrawLights()                           //
//=====================================================================================//
void ShadowUtility::DrawLights(GraphPipe *pipe)
{
	BBox b;

	for(std::list<light>::iterator i = lights.begin(); i != lights.end(); ++i)
	{
		b.Box(i->origin, sqrt(i->sqr_max_distance));
		pipe->DrawBBox(b, 0xFFFFFFFF);
	}
}

//=====================================================================================//
//                            bool ShadowUtility::NoKill()                             //
//=====================================================================================//
bool ShadowUtility::NoKill()
{
	return bNoKill;
}

//=====================================================================================//
//                              void UpdateOnModeChange()                              //
//=====================================================================================//
void UpdateOnModeChange()
{
	std::map<std::string, BaseShadow *>::iterator i;

	GVZ_LOG("UpdateOnModeChange - updating shadow surfaces:\n");

	for(i = shdwMap.begin(); i != shdwMap.end(); i->second->UpdateOnModeChange(), i++);

	GVZ_LOG("UpdateOnModeChange - Done!\n");
}

//=====================================================================================//
//                                void OptionsChanged()                                //
//=====================================================================================//
void OptionsChanged()
{
	std::map<std::string, BaseShadow *>::iterator i;
	BaseShadow *tmp = 0;

	GVZ_LOG("OptionsChanged - rebuilding shadows:\n");

	ShadowUtility::bNoKill = true;
	for (i = shdwMap.begin(); i != shdwMap.end(); ++i)
	{
		tmp = i->second;
		i->second = ReCreateShadow(tmp);
		delete tmp;
	}
	ShadowUtility::bNoKill = false;
}

//=====================================================================================//
//                      point3 ShadowUtility::getLightDirection()                      //
//=====================================================================================//
point3 ShadowUtility::getLightDirection(const point3 &pt)
{
	if (!lights.size())
	{
//		Timer::Update ();
//		return point3(sin(Timer::GetSeconds()), cos(Timer::GetSeconds()), -1)/sqrt(2);
		return DEFAULT_LIGHT;
	}


	point3 result(0,0,0);
	float dist, intenc = 0.0f;

	for (std::list<light>::iterator i = lights.begin(); i != lights.end(); ++i)
	{
		if (i->isInRadius(pt, dist))
		{
			intenc = i->getIntensity (pt, dist);
			result += (pt - i->origin)/sqrt(dist)*intenc;
		}
	}

	if(intenc == 0.0f) intenc = 0.1f;

	if (result.z < 0.0f)
	{
		return result/result.Length();
	}
	else
	{
		result.z = -0.001f;
		return result/result.Length();
	}
}

//=====================================================================================//
//                      float ShadowUtility::getLightIntensity()                       //
//=====================================================================================//
float ShadowUtility::getLightIntensity(const point3 &pt)
{
	if (!lights.size()) return 0.95;

	point3 result(0,0,0);
	float dist, intenc;

	for (std::list<light>::iterator i = lights.begin(); i != lights.end(); ++i)
	{
 		if (i->isInRadius(pt, dist))
		{
			intenc = i->getIntensity (pt, dist);
			result += (pt - i->origin)/sqrt(dist)*intenc;
		}
	}

	float t = result.Length();

	if (t>0.95f) return 0.95f;
	if (t<0.0f) return 0.0f;
	return t;
}

//=====================================================================================//
//                           void ShadowUtility::AddShadow()                           //
//=====================================================================================//
void ShadowUtility::AddShadow(std::string &shdwName, BaseShadow *shadow)
{
	if (shdwMap.find(shdwName) != shdwMap.end()) throw CASUS("Конфликт имен теней.\nКто-то попытался создать тень с неуникальным именем");
	shdwMap[shdwName] = shadow;
}

//=====================================================================================//
//                          void ShadowUtility::KillShadow()                           //
//=====================================================================================//
void ShadowUtility::KillShadow(const std::string &shdwName)
{
	shdwMap.erase(shdwName);
}

//=====================================================================================//
//                      void ShadowUtility::CopyImageToSurface()                       //
//=====================================================================================//
void ShadowUtility::CopyImageToSurface(const ComplexShadow *shdw)
{
	HDC hSurfaceDC;

    // Get a DC for the surface
    if(!FAILED(shdw->shdwSurface->GetDC(&hSurfaceDC)))
    {
		//	копируем картинку
		shdw->dbdTexture->StretchDIBits(hSurfaceDC,
								  0,0,shdw->dbdTexture->Width(),shdw->dbdTexture->Height(),
								  0,0,shdw->dbdTexture->Width(),shdw->dbdTexture->Height());
		shdw->shdwSurface->ReleaseDC(hSurfaceDC);
	}
}

//=====================================================================================//
//                      point3 ShadowUtility::projPoint2Surface()                      //
//=====================================================================================//
point3 ShadowUtility::projPoint2Surface(const point3 &p, const point3 &l, const point3 &vertex, const point3 &normal)
{
	float t;

	t = (vertex.x - p.x)*normal.x + 
		(vertex.y - p.y)*normal.y + 
		(vertex.z - p.z)*normal.z;

	t /= l.x*normal.x + l.y*normal.y + l.z*normal.z;

	return p + t*l - 0.01f*normal;
}

//=====================================================================================//
//                             BaseShadow *CreateShadow()                              //
//=====================================================================================//
BaseShadow *CreateShadow(RealEntity *owner)
{
	BaseShadow *shadow;

	if(Options::GetInt("system.video.shadowquality") == 0)
	{
		shadow = new Shadows::NullShadow(owner);
	}
	else if(Options::GetInt("system.video.shadowquality") == 1)
	{
		shadow = new Shadows::SimpleShadow(owner);
		shadow->SetShadowTexture("textures.sys/simpleshadow.tga");
	}
	else
	{
		shadow = new Shadows::ComplexShadow(owner);
	}

	return shadow;
}


//=====================================================================================//
//                            BaseShadow *ReCreateShadow()                             //
//=====================================================================================//
BaseShadow *ReCreateShadow(BaseShadow *elder)
{
	BaseShadow *shadow;

	if(Options::GetInt("system.video.shadowquality") == 0)
	{
		shadow = new Shadows::NullShadow(elder);
	}
	else if(Options::GetInt("system.video.shadowquality") == 1)
	{
		shadow = new Shadows::SimpleShadow(elder);
		shadow->SetShadowTexture("textures.sys/simpleshadow.tga");
	}
	else
	{
		shadow = new Shadows::ComplexShadow(elder);
	}

	shadow->Update(-1.0f);

	return shadow;
}

//=====================================================================================//
//                             void ShadowUtility::Line()                              //
//=====================================================================================//
void ShadowUtility::Line(const ComplexShadow *shdw, int x1, int y1, int x2, int y2, short int pinc)
{
	/* by Flif int TX = shdw->TEXTX;*/
	int	TX2 = shdw->TEXTX<<1;
	unsigned char TS = shdw->TXS;
	short int *b = shdw->buf;

	_asm {
				mov		ax, pinc

				mov		ecx, y2
				sub		ecx, y1
				jz		konetz
				jns		positive_dy

// Инициализация edi
				mov		si, cx
				mov		cl, byte ptr TS
				mov		edi, b
				mov		ebx, y2
				sal		ebx, cl
				add		ebx, x2
				sal		ebx, 1
				add		edi, ebx
				mov		cx, si

				xor		edx, edx
				neg		ecx
				mov		ebx, x1
				sub		ebx, x2
				js		negative_dx
				jmp		det_phase

positive_dy:	
// Инициализация edi
				mov		si, cx
				mov		cl, byte ptr TS
				mov		edi, b
				mov		ebx, y1
				sal		ebx, cl
				add		ebx, x1
				sal		ebx, 1
				add		edi, ebx
				mov		cx, si

				xor		edx, edx
				mov		ebx, x2
				sub		ebx, x1
				jns		det_phase
				
//	Если dx отрицательно, то инвертировать и сохранить знак
negative_dx:	neg		ebx
				mov		edx, 1

det_phase:
// Проверка случая 1
				cmp		ebx, 0				// dx == 0
				jnz		tst_case_2			// нет - проверка следующего случая

// Случай 1
				mov		ebx, TX2

case_1_loop:	add		word ptr [edi], ax
				add		edi, ebx
				loop	case_1_loop

				jmp		konetz

tst_case_2:	
// Проверка случая 2
				cmp		ebx, ecx			// dx == dy
				jnz		tst_case_3			// нет - проверка следующего случая

				test	edx, 1				// проверка знака, сохр. в dx
				jnz		case_2b				// есть знак - случай 2b

// Случай 2a
				mov		ebx, TX2

case_2a_loop:	add		word ptr [edi], ax
				add		edi, ebx
				add		edi, 2
				loop	case_2a_loop

				jmp		konetz

case_2b:
// Случай 2b
				mov		ebx, TX2

case_2b_loop:	add		word ptr [edi], ax
				add		edi, ebx
				sub		edi, 2
				loop	case_2b_loop

				jmp		konetz

tst_case_3:
// Проверка случая 3
				cmp		ebx, ecx
				jg		tst_case_4

				test	edx, 1
				jnz		case_3b

// Случай 3a
				mov		edx, ebx
				sal		edx, 1			// edx <== dx*2
				mov		ebx, ecx
				neg		ebx				// ebx <== -dy
				mov		esi, ecx		
				sal		esi, 1			// esi <== dy*2

case_3a_loop:	add		word ptr [edi], ax
				add		edi, TX2
				add		ebx, edx
				js		ecase_3a_loop

				sub		ebx, esi
				add		edi, 2

ecase_3a_loop:	loop	case_3a_loop

				jmp		konetz

case_3b:
// Случай 3b
				mov		edx, ebx
				sal		edx, 1			// edx <== dx*2
				mov		ebx, ecx
				neg		ebx				// ebx <== -dy
				mov		esi, ecx		
				sal		esi, 1			// esi <== dy*2

case_3b_loop:	add		word ptr [edi], ax
				add		edi, TX2
				add		ebx, edx
				js		ecase_3b_loop

				sub		ebx, esi
				sub		edi, 2

ecase_3b_loop:	loop	case_3b_loop

				jmp		konetz

tst_case_4:
// Предыдущие случаи не подходят => случай 4
				test	edx, 1
				jnz		case_4b

// Случай 4a
				mov		edx, ebx
				sal		edx, 1			// edx <== dx*2
				mov		esi, ecx
				sal		esi, 1			// esi <== dy*2
				
				xor		ebx, ebx		// ebx <== 0 (e)

case_4a_loop:	cmp		ebx, 0
				jl		ecase_4a_loop

				add		word ptr [edi], ax
				add		edi, TX2
				sub		ebx, edx
				dec		ecx
				jz		konetz

ecase_4a_loop:	add		ebx, esi
				add		edi, 2
				jmp		case_4a_loop

case_4b:
// Случай 4b
				mov		edx, ebx
				sal		edx, 1			// edx <== dx*2
				mov		esi, ecx
				sal		esi, 1			// esi <== dy*2
				
				xor		ebx, ebx		// ebx <== 0 (e)

case_4b_loop:	cmp		ebx, 0
				jl		ecase_4b_loop

				add		word ptr [edi], ax
				add		edi, TX2
				sub		ebx, edx
				dec		ecx
				jz		konetz

ecase_4b_loop:	add		ebx, esi
				sub		edi, 2
				jmp		case_4b_loop
konetz:	
	}
}

} // namespace
