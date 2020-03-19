//
// ����� ��� ������� �����
//

#ifndef _PUNCH_CHEATPARSER_H_
#define _PUNCH_CHEATPARSER_H_

//
// ������ �������������� ����������������� �����
//
class CheatServer{
public:

    CheatServer();
    virtual ~CheatServer();

    //���� ����� ��������� ��������
    virtual void GiveItems(const rid_t& entity, const std::string& set);
    //�������� ����������� ���
    virtual void SetAlignment(const rid_t& rid_a, const rid_t& rid_b, int value);

    enum exp_type{
        ET_HACK,
        ET_KILL,
        ET_QUEST,
    };

    //��������� ����
    virtual void AddExperience(exp_type type, int value, const rid_t& rid = rid_t());
};

//
// ����� ��� ������� 
//
class CheatParser{
public:

    CheatParser(CheatServer* server = 0);
    ~CheatParser();

    //�������� ������� ���������� �������
    int GetLastPos();
    //���������� ������ cheat'��
    void SetServer(CheatServer* server);

    // ��������� ������, ��������� ��������
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // �������:        
    //          true  - �� ���� ������
    //          false - ���� ������ 
    // NOTE:
    //          ���� ���������� ��������� �� CheatServer == 0
    //          ������������ ������ ������, ��� ���������� ������
    //
    bool Parse(const std::string& script);  

private:

    enum token_type{
        T_HACK,       // hack   
        T_KILL,       // kill
        T_QUEST,      // quest 

        T_ALIGNMENT,  // aligment
        T_EXPERIENCE, // experience
        T_FRIEND,     // friend 
        T_ENEMY,      // enemy
        T_NEUTRAL,    // neutral
        T_GIVEITEMS,  // give_items
        T_SCRIPT,     // (*)

        T_IDENT,
        T_NUMBER,

        T_END,
        T_NONE,
    };

    struct token_s{
        int         m_val;
        std::string m_str;
        token_type  m_type;
    };

    token_type GetToken();

    void ParseAlignment();
    void ParseGiveItems();
    void ParseExperience();

private:

    class LexerError{};
    class ParseError{};

    token_s      m_token;
    CheatServer* m_server;

    std::istringstream m_input;
};

#endif // _PUNCH_CHEATPARSER_H_