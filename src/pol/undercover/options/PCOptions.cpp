/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2000

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   29.10.2000

************************************************************************/
#include "Precomp.h"
#include "Options.h"
#include "XRegistry.h"
#include "PCOptions.h"

void PCOptions::Register(void)
{
	//	------------- ����������� ���������� --------------
	Options::Set("system.video.driver",0);
	Options::Set("system.video.device",0);
	Options::Set("system.video.width",1024);
	Options::Set("system.video.height",768);
	Options::Set("system.video.bpp",32);
	Options::Set("system.video.freq",0);
	Options::Set("system.video.windowed",0);
	Options::Set("system.input.sensitivity",1.7f);
	
	//	��������� �����
	Options::Set("system.sound.type",0);
	Options::Set("system.sound.update_freq",1.0f);// ������� ���������� �������� ����������
	Options::Set("system.sound.themes",1);
	Options::Set("game.sound.voicesvol",50);
	Options::Set("game.sound.ambientvol",50);
	Options::Set("game.sound.effectsvol",50);
	Options::Set("game.sound.musicvol",50);

	//
	Options::Set("system.video.gamma",0.8f);
	Options::Set("system.video.triple",0);
	Options::Set("system.video.effectsquality",1.0f);
	Options::Set("system.video.tl_optimization",1); //	���� ��������� ���� ��� T&L 
	Options::Set("system.video.lightmaps",1);		//	���� ��� ���������/����. ���� ������������
	Options::Set("game.anispeed",1.5f);				//	�������� ��������
	Options::Set("game.animaquality",1.0f);
	Options::Set("game.cursor_refreshtime",0.150f);	//	����� ���������� ������� � ����
	Options::Set("game.marks_on_walls",1);			//	�������� �� ������
	Options::Set("game.dynamic_lights",1);			//	������������ ����
	Options::Set("game.environment",1);				//	���������� �����
	Options::Set("game.camera_speed",1.5f);			//	�������� ����������� ������

	//	---- �������� ������� ---------------------------------
	Options::Set("system.video.texquality",0);
	Options::Set("system.video.mipmap",-1);
	Options::Set("system.video.usecompressing",1);
	//	-------------------------------------------------------
	Options::Set("system.net.refreshtime",3.0f);
	Options::Set("game.player","Player");
	//	---- ���� ���������� ----------------------------------
	Options::Set("system.video.shadowrate",0.1f);
	Options::Set("system.video.shadowquality",6);
	//	---- ���������/���������� �������� --------------------
	Options::Set("game.mark_wave_front",1);	 // ��������� ������ ����� ���� ����� ����� ����
	Options::Set("game.mark_exit_zone",1);	 // ��������� ��� ������
	Options::Set("game.mark_path",1);		 // ��������� ���� (�������� ��� �� ��������)
    Options::Set("game.mark_lands", 1);      // �������� ����� ������� �� �������
    Options::Set("game.show_fos", 0);        // ��������� ���������� Field Of Sight 
    Options::Set("game.show_banner",0);      // ��������� ���������� banner ��� entity
	////	��� �������� ����������� �������
	//Options::Set("game.advance",256);		 //	������� ������������ ������������ ��� (256-value)
	//																			//256 - ����� ��� ����� (������ 3 ��������)
	//																			//255 +������� 1 �������
	//																			//254 +������� 2 �������
	//																			//253 +������� 3 �������
	//																			//252 +������� 4 �������
	//																			//251 + ��������� �������

	Options::Set("game.movies",0); // ������, ������� ��� �������������

	Options::Set("game.mod","");			//	������������ ���, ������ ������ - �������� ������������ ������
	//	���������� �������
    Options::Set("game.helper",1);			// ��������� ���������� ������
	//	������� ��������� ����
	Options::Set("game.type",2);
	//	0 - Novice
	//	1 - Easy
	//	2 - Normal
	//	3 - Hard

    //	---- ��������� ������ hidden_movement --------------------

    Options::Set("game.hidden_movement.enemy_related.move",  1);  //���������� ����������� ���������� ����������
    Options::Set("game.hidden_movement.enemy_related.shoot", 1);  //���������� �������� ���������� ����������

    Options::Set("game.hidden_movement.friend_related.move",  1);  //���������� ����������� ������������� ����������
    Options::Set("game.hidden_movement.friend_related.shoot", 1);  //���������� �������� ������������� ����������

    Options::Set("game.hidden_movement.neutral_related.move",  1);  //���������� ����������� ����������� ����������
    Options::Set("game.hidden_movement.neutral_related.shoot", 1);  //���������� �������� ����������� ����������

    Options::Set("game.hidden_movement.civilian_vehicles.move",  1);  //���������� ����������� ������ �������
    Options::Set("game.hidden_movement.civilian_vehicles.shoot", 1);  //���������� �������� ������ �������

    Options::Set("game.hidden_movement.civilian_humans_traders.move",  1);  //���������� ����������� ������ �������
    Options::Set("game.hidden_movement.civilian_humans_traders.shoot", 1);  //���������� �������� ������ �������

	//*********************************************************
	//	������ � ��������
	//	�������������� ���� ***********************************
	Options::Registry()->Init(HKEY_LOCAL_MACHINE,"SOFTWARE\\MiST land\\Power of Law CE\\");
	//	����������� ���������� ********************************
	char cur_dir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH,cur_dir);
	//	���� �� �������� � ����������
	Options::Registry()->Register("Video Path",std::string(std::string(cur_dir)+"\\Video"));
	Options::Registry()->Register("Packs Path","");
	//	��������� �������� �� ������� *************************
	Options::Registry()->Read();
}
