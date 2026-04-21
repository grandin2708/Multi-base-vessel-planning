#include "StdAfx.h"
#include "LNS.h"

LNS::LNS(void)
{
}

LNS::LNS(WeeklySchedule *sched)
{
	schedule = sched;
}
/*
void LNS::goLNS(int iter)
{
	int i, j, k;
	WeeklySchedule bestSched;
	int numVisitsRemove, numRoutesRemove;
	double bestCost;

	bestSched = *schedule;
	bestCost = bestSched.computeSchedCost();

	vector <Route>::iterator routeIter;
	vector <Route>::iterator routeRelocIter;
	vector <Route>::iterator routeIntermedIter;

	vector <Visit> routeVisits;
	vector <Visit> removedVisits;
	vector <Visit>::iterator visitIter;

	int routeIdx, visitIdx;

	
	//int idx = rand() % (it->getNumVisDayComb());


	//Do LNS iterations
	for (i = 0; i < iter; i++)
	{
		//numRoutesRemove = rand() % schedule->Routes.size() + 1;
		numRoutesRemove = 2;
		for (j = 0; j < numRoutesRemove; j++)
		{
			routeIdx = rand() % schedule->Routes.size();
			numVisitsRemove = (rand() % (schedule->Routes[routeIdx].size() - 2) ) + 1;
			visitIdx = (rand() % (schedule->Routes[routeIdx].size() - 2) ) + 1;

			//Delete numVisitsRemove visits starting from visitIdx 
			for (k = 0; k < numVisitsRemove; k++)
			{
				removedVisits.push_back(schedule->Routes[routeIdx].getVisitAtPos(visitIdx));
				schedule->Routes[routeIdx].deleteInstalVisit(visitIdx);

				if (visitIdx == (schedule->Routes[routeIdx].size() - 1) )
					visitIdx = 1;
			}
			/*
			Update the solution
			- Check for empty routes
			- Update VesAvail vector;
			*/
		//}

		

		//INSERTION LOOP
		/*while there are possible insertions
			calculate/update regret values
			Keep the visit variation vector sorted for each removed visit
			Perform the insertion with the largest regret value/smallest number of insertion positions
			Auxiliary objective:
				- Schedule cost
				- Number of routes
				- Number of route days
				- Total slack
			Update the solution
			 - VesAvail vector	;
		end while*/

	//}

//}
