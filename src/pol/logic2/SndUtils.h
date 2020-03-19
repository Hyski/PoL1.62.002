/**************************************************************                 
                                                                                
                             Paradise Cracked                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: Switching music themes depending on game situations
                
   Author : Lepakhin Mikhail a.k.a Flif
   Edited by: Klimov Alexander a.k.a. DiSpeLL
***************************************************************/

//
// ������� ��� ������ �� ������
//

#ifndef _PUNCH_DISPELL_SNDUTILS_H_
#define _PUNCH_DISPELL_SNDUTILS_H_

#include <Muffle/ISound.h>

class ThemesManager;

namespace SndUtils
{
    // ���� ������
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // ��������: 
    //     ��� ��������� ������� ������ � enum'� ���������� 
    //     �������� ������� ������ � ������� SndStrReader::m_desc
    //
    enum sound_type{
        SND_DRAG_FROM_HEAD,
        SND_DROP_TO_HEAD,
        SND_DRAG_FROM_BODY,
        SND_DROP_TO_BODY,
        SND_DRAG_FROM_HANDS,
        SND_DROP_TO_HANDS,
        SND_DRAG_FROM_KNEES,
        SND_DROP_TO_KNEES,
        SND_DRAG_FROM_BOX,
        SND_DROP_TO_BOX,
        SND_DRAG_FROM_GROUND,
        SND_DROP_TO_GROUND,
        SND_DRAG_FROM_MARKET,
        SND_DROP_TO_MARKET,
        SND_DRAG_FROM_IMPLANTS,
        SND_DROP_TO_IMPLANTS,
        SND_DRAG_FROM_BACKPACK,
        SND_DROP_TO_BACKPACK,
        SND_UNSUITABLE_PARAMETER,
        SND_INCOMPATIBLE_SUIT_AND_WEAPON,
        SND_BUY_FROM_TRADER_SUCCEEDED,
        SND_BUY_FROM_TRADER_FAILED,
        SND_SELL_TO_TRADER,
        SND_NO_ANY_QUEST,
        SND_NEW_QUEST,
        SND_LEVELUP,
        SND_LOGIC_DENIED,
        SND_JOURNAL_WAS_UPDATED,

        MAX_SOUND,
    };

    //��������� ��� ����� � ��� ��������
    const std::string& Snd2Str(sound_type sound);

    //���������� �������� ����� � ���������� ��� ��������� �����
    std::string MakeSoundFileName(const std::string& file_name);

    //������ �������� ������
    class EmptySoundSession
	{
    public:
        EmptySoundSession();
        ~EmptySoundSession();

    private:
        Muffle::HThemeSession m_session;
    };
};

//
// ������������� ������
//
class SndPlayer{
public:

    virtual ~SndPlayer(){}

    //singleton
    static SndPlayer* GetInst();

    //�������������/���������������
    virtual void Init() = 0;
    virtual void Shut() = 0;

    //����� �� ����������� ����
    virtual bool CanPlay() const = 0;

    //��������� ����
    virtual void Play(const std::string& snd) = 0;
    //��������� ���� � ����� ����� ������
    virtual void Play(const point3& from, const std::string& snd) = 0;
};

//
// ��������� ������������ ����������� ���
//
class MusicChanger{
public:

    virtual ~MusicChanger(){};

    enum music_type{
        MT_NONE,
        MT_QUIET,
        MT_UNEASY,
        MT_STRAINED, 
    };

    //������� ������� ������� ��������
    virtual music_type GetMusic() = 0;
    //����� �� ������� ������?
    virtual bool NeedUpdate() const = 0;
};

//
// ������������� ������ - ����������� �������
//
class JukeBox{  
public:

    //singleton
    static JukeBox* GetInst();

    ~JukeBox();

    //���������� ����� ��������� ����� ������
    void SetChanger(MusicChanger* changer);

    //��������� � ����� ������
    void Reset();

    //������������� ��������
    void Init();
    //��������� ������� � ������ ������ �����
    void Tick();

    //����������� ������������ ����
    void Resume();
    //���������� ������������ ������
    void Suspend();

protected:

    JukeBox();

private:

    void SwitchTheme(MusicChanger::music_type type, bool fsmooth = true);

private:

    float m_update_time;   //����� ��� ���������� ����������
    float m_switch_time;   //����� ������������ ����

    const float m_update_delta;  //�������� ����������
    const float m_quiet_delta;   //����� �������� �� ����� ��������� ����
    const float m_danger_delta;  //����� �������� �� ����������� ������

    MusicChanger::music_type m_music;

	// player
	ThemesManager*  m_player;
	// interface to the music changer
    MusicChanger*   m_changer;
};

#endif //_PUNCH_DISPELL_SNDUTILS_H_

/**************************************************************************

                                 END OF FILE

***************************************************************************/
