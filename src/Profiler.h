///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#ifndef __NYX_PROFILER_H__
#define __NYX_PROFILER_H__

class Profiler
{
public:
    //
    //  Constructor.
    //
    //  Parameters:
    //      [in] name
    //          Name of the event to be profiled.
    //
    Profiler(const char* name);

    //
    //  Destructor.
    //
    ~Profiler();

    //
    //  Begins a sequence.
    //
    void Begin();

    //
    //  Ends a sequence.
    //
    void End();

private:
    //
    //  Properties.
    //
    std::string m_name;
    uint64_t m_startTime;
    size_t m_sequenceCount;
    double m_totalTime;
    double m_shortestTime;
    double m_longestTime;
};

#endif  // __NYX_PROFILER_H__
