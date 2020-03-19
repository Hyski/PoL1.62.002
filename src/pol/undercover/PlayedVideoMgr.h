#pragma once

namespace PoL
{

//=====================================================================================//
//                                  struct VideoEntry                                  //
//=====================================================================================//
struct VideoEntry
{
	std::string m_description;
	std::string m_bik;
};

//=====================================================================================//
//                                class PlayedVideoMgr                                 //
//=====================================================================================//
class PlayedVideoMgr
{
	static const VideoEntry m_videos[];

public:
	/// ���������� ������ �����, ������� �����-���� ���� ���������
	static std::vector<VideoEntry> getPlayedVideos();
	/// ���������, ��� ����� ���� ���������
	static void rememberVideo(const std::string &bik);
};

}
