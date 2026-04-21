// PSVRP_Heuristic.cpp : main project file.

#include "stdafx.h"
//#include "OffshoreInst.h"
//#include "Route.h"
#include "WeeklySchedule.h"
#include "PSVRP_Instance.h"
#include "MultiSchedule.h"
#include "Timer.h"
#include "GeneralSchedule.h"

int main(array<System::String ^> ^args)
{
	cout << "Hello" << endl;

	//Read and display supply vessel data
	int i, j, k;
	vector <string> BaseNames;
	vector <string> NumNodes;
	
	BaseNames.push_back("FMO");
	NumNodes.push_back("12TWslack4");
	
	BaseNames.push_back("FFB");
	NumNodes.push_back("07TW");
	
	MultiSchedule multSched(BaseNames,NumNodes);

	//Read in the instances
	for (j = 0; j < BaseNames.size(); j++)
	{
		PSVRP_Instance instance;
		cout << "here" << endl;
		bool isRead = instance.ReadInstance(BaseNames[j], NumNodes[j]);
		
		multSched.Instances.push_back(instance);
		//instance.PrintInstance();
	}

	double schedCost, multiCost;
	Timer timer;

	const char* fileName;
	fileName = "C:/Aliaksandr Shyshou/Supply Vessels 2008-2010/Multi base/MongstadInst_MBPSVPP_Heuristic/output.txt";
	ofstream solFile(fileName, ios::app);
	
	cout << "Instance read" << endl;
	//cout << "maxInst = " << instance.getMaxInst() << endl;

	vector <static double> bestCosts;
	double bestMultiCost;
	MultiSchedule bestMultiSchedule;
	
	timer.startTimer();

	vector <WeeklySchedule> Scheds;
	vector <WeeklySchedule> bestScheds;
	vector <WeeklySchedule>::iterator schedIter;

	bestScheds.resize(multSched.Instances.size());
	bestMultiCost = numeric_limits <double>::max();

	bestCosts.resize(multSched.Instances.size());
	for (j = 0; j < multSched.Instances.size(); j++)
		bestCosts[j] = numeric_limits<double>::max();

	multSched.Schedules.clear();
	multSched.Schedules.resize(multSched.Instances.size());
	multSched.trialSchedules.clear();
	multSched.trialSchedules.resize(multSched.Instances.size());

	Scheds.clear();
	Scheds.resize(multSched.Instances.size());

	vector <int> numIter;
	numIter.resize(multSched.Instances.size());

	//set number of iterations
	for (i = 0; i < numIter.size(); i++)
	{
		if (multSched.Instances[i].getPlatforms().size() < 6)
			numIter[i] = 5;
		else if ( (multSched.Instances[i].getPlatforms().size() >= 6)
			&& (multSched.Instances[i].getPlatforms().size() < 10) )
			numIter[i] = 10;
		else if ( (multSched.Instances[i].getPlatforms().size() >= 10)
			&& (multSched.Instances[i].getPlatforms().size() < 13) )
			numIter[i] = 75;
		else
			numIter[i] = 200;
	}

	double gapMargin = 0.15;

	vector < vector <double> > cost;
	vector < vector <double> > startTime;
	vector < vector <double> > vesCap;
	vector < vector <int> > numVesselsUsed;

	cost.resize(multSched.Instances.size());
	startTime.resize(multSched.Instances.size());
	vesCap.resize(multSched.Instances.size());
	numVesselsUsed.resize(multSched.Instances.size());

	//Restarts
	for ( i = 0; i < 20; i++ )
	{	
		for (j = 0; j < multSched.Instances.size(); j++)
		{
			WeeklySchedule Sched (&multSched.Instances[j], 3);
			Scheds[j] = Sched;	
		}

		for (j = 0; j < multSched.Instances.size(); j++)
		{
			Scheds[j].initSchedule();
			schedCost = Scheds[j].computeSchedCost();
			
			//cout << "Cost before = " << schedCost << endl;

			if (schedCost < bestCosts[j])
			{
				bestCosts[j] = schedCost;
				
				//Schedules[j].printWeeklySchedule();
				//solFile << "At iteration " << i << endl;
				Scheds[j].writeScheduleToFile(i, 0);
				bestScheds[j] = Scheds[j];
				cout << "At iteration " << i << endl;
			}

			if (i%1 == 0)
				cout << "ITERATION " << i << endl;


			Scheds[j].goLNS(numIter[j], &bestCosts[j], i, &bestScheds[j], 
				& multSched.trialSchedules[j], gapMargin, & cost[j], & startTime[j], 
				&vesCap[j], &numVesselsUsed[j]);
			
			//cout << "Cost after = " << bestScheds[j].computeSchedCost() << endl;
			//Scheds[j].printWeeklySchedule();
			
		}//end for j		
		
	}//end for i

	//cout << "Printing schedules from MAIN" << endl;

	for (j = 0; j < multSched.Instances.size(); j++)
	{
		bestScheds[j].updateRelationalInfo();
		multSched.Schedules[j] = bestScheds[j];
		multSched.Schedules[j].updateRelationalInfo();

		cout << "Printing schedules for " << BaseNames[j] << endl;
		for (k = 0; k < multSched.trialSchedules[j].size(); k++)
		{
			multSched.trialSchedules[j][k].updateRelationalInfo();
			multSched.trialSchedules[j][k].setIsolRoute();
			multSched.trialSchedules[j][k].reassignIsolatedVoyage();

			//cout << "SchedCOST = " << multSched.trialSchedules[j][k].computeSchedCost() << endl;
			//cout << "IsolatedRoute is" << endl;
			//multSched.trialSchedules[j][k].getIsolRoute()->printRoute();
			//multSched.trialSchedules[j][k].printWeeklySchedule();
		}
	}

	//Remove a priori bad trial schedules
	bool moreToRemove;

	for (j = 0; j < multSched.Instances.size(); j++)
	{
		moreToRemove = true;

		while (moreToRemove)
		{		
			moreToRemove = false;

			for (k = 0; k < multSched.trialSchedules[j].size(); k++)
			{
				if (bestScheds[j].calcNumVesselsUsed() < multSched.trialSchedules[j][k].calcNumVesselsUsed())
				{
					multSched.trialSchedules[j].erase(multSched.trialSchedules[j].begin() + k);
					moreToRemove = true;
					break;
				}			
			}
		}//end while
	}

	/*
	cout << "Best sched cost = " << bestScheds[0].computeSchedCost() << endl;
	cout << "printing best schedule" << endl;
	bestScheds[0].printWeeklySchedule();
	*/
	
	/*
	cout << "PRINTING MULTI SCHEDULE" << endl;
	multSched.printMultiSchedule();

	*/

	multSched.initMultiSchedule();
	multiCost = multSched.computeMultiSchedCost();
	cout << "MultiSchedCost = " << multiCost << endl;
	
	if (multiCost < bestMultiCost)
	{
		multSched.writeMultiScheduleToFile(i);
		bestMultiCost = multiCost;
		bestMultiSchedule = multSched;
		cout << "Best multiCost = " << bestMultiCost << endl;
	}

	//multSched.updateInterBaseInfo();
	//multSched.printMultiRelationalInfo();

	vector <MultiSchedule> multiSchedules;

	if (!multSched.shareVessels(&multSched.Schedules[0],&multSched.Schedules[1], 
		&bestMultiCost, &multiSchedules) )
		;

	multSched.updateVisitInstalConnection();
	/*
	for (j = 0; j < multSched.Schedules.size(); j++)
		multSched.Schedules[j]->updateRelationalInfo();
	*/
	//multSched.printMultiSchedule();
	//multSched.printMultiRelationalInfo();

	/*
	for (schedIter = multSched.Schedules.begin(); schedIter != multSched.Schedules.end(); ++schedIter)
	{
		for (i = 0; i < schedIter->getSchedInstalsPointers().size(); i++)
		{
			cout << "Distances from " << schedIter->getSchedInstalPointer(i)->getInstName() << 
				" seqNumb = " << schedIter->getSchedInstalPointer(i)->getSeqNumb() << endl;
			cout << "Number = " << schedIter->getSchedInstalPointer(i)->Dist.size() << endl;
			for (j = 0; j < schedIter->getSchedInstalPointer(i)->Dist.size(); j++)
				cout << "to " << schedIter->getSchedInstalPointer(j)->getInstName() << ": "
				<<	schedIter->getSchedInstalPointer( i)->Dist[j] << endl;
		}
	}
	*/
	multiCost = multSched.computeMultiSchedCost();
	//cout << "aya" << endl;

	

	if (multiCost < bestMultiCost)
	{
		multSched.writeMultiScheduleToFile(i);
		bestMultiCost = multiCost;
		bestMultiSchedule = multSched;
		cout << "Best multiCost = " << bestMultiCost << endl;

		multiSchedules.push_back(multSched);
	}

	for (i = 0; i < multSched.trialSchedules.size() - 1; i++)
		for (j = 0; j < multSched.trialSchedules[i].size(); j++)
			for (k = 0; k < multSched.trialSchedules[i+1].size(); k++)
			{
				multSched.Schedules[0] = multSched.trialSchedules[i][j];
				multSched.Schedules[0].updateRelationalInfo();

				multSched.Schedules[1] = multSched.trialSchedules[i+1][k];
				multSched.Schedules[1].updateRelationalInfo();

				multSched.initMultiSchedule();
				//multSched.writeMultiScheduleToFile(31);

				if (multSched.shareVessels(&multSched.Schedules[0], &multSched.Schedules[1], 
					&bestMultiCost, &multiSchedules) )
					;

				
				multSched.updateVisitInstalConnection();
				multiCost = multSched.computeMultiSchedCost();
				//cout << "MultiCost = " << multiCost << endl;

				if (multiCost <= bestMultiCost)
				{
					//multSched.writeMultiScheduleToFile(10);
					bestMultiCost = multiCost;
					bestMultiSchedule = multSched;
					cout << "Best multiCost = " << bestMultiCost << endl;
				}
				
			}
				

	cout << "The bestMultiCost = " << bestMultiCost << endl;
	

	cout << "MultiSchedules.size() = " << multiSchedules.size() << endl;

	for (i = 0; i < multiSchedules.size(); i++)
	{
		multiSchedules[i].updateInterBaseInfo();
		multiSchedules[i].updateVisitInstalConnection();
		//for (j = 0; j < multiSchedules[i].Schedules.size(); j++)
		//	multiSchedules[i].Schedules[j].synchrVisitInstalConnection();

		//multiSchedules[i].writeMultiScheduleToFile(55555);
	}
	
	//instantiate a GeneralSchedule object
	GeneralSchedule genSched(2, multSched.Instances.size());
	//genSched.potentialScheds.resize(multiSchedules.size());
	genSched.potentialScheds = & multiSchedules;

	//Make biweekly schedule
	genSched.makeSchedule();


	cout << "Time = " << timer.getElapsedSeconds() << endl;
	//cout << "The best cost = " << bestSched.computeSchedCost() << endl;
	solFile << endl;
	solFile << "Time = " << timer.getElapsedSeconds() << endl;

	string _str;
	cin >> _str; 
	
	return 0;
}
