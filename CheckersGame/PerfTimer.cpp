#include "StdAfx.h"
#include "PerfTimer.h"

bool CPerfTimer::s_init = false;
LARGE_INTEGER CPerfTimer::s_frequency;

std::ostream& operator <<(std::ostream& os, const CPerfTimer& timer)
{
	double callsPerUsec = (double)timer.m_totalTime / (double)timer.m_calls;
	if( timer.m_calls > 10000 )
	{
		os << "---------------------------------------------------" << std::endl;
		os << timer.m_name << ": " << callsPerUsec * 10000 << " us/kcall" << std::endl;
		os << timer.m_name << ": " << timer.m_calls / 10000.f << " kcalls" << std::endl;
	}
	else
	{
		os << "---------------------------------------------------" << std::endl;
		os << timer.m_name << ": " << callsPerUsec << " us/call" << std::endl;
		os << timer.m_name << ": " << timer.m_calls << " calls" << std::endl;
	}
	return os;
}
