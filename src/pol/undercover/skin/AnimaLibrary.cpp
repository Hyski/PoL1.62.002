// AnimaLibrary.cpp: implementation of the AnimaLibrary class.
//
//////////////////////////////////////////////////////////////////////
#include "precomp.h"
#include "skeleton.h"
#include "skAnim.h"
#include "skin.h"
#include "AnimaLibrary.h"
#include "../globals.h"
#include <common/utils/optslot.h>

AnimaLibrary AnimaLibrary::Inst;

//=====================================================================================//
//                            AnimaLibrary::AnimaLibrary()                             //
//=====================================================================================//
AnimaLibrary::AnimaLibrary()
{
}

//=====================================================================================//
//                            AnimaLibrary::~AnimaLibrary()                            //
//=====================================================================================//
AnimaLibrary::~AnimaLibrary()
{
	Clear();
}

//=====================================================================================//
//                             void AnimaLibrary::Clear()                              //
//=====================================================================================//
void AnimaLibrary::Clear()
{
	AnimaStorage::iterator it=Animas.begin();
	while(it!=Animas.end())
	{
		DESTROY(it->second);
		it++;
	}
	Animas.clear();
	SkAnimaStorage::iterator it1=SkAnimas.begin();
	while(it1!=SkAnimas.end())
	{
		DESTROY(it1->second);
		it1++;
	}
	SkAnimas.clear();
	SkSkinsStorage::iterator it2=SkSkins.begin();
	while(it2!=SkSkins.end())
	{
		DESTROY(it2->second);
		it2++;
	}
	SkSkins.clear();
}

//=====================================================================================//
//                            Skin *AnimaLibrary::GetSkin()                            //
//=====================================================================================//
Skin *AnimaLibrary::GetSkin(const namestring &Name)
{
	Skin *s=new Skin;
	bool ret=s->Load(Name);
	if(!ret) {delete s;s=NULL;}
	return s;
}

//=====================================================================================//
//                          SkSkin *AnimaLibrary::GetSkSkin()                          //
//=====================================================================================//
SkSkin *AnimaLibrary::GetSkSkin(const namestring &Name)
{
	SkSkinsStorage::iterator it;
	it=SkSkins.find(Name);
	if(it!=SkSkins.end())
		return (new SkSkin(*it->second));


	//1. ��������� ��������� ����� ������
	VFile *in=DataMgr::Load(Name.c_str());
	if(!in->Size()) 
	{
		DataMgr::Release(Name.c_str());
		return NULL;
	}
	char ID[5];
	in->Read(ID,5);
	if(!strcmp(ID,"SKN2"))
	{
		SkSkin *s = new SkSkin;
		OptSlot data;
		data.Load(in);
		s->Load2(data);
		DataMgr::Release(Name.c_str());

		SkSkins[Name] = new SkSkin(*s);
		return s;
	}
	else if(!strcmp(ID,"SKIN"))
	{
		SkSkin *s = new SkSkin;
		OptSlot data;
		data.Load(in);
		s->Load(data);
		DataMgr::Release(Name.c_str());

		SkSkin *a = new SkSkin(*s);
		delete s;

		//3. ������� � ����� �������
		FILE *f=fopen(Name.c_str(),"w+b");
		if(f)
		{
			char ID[5]="SKN2";
			fwrite(ID,1,5,f);
			OptSlot data;
			a->Save(data);
			data.Save(f);
			fclose(f);
		}

		SkSkins[Name] = new SkSkin(*a);
		return a;
	}
	else
	{
		//2. ��������� ��������� ������ ������
		DataMgr::Release(Name.c_str());
		Skin *s=new Skin;
		bool ret=s->Load(Name);
		if(!ret)
		{
			delete s;
			s=NULL;
			return NULL;
		}

		SkSkin *a=new SkSkin(s);
		delete s;

		//3. ������� � ����� �������
		FILE *f=fopen(Name.c_str(),"w+b");
		if(f)
		{
			char ID[5]="SKN2";
			fwrite(ID,1,5,f);
			OptSlot data;
			a->Save(data);
			data.Save(f);
			fclose(f);
		}

		SkSkins[Name]=new SkSkin(*a);
		return a;
	}
}

//=====================================================================================//
//                       Skeleton *AnimaLibrary::GetAnimation()                        //
//=====================================================================================//
Skeleton *AnimaLibrary::GetAnimation(const namestring &Name)
{
	AnimaStorage::iterator it;
	it=Animas.find(Name);
	if(it!=Animas.end())
		return it->second;
	else
	{
		Skeleton *t=new Skeleton();
		if(t->Load(Name))
		{
			Animas[Name]=t;
			t->SetLinks("Bip01 R Foot","Bip01 L Foot");
			if((!t->Link1||!t->Link2)&&t->BonesNum)
				t->SetLinks(t->Body[0].Name,t->Body[0].Name);
			//��������� �������� � ����
			Quaternion q;
			Transformation World;
			q.FromAngleAxis(0,NULLVEC);
			q.ToRotationMatrix(World.rot);
			World.trans=NULLVEC;
			t->SetWorld(&World);

			return t;
		}
		else
			delete t;
		return NULL;
	}
}

//=====================================================================================//
//                       SkAnim *AnimaLibrary::GetSkAnimation()                        //
//=====================================================================================//
SkAnim *AnimaLibrary::GetSkAnimation(const namestring &Name)
{
	//logFile["failed_skels.log"]("������ ��������: '%s'\n",Name.c_str()); 
	SkAnimaStorage::iterator it;
	it=SkAnimas.find(Name);
	if(it!=SkAnimas.end())
		return it->second;
	else
	{
		//1. ��������� ����� �� ����� ����� ������
		VFile *in=DataMgr::Load(Name.c_str());
		if(!in) return NULL;
		char ID[5];
		in->Read(ID,5);
		if(!strcmp(ID,"SKEL"))
		{
			SkAnim *tt=new SkAnim(Name);
			OptSlot data;
			data.Load(in);
			tt->Load(data);
			SkAnimas[Name]=tt;
			DataMgr::Release(Name.c_str());
			return tt;
		}
		else
		{
			//2. ���� ������ ������� ���, ������ ������ � ������� � ����� �������.
			DataMgr::Release(Name.c_str());

			Skeleton *t=new Skeleton();
			if(t->Load(Name))
			{
				Animas[Name]=t;
				t->SetLinks("Bip01 R Foot","Bip01 L Foot");
				if((!t->Link1||!t->Link2)&&t->BonesNum)
					t->SetLinks(t->Body[0].Name,t->Body[0].Name);

				//��������� �������� � ����
				Quaternion q;
				Transformation World;
				q.FromAngleAxis(0,NULLVEC);
				q.ToRotationMatrix(World.rot);
				World.trans=NULLVEC;
				t->SetWorld(&World);
				SkAnim *tt=new SkAnim(t,Name);

				//����������� ������ �� ���� ������ �������
				FILE *f=fopen(Name.c_str(),"w+b");
				if(f)
				{
					char ID[5]="SKEL";
					fwrite(ID,1,5,f);
					OptSlot data;
					tt->Save(data);
					data.Save(f);
					fclose(f);
				}
				SkAnimas[Name]=tt;
				return tt;

			}
			else
				delete t;
		}

		//if(Name.size()) logFile["failed_skels.log"]("������ ��� �������� �����: '%s'\n",Name.c_str()); 
		return NULL;
	}
}

//=====================================================================================//
//                          TxtFile *AnimaLibrary::GetTable()                          //
//=====================================================================================//
TxtFile *AnimaLibrary::GetTable(const std::string& TableName)
{
	XlsStorage::iterator it;
	it=Tables.find(TableName);
	if(it!=Tables.end())
		return &it->second;
	else
	{
		Tables[TableName]=TxtFile(DataMgr::Load(TableName.c_str()));
		return &Tables[TableName];
	}
}

