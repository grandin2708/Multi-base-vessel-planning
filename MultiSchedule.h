#pragma once
#include "WeeklySchedule.h"
#include "PSVRP_Instance.h"
#include "stdafx.h"
#include "SupVes.h"


class MultiSchedule
{
	vector <string> SupBaseNames;
	vector <string> NumNodes;

	SupVes* sharedVessel;
	Route* anotherBaseRoute;
	Route* sameBaseRoute;

	int multiSchedID;
	
public:
	MultiSchedule(void);
	MultiSchedule(vector <string> bases, vector <string> nodes);
	
	vector <WeeklySchedule> Schedules;
	vector <PSVRP_Instance> Instances;

	//Schedules we'll be trying to combine pairwise
	vector <vector <WeeklySchedule> > trialSchedules;
	
	int getMultiSchedID(){return multiSchedID;}
	void setMultiSchedID(int num) {multiSchedID = num;}

	double computeMultiSchedCost();
	void printMultiSchedule();
	void writeMultiScheduleToFile(int iter);

	void setSharedVessel(SupVes* ves) {sharedVessel = ves;}
	SupVes* getSharedVessel() {return sharedVessel;}

	void setAnotherBaseRoute (Route* routePtr) {anotherBaseRoute = routePtr;}
	Route* getAnotherBaseRoute () {return anotherBaseRoute;}

	double getEarthRad() {return 3440.07019148119;}
	bool potentialSynergy(WeeklySchedule *schedIter, WeeklySchedule *otherSchedIter);

	//Methods to share vessels between the bases
	bool shareVessels(WeeklySchedule *schedIter, WeeklySchedule *otherSchedIter, 
		double *bestCost, vector <MultiSchedule>* multiScheds);

	//Methods to change the assignment of installations to bases
	void initInterBaseDistances(WeeklySchedule *schedIter, WeeklySchedule *otherSchedIter);
	void updateInterBaseInfo();
	void printMultiRelationalInfo();
	void updateVisitInstalConnection();
	void initMultiSchedule();

	void synchrScheduleVector();

	
};
