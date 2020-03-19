// KeyAnimation.cpp: implementation of the KeyAnimation class.
//
//////////////////////////////////////////////////////////////////////
#include "precomp.h"

#include "KeyAnimation.h"
#include <common/utils/optslot.h>

namespace KeyAnimationDetails {}
using namespace KeyAnimationDetails;

namespace KeyAnimationDetails
{
	template<typename T>
	struct ValueCompare
	{
		typedef std::pair<float,T> ValType;
		bool operator()(const ValType &l, const ValType &r) const
		{
			return l.first < r.first;
		}
		bool operator()(float l, const ValType &r) const
		{
			return l < r.first;
		}
		bool operator()(const ValType &l, float r) const
		{
			return l.first < r;
		}
	};
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
bool KeyAnimation::Save(OptSlot &slot)
{
	slot<<Angles.size();
	slot<<Trans.size();
	//запишем углы поворота
	RotKeys::const_iterator ang;
	for(ang=Angles.begin();ang!=Angles.end();ang++)
	{
		slot<<ang->first<<ang->second.x<<ang->second.y<<ang->second.z<<ang->second.w;
	}
	//запишем  данные о переносе
	TransKeys::const_iterator tr;
	for(tr=Trans.begin();tr!=Trans.end();tr++)
	{
		slot<<tr->first<<tr->second.x<<tr->second.y<<tr->second.z;
	}
	return true;
}

bool KeyAnimation::Load(OptSlot &slot)
{
	int asize,tsize;
	slot>>asize>>tsize;

	//прочитаем углы поворота
	Angles.swap(RotKeys(asize));
	Trans.swap(TransKeys(tsize));

	slot.Read(&Angles[0],Angles.size()*sizeof(RotKey));
	slot.Read(&Trans[0],Trans.size()*sizeof(TransKey));
	for(RotKeys::iterator i = Angles.begin(); i != Angles.end(); ++i)
	{
		std::swap(i->second.w,i->second.z);
		std::swap(i->second.y,i->second.z);
		std::swap(i->second.x,i->second.y);
	}

	Decimate(0);
	return true;
}

KeyAnimation::KeyAnimation()
{

}

KeyAnimation::~KeyAnimation()
{

}

void KeyAnimation::GetTrans(point3 *Offs,float Time) const
{
	TransKeys::const_iterator st,et;
	et = std::upper_bound(Trans.begin(),Trans.end(),Time,ValueCompare<point3>());
	//et=Trans.upper_bound(Time);
	if(et==Trans.end())
	{//не нужно интерпол€ции - да, прошли те времена
		*Offs=Trans.back().second;
	}
	else
	{
		st=et;
		if(et==Trans.begin())
		{//Ќе нужно интерпол€ции - врем€ анимации еще не пришло
			*Offs=et->second;
		}
		else
		{
			//интерпол€ци€
			--st;
			float in;
			in = (Time-st->first)/(et->first-st->first);
			Interpolate(Offs, et->second, st->second,in);
			//*Trans=et->second*in+st->second*(1-in);
		}
	}
}

void KeyAnimation::GetKeyTimes(std::set<float> *times)
{
	TransKeys::iterator tit=Trans.begin();       //ключи дл€ смещени€ объекта
	RotKeys::iterator rit=Angles.begin();  //ключи дл€ поворота объекта
	for(;tit!=Trans.end();tit++)
		times->insert(tit->first);
	for(;rit!=Angles.end();rit++)
		times->insert(rit->first);
}

void KeyAnimation::GetAngle(Quaternion *Quat,float Time) const
{
	RotKeys::const_iterator sq,eq;
	eq = std::upper_bound(Angles.begin(),Angles.end(),Time,ValueCompare<Quaternion>());
	//eq=Angles.upper_bound(Time);

	if(eq==Angles.end())
	{//не нужно интерпол€ции - да, прошли те времена
		*Quat=Angles.back().second;
	}
	else
	{
		sq=eq;
		if(eq==Angles.begin())
		{//Ќе нужно интерпол€ции - врем€ анимации еще не пришло
			*Quat=eq->second;
		}
		else
		{
			//интерпол€ци€
			sq--;
			float in;
			in = (Time-sq->first)/(eq->first-sq->first);
			*Quat=Quat->Slerp(in,sq->second,eq->second);
		}
	}
}

void KeyAnimation::SetKey(float Time, const point3 &Offs)
{
	TransKeys::iterator i = std::lower_bound(Trans.begin(),Trans.end(),Time,ValueCompare<point3>());
	if(i == Trans.end() || i->first != Time) Trans.insert(i,TransKey(Time,Offs));
	//Trans[Time]=Offs;
}

void KeyAnimation::SetKey(float Time, const Quaternion &Quat)
{
	RotKeys::iterator i = std::lower_bound(Angles.begin(),Angles.end(),Time,ValueCompare<Quaternion>());
	if(i == Angles.end() || i->first != Time) Angles.insert(i,RotKey(Time,Quat));
	//Angles[Time]=Quat;
}

void KeyAnimation::SetKey(float Time, const Quaternion &Quat, const point3 &Offs)
{
	SetKey(Time,Quat);
	SetKey(Time,Offs);
}

void KeyAnimation::Load(FILE *f)
{
	int asize,tsize;
	fscanf(f,"Angles Keys:%d\n",&asize);
	fscanf(f,"Trans Keys:%d\n",&tsize);
	int i;
	float x,y,z,w,t;
	//прочитаем углы поворота
	Angles.clear();
	Trans.clear();
	Angles.reserve(asize);
	Trans.reserve(tsize);
	for(i=0;i<asize;i++)
	{
		fscanf(f,"%f[%f %f %f %f] ",&t,&x,&y,&z,&w);
		SetKey(t,Quaternion(w,x,y,z));
		//Angles[t]=Quaternion(w,x,y,z);
	}
	fscanf(f,"\n");

	//прочитаем  данные о переносе
	for(i=0;i<tsize;i++)
	{
		fscanf(f,"%f[%f %f %f] ",&t,&x,&y,&z);
		SetKey(t,point3(x,y,z));
		//Trans[t]=point3(x,y,z);
	}
	fscanf(f,"\n");
	//char a;
	//fread(&a,1,sizeof(char),f);
}
void KeyAnimation::Save(FILE *f) const
{
	fprintf(f,"Angles Keys:%d\n",Angles.size());
	fprintf(f,"Trans Keys:%d\n",Trans.size());
	//запишем углы поворота
	RotKeys::const_iterator ang;
	for(ang=Angles.begin();ang!=Angles.end();ang++)
	{
		fprintf(f,"%f[%f %f %f %f] ",ang->first,ang->second.x,ang->second.y,ang->second.z,ang->second.w);
	}
	fprintf(f,"\n");

	//запишем  данные о переносе
	TransKeys::const_iterator tr;
	for(tr=Trans.begin();tr!=Trans.end();tr++)
	{
		fprintf(f,"%f[%f %f %f] ",tr->first,tr->second.x,tr->second.y,tr->second.z);
	}
	fprintf(f,"\n");

}
void KeyAnimation::SaveBin(FILE *f) const
{
#define _SAVE(a) fwrite(&a,1,sizeof(a),f);
	int asize=Angles.size();
	int tsize=Trans.size();
	_SAVE(asize);
	_SAVE(tsize);
	//запишем углы поворота
	RotKeys::const_iterator ang;
	for(ang=Angles.begin();ang!=Angles.end();ang++)
	{
		_SAVE(ang->first);
		_SAVE(ang->second);
	}
	//запишем  данные о переносе
	TransKeys::const_iterator tr;
	for(tr=Trans.begin();tr!=Trans.end();tr++)
	{
		_SAVE(tr->first);
		_SAVE(tr->second);
	}
#undef _SAVE
}

void KeyAnimation::LoadBin(VFile *f)
{
#define _LOAD(a) f->Read(&a,sizeof(a));
	int asize,tsize;
	_LOAD(asize);
	_LOAD(tsize);
	Quaternion lq;
	point3 lp;
	Angles.swap(RotKeys(asize));
	Trans.swap(TransKeys(tsize));
	f->Read(&Angles[0],Angles.size()*sizeof(RotKey));
	f->Read(&Trans[0],Trans.size()*sizeof(TransKey));
#undef _LOAD
}

namespace KeyAnimationDetails
{

#pragma pack(push,1)

struct RotData
{
	float tau;
	float x,y,z,w;
};

struct TransData
{
	float tau;
	float x,y,z;
};

#pragma pack(pop)

}

using namespace KeyAnimationDetails;

//=====================================================================================//
//                          bool KeyAnimation::MakeSaveLoad()                          //
//=====================================================================================//
bool KeyAnimation::MakeSaveLoad(SavSlot &slot)
{
	if(slot.IsSaving())
	{
		slot<<Angles.size();
		slot<<Trans.size();
		//запишем углы поворота
		RotKeys::const_iterator ang;
		for(ang=Angles.begin();ang!=Angles.end();ang++)
		{
			slot<<ang->first<<ang->second.x<<ang->second.y<<ang->second.z<<ang->second.w;
		}
		//запишем  данные о переносе
		TransKeys::const_iterator tr;
		for(tr=Trans.begin();tr!=Trans.end();tr++)
		{
			slot<<tr->first<<tr->second.x<<tr->second.y<<tr->second.z;
		}
	}
	else
	{
		Angles.clear();
		Trans.clear();

		int asize,tsize;
		slot>>asize>>tsize;

		//прочитаем углы поворота
		Angles.reserve(asize);
		Trans.reserve(tsize);

		std::vector<RotData> rots(asize);
		std::vector<TransData> trans(tsize);

		slot.Load(&rots[0],sizeof(rots[0])*asize);
		slot.Load(&trans[0],sizeof(trans[0])*tsize);

		for(int i = 0; i < asize; ++i)
		{
			SetKey(rots[i].tau,Quaternion(rots[i].w,rots[i].x,rots[i].y,rots[i].z));
		}

		for(int i = 0; i < tsize; ++i)
		{
			SetKey(rots[i].tau,point3(trans[i].x,trans[i].y,trans[i].z));
		}
	}

	return true;
}

//=====================================================================================//
//                              void KeyAnimation::Load()                              //
//=====================================================================================//
void KeyAnimation::Load(std::istream &stream)
{
	int asize,tsize;
	(stream.ignore(strlen("Angles Keys:"))>>asize).ignore(strlen("\n"));
	(stream.ignore(strlen("Trans Keys:"))>>tsize).ignore(strlen("\n"));
	int i;
	float x,y,z,w,t;
	Angles.clear();
	Trans.clear();
	Angles.reserve(asize);
	Trans.reserve(tsize);
	//прочитаем углы поворота
	for(i=0;i<asize;i++)
	{
		((stream>>t).ignore(1)>>x>>y>>z>>w).ignore(1);
		SetKey(t,Quaternion(w,x,y,z));
		//Angles[t]=Quaternion(w,x,y,z);
	}
	stream.ignore(strlen("\n"));

	//прочитаем  данные о переносе
	for(i=0;i<tsize;i++)
	{
		((stream>>t).ignore(1)>>x>>y>>z).ignore(1);
		SetKey(t,point3(x,y,z));
		//Trans[t]=point3(x,y,z);
	}
	stream.ignore(strlen("\n"));
}

//=====================================================================================//
//                           void KeyAnimation::Save() const                           //
//=====================================================================================//
void KeyAnimation::Save(std::ostream &stream) const
{
	stream<<"Angles Keys:"<<Angles.size()<<"\n";
	stream<<"Trans Keys:"<<Trans.size()<<"\n";
	//запишем углы поворота
	RotKeys::const_iterator ang;
	for(ang=Angles.begin();ang!=Angles.end();ang++)
	{
		stream<<ang->first<<"["<<ang->second.x<<" "
			<<ang->second.y<<" "<<ang->second.z<<" "
			<<ang->second.w<<"]";
	}
	stream<<"\n";

	//запишем  данные о переносе
	TransKeys::const_iterator tr;
	for(tr=Trans.begin();tr!=Trans.end();tr++)
	{
		stream<<tr->first<<"["
			<<tr->second.x<<" "
			<<tr->second.y<<" "
			<<tr->second.z<<"]";
	}
	stream<<"\n";
}
bool _eq(const Quaternion &a,const Quaternion &b)
{
	return a.x==b.x&&a.y==b.y&&a.z==b.z&&a.w==b.w;
}
void 	KeyAnimation::Decimate(float /*_err*/)
{
	return;
	//int size=Angles.size()+Trans.size();
	{
		unsigned int ae = Angles.size(), ang=0, ang_1=ae, ang_2=ae;
		//RotKeys::iterator ae=Angles.end(), ang=Angles.begin(), ang_1=ae, ang_2=ae;
		for(int i=0; i<2 && ang!=ae ; i++, ang_2=ang_1, ang_1=ang, ang++) {/**/};

		for(; ang!=ae; ang++)
		{
			if(    _eq(Angles[ang].second,Angles[ang_2].second)
				&& _eq(Angles[ang_1].second,Angles[ang_2].second))
			{
				Angles.erase(Angles.begin()+ang_1);
				ae=Angles.size();
				//ae=Angles.end();
				ang_1=ang;
				continue;
			}
			ang_2=ang_1;
			ang_1=ang;
		}
	}
	{
		unsigned int te = Trans.size(), tr = 0, tr_1 = te, tr_2 = te;
		//TransKeys::iterator te=Trans.end(), tr=Trans.begin(), tr_1=te, tr_2=te;
		for(int i=0; i<2 && tr!=te ; i++, tr_2=tr_1, tr_1=tr, tr++) {/**/};

		for(; tr!=te; tr++)
		{
			if(   Trans[tr].second==Trans[tr_2].second
				&& Trans[tr_1].second==Trans[tr_2].second)
			{
				Trans.erase(Trans.begin()+tr_1);
				te=Trans.size();
				tr_1=tr;
				continue;
			}
			tr_2=tr_1;
			tr_1=tr;
		}
	}


	//	int size2=size-(Angles.size()+Trans.size());
	return;
}

