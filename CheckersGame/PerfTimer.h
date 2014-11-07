#pragma once

#include <string>
#include <windows.h>
#include <iostream>

class CPerfTimerCall;

class CPerfTimer
{
	friend class CPerfTimerCall;
public:
	CPerfTimer( const std::string& name ) : m_name(name), m_totalTime(0), m_calls(0) { Init(); }
	~CPerfTimer(void) {}

	friend std::ostream& operator <<(std::ostream& os, const CPerfTimer& timer);

protected:
	void StartCall() 
	{ 
		QueryPerformanceCounter(&m_lastTime); 
		m_calls++; 
	}
	void EndCall() 
	{ 		
		LARGE_INTEGER endTime; 
		QueryPerformanceCounter(&endTime); 
		m_totalTime += ((endTime.QuadPart - m_lastTime.QuadPart) * 1000000) / s_frequency.QuadPart; 
	}

private:
	static bool s_init;
	static LARGE_INTEGER s_frequency;
	static void Init()
	{
		if( !s_init )
		{
			s_init = true;
			QueryPerformanceFrequency(&s_frequency);
		}
	}

private:
	std::string m_name;
	LARGE_INTEGER m_lastTime;
	unsigned __int64 m_totalTime;
	unsigned int m_calls;
};

class CPerfTimerCall
{
public:
	CPerfTimerCall( CPerfTimer& timer ) : m_timer(timer) { m_timer.StartCall(); }
	~CPerfTimerCall() { m_timer.EndCall(); }
private:
	CPerfTimer& m_timer;
};
