/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   22.03.2001

************************************************************************/
#ifndef _DIALOG_SCREEN_H_
#define _DIALOG_SCREEN_H_

#include "Dialog.h"
#include <common/Shell/Shell.h>

class DialogScreenController;

class DialogScreen : public Dialog, private ModalLoop
{
public:
	static const char* m_pDialogName;
	static const char* m_PortraitPath;
	static const char* m_PortraitExt;
private:
	DialogScreenController* m_pController;
	Widget::Player::Sample* m_SpeechSound;
public:
	DialogScreen();
	virtual ~DialogScreen();
private:
	//	����������� �� ��������� ��������� �������
	void OnSystemChange(void);
	//	��������� �� ������
	void OnButton(Button* pButton);
	//	��������� �� ��������
	void OnStatic(Static* pStatic);
	//	���������� �����-�� ��������
	bool Tick(void);
public:
	struct DialogTraits
	{
		std::string m_PortraitName;			//	��������� ��� �����
		std::string m_Text;					//	����� :)
		std::string m_Speech;				//	���� �� �������
		float m_Volume;
	};
	//	��������� ��������� ����������� ���� 
	void SetTraits(const DialogTraits& dt);
	//	���������� ����������
	void SetController(DialogScreenController* pController);
	//	������� ������ ��� ���������
	void DoModal(const DialogTraits& dt);
};

//**********************************************************************//
//*********	       class DialogScreenController            *************//
class DialogScreenController
{
public:
	virtual ~DialogScreenController(){};
public:
	// ������� �������� ����
	virtual void OnClose(void) = 0;
};

//**********************************************************************//
//	class HeroInfoScreen
class HeroInfoScreenController;

class HeroInfoScreen : public Dialog, private ModalLoop
{
public:
	static const char* m_pDialogName;
	static const char* m_PortraitPath;
	static const char* m_PortraitExt;
private:
	HeroInfoScreenController* m_Controller;
public:
	HeroInfoScreen();
	virtual ~HeroInfoScreen();
private:
	//	����������� �� ��������� ��������� �������
	void OnSystemChange(void);
	//	��������� �� ������
	void OnButton(Button* pButton);
	//	���������� �����-�� ��������
	bool Tick(void);
public:
	struct HeroTraits
	{
		std::string m_PortraitName;			//	��������� ��� �����
		std::string m_Text;					//	����� :)
	};
	//	���������� ����������
	void SetController(HeroInfoScreenController* controller);
	//	������� ������ ��� ���������
	void DoModal(const HeroTraits& ht);
private:
	//	��������� ��������� ����������� ���� 
	void SetTraits(const HeroTraits& ht);
};

//**********************************************************************//
//	class HeroInfoScreenController
class HeroInfoScreenController
{
public:
	virtual ~HeroInfoScreenController(){};
public:
	// ������� �������� ����
	virtual void OnClose(void) = 0;
};

//**********************************************************************//
//	class HelperScreen
class HelperScreen : public Dialog, private ModalLoop
{
public:
	struct Traits
	{
		std::string m_Image;				//	��������
		std::string m_Text;					//	����� :)
		bool m_ShowThisMessage;				//	���������� ��� ��������� ������������
		bool m_ShowHelper;					//	���������� ����� ��������� ������������
	};
	static const char* m_pDialogName;
private:
	VRegion m_ImagePlace;
	Widget* m_Image;
	Static* m_ShowThisMessage;
	Static* m_ShowHelper;
public:
	HelperScreen();
	virtual ~HelperScreen();
private:
	//	����������� �� ��������� ��������� �������
	void OnSystemChange(void);
	//	��������� �� ��������
	void OnStatic(Static* pStatic);
	//	���������� �����-�� ��������
	bool Tick(void);
public:
	//	������� ������ ��� ���������
	void DoModal(Traits& t);
private:
	//	��������� ��������� ����������� ���� 
	void SetTraits(const Traits& t);
};

//**********************************************************************//
//	class FlagDescScreen
class FlagDescScreen : public Dialog, private ModalLoop
{
public:
	struct Traits
	{
		std::string m_Image;				//	��������
		std::string m_Text;					//	����� :)
	};
	static const char* m_pDialogName;
private:
	VRegion m_ImagePlace;
	Widget* m_Image;
public:
	FlagDescScreen();
	virtual ~FlagDescScreen();
private:
	//	����������� �� ��������� ��������� �������
	void OnSystemChange(void);
	//	��������� �� ��������
	void OnStatic(Static* pStatic);
	//	���������� �����-�� ��������
	bool Tick(void);
public:
	//	������� ������ ��� ���������
	void DoModal(const Traits& t);
private:
	//	��������� ��������� ����������� ���� 
	void SetTraits(const Traits& t);
};

#endif	//_DIALOG_SCREEN_H_