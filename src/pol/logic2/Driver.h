#pragma once

#include "ActivityCommand.h"

class Activity;

//=====================================================================================//
//                                    class Driver                                     //
//=====================================================================================//
// ������� - ����� ����������� ���������
class Driver
{
public:
    virtual ~Driver() {}

    //�������� ���������� ��������
    virtual bool Run(Activity* activity, bool activity_result) = 0;

    //������ ������ ������������� ���� ��������
    virtual activity_command CalcStep(Activity* activity, activity_command user_cmd) = 0;
    //������ ����� �� ����� ��������
    virtual activity_command CalcMovepnts(Activity* activity, activity_command user_cmd) = 0;
};
