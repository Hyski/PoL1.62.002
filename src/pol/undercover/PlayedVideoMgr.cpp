#include "precomp.h"
#include "PlayedVideoMgr.h"
#include "Options/Options.h"

namespace PoL
{

const VideoEntry PlayedVideoMgr::m_videos[] = 
{
	{"vmi_pub_logo",		"pub_logo.bik"},
	{"vmi_mist_logo",		"mist_logo.bik"},
	{"vmi_intro",			"intro.bik"},
	{"vmi_episode1",		"episode1.bik"},
	{"vmi_episode2",		"episode2.bik"},
	{"vmi_episode3",		"episode3.bik"},
	{"vmi_final3",			"final3.bik"},
	{"mvi_final_best",		"final_best.bik"}
};

//=====================================================================================//
//           std::vector<VideoEntry> PlayedVideoMgr::getPlayedVideos() const           //
//=====================================================================================//
std::vector<VideoEntry> PlayedVideoMgr::getPlayedVideos()
{
	std::vector<VideoEntry> videos;
	const int videoCount = sizeof(PlayedVideoMgr::m_videos)/sizeof(PlayedVideoMgr::m_videos[0]);

	for(int i = 0; i < videoCount; ++i)
	{
		const int mask = 1 << i;
		if(Options::GetInt("game.movies") & mask)
		{
			videos.push_back(m_videos[i]);
		}
	}

	return videos;
}

//=====================================================================================//
//                        void PlayedVideoMgr::rememberVideo()                         //
//=====================================================================================//
void PlayedVideoMgr::rememberVideo(const std::string &bik)
{
	const int videoCount = sizeof(PlayedVideoMgr::m_videos)/sizeof(PlayedVideoMgr::m_videos[0]);

	for(int i = 0; i < videoCount; ++i)
	{
		if(m_videos[i].m_bik == bik)
		{
			Options::Set("game.movies",Options::GetInt("game.movies")|(1<<i));
			break;
		}
	}
}

}

