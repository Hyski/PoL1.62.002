#pragma once

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
