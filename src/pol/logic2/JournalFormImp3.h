//
// ���������� ���� ������� iteration3
//

#ifndef _PUNCH_JOURNALFORMIMP3_H_
#define _PUNCH_JOURNALFORMIMP3_H_

#include "form.h"
#include <undercover/interface/journalscreen.h>

//
// ���������� ���� �������
//
class JournalFormImp3 : public  JournalForm,
                        private JournalScreen::Controller
{
public:

    JournalFormImp3();
    ~JournalFormImp3();

    //�������� ���� 
    void Show();
    //������������� ����
    void Init(const ini_s& ini);
    //���������/��������� ��������� ����
    void MakeSaveLoad(SavSlot& slot);
    //��������� �����
    void HandleInput(state_type* st);

    //���� �� ����� ���������� � ����
    bool IsUpdated() const;

    //
    // ��������� ������� �������
    //
    class Sheet{
    public:
        
        virtual ~Sheet(){}
        
        enum show_mode{
            SM_USUAL,   //������� ����� �������
            SM_UPDATES, //������� ���������� ��� ������� 
        };
        
        //���� �� ������� ��������� �� ������� ���������� ���������
        virtual bool IsUpdated() const = 0;
        //�������� �������
        virtual void Show(show_mode mode) = 0;
        //��������� �������
        virtual void MakeSaveLoad(SavSlot& slot) = 0;
    };

private:

    //������� ���� �������
    void HandleExitFormReq();

    //��������� �������� ����
    void OnBack(void);
    //��������� ��������� �������
    void OnSheetClick(JournalScreen::SHEET id);
    //��������� ������� ������ ����������
    void OnUpdateClick(JournalScreen::UPDATE_BUTTON id);

private:

    enum{ MAX_SHEETS = 5, };

    Sheet* m_sheet[MAX_SHEETS];

    float m_delta_time;   //�������� ���������� ������
    float m_update_time;  //����� ���������� ���������� ������
};

#endif // _PUNCH_JOURNALFORMIMP3_H_