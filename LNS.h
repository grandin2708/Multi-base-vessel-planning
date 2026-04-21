#pragma once

#include "WeeklySchedule.h"

class LNS
{
	WeeklySchedule *schedule;

public:
	LNS(void);
	LNS(WeeklySchedule *sched);
	void goLNS (int iter);//perform a given number of LNS iterations


};
