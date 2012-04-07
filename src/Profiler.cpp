///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#include "Prefix.h"
#include "Profiler.h"

Profiler::Profiler(const char* name)
: m_name(name),
  m_totalTime(0),
  m_sequenceCount(0),
  m_shortestTime(std::numeric_limits<double>::infinity()),
  m_longestTime(-std::numeric_limits<double>::infinity())
{
}

Profiler::~Profiler()
{
    char buf[512];
    sprintf_s(buf, "Event: %s\nMean time: %.08f\t\t\t(%.04f)\nShortest time: %.08f\t\t\t(%.04f)\nLongest time: %.08f\t\t\t(%.04f)\n",
                   m_name.c_str(),
                   m_totalTime / static_cast<double>(m_sequenceCount), (m_totalTime / static_cast<double>(m_sequenceCount)) / (1 / 60.0),
                   m_shortestTime, m_shortestTime / (1 / 60.0),
                   m_longestTime, m_longestTime / (1 / 60.0));
    OutputDebugStringA("===================== PROFILER RESULTS =====================\n");
    OutputDebugStringA(buf);
}

void Profiler::Begin()
{
    QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&m_startTime));
}

void Profiler::End()
{
    uint64_t endTime;
    QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&endTime));

    uint64_t pcFreq;
    QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&pcFreq));

    double elapsedTime = static_cast<double>(endTime - m_startTime) /
                         static_cast<double>(pcFreq);

    m_sequenceCount++;
    m_totalTime += elapsedTime;
    m_shortestTime = min(m_shortestTime, elapsedTime);
    m_longestTime = max(m_longestTime, elapsedTime);
}


