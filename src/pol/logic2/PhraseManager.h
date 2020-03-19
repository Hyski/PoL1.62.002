//
// ������� ��� ������ � ������� ����������
//

#ifndef _PUNCH_PHRASEMANAGER_H_
#define _PUNCH_PHRASEMANAGER_H_

#error PhraseManager is obsolete

//=====================================================================================//
//                                   struct phrase_s                                   //
//=====================================================================================//
// ����� ������� ������� ��������
struct phrase_s
{
    std::string m_text;
    std::string m_sound;

    phrase_s(){}

    //�������� ����� ����� �������� ������
    phrase_s(const std::string& str4parse);
    //����� �������� �����
    phrase_s(const std::string& txt, const std::string& snd) :
        m_text(txt), m_sound(snd) {}
};

//=====================================================================================//
//                                 class PhraseManager                                 //
//=====================================================================================//
// �������� ���� ��� ���������� 
class PhraseManager
{
public:

    virtual ~PhraseManager(){}

    //singleton
    static PhraseManager* GetUsePhrases();
    static PhraseManager* GetFirstPhrases();

    //�������������/���������������
    virtual void Init() = 0;
    virtual void Shut() = 0;

    //������/������ �� Save'a
    virtual void MakeSaveLoad(SavSlot& st) = 0;

    //����� �� ������� �����
    virtual bool CanSay(BaseEntity* entity) = 0;
    //�������� ����� ��� ���������
    virtual void MarkAsSaid(BaseEntity* entity) = 0;

    //���������� �� �����
    virtual bool IsExists(BaseEntity* entity) = 0;
    //�������� �����
    virtual const phrase_s& GetPhrase(BaseEntity* entity) = 0;
};    

#endif // _PUNCH_PHRASEMANAGER_H_