
#include <precomp.h>
#include <mll/debug/profiler.h>

#include <fstream>
#include <numeric>
#include <functional>

using namespace mll::utils;
using namespace mll::debug;


profiler::times_t profiler::m_times;


//=====================================================================================//
//                    profiler::time_count_t *profiler::get_record()                    //
//=====================================================================================//
profiler::time_count_t *profiler::get_record(const std::string &str)
{
	times_t::iterator i = m_times.find(str);

	if (i != m_times.end())
	{
		return &(i->second);
	}
	else
	{
		const time_count_t init_val(0.0, 0);
		return &(m_times.insert(times_t::value_type(str, init_val)).first->second);
	}
	return 0;
}



//=====================================================================================//
//                                  profiler::flush()                                  //
//=====================================================================================//
void profiler::flush(std::ostream &stream)
{
	using namespace std;
	float max_time=0;
	for (times_t::iterator it = m_times.begin(); it != m_times.end(); ++it)
	{
		if(max_time<it->second.first) max_time = it->second.first;
	}



		

	stream << "Block\tTotal (msec)\tAverage (msec)\tCalls" << endl;
	for (times_t::iterator i = m_times.begin(); i != m_times.end(); ++i)
	{
		stream << i->first << "\t" 
			<< fixed << 1000.f*i->second.first 
					<<"("<<setprecision(2)<<100.f*i->second.first/max_time<<"%)" 
					<< setprecision(6)<<"\t" 

			<< (1000.f*i->second.first/i->second.second) << "\t" 
			<< i->second.second << endl;
	}
}