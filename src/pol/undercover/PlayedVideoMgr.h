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
	/// ¬озвращает список видео, которые когда-либо были проиграны
	static std::vector<VideoEntry> getPlayedVideos();
	/// «апомнить, что видео было проиграно
	static void rememberVideo(const std::string &bik);
};

}
