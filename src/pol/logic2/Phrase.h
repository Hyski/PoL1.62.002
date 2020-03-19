#pragma once

//=====================================================================================//
//                                   struct phrase_s                                   //
//=====================================================================================//
// фраза которую говорит персонаж
struct phrase_s
{
    std::string m_text;
    std::string m_sound;

    phrase_s(){}

    //создание фразы путем парсинга строки
    phrase_s(const std::string& str4parse);
    //явное создание фразы
    phrase_s(const std::string& txt, const std::string& snd) :
        m_text(txt), m_sound(snd) {}
};
