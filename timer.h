#ifndef __JOG_TIMER_H
#define __JOG_TIMER_H

typedef unsigned long long u64;
typedef long long i64;

class CycleCount
{
public:
	CycleCount() {}
	void Sample();
	u64 m_uint64;
};

class FastTimer
{
public:

	void Start();
	void End();

	const CycleCount& GetDuration() const;
	double            GetDurationInMillisecnds(); 
	static double     GetDurationInMillisecnds(u64 cycles);

	static void       Initialize();
	static u64        GetMachineFrequency();

	CycleCount        cycleCounter;
	static u64        initializeTime;
	static u64        machineFrequency;
	static bool       initialized;
};



#endif