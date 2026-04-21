#pragma once
#include "WeeklySchedule.h"
#include "MultiSchedule.h"
#include "PSVRP_Instance.h"
#include "stdafx.h"

class GeneralSchedule
{
	int numWeeks;
	int numBases;
	vector < MultiSchedule > schedVector;
	MultiSchedule *firstWeekMultiSchedule, *secondWeekMultiSchedule;

public:
	GeneralSchedule(void);
	GeneralSchedule(int numberWeeks, int numberBases);
	vector <MultiSchedule>* potentialScheds;

	//Make an n-week schedule
	void makeSchedule();

	//Utility functions
	void printGeneralSchedule();
	void writeGeneralScheduleToFile();
	double computeGeneralSchedCost();

};
