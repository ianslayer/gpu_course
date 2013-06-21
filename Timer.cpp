#ifdef _WIN32
#include <windows.h>
#include <intrin.h>
#endif

#include "Timer.h"

void CycleCount::Sample()
{
	
	#if defined(_WIN32)		
	m_uint64 = (u64) __rdtsc();

	#elif defined _MACOS_ && defined (__x86_64__)

	unsigned long* pSample = (unsigned long *)&m_uint64;

	__asm
	{
		mov		rcx, pSample			
		rdtsc
		mov		[rcx], eax
		mov		[rcx + 4], edx
	}

	#elif defined _MACOS_

	unsigned long* pSample = (unsigned long *)&m_uint64;
	__asm
	{
		// force the cpu to synchronize the instruction queue
		// NJS: CPUID can really impact performance in tight loops.
		//cpuid
		//cpuid
		//cpuid
		mov		ecx, pSample
			rdtsc
			mov		[ecx], eax
			mov		[ecx+4], edx
	}
	#endif			
}

bool FastTimer::initialized = false;
u64 FastTimer::initializeTime = 0;
u64 FastTimer::machineFrequency = 0;

void FastTimer::Start()
{
    cycleCounter.Sample();
}

void FastTimer::End()
{
    CycleCount cc;
    cc.Sample();
    cycleCounter.m_uint64 = cc.m_uint64 - cycleCounter.m_uint64;
}

const CycleCount& FastTimer::GetDuration() const
{
    return cycleCounter;
}

double FastTimer::GetDurationInMillisecnds()
{
	return GetDurationInMillisecnds(cycleCounter.m_uint64);
}

double FastTimer::GetDurationInMillisecnds(u64 cycles)
{
	i64 time_s     = (i64)(cycles / machineFrequency);  // unsigned->sign conversion should be safe here
	i64 time_fract = (i64)(cycles % machineFrequency);  // unsigned->sign conversion should be safe here
	double ret = (time_s) + (double)time_fract/(double)((i64)machineFrequency);
	return ret * 1000;
}

void FastTimer::Initialize()
{
	if(!initialized)
	{
		FastTimer timer;
		timer.cycleCounter.Sample();
		initializeTime = timer.cycleCounter.m_uint64;
		machineFrequency = GetMachineFrequency();
		initialized = true;
	}
}

#ifdef _WIN32
u64 FastTimer::GetMachineFrequency()
{
	HANDLE currentThreadHandle = ::GetCurrentThread();
	DWORD_PTR affinity_mask = ::SetThreadAffinityMask(currentThreadHandle, 1);

	FastTimer timer;
	timer.Start();
	Sleep(1000);
	timer.End();

	return timer.GetDuration().m_uint64;
}
#endif

