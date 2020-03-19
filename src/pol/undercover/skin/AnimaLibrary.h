// AnimaLibrary.h: interface for the AnimaLibrary class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

#include <common/datamgr/txtfile.h>

class Skeleton;
class SkAnim;
class SkSkin;
class Skin;

class AnimaLibrary
{
	typedef std::map<std::string,Skeleton*> AnimaStorage;
	typedef std::map<std::string,SkAnim*> SkAnimaStorage;
	typedef std::map<std::string,SkSkin*> SkSkinsStorage;
	typedef std::string namestring;
	typedef std::map<std::string,TxtFile> XlsStorage;
protected:
	static AnimaLibrary Inst;
	AnimaStorage Animas;  //сюда загружаютс€ анимации дл€ совместного использовани€
	SkAnimaStorage SkAnimas;  //сюда загружаютс€ анимации дл€ совместного использовани€
	SkSkinsStorage SkSkins;  
	XlsStorage Tables;    //кеш дл€ .xls файлов
public:
	Skeleton *GetAnimation(const namestring &Name);
	SkAnim *GetSkAnimation(const namestring &Name);
	Skin *GetSkin(const namestring &Name);
	SkSkin *GetSkSkin(const namestring &Name);
	TxtFile *GetTable(const std::string& TableName);
	void Clear();
private:
	AnimaLibrary();
public:
	virtual ~AnimaLibrary();
public:
	static AnimaLibrary* GetInst(){return &Inst;};
};

