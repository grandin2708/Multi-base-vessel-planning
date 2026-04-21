#include "StdAfx.h"
#include "MultiSchedule.h"

MultiSchedule::MultiSchedule(void)
{
}

MultiSchedule::MultiSchedule(vector <string> bases, vector<string> nodes)
{
	SupBaseNames.clear();
	SupBaseNames.resize(bases.size());
	SupBaseNames = bases;

	NumNodes.clear();
	NumNodes.resize(nodes.size());
	NumNodes = nodes;
}

bool MultiSchedule::potentialSynergy(WeeklySchedule *schedIter, WeeklySchedule *otherSchedIter)
{

	int i, routeStartIdx, routeEndIdx, planHorizon;

	routeStartIdx = (int) ceil( schedIter->getIsolRoute()->getRouteStartTime()/24);
					
	routeEndIdx = (int) ceil( ( schedIter->getIsolRoute()->getRouteEndTime() - 
				schedIter->getIsolRoute()->getInstAtPos(0)->getLayTime() +
				schedIter->getIsolRoute()->getRouteMinSlack() )/24 );

	planHorizon = otherSchedIter->NUMBER_OF_DAYS + (int) ceil(
					(double)( otherSchedIter->getRoutePointer(0)->MAX_ROUTE_DUR 
					+ otherSchedIter->getRoutePointer(0)->getInstAtPos(0)->getLayTime() 
					+ otherSchedIter->getRoutePointer(0)->getRouteMinSlack() )/24);


	bool isAssignmentFeasible = true, endOfWeekOverlap = false;

	for (i = routeStartIdx; i <= routeEndIdx; i++)
	{
		if(!otherSchedIter->VesAvail[otherSchedIter->getIsolRoute()->getRouteVes()->getID()][i])
		{
			isAssignmentFeasible = false;
			break;
		}

		if ( (i <= planHorizon - (otherSchedIter->NUMBER_OF_DAYS+1))
			&& (!otherSchedIter->VesAvail[otherSchedIter->getIsolRoute()->getRouteVes()->getID()][i + otherSchedIter->NUMBER_OF_DAYS]) )
			{
				isAssignmentFeasible = false;
				break;
			}
	}

	//Additional check to see if the route overlaps 
	//with the first route next week
	if (routeEndIdx >= (otherSchedIter->NUMBER_OF_DAYS+1) )
		for (i = 0; i <= routeEndIdx % (otherSchedIter->NUMBER_OF_DAYS+1); i++)
			if(!otherSchedIter->VesAvail[otherSchedIter->getIsolRoute()->getRouteVes()->getID()][i+1])
			{
				endOfWeekOverlap = true;
				break;
			}

	
	return (isAssignmentFeasible && !endOfWeekOverlap);
}

void MultiSchedule::initInterBaseDistances(WeeklySchedule *schedIter, WeeklySchedule *otherSchedIter)
{

	int i;
	double distance;
	OffshoreInst *otherBase, *base, *platform;
	PSVRP_Instance *inst, *otherInst;
	Route isolRoute, otherIsolRoute;

	inst = schedIter->getInstance();
	isolRoute = *(schedIter->getIsolRoute());

	otherInst = otherSchedIter->getInstance();
	otherIsolRoute = *(otherSchedIter->getIsolRoute());


	//Add another Base to each WeeklySchedule object
	otherSchedIter->addInstallation( *(schedIter->getSchedInstalPointer(0)));
	otherBase = otherSchedIter->getSchedInstalPointer(0);

	base = schedIter->getSchedInstalPointer(0);
	otherSchedIter->getSchedInstalPointer( otherSchedIter->getSchedInstalsPointers().size() - 1)->setSeqNumb(
		 otherSchedIter->getSchedInstalsPointers().size() - 1);

	otherSchedIter->getSchedInstalPointer( otherSchedIter->getSchedInstalsPointers().size() - 1)->Dist.clear();

	/*
	cout << "other OP: Latitude = " << otherInst->getOPlat() << " Longitude = " << otherInst->getOPlon() << endl;

	cout << "Other Base Name: "<< otherSchedIter->getSchedInstalPointer( otherSchedIter->getSchedInstalsPointers().size() - 1)->getInstName() << endl;
	cout << "Latitude: " <<otherSchedIter->getSchedInstalPointer( otherSchedIter->getSchedInstalsPointers().size() - 1)->getLatitude() 
		<< " Longitude: " << otherSchedIter->getSchedInstalPointer( otherSchedIter->getSchedInstalsPointers().size() - 1)->getLongitude() << endl;

	cout << "OP: Latitude = " << inst->getOPlat() << " Longitude = " << inst->getOPlon() << endl;
*/
	//Initialize Dist vector
	for (i = 0; i < otherSchedIter->getSchedInstalsPointers().size(); i++)
	{
		if ((i > 0) && (i < otherSchedIter->getSchedInstalsPointers().size() - 1) )
		{
			platform = otherSchedIter->getSchedInstalPointer(i);
			//Distance from platform to OP
			distance = getEarthRad()*acos( sin(platform->getLatitude()*M_PI/180)*sin(inst->getOPlat()*M_PI/180)
				+ cos(platform->getLatitude()*M_PI/180)*cos(inst->getOPlat()*M_PI/180)
				* cos( (platform->getLongitude() - inst->getOPlon())*M_PI/180 ) );

			//Distance from OP the other base
			distance += getEarthRad()*acos (sin(inst->getOPlat()*M_PI/180) * sin(base->getLatitude()*M_PI/180)
				+ cos(inst->getOPlat()*M_PI/180) * cos(base->getLatitude()*M_PI/180)
				* cos ( (inst->getOPlon() - base->getLongitude())*M_PI/180) ); 

			otherSchedIter->getSchedInstalPointer( otherSchedIter->getSchedInstalsPointers().size() - 1)->Dist.push_back(distance);
			otherSchedIter->getSchedInstalPointer(i)->Dist.push_back(distance);
		}
		else if (i == 0)
		{//New base
			//otherSchedIter->getSchedInstalPointer(i)->setSeqNumb(i);

			platform = otherSchedIter->getSchedInstalPointer(i);

			//Distance from the Base to OP
			distance = getEarthRad()*acos( sin(otherBase->getLatitude()*M_PI/180)*sin(otherInst->getOPlat()*M_PI/180)
			+ cos(otherBase->getLatitude()*M_PI/180)*cos(otherInst->getOPlat()*M_PI/180)
			* cos( (otherBase->getLongitude() - otherInst->getOPlon())*M_PI/180 ) );

			//Distance from OP to the other OP
			distance += getEarthRad()*acos (sin(otherInst->getOPlat()*M_PI/180) * sin(inst->getOPlat()*M_PI/180)
			+ cos(otherInst->getOPlat()*M_PI/180) * cos(inst->getOPlat()*M_PI/180)
			* cos ( (otherInst->getOPlon() - inst->getOPlon())*M_PI/180) ); 

			//Distance from the other OP to the other Base
			distance += getEarthRad()*acos (sin(inst->getOPlat()*M_PI/180) * sin(base->getLatitude()*M_PI/180)
			+ cos(inst->getOPlat()*M_PI/180) * cos(base->getLatitude()*M_PI/180)
			* cos ( (inst->getOPlon() - base->getLongitude())*M_PI/180) ); 

			platform->Dist.push_back(distance);
			otherSchedIter->getSchedInstalPointer( otherSchedIter->getSchedInstalsPointers().size() - 1)->Dist.push_back(distance);
		}	
		else if (i == otherSchedIter->getSchedInstalsPointers().size() - 1)
		{
			distance = 0;
			otherSchedIter->getSchedInstalPointer( otherSchedIter->getSchedInstalsPointers().size() - 1)->Dist.push_back(distance);
		}

	}//end for i



	//The same thing for the other base and the base - roles reversed
	schedIter->addInstallation(* (otherSchedIter->getSchedInstalPointer(0)));

	base = schedIter->getSchedInstalPointer(0);
	inst = schedIter->getInstance();

	schedIter->getSchedInstalPointer(schedIter->getSchedInstalsPointers().size() - 1)->setSeqNumb(
		schedIter->getSchedInstalsPointers().size() - 1);
	schedIter->getSchedInstalPointer(schedIter->getSchedInstalsPointers().size() - 1)->Dist.clear();
	schedIter->getSchedInstalPointer(schedIter->getSchedInstalsPointers().size() - 1)->Dist.resize(
		schedIter->getSchedInstalsPointers().size());

	//Initialize Dist vector
	for (i = 0; i < schedIter->getSchedInstalsPointers().size(); i++)
	{
		if ((i > 0) && (i < schedIter->getSchedInstalsPointers().size() - 1))
		{
			platform = schedIter->getSchedInstalPointer(i);
			//Distance from platform to the other OP
			distance = getEarthRad()*acos( sin(platform->getLatitude()*M_PI/180)*sin(otherInst->getOPlat()*M_PI/180)
				+ cos(platform->getLatitude()*M_PI/180)*cos(otherInst->getOPlat()*M_PI/180)
				* cos( (platform->getLongitude() - otherInst->getOPlon())*M_PI/180 ) );

			//Distance from the other OP the other base
			distance += getEarthRad()*acos (sin(otherInst->getOPlat()*M_PI/180) * sin(otherBase->getLatitude()*M_PI/180)
				+ cos(otherInst->getOPlat()*M_PI/180) * cos(otherBase->getLatitude()*M_PI/180)
				* cos ( (otherInst->getOPlon() - otherBase->getLongitude())*M_PI/180) ); 

			platform->Dist.push_back(distance);
			schedIter->getSchedInstalPointer( schedIter->getSchedInstalsPointers().size() - 1)->Dist[i] = distance;
		}
		else if ( i == 0)
		{//New base

			platform = schedIter->getSchedInstalPointer(i);

			
			//Distance from the Base to OP
			distance = getEarthRad()*acos( sin(otherBase->getLatitude()*M_PI/180)*sin(otherInst->getOPlat()*M_PI/180)
			+ cos(otherBase->getLatitude()*M_PI/180)*cos(otherInst->getOPlat()*M_PI/180)
			* cos( (otherBase->getLongitude() - otherInst->getOPlon())*M_PI/180 ) );
			
			//Distance from OP to the other OP
			distance += getEarthRad()*acos (sin(otherInst->getOPlat()*M_PI/180) * sin(inst->getOPlat()*M_PI/180)
			+ cos(otherInst->getOPlat()*M_PI/180) * cos(inst->getOPlat()*M_PI/180)
			* cos ( (otherInst->getOPlon() - inst->getOPlon())*M_PI/180) ); 

			//Distance from the other OP to the other Base
			distance += getEarthRad()*acos (sin(inst->getOPlat()*M_PI/180) * sin(base->getLatitude()*M_PI/180)
			+ cos(inst->getOPlat()*M_PI/180) * cos(base->getLatitude()*M_PI/180)
			* cos ( (inst->getOPlon() - base->getLongitude())*M_PI/180) ); 

			platform->Dist.push_back(distance);
			schedIter->getSchedInstalPointer( schedIter->getSchedInstalsPointers().size() - 1)->Dist[i] = distance;
		}	
		else if (i == schedIter->getSchedInstalsPointers().size() - 1)
		{
			distance = 0;
			schedIter->getSchedInstalPointer( schedIter->getSchedInstalsPointers().size() - 1)->Dist[i] = distance;
		}
	}//end for i

	//Control print
	/*
	int j;

	for (i = 0; i < otherSchedIter->getSchedInstalsPointers().size(); i++)
		cout << "Distances from " << otherSchedIter->getSchedInstalPointer(i)->getInstName() << endl;

	for (i = 0; i < otherSchedIter->getSchedInstalsPointers().size(); i++)
	{
		cout << "Distances from " << otherSchedIter->getSchedInstalPointer(i)->getInstName() << endl;
		for (j = 0; j < otherSchedIter->getSchedInstalPointer(i)->Dist.size(); j++)
		{
			cout << "to " << otherSchedIter->getSchedInstalPointer(j)->getInstName() << ": "
			<< otherSchedIter->getSchedInstalPointer(i)->Dist[j] << endl;
		}
	}

	for (i = 0; i < schedIter->getSchedInstalsPointers().size(); i++)
		cout << "Distances from " << schedIter->getSchedInstalPointer(i)->getInstName() << endl;

	for (i = 0; i < schedIter->getSchedInstalsPointers().size(); i++)
	{
		cout << "Distances from " << schedIter->getSchedInstalPointer(i)->getInstName() << endl;
		cout << "Number = " << schedIter->getSchedInstalPointer(i)->Dist.size() << endl;
		for (j = 0; j < schedIter->getSchedInstalPointer(i)->Dist.size(); j++)
			cout << "to " << schedIter->getSchedInstalPointer(j)->getInstName() << ": "
			<<	schedIter->getSchedInstalPointer( i)->Dist[j] << endl;
	}
	*/

}
void MultiSchedule::printMultiRelationalInfo()
{
	vector <WeeklySchedule>::iterator schedIter;

	vector <Route*> vesRoutes;
	vector <SupVes*>::iterator vesIter;
	vector <SupVes*> vesPointers;

	int i;

	for (schedIter = Schedules.begin(); schedIter != Schedules.end(); ++schedIter)
	{
		vesPointers.clear();
		vesPointers = schedIter->getSchedVesselsPointers();

		for (vesIter = vesPointers.begin();vesIter != vesPointers.end(); ++vesIter)
		{
			
			vesRoutes.clear();
			vesRoutes = (*vesIter)->getVesselRoutes();

			cout << "Vessel: " << (*vesIter)->getName() << " routeSize =" << vesRoutes.size() << endl;
			for (i = 0; i < vesRoutes.size(); i++)
				vesRoutes[i]->printRoute();
		}
	}
}
bool MultiSchedule::shareVessels(WeeklySchedule *schedIter, 
	WeeklySchedule *otherSchedIter, double *bestCost, 
	vector <MultiSchedule>* multiScheds)
{
	/*
	Try to do reassignment of vessels to voyages
	so that in at least two WeeklySchedules there is 
	a vessel available for 3-5 days
	*/

	vector <vector <bool> > vesAvailability;
	vector <vector <bool> > otherVesAvailability;
	int i = 0, j, k;

	vesAvailability.resize(schedIter->VesAvail.size());
	otherVesAvailability.resize(otherSchedIter->VesAvail.size());

	for (j = 0; j < schedIter->VesAvail.size(); j++)
	{
		vesAvailability[j].resize(schedIter->VesAvail[j].size());
		for (k = 0; k < schedIter->VesAvail[j].size(); k++)
			vesAvailability[j][k] = schedIter->VesAvail[j][k];
	}

	for (j = 0; j < otherSchedIter->VesAvail.size(); j++)
	{
		otherVesAvailability[j].resize(otherSchedIter->VesAvail[j].size());
		for (k = 0; k < otherSchedIter->VesAvail[j].size(); k++)
			otherVesAvailability[j][k] = otherSchedIter->VesAvail[j][k];
	}

	cout << "IN SHARE VESSELS!!!!!!!!" << endl;

	/*
	cout << "schedIter->VesAvail " << endl;
	for (j = 0; j < schedIter->VesAvail.size(); j++)
	{
		for (k = 0; k < schedIter->VesAvail[j].size(); k++)
			cout << schedIter->VesAvail[j][k] << " ";

		cout << endl;
	}

	cout << "otherSchedIter->VesAvail " << endl;
	for (j = 0; j < otherSchedIter->VesAvail.size(); j++)
	{
		
		for (k = 0; k < otherSchedIter->VesAvail[j].size(); k++)
			cout << otherSchedIter->VesAvail[j][k]<< " ";

		cout << endl;
	}
	*/
	int planHorizon;
	
	int routeStartIdx, routeEndIdx;
	int otherRouteStartIdx, otherRouteEndIdx;
	double minVesCap;
	bool minCapIsOk, routesOverlap;

	PSVRP_Instance *inst, *otherInst;
	Route isolRoute, otherIsolRoute;

	int isolateBegin, isolateEnd;
	bool isolationSuccess;

	vector <Route*> vesRoutes;
	vector <SupVes*>::iterator vesIter;
	vector <SupVes*> vesPointers;

	//updateInterBaseInfo();
	//printMultiRelationalInfo();

	vector <double> isolStartTimes;
	vector <double> otherIsolStartTimes;

	WeeklySchedule bestSched, bestOtherSched, localSched, localOtherSched;

	WeeklySchedule best, otherBest;

	bestSched = *schedIter;
	bestOtherSched = *otherSchedIter;

	double multiCost;

	bool moreIsolations = true;
	bool oneIterationPerformed = false;
	bool wasDecremented;
	bool wasIsolated, otherWasIsolated;
	
	if( ( schedIter->getIsolRoute() != NULL) && (otherSchedIter->getIsolRoute() != NULL)
		 && (schedIter->getSchedID() != otherSchedIter->getSchedID()) )
	{
		localSched = *schedIter;
		localSched.synchrVisitInstalConnection();

		localOtherSched = *otherSchedIter;
		localOtherSched.synchrVisitInstalConnection();

		while (moreIsolations)
		{
			
			routeStartIdx = (int) ceil( schedIter->getIsolRoute()->getRouteStartTime()/24);
			//cout << "isolRouteStartIdx = " << routeStartIdx << endl;

			routeEndIdx = (int) ceil( ( schedIter->getIsolRoute()->getRouteEndTime() - 
					schedIter->getIsolRoute()->getInstAtPos(0)->getLayTime() +
					schedIter->getIsolRoute()->getRouteMinSlack() )/24 );

			inst = schedIter->getInstance();
			isolRoute = *schedIter->getIsolRoute();


			otherRouteStartIdx = (int) ceil( otherSchedIter->getIsolRoute()->getRouteStartTime()/24);

			otherRouteEndIdx = (int) ceil( ( otherSchedIter->getIsolRoute()->getRouteEndTime() -
				otherSchedIter->getIsolRoute()->getInstAtPos(0)->getLayTime() +
				otherSchedIter->getIsolRoute()->getRouteMinSlack() )/24 );

			//cout << "otherIsolRouteStartIdx = " << otherRouteStartIdx << endl;

			otherInst = otherSchedIter->getInstance();
			otherIsolRoute = *otherSchedIter->getIsolRoute();

			//Check for "sharing a vessel" potential
			isolationSuccess = false;

			if ( !potentialSynergy(&(*schedIter), &(*otherSchedIter)) ||
				oneIterationPerformed )
			{

				wasDecremented = false;

				planHorizon = schedIter->NUMBER_OF_DAYS + (int) ceil(
						(double)( schedIter->getRoutePointer(0)->MAX_ROUTE_DUR 
						+ schedIter->getRoutePointer(0)->getInstAtPos(0)->getLayTime() 
						+ schedIter->getRoutePointer(0)->getRouteMinSlack() )/24);

				//Try to isolate route
				//cout << "otherRouteEndIdx = " << otherRouteEndIdx << endl;
				isolateBegin = (otherRouteEndIdx + 1)%(schedIter->NUMBER_OF_DAYS);
				
				if (isolateBegin == 0)
					++isolateBegin;
				//cout << "isolateBegin = " << isolateBegin << endl;


				//cout << "otherRouteStartIdx = " << otherRouteStartIdx << endl;
				isolateEnd = otherRouteStartIdx - 1 + schedIter->NUMBER_OF_DAYS;
				
				if (isolateEnd > planHorizon)
					isolateEnd -= schedIter->NUMBER_OF_DAYS;

				if ( (isolateEnd == planHorizon)
					&& ((otherRouteEndIdx + 1) % schedIter->NUMBER_OF_DAYS > 0) )
				{
						isolateEnd--;
						wasDecremented = true;
				}
				else if ( (isolateEnd == planHorizon)
					&& ((otherRouteEndIdx + 1) % schedIter->NUMBER_OF_DAYS == 0) )
					isolateEnd -= schedIter->NUMBER_OF_DAYS;

				//cout << "IsolateEnd = " << isolateEnd << endl;


				isolationSuccess = schedIter->isolateVoyageBetween(isolateBegin, isolateEnd,
					schedIter->getIsolRoute()->getRouteVes()->getID(), isolStartTimes);

				if (!isolationSuccess && wasDecremented) 
					isolateEnd++;

				if ( !isolationSuccess && (isolateEnd > schedIter->NUMBER_OF_DAYS)
					&& (isolateEnd % schedIter->NUMBER_OF_DAYS >=
					otherRouteEndIdx - otherRouteStartIdx + 1) )
				{
					isolateBegin = 1;
					isolateEnd = isolateEnd % schedIter->NUMBER_OF_DAYS;
					//cout << "isolateBegin = " << isolateBegin << endl;
					//cout << "IsolateEnd = " << isolateEnd << endl;

					isolationSuccess = schedIter->isolateVoyageBetween(isolateBegin, isolateEnd,
					schedIter->getIsolRoute()->getRouteVes()->getID(), isolStartTimes);

				}

				if (isolationSuccess)
				{
					schedIter->setIsolRoute();
					isolRoute = *schedIter->getIsolRoute();

					//cout << "Newisolated route " << endl;
					//schedIter->getIsolRoute()->printRoute();
				}
				else //try to isolate otherRoute
				{

					wasDecremented = false;

					planHorizon = otherSchedIter->NUMBER_OF_DAYS + (int) ceil(
						(double)( otherSchedIter->getRoutePointer(0)->MAX_ROUTE_DUR 
						+ otherSchedIter->getRoutePointer(0)->getInstAtPos(0)->getLayTime() 
						+ otherSchedIter->getRoutePointer(0)->getRouteMinSlack() )/24);

					//cout << "routeEndIdx = " << routeEndIdx << endl;
					isolateBegin = (routeEndIdx + 1)%(otherSchedIter->NUMBER_OF_DAYS);
					
					if (isolateBegin == 0)
						++isolateBegin;

					//cout << "isolateBegin = " << isolateBegin << endl;

					//cout << "routeStartIdx = " << routeStartIdx << endl;
					isolateEnd = routeStartIdx - 1 + otherSchedIter->NUMBER_OF_DAYS;
					

					if (isolateEnd > planHorizon)
						isolateEnd -= otherSchedIter->NUMBER_OF_DAYS;

					if ( (isolateEnd == planHorizon) 
						&& ((routeEndIdx + 1)%otherSchedIter->NUMBER_OF_DAYS > 0) )
					{
						isolateEnd--;
						wasDecremented = true;
					}
					else if ( (isolateEnd == planHorizon) 
						&& ((routeEndIdx + 1)%otherSchedIter->NUMBER_OF_DAYS == 0) )
						isolateEnd -= otherSchedIter->NUMBER_OF_DAYS;

					//cout << "IsolateEnd = " << isolateEnd << endl;

					isolationSuccess = otherSchedIter->isolateVoyageBetween(isolateBegin, isolateEnd,
						otherSchedIter->getIsolRoute()->getRouteVes()->getID(), otherIsolStartTimes);

					if (!isolationSuccess && wasDecremented)
						isolateEnd++;

					if ( !isolationSuccess && (isolateEnd > otherSchedIter->NUMBER_OF_DAYS)
						&& (isolateEnd % otherSchedIter->NUMBER_OF_DAYS >=
						routeEndIdx - routeStartIdx + 1) )
					{
						isolateBegin = 1;
						
						isolateEnd = isolateEnd % otherSchedIter->NUMBER_OF_DAYS;

						isolationSuccess = otherSchedIter->isolateVoyageBetween(isolateBegin, isolateEnd,
						otherSchedIter->getIsolRoute()->getRouteVes()->getID(), otherIsolStartTimes);
					}

					if (isolationSuccess)
					{
						otherSchedIter->setIsolRoute();
						otherIsolRoute = *otherSchedIter->getIsolRoute();

						//cout << "New other isolated route " << endl;
						//otherSchedIter->getIsolRoute()->printRoute();
					}
					else
						moreIsolations = false;
				}//end else
			}// end if ( !potentialSynergy)

			wasIsolated = false;
			otherWasIsolated = false;

			for ( i = 0; i < isolStartTimes.size(); i++)
			{
				if (isolStartTimes[i] == schedIter->getIsolRoute()->getRouteStartTime())
					wasIsolated = true;

				if (otherIsolStartTimes[i] == otherSchedIter->getIsolRoute()->getRouteStartTime())
					otherWasIsolated = true;
			}


			if ( potentialSynergy(&(*schedIter), &(*otherSchedIter))
				&& (!wasIsolated || !otherWasIsolated) )
			{
				cout << "There's synergy" << endl;

				//Initialize interbase distances
				initInterBaseDistances(&(*schedIter), &(*otherSchedIter));		

				isolStartTimes.push_back(schedIter->getIsolRoute()->getRouteStartTime());
				otherIsolStartTimes.push_back(otherSchedIter->getIsolRoute()->getRouteStartTime());

				//Delete the base at the end and insert the other base
				otherSchedIter->getIsolRoute()->deleteInstalVisit(otherIsolRoute.getVisitObjects().size() - 1);
				otherSchedIter->getIsolRoute()->addRouteInstal( 
					otherSchedIter->getSchedInstalPointer( otherSchedIter->getSchedInstalsPointers().size() - 1 ) );
				otherSchedIter->getIsolRoute()->addVisit(Visit(
					otherSchedIter->getSchedInstalPointer( otherSchedIter->getSchedInstalsPointers().size() - 1 )) );

				otherSchedIter->getIsolRoute()->updateVisitVector();

				//cout << "Before intelligent reorder: otherRoute" << endl;
				//otherSchedIter->getIsolRoute()->printRoute();

				otherSchedIter->getIsolRoute()->intelligentReorder();

				//cout << "After intelligent reorder: otherRoute" << endl;
				//otherSchedIter->getIsolRoute()->printRoute();

				updateInterBaseInfo();

				//cout << "___________________________________" << endl;
				//printMultiRelationalInfo();
				//cout << "____________________________________" << endl;

				
				//same thing for schedIter
				schedIter->getIsolRoute()->deleteInstalVisit(isolRoute.getVisitObjects().size() - 1);
				schedIter->getIsolRoute()->addRouteInstal(
					schedIter->getSchedInstalPointer( schedIter->getSchedInstalsPointers().size() - 1) );
				schedIter->getIsolRoute()->addVisit(Visit(
					schedIter->getSchedInstalPointer( schedIter->getSchedInstalsPointers().size() - 1)) );

				schedIter->getIsolRoute()->updateVisitVector();

				//cout << "Before intelligent reorder: route" << endl;
				//schedIter->getIsolRoute()->printRoute();

				schedIter->getIsolRoute()->intelligentReorder();

				//cout << "After intelligent reorder: route" << endl;
				//schedIter->getIsolRoute()->printRoute();

				
				//Check if we are ok
				routesOverlap = false;

				if (routeStartIdx < otherRouteStartIdx) 
				{
					if ( !( schedIter->getIsolRoute()->getRouteEndTime() +
						schedIter->getIsolRoute()->getRouteMinSlack() +
						schedIter->getIsolRoute()->getInstAtPos( 
						schedIter->getIsolRoute()->getRouteInstalPointers().size() - 1)->getLayTime()
						< otherSchedIter->getIsolRoute()->getRouteStartTime() +
						otherSchedIter->getIsolRoute()->getRouteAcceptanceTime() )
						&& !( otherSchedIter->getIsolRoute()->getRouteEndTime() +
						otherSchedIter->getIsolRoute()->getRouteMinSlack() +
						otherSchedIter->getIsolRoute()->getInstAtPos( 
						otherSchedIter->getIsolRoute()->getRouteInstalPointers().size() - 1)->getLayTime()
						< 168 + schedIter->getIsolRoute()->getRouteStartTime() +
						schedIter->getIsolRoute()->getRouteAcceptanceTime() ) )

						routesOverlap = true;
						
				}
				else // routeStartIdx > otherRouteStartIdx
				{
					//We are HERE NOW
					if ( !( otherSchedIter->getIsolRoute()->getRouteEndTime() +
						otherSchedIter->getIsolRoute()->getRouteMinSlack() +
						otherSchedIter->getIsolRoute()->getInstAtPos( 
						otherSchedIter->getIsolRoute()->getRouteInstalPointers().size() - 1)->getLayTime() 
						< schedIter->getIsolRoute()->getRouteStartTime() +
						schedIter->getIsolRoute()->getRouteAcceptanceTime() )
						&& !( schedIter->getIsolRoute()->getRouteEndTime() +
						schedIter->getIsolRoute()->getRouteMinSlack() +
						schedIter->getIsolRoute()->getInstAtPos( 
						schedIter->getIsolRoute()->getRouteInstalPointers().size() - 1)->getLayTime()
						< 168 + otherSchedIter->getIsolRoute()->getRouteStartTime() +
						otherSchedIter->getIsolRoute()->getRouteAcceptanceTime() ) )

						routesOverlap = true;			
				}

				if (!routesOverlap)
				{
					routeStartIdx = (int) ceil( schedIter->getIsolRoute()->getRouteStartTime()/24);
				
					routeEndIdx = (int) ceil( ( schedIter->getIsolRoute()->getRouteEndTime() - 
							schedIter->getIsolRoute()->getInstAtPos(0)->getLayTime() +
							schedIter->getIsolRoute()->getRouteMinSlack() )/24 );

					otherRouteStartIdx = (int) ceil( otherSchedIter->getIsolRoute()->getRouteStartTime()/24);

					otherRouteEndIdx = (int) ceil( ( otherSchedIter->getIsolRoute()->getRouteEndTime() -
						otherSchedIter->getIsolRoute()->getInstAtPos(0)->getLayTime() +
						otherSchedIter->getIsolRoute()->getRouteMinSlack() )/24 );

					minCapIsOk = false;

					if (schedIter->getIsolRoute()->getRouteVes()->getCapacity() >
						otherSchedIter->getIsolRoute()->getRouteVes()->getCapacity() )
						minVesCap = otherSchedIter->getIsolRoute()->getRouteVes()->getCapacity();
					else
						minVesCap = schedIter->getIsolRoute()->getRouteVes()->getCapacity();

					if ( (schedIter->getIsolRoute()->computeRouteDemand() < minVesCap)
						&& (otherSchedIter->getIsolRoute()->computeRouteDemand() < minVesCap) )
						minCapIsOk = true;

					

					if ( (minCapIsOk && (minVesCap ==
						otherSchedIter->getIsolRoute()->getRouteVes()->getCapacity()) )
						|| (!minCapIsOk && otherSchedIter->getIsolRoute()->getRouteVes()->getCapacity() 
						> minVesCap) )
					{
						//Update VesAvail
						for (i = routeStartIdx; i <= routeEndIdx; i++)
						{
							schedIter->VesAvail[schedIter->getIsolRoute()->getRouteVes()->getID()][i] = true;
							otherSchedIter->VesAvail[otherSchedIter->getIsolRoute()->getRouteVes()->getID()][i] = false;
						}

						//Free the isolated vessel of schedIter
						schedIter->getIsolRoute()->getRouteVes()->setIsVesselUsed(false);

						//cout << "Removing vessel " << schedIter->getIsolRoute()->getRouteVes()->getName()<< endl;		
					
						/*
						cout << "Its route before" << endl;
						vesRoutes.clear();
						vesRoutes = otherSchedIter->getIsolRoute()->getRouteVes()->getVesselRoutes();
						for (i = 0; i < vesRoutes.size(); i++)
							vesRoutes[i]->printRoute();
						*/

						setAnotherBaseRoute(schedIter->getIsolRoute());

						//Reassign to otherSchedIter
						otherSchedIter->getIsolRoute()->getRouteVes()->addVesselRoute(
							schedIter->getIsolRoute() );

						
						setSharedVessel(otherSchedIter->getIsolRoute()->getRouteVes());

						//cout << "Reassigning to vessel " << otherSchedIter->getIsolRoute()->getRouteVes()->getName() << endl;
						schedIter->getIsolRoute()->setRouteVessel(
							otherSchedIter->getIsolRoute()->getRouteVes() );
						//cout << "Its routes are now: " << endl;

						/*
						vesRoutes.clear();
						vesRoutes = otherSchedIter->getIsolRoute()->getRouteVes()->getVesselRoutes();
						for (i = 0; i < vesRoutes.size(); i++)
							vesRoutes[i]->printRoute();
						*/

						
						//Clear routes vector
						schedIter->getIsolRoute()->getRouteVes()->clearVesselRoutes();

						//schedIter->updateRelationalInfo();
						
						updateInterBaseInfo();
						schedIter->reassignVesselsToRoutes();
						updateVisitInstalConnection();
						
						//cout << "Before postLNS bestCost = " << *bestCost << endl;

						//Post-optimize using LNS here
						best = *schedIter; 
						otherBest = *otherSchedIter;
						
						//cout << "Before otherSchedIter->multiLNS" << endl;
						otherSchedIter->multiLNS(otherSchedIter->calcNumLNSiter(),
							bestCost, schedIter, &otherBest, getAnotherBaseRoute());

						//cout << "After otherSchedIter->multiLNS" << endl;
						*otherSchedIter = otherBest;
						updateInterBaseInfo();
						updateVisitInstalConnection();

						//cout << "Before schedIter->multiLNS" << endl;
						schedIter->multiLNS(schedIter->calcNumLNSiter(), bestCost,
							otherSchedIter, &best, getAnotherBaseRoute());
						//cout << "After schedIter->multiLNS" << endl;

						*schedIter = best;
						updateInterBaseInfo();
						updateVisitInstalConnection();


						multiCost = computeMultiSchedCost();

						multiScheds->push_back(*this);

						cout << "multiCost = " << multiCost << endl;
						cout << "bestCost = " << *bestCost << endl;


						if (multiCost <= *bestCost)
						{
							*bestCost = multiCost;
							bestSched = *schedIter;
							cout << "bestMultiCost = " << *bestCost << endl;
							bestOtherSched = *otherSchedIter;
							writeMultiScheduleToFile(27);
						}
						//return true;
					}

					else if ( (minCapIsOk && (schedIter->getIsolRoute()->getRouteVes()->getCapacity() ==
						minVesCap) )
						|| (!minCapIsOk && (schedIter->getIsolRoute()->getRouteVes()->getCapacity() >
						minVesCap)) )				
					{
						//Update VesAvail
						for (i = otherRouteStartIdx; i <= otherRouteEndIdx; i++)
						{
							otherSchedIter->VesAvail[otherSchedIter->getIsolRoute()->getRouteVes()->getID()][i] = true;
							schedIter->VesAvail[schedIter->getIsolRoute()->getRouteVes()->getID()][i] = false;
						}

						//Free isolated vessel of otherSchedIter			
						otherSchedIter->getIsolRoute()->getRouteVes()->setIsVesselUsed(false);
						//cout << "Removing vessel " << otherSchedIter->getIsolRoute()->getRouteVes()->getName()<< endl;

						setAnotherBaseRoute(otherSchedIter->getIsolRoute());
						//Reassign to schedIter
						schedIter->getIsolRoute()->getRouteVes()->addVesselRoute(
							otherSchedIter->getIsolRoute() );

						setSharedVessel(schedIter->getIsolRoute()->getRouteVes());


						//cout << "Reassigning to vessel " << schedIter->getIsolRoute()->getRouteVes()->getName() << endl;
						otherSchedIter->getIsolRoute()->setRouteVessel(
							schedIter->getIsolRoute()->getRouteVes() );

						/*
						cout << "Its routes are now: " << endl;

						vesRoutes.clear();
						vesRoutes = schedIter->getIsolRoute()->getRouteVes()->getVesselRoutes();
						for (i = 0; i < vesRoutes.size(); i++)
							vesRoutes[i]->printRoute();
						*/

						//Clear vessel routes
						otherSchedIter->getIsolRoute()->getRouteVes()->clearVesselRoutes();

						//otherSchedIter->updateRelationalInfo();
						updateInterBaseInfo();
						otherSchedIter->reassignVesselsToRoutes();
						updateVisitInstalConnection();

						//cout << "Before postLNS bestCost = " << *bestCost << endl;

						//Post-optimize using LNS here
						best = *schedIter; 
						otherBest = *otherSchedIter;
						
						//cout << "before schedIter->multiLNS" << endl;
						schedIter->multiLNS(schedIter->calcNumLNSiter(), bestCost,
							otherSchedIter, &best, getAnotherBaseRoute());

						//cout << "after schedIter->multiLNS" << endl;

						*schedIter = best;
						updateInterBaseInfo();
						updateVisitInstalConnection();

						//cout << "before otherSchedIter->multiLNS" << endl;

						otherSchedIter->multiLNS(otherSchedIter->calcNumLNSiter(),
							bestCost, schedIter, &otherBest, getAnotherBaseRoute());

						//cout << "after otherSchedIter->multiLNS" << endl;

						*otherSchedIter = otherBest;
						updateInterBaseInfo();
						updateVisitInstalConnection();

						multiCost = computeMultiSchedCost();

						multiScheds->push_back(*this);

						cout << "multiCost = " << multiCost << endl;
						cout << "bestCost = " << *bestCost << endl;

						if (multiCost <= *bestCost)
						{
							*bestCost = multiCost;
							bestSched = *schedIter;
							cout << "bestMultiCost = " << *bestCost << endl;
							bestOtherSched = *otherSchedIter;
							writeMultiScheduleToFile(27);
						}

						//return true;
					}
				} //end if (!routesOverlap)
				else //(routesOverlap)
				{
					//restore routes here
					schedIter->setRoute( schedIter->getIsolRoute()->getRouteNum(), isolRoute);
					otherSchedIter->setRoute( otherSchedIter->getIsolRoute()->getRouteNum(), otherIsolRoute);
				}

			}// end if ( potentialSynergy )
			else
				moreIsolations = false;
			
			oneIterationPerformed = true;

			*schedIter = localSched;
			schedIter->updateRelationalInfo();
			schedIter->setIsolRoute();
			schedIter->VesAvail = vesAvailability;
			schedIter->synchrVisitInstalConnection();
	
			*otherSchedIter = localOtherSched;
			otherSchedIter->updateRelationalInfo();
			otherSchedIter->setIsolRoute();
			otherSchedIter->VesAvail = otherVesAvailability;
			otherSchedIter->synchrVisitInstalConnection();

			initMultiSchedule();

		}//end while (moreIsolations)
	}//end if

	*schedIter = bestSched;
	*otherSchedIter = bestOtherSched;

	//schedIter->updateRelationalInfo();
	//otherSchedIter->updateRelationalInfo();

	updateInterBaseInfo();
	updateVisitInstalConnection();

	return false;
}

double MultiSchedule::computeMultiSchedCost()
{
	vector <WeeklySchedule >::iterator schedIter;
	double cost;

	//cout << "Schedules.size() = " << Schedules.size() << endl;

	for (schedIter = Schedules.begin(); schedIter != Schedules.end(); ++schedIter)
		cost +=schedIter->computeSchedCost();

	return cost;
}

void MultiSchedule::printMultiSchedule()
{
	vector <WeeklySchedule >::iterator schedIter;

	for (schedIter = Schedules.begin(); schedIter != Schedules.end(); ++schedIter)
	{
		//cout<< "SCHED COST = " << schedIter->computeSchedCost() << endl;
		schedIter->printWeeklySchedule();
	}
}
void MultiSchedule::writeMultiScheduleToFile(int iter)
{

	const char* fileName;
	fileName = "C:/Aliaksandr Shyshou/Supply Vessels 2008-2010/Multi base/MongstadInst_MBPSVPP_Heuristic/output.txt";
	ofstream solFile(fileName, ios::app);

	vector <WeeklySchedule >::iterator schedIter;

	solFile << "_____________________________________________________________" << endl;
	solFile << "MULTI-SCHEDULE OUTPUT" << endl;
	solFile << "MULTI COST = " << computeMultiSchedCost() << endl;
	
	for (schedIter = Schedules.begin(); schedIter != Schedules.end(); ++schedIter)
		schedIter->writeScheduleToFile(iter, 0);

	solFile << "_____________________________________________________________" << endl;
}

void MultiSchedule::updateInterBaseInfo()
{
	vector<WeeklySchedule>::iterator schedIter;
	vector<WeeklySchedule>::iterator otherSchedIter;

	vector<SupVes*>::iterator vesIter;
	vector <SupVes*> vesPointers;

	vector<Route*>::iterator rouIter;
	vector<Route*> routePointers;

	for (otherSchedIter = Schedules.begin(); otherSchedIter != Schedules.end(); ++otherSchedIter)
	{
		vesPointers.clear();
		vesPointers = otherSchedIter->getSchedVesselsPointers();

		for (vesIter = vesPointers.begin(); vesIter != vesPointers.end(); ++vesIter)
			(*vesIter)->clearVesselRoutes();

		for (schedIter = Schedules.begin(); schedIter != Schedules.end(); ++schedIter)
		{
			
			routePointers.clear();
			routePointers = schedIter->getRoutePointers();

			for (vesIter = vesPointers.begin(); vesIter != vesPointers.end(); ++vesIter)
				{
					
					for (rouIter = routePointers.begin(); rouIter != routePointers.end(); ++rouIter)
						if ((*vesIter)->getName() == (*rouIter)->getRouteVes()->getName() )
						{
							//cout << "Vessel: " << (*vesIter)->getName() << endl;
							//cout << "Adding route: " << endl;
							//(*rouIter)->printRoute();
							(*vesIter)->addVesselRoute(*rouIter);
						}
				
				}//end for vesIter

		}//end for schedIter

		for (vesIter = vesPointers.begin(); vesIter != vesPointers.end(); ++vesIter)
		{
			if ((*vesIter)->getVesselRoutes().size() > 0)
				(*vesIter)->setIsVesselUsed(true);
			else
				(*vesIter)->setIsVesselUsed(false);
		}

		otherSchedIter->setOnlyRoutes();
	}//end for otherSchedIter
}
void MultiSchedule::updateVisitInstalConnection()
{
	vector <WeeklySchedule>::iterator schedIter;

	vector <OffshoreInst*> instalPointers;
	vector <OffshoreInst *>::iterator instalIter;

	vector <Visit*> routeVisits;
	vector <Visit*>::iterator visIter;

	vector <Route*> routePointers;
	vector <Route*>::iterator routeIter;

	for (schedIter = Schedules.begin(); schedIter != Schedules.end(); ++schedIter)
	{
		routePointers.clear();
		routePointers = schedIter->getRoutePointers();

		instalPointers.clear();
		instalPointers = schedIter->getSchedInstalsPointers();

		for (routeIter = routePointers.begin(); routeIter != routePointers.end(); ++routeIter)
		{
			routeVisits.clear();
			routeVisits = (*routeIter)->getVisitPointers();

			for (visIter = routeVisits.begin(); visIter != routeVisits.end(); ++visIter)
				for (instalIter = instalPointers.begin(); instalIter != instalPointers.end(); ++instalIter)
					if ( (*instalIter)->getInstName() == (*visIter)->getOffshoreInst()->getInstName() )
						(*visIter)->setOffshoreInst(*instalIter);
		} // end for routeIter
	}//end for schedIter
}

void MultiSchedule::initMultiSchedule()
{
	int i = 0;
	vector <WeeklySchedule>::iterator schedIter;
	for (schedIter = Schedules.begin(); schedIter != Schedules.end(); ++schedIter)
	{
		schedIter->setIsolRouteNull();
		schedIter->setSchedID(i);
		i++;
		//schedIter->getIsolRoute()->printRoute();
	}//end for (schedIter)
}
void MultiSchedule::synchrScheduleVector()
{
	vector <WeeklySchedule>::iterator schedIter;

	for (schedIter = Schedules.begin(); schedIter != Schedules.end(); ++schedIter)
		schedIter->synchrVisitInstalConnection();
}
