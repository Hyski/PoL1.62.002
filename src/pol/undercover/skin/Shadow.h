#if !defined(__GVOZDODER_SHADOW_TEXTURE__)
#define __GVOZDODER_SHADOW_TEXTURE__

#include <Common\GraphPipe\SimpleTexturedObject.h>
//#include <Options\Options.h>
#include <Common\TextureMgr\TextureMgr.h>
#include <Common\GraphPipe\GraphPipe.h>
#include "person.h"

#include "Slicer.h"

class VShader;
namespace PoLLevelUtilsDetail { class ShdTriangleCollector; }

namespace Shadows
{
	class BaseShadow;
	class ComplexShadow;
	class SimpleShadow;
	class ShadowUtility;

//#	define GVZ_SHADOW_LOG

#	if defined(GVZ_SHADOW_LOG)
#		include "..\Common\Log\log.h"
		extern CLog gvzLog;
#		define GVZ_LOG	gvzLog["Shadow.log"]
#	endif // GVZ_SHADOW_LOG

#	if !defined(GVZ_SHADOW_LOG)
#		define GVZ_LOG	;/##/
#	endif

//=====================================================================================//
//                                     class Sync                                      //
//=====================================================================================//
	class Sync
	{
	public:
		HANDLE m_event;

		Sync();
		~Sync();

		void wait();
		void acquire();
		void leave();

		static Sync &inst();
	};

//=====================================================================================//
//                                   class SyncGuard                                   //
//=====================================================================================//
	class SyncGuard
	{
	public:
		SyncGuard() { Sync::inst().acquire(); }
		~SyncGuard() { Sync::inst().leave(); }
	};

//=====================================================================================//
//                                    struct HtData                                    //
//=====================================================================================//
	struct HtData
	{
		unsigned char *m_data;
		long m_pitch;
		LPDIRECTDRAWSURFACE7 m_surface;
		ComplexShadow *m_shadow;

		HtData() : m_data(0), m_surface(0), m_shadow(0) {}

		void clear()
		{
			if(m_data)
			{
				m_surface->Unlock(NULL);
				m_data = 0;
				m_surface = 0;
				m_shadow = 0;
			}
		}

		static HtData g_inst;
	};

	typedef std::list<Triangle> trilist;

//=====================================================================================//
//                                 class ShadowBuffer                                  //
//=====================================================================================//
	class ShadowBuffer
	{
		Primi m_primitive;
		enum {MAXTRIANGLES = 200};

	public:
		ShadowBuffer();
		~ShadowBuffer();

		void CollectPlanes (BaseShadow *, float, int CurIndex = 0);
		void Draw (GraphPipe *, const std::string &);
		void FlushGeometry ();

	private:
		void cutPoly (gPoly &, gPoly &, float ,float ,float, float);
	};

////////////////////////////////////////////////////////// BaseShadow 
	class BaseShadow // Базовый класс для тени
	{
	friend ShadowUtility;
	friend ShadowBuffer;
	friend PoLLevelUtilsDetail::ShdTriangleCollector;

	protected:
		bool m_dirty;
		std::string m_shader;

	public:
		BaseShadow(RealEntity *);
		BaseShadow(BaseShadow *);
		virtual ~BaseShadow();

		virtual void Update(float) = 0;					// обновление тени
		virtual bool PoLNeedUpdate(float) { return true; }
		virtual bool IsComplex() const { return false; }

		// Объекты для тени
		virtual void AddObject(const TexObject *);		// Добавить объект
		virtual void AddObject(SimpleTexturedObject *);		// Добавить объект
		virtual void Clear();						// Очистить информацию обо всех объектах

		virtual void Draw(GraphPipe *) = 0;		// отрисовка тени
		virtual void UpdateOnModeChange() {}

		virtual void SetShadowTexture(const std::string &) {}
		void SetRedrawPeriod(float p);
		void SetLight(point3 &);

		float GetKoef(float);

		ShadowBuffer &GetCache() { return cache; }

		std::list<SimpleTexturedObject *> objects;
		RealEntity *entity; // владелец тени

	protected:
		D3DMATRIX axis;
		point3 *bindPoint;		// точка привязки тени
		const D3DMATRIX *parentWorld;	// матрица преобразований объекта


		const D3DMATRIX *invParentWorld;// обратная
		std::string txtName;	// имя текстуры

		static unsigned int cntShdwGen; // счетчик для генерации уникальных названий текстур

		point3 lt;				// вектор освещения

		float tRedrawPeriod;		// С каким периодом обновлять тень
		float tLastCalc;			// Время последнего обновления
		float deadTime;

		ShadowBuffer cache;	// кэш
		bool Updated, NeedUpdate;

		float minx, maxx, miny, maxy;
	};

//////////////////////////////////////////////////////////// SimpleShadow
	class SimpleShadow : public BaseShadow // простая тень
	{
	friend ShadowUtility;
	public:
		SimpleShadow (RealEntity *);
		SimpleShadow (BaseShadow *);
		~SimpleShadow ();

		void SetShadowTexture (const std::string &);

		void Update (float);

		void AddObject (const TexObject *);
		void AddObject (SimpleTexturedObject *);		// Добавить объект
		void Clear ();

		void Draw (GraphPipe *);
	private:
		std::string txtSimple;
		float d;

		void Calc ();
	};

	class NullShadow : public BaseShadow
	{
	friend ShadowUtility;
	public:
		NullShadow (RealEntity *entity) : BaseShadow(entity) {}
		NullShadow (BaseShadow *elder) : BaseShadow(elder)
		{
			for (std::list<SimpleTexturedObject *>::iterator i = elder->objects.begin(); i != elder->objects.end(); ++i)
			{
				AddObject(*i);
			}
		}

		virtual ~NullShadow() {}

		void Update(float) {}					// обновление тени

		// Объекты для тени
		//void AddObject(const TexObject *o) { BaseShadow::AddObject(o); }		// Добавить объект
		//void AddObject(SimpleTexturedObject *o) { BaseShadow::AddObject(o); }// Добавить объект
		void Clear() {BaseShadow::Clear(); } // Очистить информацию обо всех объектах

		void Draw(GraphPipe *) {}		// отрисовка тени
		void UpdateOnModeChange() {}
	};

////////////////////////////////////////////////////////// ShadowUtility
	class ShadowUtility
	{
	friend BaseShadow;
	friend ComplexShadow;
	friend SimpleShadow;
	friend void OptionsChanged();
	public:
		static void AddShadow (std::string &, BaseShadow *);
		static void KillShadow (const std::string &);

		static void CopyImageToSurface (const ComplexShadow *);

		static point3 projPoint2Surface (const point3 &, const point3 &, const point3 &, const point3 &);

		static void Line (const ComplexShadow *, int, int, int, int, short int);

		static void DrawLights (GraphPipe *);

		static void AddTime (float curTime, float relTime);
		static float GetTime ();

		static void AddTimeD (float curTime, float relTime);
		static float GetTimeD ();

		static float getLightIntensity (const point3 &); // получить интенсивность света в точке
		static point3 getLightDirection (const point3 &); // получить направление света в точке
		static bool NoKill ();

	private:

		static float timeCalc;
		static float timeInProcess;
		static float timeBegin;

		static float timeCalcD;
		static float timeInProcessD;
		static float timeBeginD;

		static bool bNoKill;
	};

	void UpdateOnModeChange ();
	void OptionsChanged ();
	void DrawShadows (GraphPipe *);

	// Источники света
	void AddLight (const point3 &, float, float);
	void ClearLights ();
	BaseShadow *CreateShadow (RealEntity *);
	BaseShadow *ReCreateShadow (BaseShadow *);

} // namespace

#endif