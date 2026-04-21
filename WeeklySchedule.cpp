#include "StdAfx.h"
#include "WeeklySchedule.h"
#include "PSVRP_Instance.h"
#include "Route.h"
#include "Visit.h"


WeeklySchedule::WeeklySchedule(void)
{
}

WeeklySchedule::WeeklySchedule(WeeklySchedule *sched)
{
	*this = *sched;
}


vector <Route*> WeeklySchedule::getRoutePointers()
{
	vector <Route*> routePtrs;
	vector <Route>::iterator routeIter;

	for (routeIter = Routes.begin(); routeIter != Routes.end(); ++routeIter)
		routePtrs.push_back(&(*routeIter));

	return routePtrs;
}

vector <SupVes*> WeeklySchedule::getSchedVesselsPointers()
{
	vector <SupVes*> vesPtrs;
	vector <SupVes>::iterator vesIter;

	for (vesIter = SchedVessels.begin(); vesIter != SchedVessels.end(); ++vesIter)
		vesPtrs.push_back(&(*vesIter));

	return vesPtrs;
}
vector <OffshoreInst*> WeeklySchedule::getSchedInstalsPointers()
{
	vector <OffshoreInst*> instPtrs;
	vector <OffshoreInst>::iterator instIter;

	for (instIter = SchedInstals.begin(); instIter != SchedInstals.end(); ++instIter)
		instPtrs.push_back(&(*instIter));

	return instPtrs;
}

WeeklySchedule::WeeklySchedule(PSVRP_Instance *Inst, int maxDep)
{
	Instance = Inst;
	
	vector < OffshoreInst > Platfs = Instance->getPlatforms();
	vector < OffshoreInst >::iterator it;

	vector < SupVes > Ships = Instance->getVessels();
	vector < SupVes >::iterator iter;

	for (iter = Ships.begin(); iter != Ships.end(); ++iter)
		addVessel(*iter);

	for (it = Platfs.begin(); it != Platfs.end(); ++it)
		addInstallation(*it);

	//Set maximum number of departures per day
	setDepPerDay(maxDep);
	setIsolRoute(NULL);
}

void WeeklySchedule::initSchedule()
{
	int i, j, k, l, m;
	//vector < OffshoreInst > Platfs = Instance->getPlatforms();
	vector < OffshoreInst >::iterator it;

	//vector < SupVes > Ships = Instance->getVessels();
	vector < SupVes >::iterator iter;

	bool routesShortEnough;
	vector <int> routesPerDay;


	vector <SupVes >::iterator vesPointIter;

	vector <int> visComb;
	vector <int> numInstDay;
	int maxNumInstDay;
	int numTries = 0;
	double routeDur;
	int middle;

	bool isSchedFeasible = false;
	int numVis = 0;
	bool combIsOk;

	//START INITIALIZING THE SCHEDUlE
	while (!isSchedFeasible)
	{
		numTries++;
		//reinitialize all vectors
		VisDayAssign.clear();
		numInstDay.clear();

		VisDayAssign.resize(NUMBER_OF_DAYS); //# of rows equals # of days + 1
		numInstDay.resize(NUMBER_OF_DAYS); //# of rows equals # of days + 1
		routesPerDay.resize(NUMBER_OF_DAYS);

		maxNumInstDay = 0;

		for (it = SchedInstals.begin(); it != SchedInstals.end(); ++it)
		{
			//Skip the base
			if (it->getSeqNumb() > 0)
			{
				combIsOk = false;
				
				while (!combIsOk)
				{
					visComb.clear();
					visComb.resize(it->getVisitFreq());
		
					//Randomly choose visit day combination
					int idx = rand() % (it->getNumVisDayComb());
					//cout << "idx = " << idx << endl;
					copy(it->VisitDayCombs[idx].begin(), it->VisitDayCombs[idx].end(),
						visComb.begin() );

					combIsOk = true;

					if (it->ObligatoryVisitDays.size() > 0)
					{
						k = 0;
						for (i = 0; i < visComb.size(); i++)
							for (j = 0; j < it->ObligatoryVisitDays.size(); j++)
								if (visComb[i] == it->ObligatoryVisitDays[j])
									k++;

						if (k < it->ObligatoryVisitDays.size() )
							combIsOk = false;
					}
				}//end while
				
				it->CurVisitDayComb.resize(it->getVisitFreq());
				it->CurVisitDayComb = visComb;

				for (i = 0; i < it->getVisitFreq(); i++)
					VisDayAssign[visComb[i]].push_back(it->getSeqNumb());
			}
		}

		
		for (j = 0; j < NUMBER_OF_DAYS; j++)
		{
			if (VisDayAssign[j].size() > maxNumInstDay)
				maxNumInstDay = VisDayAssign[j].size();

			routesPerDay[j] = ceil( (double)VisDayAssign[j].size()/ (Instance->getMaxInst()) );
		}

		if (maxNumInstDay > Instance->getMaxInst()*getDepPerDay())
		{
			isSchedFeasible = false;
			//cout << "Too many installations per day!!" << endl;
			continue; // we are continuing in outer loop -> ok
		}


		j = 0;
		numVis = 0;
		
		Routes.clear();
		routesShortEnough = true;
		//Initialize route objects for each day
		for (i = 1; i < NUMBER_OF_DAYS; i++)
		{
			middle = ceil( (double)VisDayAssign[i].size()/routesPerDay[i]);
			for (l = 0; l < routesPerDay[i]; l++)
			{
				//Initialize Route objects
				Route route(j, i*24 - 8);
				j++;

				//start at the base
				route.addRouteInstal( &SchedInstals[0] );
				route.addVisit( Visit( &SchedInstals[0] ));

				m = 0;
				//Populate route installations vector
				for (k = l*middle; k < VisDayAssign[i].size(); k++)
				{
					if (m == middle)
						break;

					route.addRouteInstal( &SchedInstals[VisDayAssign[i][k]] );
					route.addVisit( Visit(&SchedInstals[VisDayAssign[i][k]]) );
					numVis++;
					m++;
					

					//cout << Platfs[VisDayAssign[i][k]].getInstName() << " ";
				}
				//cout << endl;

				//end route at the base
				route.addRouteInstal( &SchedInstals[0] );
				route.addVisit( Visit( &SchedInstals[0] ));
			
				//Set route parameters
				route.setRouteAcceptanceTime(Instance->getAcceptTime());
				route.setRouteMinSlack(Instance->getMinSlack());
				route.setRouteLoadFactor(Instance->getLoadFactor());
				route.setMaxVisits(Instance->getMaxInst());
				route.setMinVisits(Instance->getMinInst());	

				route.updateVisitVector();
				//cout << "Route duration = " << route.computeRouteDuration() << endl;
				//route.printRoute();
				
				//route.cheapInsertSeq();
				//route.synchrRouteInstalVisits();
				//route.updateVisitVector();

				route.intelligentReorder();
				routeDur = route.computeRouteDuration();

				if ( (routeDur + route.getRouteAcceptanceTime() >
					route.MAX_ROUTE_DUR) || (routeDur + route.getRouteAcceptanceTime() 
					< route.MIN_ROUTE_DUR) )
				{
					routesShortEnough = false;
					isSchedFeasible = false;
					//cout << "Route too short/long!!!" << endl;
					break;
				}
				//cout << "After intelligentReorder"<< endl;
				//cout << "Route duration = " << route.computeRouteDuration() << endl;
				//route.printRoute();
			
				Routes.push_back(route);

			}// end for l < routesPerDay[i]
				
		}// end for i < NUMBER_OF_DAYS

		if (!routesShortEnough) //one of the routes is too long/too short
			continue; //try again

		setTotNumVis(numVis);
		cout << "TotalNumberOfVisits = " << getTotNumVis() << endl;

		//Assign vessels to routes
		VesAvail.resize(SchedVessels.size());
		int planHorizon = NUMBER_OF_DAYS + (int) ceil(
			( (double)Routes.begin()->MAX_ROUTE_DUR 
			+ Routes.begin()->getInstAtPos(0)->getLayTime() 
			+ Routes.begin()->getRouteMinSlack() )/24);

		//Make all vessels available
		for (k = 0; k < VesAvail.size(); k++)
		{
			VesAvail[k].resize(planHorizon);
			for (i = 0; i < planHorizon; i++)
				VesAvail[k][i] = true;
		}

		//Clear vessel data
		for (vesPointIter = SchedVessels.begin(); vesPointIter != SchedVessels.end();
					++vesPointIter)
		{
			vesPointIter->setIsVesselUsed(false);
			vesPointIter->clearVesselRoutes();
		}

		vector <Route>::iterator routeIter;
		bool vesAssigned = false;
		int routeEndIdx, routeStartIdx;
		bool endOfWeekOverlap = false;
		double rouDur;


		for (routeIter = Routes.begin(); routeIter != Routes.end(); ++routeIter)
		{
			routeStartIdx = (int) ceil(routeIter->getRouteStartTime()/24);
			routeEndIdx = (int) ceil((routeIter->getRouteEndTime() - 
							routeIter->getInstAtPos(0)->getLayTime() +
							routeIter->getRouteMinSlack() )/24);
			rouDur = routeIter->getRouteEndTime() - routeIter->getRouteStartTime();
			while (!vesAssigned) 
			{		
				//Take a vessel. See if it fits by demand and is available
				for (vesPointIter = SchedVessels.begin(); vesPointIter != SchedVessels.end();
					++vesPointIter)
				{
					//Additional check to see if the route overlaps 
					//with the first route next week
					if (routeEndIdx >= (NUMBER_OF_DAYS+1))
						for (i = 0; i <= routeEndIdx % (NUMBER_OF_DAYS+1); i++)
							if(!VesAvail[vesPointIter->getID()][i+1])
							{
								endOfWeekOverlap = true;
								break;
							}

					if ((vesPointIter->getCapacity() >= routeIter->computeRouteDemand()) &&
						(VesAvail[vesPointIter->getID()][(int) ceil(routeIter->getRouteStartTime()/24)] ) 
						&& (!endOfWeekOverlap) && 
						(rouDur < routeIter->MAX_ROUTE_DUR + routeIter->getRouteAcceptanceTime()) )
					{
							//Assign vessel to the route
							routeIter->setRouteVessel(& (*vesPointIter) );
							vesPointIter->setIsVesselUsed(true);
							vesPointIter->addVesselRoute(&(*routeIter)); //add route to "vesselRoutes" vector

							//Update vessel availability vector
							for (i = routeStartIdx; i <= routeEndIdx; i++)
								VesAvail[vesPointIter->getID()][i] = false;

							vesAssigned = true;
							break;
					}//end if 
					endOfWeekOverlap = false;
				}//end for
				
				/*if (!vesAssigned) //no assignment was made
				{
					for (vesPointIter = SchedVessels.begin(); vesPointIter != SchedVessels.end();
					++vesPointIter)
					{
						//Additional check to see if the route overlaps 
						//with the first route next week
						if (routeEndIdx >= NUMBER_OF_DAYS)
							for (i = 0; i <= routeEndIdx % NUMBER_OF_DAYS; i++)
								if(!VesAvail[vesPointIter->getID()][i+1])
								{
									endOfWeekOverlap = true;
									break;
								}

						if ( (VesAvail[vesPointIter->getID()][(int)ceil(routeIter->getRouteStartTime()/24)])
							&& (!endOfWeekOverlap) )
						{
							//Assign vessel to the route
							routeIter->setRouteVessel(&(*vesPointIter));
							vesPointIter->setIsVesselUsed(true);
							vesPointIter->addVesselRoute(&(*routeIter)); //add route to "vesselRoutes" vector

							//Update vessel availability vector
							for (i = routeStartIdx; i <= routeEndIdx; i++)
								VesAvail[vesPointIter->getID()][i] = false;

							vesAssigned = true;
							break;
						}//end if 
						endOfWeekOverlap = false;
					}//end for
				}//end if*/

				if (!vesAssigned) //Still not assigned? Bad bad
				{
					//cout << "Cannot find available vessel!!!" << endl;
					isSchedFeasible = false;
					break; //breaking out of while(!vesAssigned)
				}
				else
					isSchedFeasible = true; //if we reach here then the schedule is feasible
			}//end while(!vesAssigned)

			if (!isSchedFeasible)
				break; //break out of "for each route" loop

			//cout << routeIter->getRouteEndTime() << endl;

			
			
			vesAssigned = false;
		}//end for (each route)

}//end while(!isSchedFeasible)

	setOnlyRoutes();
	setTotNumVis(numVis);

		/*	for (k = 0; k < VesAvail.size(); k++)
			{
				for (i = 0; i < VesAvail[k].size(); i++)
					cout << VesAvail[k][i] << " ";
				cout << endl;
			}
			cout << endl;*/
	//printWeeklySchedule();
//cout << "Attemts to generate initial feasible solution = " << numTries << endl;
//cout << "LB on number of routes = " << computeLB_numRoutes() << endl;
}

WeeklySchedule::WeeklySchedule(PSVRP_Instance* Inst)
{
	Instance = Inst;
	//Randomly assign visit day combinations
	int i, j, k, l, m;
	vector < OffshoreInst > Platfs = Instance->getPlatforms();
	vector < OffshoreInst >::iterator it;

	vector < SupVes > Ships = Instance->getVessels();
	vector < SupVes >::iterator iter;

	bool routesShortEnough;
	vector <int> routesPerDay;

	for (iter = Ships.begin(); iter != Ships.end(); ++iter)
		addVessel(*iter);

	for (it = Platfs.begin(); it != Platfs.end(); ++it)
		addInstallation(*it);

	setIsolRoute(NULL);

	//Initialize a vector of obligatory days for each installation
	for (it = SchedInstals.begin(); it != SchedInstals.end(); ++it)
	{
		
		
		//if (it->getVisitFreq() <= 2)
			//it->ObligatoryVisitDays.push_back(1);
		/*	
		else if (it->getVisitFreq() > 2)
		{
			it->ObligatoryVisitDays.push_back(1);
			it->ObligatoryVisitDays.push_back(2);
			it->ObligatoryVisitDays.push_back(4);
		}

		
		if (it->getInstName() == "GRA")
			it->ObligatoryVisitDays.push_back(2);

		if (it->getInstName() == "HDA")
			it->ObligatoryVisitDays.push_back(2);

		if (it->getInstName() == "OSAD")
		{
			it->ObligatoryVisitDays.push_back(2);
			it->ObligatoryVisitDays.push_back(5);
		}

		if (it->getInstName() == "TBC")
			it->ObligatoryVisitDays.push_back(3);
		*/
	}

	//Set maximum number of departures per day
	setDepPerDay(3);
	
	vector <SupVes >::iterator vesPointIter;

	vector <int> visComb;
	vector <int> numInstDay;
	int maxNumInstDay;
	int numTries = 0;
	double routeDur;
	int middle;

	bool isSchedFeasible = false;
	int numVis = 0;
	bool combIsOk;

	//START INITIALIZING THE SCHEDUlE
	while (!isSchedFeasible)
	{
		numTries++;
		//reinitialize all vectors
		VisDayAssign.clear();
		numInstDay.clear();

		VisDayAssign.resize(NUMBER_OF_DAYS); //# of rows equals # of days + 1
		numInstDay.resize(NUMBER_OF_DAYS); //# of rows equals # of days + 1
		routesPerDay.resize(NUMBER_OF_DAYS);

		maxNumInstDay = 0;

		for (it = SchedInstals.begin(); it != SchedInstals.end(); ++it)
		{
			//Skip the base
			if (it->getSeqNumb() > 0)
			{
				combIsOk = false;
				
				while (!combIsOk)
				{
					visComb.clear();
					visComb.resize(it->getVisitFreq());
		
					//Randomly choose visit day combination
					int idx = rand() % (it->getNumVisDayComb());
					//cout << "idx = " << idx << endl;
					copy(it->VisitDayCombs[idx].begin(), it->VisitDayCombs[idx].end(),
						visComb.begin() );

					combIsOk = true;

					if (it->ObligatoryVisitDays.size() > 0)
					{
						k = 0;
						for (i = 0; i < visComb.size(); i++)
							for (j = 0; j < it->ObligatoryVisitDays.size(); j++)
								if (visComb[i] == it->ObligatoryVisitDays[j])
									k++;

						if (k < it->ObligatoryVisitDays.size() )
							combIsOk = false;
					}
				}//end while
				
				it->CurVisitDayComb.resize(it->getVisitFreq());
				it->CurVisitDayComb = visComb;

				for (i = 0; i < it->getVisitFreq(); i++)
					VisDayAssign[visComb[i]].push_back(it->getSeqNumb());
			}
		}

		
		for (j = 0; j < NUMBER_OF_DAYS; j++)
		{
			if (VisDayAssign[j].size() > maxNumInstDay)
				maxNumInstDay = VisDayAssign[j].size();

			routesPerDay[j] = ceil( (double)VisDayAssign[j].size()/ (Inst->getMaxInst()) );
		}

		if (maxNumInstDay > Inst->getMaxInst()*getDepPerDay())
		{
			isSchedFeasible = false;
			//cout << "Too many installations per day!!" << endl;
			continue; // we are continuing in outer loop -> ok
		}


		j = 0;
		numVis = 0;
		
		Routes.clear();
		routesShortEnough = true;
		//Initialize route objects for each day
		for (i = 1; i < NUMBER_OF_DAYS; i++)
		{
			middle = ceil( (double)VisDayAssign[i].size()/routesPerDay[i]);
			for (l = 0; l < routesPerDay[i]; l++)
			{
				//Initialize Route objects
				Route route(j, i*24 - 8);
				j++;

				//start at the base
				route.addRouteInstal( &SchedInstals[0] );
				route.addVisit( Visit( &SchedInstals[0] ));

				m = 0;
				//Populate route installations vector
				for (k = l*middle; k < VisDayAssign[i].size(); k++)
				{
					if (m == middle)
						break;

					route.addRouteInstal( &SchedInstals[VisDayAssign[i][k]] );
					route.addVisit( Visit(&SchedInstals[VisDayAssign[i][k]]) );
					numVis++;
					m++;
					

					//cout << Platfs[VisDayAssign[i][k]].getInstName() << " ";
				}
				//cout << endl;

				//end route at the base
				route.addRouteInstal( &SchedInstals[0] );
				route.addVisit( Visit( &SchedInstals[0] ));
			
				//Set route parameters
				route.setRouteAcceptanceTime(Inst->getAcceptTime());
				route.setRouteMinSlack(Inst->getMinSlack());
				route.setRouteLoadFactor(Inst->getLoadFactor());
				route.setMaxVisits(Inst->getMaxInst());
				route.setMinVisits(Inst->getMinInst());	

				route.updateVisitVector();
				//cout << "Route duration = " << route.computeRouteDuration() << endl;
				//route.printRoute();
				
				//route.cheapInsertSeq();
				//route.synchrRouteInstalVisits();
				//route.updateVisitVector();

				route.intelligentReorder();
				routeDur = route.computeRouteDuration();

				if ( (routeDur + route.getRouteAcceptanceTime() >
					route.MAX_ROUTE_DUR) || (routeDur + route.getRouteAcceptanceTime() 
					< route.MIN_ROUTE_DUR) )
				{
					routesShortEnough = false;
					isSchedFeasible = false;
					//cout << "Route too short/long!!!" << endl;
					break;
				}
				//cout << "After intelligentReorder"<< endl;
				//cout << "Route duration = " << route.computeRouteDuration() << endl;
				//route.printRoute();
			
				Routes.push_back(route);

			}// end for l < routesPerDay[i]
				
		}// end for i < NUMBER_OF_DAYS

		if (!routesShortEnough) //one of the routes is too long/too short
			continue; //try again

		setTotNumVis(numVis);
		cout << "TotalNumberOfVisits = " << getTotNumVis() << endl;

		//Assign vessels to routes
		VesAvail.resize(SchedVessels.size());
		int planHorizon = NUMBER_OF_DAYS + (int) ceil(
			( (double)Routes.begin()->MAX_ROUTE_DUR 
			+ Routes.begin()->getInstAtPos(0)->getLayTime() 
			+ Routes.begin()->getRouteMinSlack() )/24);

		//Make all vessels available
		for (k = 0; k < VesAvail.size(); k++)
		{
			VesAvail[k].resize(planHorizon);
			for (i = 0; i < planHorizon; i++)
				VesAvail[k][i] = true;
		}

		//Clear vessel data
		for (vesPointIter = SchedVessels.begin(); vesPointIter != SchedVessels.end();
					++vesPointIter)
		{
			vesPointIter->setIsVesselUsed(false);
			vesPointIter->clearVesselRoutes();
		}

		vector <Route>::iterator routeIter;
		bool vesAssigned = false;
		int routeEndIdx, routeStartIdx;
		bool endOfWeekOverlap = false;
		double rouDur;


		for (routeIter = Routes.begin(); routeIter != Routes.end(); ++routeIter)
		{
			routeStartIdx = (int) ceil(routeIter->getRouteStartTime()/24);
			routeEndIdx = (int) ceil((routeIter->getRouteEndTime() - 
							routeIter->getInstAtPos(0)->getLayTime() +
							routeIter->getRouteMinSlack() )/24);
			rouDur = routeIter->getRouteEndTime() - routeIter->getRouteStartTime();
			while (!vesAssigned) 
			{		
				//Take a vessel. See if it fits by demand and is available
				for (vesPointIter = SchedVessels.begin(); vesPointIter != SchedVessels.end();
					++vesPointIter)
				{
					//Additional check to see if the route overlaps 
					//with the first route next week
					if (routeEndIdx >= (NUMBER_OF_DAYS+1))
						for (i = 0; i <= routeEndIdx % (NUMBER_OF_DAYS+1); i++)
							if(!VesAvail[vesPointIter->getID()][i+1])
							{
								endOfWeekOverlap = true;
								break;
							}

					if ((vesPointIter->getCapacity() >= routeIter->computeRouteDemand()) &&
						(VesAvail[vesPointIter->getID()][(int) ceil(routeIter->getRouteStartTime()/24)] ) 
						&& (!endOfWeekOverlap) && 
						(rouDur < routeIter->MAX_ROUTE_DUR + routeIter->getRouteAcceptanceTime()) )
					{
							//Assign vessel to the route
							routeIter->setRouteVessel(& (*vesPointIter) );
							vesPointIter->setIsVesselUsed(true);
							vesPointIter->addVesselRoute(&(*routeIter)); //add route to "vesselRoutes" vector

							//Update vessel availability vector
							for (i = routeStartIdx; i <= routeEndIdx; i++)
								VesAvail[vesPointIter->getID()][i] = false;

							vesAssigned = true;
							break;
					}//end if 
					endOfWeekOverlap = false;
				}//end for
				
				/*if (!vesAssigned) //no assignment was made
				{
					for (vesPointIter = SchedVessels.begin(); vesPointIter != SchedVessels.end();
					++vesPointIter)
					{
						//Additional check to see if the route overlaps 
						//with the first route next week
						if (routeEndIdx >= NUMBER_OF_DAYS)
							for (i = 0; i <= routeEndIdx % NUMBER_OF_DAYS; i++)
								if(!VesAvail[vesPointIter->getID()][i+1])
								{
									endOfWeekOverlap = true;
									break;
								}

						if ( (VesAvail[vesPointIter->getID()][(int)ceil(routeIter->getRouteStartTime()/24)])
							&& (!endOfWeekOverlap) )
						{
							//Assign vessel to the route
							routeIter->setRouteVessel(&(*vesPointIter));
							vesPointIter->setIsVesselUsed(true);
							vesPointIter->addVesselRoute(&(*routeIter)); //add route to "vesselRoutes" vector

							//Update vessel availability vector
							for (i = routeStartIdx; i <= routeEndIdx; i++)
								VesAvail[vesPointIter->getID()][i] = false;

							vesAssigned = true;
							break;
						}//end if 
						endOfWeekOverlap = false;
					}//end for
				}//end if*/

				if (!vesAssigned) //Still not assigned? Bad bad
				{
					//cout << "Cannot find available vessel!!!" << endl;
					isSchedFeasible = false;
					break; //breaking out of while(!vesAssigned)
				}
				else
					isSchedFeasible = true; //if we reach here then the schedule is feasible
			}//end while(!vesAssigned)

			if (!isSchedFeasible)
				break; //break out of "for each route" loop

			//cout << routeIter->getRouteEndTime() << endl;

			
			
			vesAssigned = false;
		}//end for (each route)

}//end while(!isSchedFeasible)

	setOnlyRoutes();
	setTotNumVis(numVis);

		/*	for (k = 0; k < VesAvail.size(); k++)
			{
				for (i = 0; i < VesAvail[k].size(); i++)
					cout << VesAvail[k][i] << " ";
				cout << endl;
			}
			cout << endl;*/
	//printWeeklySchedule();
//cout << "Attemts to generate initial feasible solution = " << numTries << endl;
//cout << "LB on number of routes = " << computeLB_numRoutes() << endl;
}

Route* WeeklySchedule::getRoute(int routeNum)
{
	return &Routes[routeNum];
}



int WeeklySchedule::computeLB_numRoutes ()
{
	int lb1 = 0;
	double lb2 = 0;
	double lb3 = 0;
	double maxCap = 0;

	vector <OffshoreInst>::iterator it;
	vector <SupVes>::iterator iter;

	for (iter = SchedVessels.begin(); iter != SchedVessels.end(); ++iter)
		if (iter->getCapacity() > maxCap)
			maxCap = iter->getCapacity();

	for (it = SchedInstals.begin(); it != SchedInstals.end(); ++it)
	{
		lb2 += it->getVisitFreq();
		lb3 += it->getWeeklyDemand();
		if (it->getVisitFreq() > lb1)
			lb1 = it->getVisitFreq();
	}

	//cout << "lb1 = " << lb1 << ", lb2 = " << lb2 << ", lb3 = " << lb3 << endl;
	//cout << "maxCap = " << maxCap << "MaxInst = " << Instance->getMaxInst() << endl;

	lb2 = max(lb1, (int) ceil( lb2/Instance->getMaxInst() ) );
	return max( (int)lb2, (int)ceil (lb3/maxCap) );

}


void WeeklySchedule::printWeeklySchedule()
{
	cout << "Number of routes = " << Routes.size() << endl;
	vector <Route>::iterator it;
	int i, k, l;
	vector <OffshoreInst>::iterator instIter;

	for (k = 0; k < VesAvail.size(); k++)
	{
		for (i = 0; i < VesAvail[k].size(); i++)
			cout << VesAvail[k][i] << " ";

		cout << endl;
	}

	//Print visit day combinations
	cout << "Installations: Visit day combinations" << endl;
	for (instIter = SchedInstals.begin(); instIter != SchedInstals.end(); ++instIter)
	{
		cout << instIter->getInstName() << ": ";
		for (l = 0; l < instIter->CurVisitDayComb.size(); l++)
			cout << instIter->CurVisitDayComb[l] << " ";

		cout << endl;
	}

	for (it = Routes.begin(); it != Routes.end(); ++it)
	{
		
		cout << "____________" << it->getRouteVes()->getName();
		cout << " id = " << it->getRouteVes()->getID() << "_____________" << endl;
		cout << "Route number " << it->getRouteNum();
		it->getOnlyRoute() ? cout << " Single " : cout << " Multiple ";
		cout << endl;

		it->printRoute();
	}
}


double WeeklySchedule::computeSchedCost()
{	
	vector < SupVes >::iterator vesIter;

	vector <Visit> routeVisits;
	vector <Visit>::iterator visitIter;

	vector <Route*> vesRoutes;
	vector <Route*>::iterator routeIter;
	
	double cost = 0;
	double sailTime, baseTime, instTime;
	double routeSlack;
	int i;

	for (vesIter = SchedVessels.begin(); vesIter != SchedVessels.end(); ++vesIter)
	{
			
		if (vesIter->getIsVesselUsed())
		{
			sailTime = 0; 
			baseTime = 0;
			instTime = 0;

			vesRoutes.clear();
			vesRoutes = vesIter->getVesselRoutes();
			//cout << "Vessel = " << vesIter->getName() << endl;
			//cout << "VesRoutes size = " << vesRoutes.size() << endl;

			for (routeIter = vesRoutes.begin(); routeIter != vesRoutes.end(); ++routeIter)
			{
				//(*routeIter)->printRoute();

				routeSlack = 0;
				routeSlack = (*routeIter)->MAX_ROUTE_DUR - 
					(*routeIter)->computeRouteDuration();
				while (routeSlack > 24)
					routeSlack -= 24;

				
				routeVisits.clear();
				routeVisits = (*routeIter)->getVisitObjects();

				//cout << "Route " << (*routeIter)->getRouteNum() << endl;

				baseTime += (*routeIter)->getInstAtPos(0)->getLayTime() + 
							routeSlack;

				for (visitIter = routeVisits.begin(); visitIter != routeVisits.end(); ++visitIter)
				{//Compute Sailing and Installation times
					if (visitIter < routeVisits.end() - 1 )
					{
						//cout << visitIter->getOffshoreInst()->getInstName() << endl;
						if (visitIter != routeVisits.begin())
						{
							instTime += (visitIter->getVisitEnd() - visitIter->getVisitStart() );
						
							/*if (visitIter->getVisitWaitTime() > 0)
								instTime += visitIter->getVisitWaitTime();*/
						}

						//cout << (visitIter+1)->getOffshoreInst()->getSeqNumb();
						/*
						if ( ((visitIter+1)->getOffshoreInst()->getSeqNumb() == SchedInstals.size() - 1)
							&& ((visitIter+1)->getOffshoreInst()->getSeqNumb() == 5) )
						{
							
								cout << "(visitIter+1).Dist.size() = " << (visitIter+1)->getOffshoreInst()->Dist.size() << endl;
								cout << "(visitIter+1)->getOffshoreInst()->getSeqNumb() = " 
									<< (visitIter+1)->getOffshoreInst()->getSeqNumb() << endl;
								cout << "Dist vector" << endl;

								for (i = 0; i < (visitIter+1)->getOffshoreInst()->Dist.size();i++)
									cout << (visitIter+1)->getOffshoreInst()->Dist[i] << " ";

								cout << "visitIter->Dist.size() = " << (visitIter)->getOffshoreInst()->Dist.size() << endl;
								cout << "visitIter->SeqNumb() = " << (visitIter)->getOffshoreInst()->getSeqNumb() << endl;
								
								cout << "Dist vector" << endl;
								for (i = 0; i < (visitIter)->getOffshoreInst()->Dist.size();i++)
									cout << (visitIter)->getOffshoreInst()->Dist[i] << " ";
								cout << endl;
								

								cout << "visitIter-1->Dist.size() = " << (visitIter - 1)->getOffshoreInst()->Dist.size() << endl;
								cout << "visitIter-2->Dist.size() = " << (visitIter - 2)->getOffshoreInst()->Dist.size() << endl;


						}
						*/
						sailTime += visitIter->getOffshoreInst()->Dist[(visitIter+1)->getOffshoreInst()->getSeqNumb()]
							/ vesIter->getSpeed();		
					}
				}//end for visitIter
				//cout << endl;
			}//end for routeIter
			

			
			//cout << "Vessel " << vesIter->getName() << " time distribution:" << endl;
			//cout << "Base = " << baseTime << " Installation = " << instTime <<
			//	" Sailing = " << sailTime << endl;

			cost += vesIter->getVesselCost() + 
				baseTime * (vesIter->getFCBase() ) * (vesIter->getFCCosts() ) + 
				sailTime * (vesIter->getFCSailing() ) * (vesIter->getFCCosts() ) +
				instTime * (vesIter->getFCInstallation() ) * (vesIter->getFCCosts() );

			/*
			cout << "Vessel " << vesIter->getName() << endl;
			cout << "Number of routes = " << vesRoutes.size() << endl;
			cout << "Base time = " << baseTime << endl
				<< "Inst time = " << instTime << endl
				<< "Sailing time = " << sailTime << endl<< endl;

				*/

		}//end if (isVesselUsed)

	}//end for vesIter

	return cost;
}

double WeeklySchedule::computeTotalSlack()
{
	double totSlack, routeSlack;
	vector <Route>::iterator routeIter;

	totSlack = 0;

	for (routeIter = Routes.begin(); routeIter != Routes.end(); ++routeIter)
	{
					routeSlack = routeIter->MAX_ROUTE_DUR - 
						routeIter->computeRouteDuration();
					while (routeSlack > 24)
						routeSlack -= 24;

					totSlack += routeSlack;
	}

	return totSlack;
}

int WeeklySchedule::computeDurTotDays()
{
	int totDays, l;

	for (l = 0; l < Routes.size(); l++)
		totDays += (int)ceil( (Routes[l].computeRouteDuration()+
			Routes[l].getInstAtPos(0)->getLayTime() + Routes[l].getRouteMinSlack())/24);

	return totDays;
}

bool WeeklySchedule::assignVessels()
{
	//Assign vessels to routes
		VesAvail.resize(SchedVessels.size());
		int planHorizon = NUMBER_OF_DAYS + (int) ceil(
			( (double)Routes.begin()->MAX_ROUTE_DUR 
			+ Routes.begin()->getInstAtPos(0)->getLayTime() 
			+ Routes.begin()->getRouteMinSlack() )/24);

		vector <SupVes>::iterator vesPointIter;
		int i, k;

		//Make all vessels available
		for (k = 0; k < VesAvail.size(); k++)
		{
			VesAvail[k].resize(planHorizon);
			for (i = 0; i < planHorizon; i++)
				VesAvail[k][i] = true;
		}

		//Clear vessel data
		for (vesPointIter = SchedVessels.begin(); vesPointIter != SchedVessels.end();
					++vesPointIter)
		{
			vesPointIter->setIsVesselUsed(false);
			vesPointIter->clearVesselRoutes();
		}

		vector <Route>::iterator routeIter;
		bool vesAssigned = false;
		int routeEndIdx, routeStartIdx;
		bool endOfWeekOverlap = false;
		double rouDur;
		bool isSchedFeasible;


		for (routeIter = Routes.begin(); routeIter != Routes.end(); ++routeIter)
		{
			routeStartIdx = (int) ceil(routeIter->getRouteStartTime()/24);
			routeEndIdx = (int) ceil( (routeIter->getRouteEndTime() - 
							routeIter->getInstAtPos(0)->getLayTime() +
							routeIter->getRouteMinSlack() )/24 );
			rouDur = routeIter->getRouteEndTime() - routeIter->getRouteStartTime();
			while (!vesAssigned) 
			{		
				//Take a vessel. See if it fits by demand and is available
				for (vesPointIter = SchedVessels.begin(); vesPointIter != SchedVessels.end();
					++vesPointIter)
				{
					//Additional check to see if the route overlaps 
					//with the first route next week
					if (routeEndIdx >= (NUMBER_OF_DAYS+1))
						for (i = 0; i <= routeEndIdx % (NUMBER_OF_DAYS+1); i++)
							if(!VesAvail[vesPointIter->getID()][i+1])
							{
								endOfWeekOverlap = true;
								break;
							}

					if ((vesPointIter->getCapacity() >= routeIter->computeRouteDemand()) &&
						(VesAvail[vesPointIter->getID()][(int) ceil(routeIter->getRouteStartTime()/24)] ) 
						&& (!endOfWeekOverlap) && 
						(rouDur < routeIter->MAX_ROUTE_DUR + routeIter->getRouteAcceptanceTime())
						&& vesPointIter->getIsVesselUsed() )
					{
							//Assign vessel to the route
							routeIter->setRouteVessel(& (*vesPointIter) );
							vesPointIter->setIsVesselUsed(true);
							vesPointIter->addVesselRoute(&(*routeIter)); //add route to "vesselRoutes" vector

							//Update vessel availability vector
							for (i = routeStartIdx; i <= routeEndIdx; i++)
								VesAvail[vesPointIter->getID()][i] = false;

							vesAssigned = true;
							break;
					}//end if 

					if ((vesPointIter->getCapacity() >= routeIter->computeRouteDemand()) &&
						(VesAvail[vesPointIter->getID()][(int) ceil(routeIter->getRouteStartTime()/24)] ) 
						&& (!endOfWeekOverlap) && 
						(rouDur < routeIter->MAX_ROUTE_DUR + routeIter->getRouteAcceptanceTime()) )
					{
							//Assign vessel to the route
							routeIter->setRouteVessel(& (*vesPointIter) );
							vesPointIter->setIsVesselUsed(true);
							vesPointIter->addVesselRoute(&(*routeIter)); //add route to "vesselRoutes" vector

							//Update vessel availability vector
							for (i = routeStartIdx; i <= routeEndIdx; i++)
								VesAvail[vesPointIter->getID()][i] = false;

							vesAssigned = true;
							break;
					}//end if 

					endOfWeekOverlap = false;
				}//end for

				if (!vesAssigned) //Still not assigned? Bad bad
				{
					cout << "Cannot find available vessel!!!" << endl;
					isSchedFeasible = false;
					break; //breaking out of while(!vesAssigned)
				}
				else
					isSchedFeasible = true; //if we reach here then the schedule is feasible
			}//end while(!vesAssigned)

			if (!isSchedFeasible)
				break; //break out of "for each route" loop

			//cout << routeIter->getRouteEndTime() << endl;

			/*
			for (k = 0; k < VesAvail.size(); k++)
			{
				for (i = 0; i < VesAvail[k].size(); i++)
					cout << VesAvail[k][i] << " ";
				cout << endl;
			}*/
			vesAssigned = false;
		}//end for (each route)

	return isSchedFeasible;
}

void WeeklySchedule::reassignVesselsToRoutes()
{
	//Assign vessels prioritizing already used ones
	//Assign them intelligently
	//We have to clear and update all the vessel data:
	//1)Route class; 2) WeeklySchedule class; 3) Visit class;


	int i, j, k;
	vector <SupVes>::iterator vesPointIter;
	vector <SupVes>::iterator vesIter;

	vector <vector <bool> > vesAvailability; //local vessel availability vector
	vesAvailability.resize(VesAvail.size());

	int planHorizon = NUMBER_OF_DAYS + (int) ceil(
			( (double)Routes.begin()->MAX_ROUTE_DUR 
			+ Routes.begin()->getInstAtPos(0)->getLayTime() 
			+ Routes.begin()->getRouteMinSlack() )/24);
	
	for (k = 0; k < VesAvail.size(); k++)
		{
			vesAvailability[k].resize(planHorizon);
			for (i = 0; i < planHorizon; i++)
				vesAvailability[k][i] = VesAvail[k][i];
		}

	bool isAssignmentFeasible;
	bool allRoutesReassigned;
	bool endOfWeekOverlap;

	vector <Route*>::iterator routeIter;
	vector <Route*> vesRoutes;

	int routeEndIdx, routeStartIdx;
	double rouDur;

	vector <bool> isRouteReassigned;
	vector <SupVes*> assignToVessel;

	/*
	cout << "Before reassignVessels VesAvail is " << endl;
	for (k = 0; k < VesAvail.size(); k++)
	{
		for (i = 0; i < VesAvail[k].size(); i++)
			cout << VesAvail[k][i] << " ";
		cout << endl;
	}
	cout << endl;
	*/

	for (vesPointIter = SchedVessels.begin(); vesPointIter != SchedVessels.end(); ++vesPointIter)
	{
		if (vesPointIter->getIsVesselUsed())
		{
			vesRoutes.clear();
			vesRoutes = vesPointIter->getVesselRoutes();
			isRouteReassigned.clear();
			isRouteReassigned.resize(vesRoutes.size());
			assignToVessel.resize(vesRoutes.size());

			j=0;
			allRoutesReassigned = true;

			for (routeIter = vesRoutes.begin(); routeIter != vesRoutes.end(); ++routeIter)
			{
				routeStartIdx = (int) ceil((*routeIter)->getRouteStartTime()/24);
				//cout << "RouteStartIdx = " << routeStartIdx << endl;

				routeEndIdx = (int) ceil( ( (*routeIter)->getRouteEndTime() - 
								(*routeIter)->getInstAtPos(0)->getLayTime() +
								(*routeIter)->getRouteMinSlack() )/24 );
				//cout<< "RouteEndIdx = " << routeEndIdx << endl;

				rouDur = (*routeIter)->getRouteEndTime() - (*routeIter)->getRouteStartTime();

				//Try to assign route to a different vessel
				for (vesIter = SchedVessels.begin(); vesIter != SchedVessels.end(); ++vesIter)
					if ( (vesIter->getID() < vesPointIter->getID()) ||
						(vesIter->getIsVesselUsed()) )
					{
						isAssignmentFeasible = true;
						endOfWeekOverlap = false;

						for (i = routeStartIdx; i <= routeEndIdx; i++)
						{
							if(!vesAvailability[vesIter->getID()][i])
							{
								isAssignmentFeasible = false;
								break;
							}

							if ( (i <= planHorizon - (NUMBER_OF_DAYS+1))
								&& (!vesAvailability[vesIter->getID()][i + NUMBER_OF_DAYS]) )
								{
									isAssignmentFeasible = false;
									break;
								}
						}

						//Additional check to see if the route overlaps 
						//with the first route next week
						if (routeEndIdx >= (NUMBER_OF_DAYS+1) )
							for (i = 0; i <= routeEndIdx % (NUMBER_OF_DAYS+1); i++)
								if(!vesAvailability[vesIter->getID()][i+1])
								{
									endOfWeekOverlap = true;
									break;
								}
						//Can we reassign the route to another vessel?
						if ((vesIter->getCapacity() >= (*routeIter)->computeRouteDemand()) && 
							(isAssignmentFeasible) &&
							(rouDur < (*routeIter)->MAX_ROUTE_DUR + (*routeIter)->getRouteAcceptanceTime())
							&& (!endOfWeekOverlap) )
						{
								//Assign vessel to the route
							isRouteReassigned[j] = true;
							assignToVessel[j] = &(*vesIter);

							//Update vessel availability vector
							for (i = routeStartIdx; i <= routeEndIdx; i++)
								vesAvailability[vesIter->getID()][i] = false;

							break; //no need to assign the same route to several vessels
						}//end if 
						else
							isRouteReassigned[j] = false;
					}// end if (vesIter != vesPointIter) and for (vesIter)
					j++;		
			}//end for (routeIter)

			//Did we relocate all the routes
			for (j = 0; j < isRouteReassigned.size(); j++)
				if (!isRouteReassigned[j])
				{
					allRoutesReassigned = false;
					break;
				}

			if (allRoutesReassigned) //Do the actual modifications
			{
				for (k = 0; k < planHorizon; k++)
					vesAvailability[vesPointIter->getID()][k] = true;

				VesAvail = vesAvailability;
				vesPointIter->setIsVesselUsed(false);

				for (i = 0; i < vesRoutes.size(); i++)
				{
					//Assign vessel to the route
					//cout << "Reassigning Route " << vesRoutes[i]->getRouteNum() << endl;
					//cout << "To vessel " << assignToVessel[i]->getName() << endl;
					vesRoutes[i]->setRouteVessel(assignToVessel[i]);
					//add route to "vesselRoutes" vector
					assignToVessel[i]->setIsVesselUsed(true);
					assignToVessel[i]->addVesselRoute(vesRoutes[i]); 
				}
				vesPointIter->clearVesselRoutes(); 

				
				cout << "Improvement in ReassignVesselsToRoutes()!!!!" << endl;
				for (k = 0; k < VesAvail.size(); k++)
				{
					for (i = 0; i < VesAvail[k].size(); i++)
						cout << VesAvail[k][i] << " ";
					cout << endl;
				}
				cout << endl;
				
			}
			else
				vesAvailability = VesAvail;
			/*			
			for (k = 0; k < VesAvail.size(); k++)
			{
				for (i = 0; i < VesAvail[k].size(); i++)
					cout << VesAvail[k][i] << " ";
				cout << endl;
			}
			cout << endl;
			*/
		}//end if
	
	}//end for (vesPointIter)
}

void WeeklySchedule::reassignIsolatedVoyage()
{
	//Assign isolated voyage to a vessel with largest feasible capacity
	int i, j, k;
	vector <SupVes>::iterator vesPointIter;
	
	vector <vector <bool> > vesAvailability; //local vessel availability vector
	vesAvailability.resize(VesAvail.size());

	int planHorizon = NUMBER_OF_DAYS + (int) ceil(
			( (double)Routes.begin()->MAX_ROUTE_DUR 
			+ Routes.begin()->getInstAtPos(0)->getLayTime() 
			+ Routes.begin()->getRouteMinSlack() )/24);
	
	for (k = 0; k < VesAvail.size(); k++)
		{
			vesAvailability[k].resize(planHorizon);
			for (i = 0; i < planHorizon; i++)
				vesAvailability[k][i] = VesAvail[k][i];
		}

	vector <Route*>::iterator routeIter;
	vector <Route*> vesRoutes;

	vector <bool> isRouteReassigned;
	
	isRouteReassigned.resize(SchedVessels.size());
	
	for (i = 0; i < isRouteReassigned.size(); i++)
		isRouteReassigned[i] = false;

	for (vesPointIter = SchedVessels.begin(); vesPointIter != SchedVessels.end(); ++vesPointIter)
	{
		if ( (vesPointIter->getIsVesselUsed()) && (vesPointIter->getID() > getIsolRoute()->getRouteVes()->getID() ) )
		{
			vesRoutes.clear();
			vesRoutes = vesPointIter->getVesselRoutes();
			
			for (routeIter = vesRoutes.begin(); routeIter != vesRoutes.end(); ++routeIter)
			{
				if ((*routeIter)->computeRouteDemand() > getIsolRoute()->getRouteVes()->getCapacity())
				{
					isRouteReassigned[vesPointIter->getID()] = false;
					break;
				}
				isRouteReassigned[vesPointIter->getID()] = true;
			}//end for (routeIter)
		}
	}

	int bestIdx = getIsolRoute()->getRouteVes()->getID();

	for ( i = getIsolRoute()->getRouteVes()->getID(); i < SchedVessels.size(); i++)
		if (isRouteReassigned[i])
			bestIdx = i;

	if (bestIdx > getIsolRoute()->getRouteVes()->getID())
	{//Do actual reassignments

		//Update VesAvail
		for (k = 0; k < planHorizon; k++)
		{
			VesAvail[bestIdx][k] = vesAvailability[getIsolRoute()->getRouteVes()->getID()][k];
			VesAvail[getIsolRoute()->getRouteVes()->getID()][k] = vesAvailability[bestIdx][k];
		}


		for (vesPointIter = SchedVessels.begin(); vesPointIter != SchedVessels.end(); ++vesPointIter)
			if (vesPointIter->getID() == bestIdx)
				break;

		vesRoutes.clear();
		vesRoutes = vesPointIter->getVesselRoutes();
		
		for (i = 0; i < vesRoutes.size(); i++)
			vesRoutes[i]->setRouteVessel(getIsolRoute()->getRouteVes());
		
		getIsolRoute()->setRouteVessel(&(*vesPointIter));

		updateRelationalInfo();

		cout << "Improvement in Reassign_ISOLATED_VOYAGE" << endl;
		for (k = 0; k < VesAvail.size(); k++)
		{
			for (i = 0; i < VesAvail[k].size(); i++)
				cout << VesAvail[k][i] << " ";
			cout << endl;
		}
		cout << endl;
		
	}
}

void WeeklySchedule::reduceNumberOfRoutes()
{
	//Redistributing a route visits between other routes
	//Making sure they are still evenly spread

	//cout << "reduceNumberOfRoutes_BEGIN" << endl;

	vector <Route>::iterator routeIter;
	vector <Route>::iterator routeRelocIter;
	vector <Route>::iterator routeIntermedIter;

	vector <Visit> routeVisits;
	vector <Visit> routeRelocVisits;
	vector <Visit>::iterator visitIter;

	vector <SupVes>::iterator vesIter;

	int i = 0, j = 1, k = 0, l, m;
	int routeStartIndex, routeEndIndex;

	vector <Route> savedRoutes; //additional vector of routes
	vector <Route> intermedRoutes;
	savedRoutes.resize(Routes.size());
	intermedRoutes.resize(Routes.size());

	vector < vector <int> > VisCombs;

	int LB = computeLB_numRoutes();

	if (Routes.size() == LB)
		return;

	for (l = 0; l < Routes.size(); l++)
	{
		savedRoutes[l] = Routes[l];
		intermedRoutes[l] = Routes[l];
	}

	double routeToDurBefore, routeToDurAfter;
	double schedCostBefore, schedCostAfter;
	double routeSlackBefore, routeSlackAfter;

	double bestDeltaObj;
	int bestVarIdx;
	int routeDaysBefore, routeDaysAfter, daysDelta;
	int bestIdxRegret, bestIdxInsert;
	double bestCostRouteRemove, bestIdxRouteRemove;

	int routeStartIdx, routeEndIdx;
	int routeDays;

	double smallestInsertionCost, largestRegret;

	bool insertionOverlaps = false;
	bool improvementFound = true;
	bool insertionPossible = true;

	vector <vector <bool> > vesAvailability; //local vessel availability vector

	vesAvailability.resize(VesAvail.size());

	int planHorizon = NUMBER_OF_DAYS + (int) ceil(
			( (double)Routes.begin()->MAX_ROUTE_DUR 
			+ Routes.begin()->getInstAtPos(0)->getLayTime() 
			+ Routes.begin()->getRouteMinSlack() )/24);

	for (k = 0; k < VesAvail.size(); k++)
		{
			vesAvailability[k].resize(planHorizon);
			for (i = 0; i < planHorizon; i++)
				vesAvailability[k][i] = VesAvail[k][i];
		}

	vector <double> routeRemovalCost;
	vector <int> routeRemovedNumber;
	double costAfterRemove, costBeforeRemove;

	//cout << "-----------Entering reduceNumberOfRoutes----------" << endl;
	//cout << "Routes.size() = " << Routes.size() << endl << endl;
	//printWeeklySchedule();

while (improvementFound)
{
	routeRemovalCost.clear();
	routeRemovedNumber.clear();

	for (routeIter = Routes.begin(); routeIter != Routes.end(); ++routeIter)
		//if (!routeIter->getOnlyRoute())
	{
		costBeforeRemove = computeSchedCost();
		insertionPossible = true;
		//cout << "Route number " << routeIter->getRouteNum() << endl;

		routeRelocVisits.clear();
		routeRelocVisits = routeIter->getVisitObjects();

		VisCombs.clear();
		VisCombs.resize(routeIter->getVisitObjects().size());

		//Save the visit day combinations of installations on the route
		for (l = 1; l < routeIter->getVisitObjects().size()-1; l++)
			for (m = 0; m < routeRelocVisits[l].getOffshoreInst()->getVisitFreq(); m++)
				VisCombs[l].push_back(routeRelocVisits[l].getOffshoreInst()->CurVisitDayComb[m]);

		

		do 
		{
			routeVisits.clear();
			routeVisits = routeIter->getVisitObjects();
			
			for (visitIter = routeVisits.begin()+1; visitIter != routeVisits.end()-1; ++visitIter)
			{
				visitIter->VisVars.clear();
				visitIter->RegretValues.clear();
				VisitVariation exchVar(&(*visitIter));

				//cout << "Let's see" << endl;

				//Evaluate and sort all possible insertions for a given visit
				for (routeRelocIter = Routes.begin(); routeRelocIter != Routes.end(); ++routeRelocIter)
				{
					routeDays = (int)ceil( (routeRelocIter->computeRouteDuration()+ 
					routeRelocIter->getInstAtPos(0)->getLayTime()
					+ routeRelocIter->getRouteMinSlack() )/24);

					routeStartIdx = (int) ceil(routeRelocIter->getRouteStartTime()/24);
					routeEndIdx = routeStartIdx + routeDays - 1;

					if ( (routeRelocIter->computeRouteDemand() + 
						visitIter->getOffshoreInst()->getWeeklyDemand()/visitIter->getOffshoreInst()->getVisitFreq()
						< routeRelocIter->getRouteVes()->getCapacity()) &&  
						(routeRelocIter->getVisitObjects().size() - 2 < routeRelocIter->getMaxVisits() ) &&
						!routeRelocIter->isInstOnRoute(visitIter->getOffshoreInst()) &&
						isDepartureSpreadEven(&(*routeIter), &(*routeRelocIter),&(*visitIter)) )
					{
						//Store the pre-insertion info
						routeToDurBefore = routeRelocIter->computeRouteDuration();
						routeDaysBefore = (int)ceil( (routeRelocIter->computeRouteDuration()+ 
							routeRelocIter->getInstAtPos(0)->getLayTime()
							+ routeRelocIter->getRouteMinSlack() )/24);
						routeSlackBefore = routeRelocIter->computeRouteSlack();
						schedCostBefore = computeSchedCost();
						//numRoutesBefore = Routes.size();

						//Do tentative insertion
						routeRelocIter->insertInstalVisit(1, *visitIter);
						routeRelocIter->updateVisitVector();
						routeRelocIter->intelligentReorder();
						routeToDurAfter = routeRelocIter->computeRouteDuration();

						if (routeToDurAfter < routeRelocIter->MAX_ROUTE_DUR + routeRelocIter->getRouteAcceptanceTime())
						{				
							routeDaysAfter = (int)ceil( (routeRelocIter->computeRouteDuration()+ 
							routeRelocIter->getInstAtPos(0)->getLayTime()
							+ routeRelocIter->getRouteMinSlack() )/24);

							daysDelta = routeDaysAfter - routeDaysBefore;

							for (j = 1; j <= daysDelta; j++)
							{
								if (!VesAvail[routeRelocIter->getRouteVes()->getID()]
								[routeEndIdx + j] )
								{
									insertionOverlaps = true;
									break;
								}

								if ( ( routeEndIdx + j > NUMBER_OF_DAYS) && 
									(!VesAvail[routeRelocIter->getRouteVes()->getID()]
								[(routeEndIdx+j)%NUMBER_OF_DAYS]) )
								{
									insertionOverlaps = true;
									break;
								}
							}

							if (!insertionOverlaps)
							{

								schedCostAfter = computeSchedCost();
								routeSlackAfter = routeRelocIter->computeRouteSlack();

								//Initialize VisitVariation objects
								VisitVariation visVar(&(*visitIter));
								visVar.setRouteTo(&(*routeRelocIter));
								visVar.setRouteFrom(&(*routeIter));
								visVar.setRouteToDurIncrease(routeToDurAfter - routeToDurBefore);
								visVar.setDeltaObj(schedCostAfter - schedCostBefore);
								visVar.setRouteDaysDelta(routeDaysAfter - routeDaysBefore);
								visVar.setRouteSlackDelta(routeSlackAfter - routeSlackBefore);

								visitIter->VisVars.push_back(visVar);
							
							//Use auxiliary objective???
							/*Auxiliary objective:
							- Schedule cost
							- Number of routes
							- Number of route days
							- Total slack
							*/
							}//end if
							insertionOverlaps = false;
						}//end if

						//Restore the affected route
						for (routeIntermedIter = intermedRoutes.begin(); 
							routeIntermedIter->getRouteNum() !=  routeRelocIter->getRouteNum();
							++routeIntermedIter)
							;
						*routeRelocIter = *routeIntermedIter;

					}//end if
			
				}//end for routeRelocIter

				//cout << "Aga" << endl;
				
				//Sort VisVars vector using insertion sort
				//Note - some may already be sorted from previous iterations
				if (visitIter->VisVars.size() > 0)
				{
					//cout << "visitIter->VisVars.size() = " << visitIter->VisVars.size()<< endl;
					for (j = 1; j < visitIter->VisVars.size(); j++)
					{
						exchVar = visitIter->VisVars[j]; 
						k = j;
						
						while ( ( (visitIter->VisVars[k-1].getDeltaObj() > exchVar.getDeltaObj())
							&& (visitIter->VisVars[k-1].getRouteTo()->getVisitObjects().size() > 2)
							&& (exchVar.getRouteTo()->getVisitObjects().size() > 2) )
							|| ( (visitIter->VisVars[k-1].getRouteTo()->getVisitObjects().size() == 2)
							&& (exchVar.getRouteTo()->getVisitObjects().size() > 2) )
							|| ( (visitIter->VisVars[k-1].getDeltaObj() > exchVar.getDeltaObj())
							&& (visitIter->VisVars[k-1].getRouteTo()->getVisitObjects().size() == 2)
							&& (exchVar.getRouteTo()->getVisitObjects().size() == 2) ) )
						{
							//cout << "k = " << k << endl;
							visitIter->VisVars[k] = visitIter->VisVars[k-1];
							k--;
							//cout << "Inside while" << endl;
							if (k == 0)
							{
								//cout << "before break" << endl;
								break;
							}
						}//end while
									
						//cout << "sorted" << endl;
						visitIter->VisVars[k] = exchVar;
						
					}//end for j

					//cout << "We are here" << endl;
					/*
					cout << "VisVars for " << visitIter->getOffshoreInst()->getInstName() << endl;
					for (j = 0; j < visitIter->VisVars.size(); j++)
					{
						cout << "\tRouteTo # of visits = " << visitIter->VisVars[j].getRouteTo()->getVisitObjects().size(); 
						
						cout << "  DeltaObj = " << visitIter->VisVars[j].getDeltaObj() << endl;
					}
					cout << endl;
					*/

					//Initialize regret vector
					if (visitIter->VisVars.size() == 1)
					{
						visitIter->RegretValues.clear();
						visitIter->RegretValues.resize(1);
						visitIter->RegretValues[0] = visitIter->VisVars[0].getDeltaObj();
					}
					else
					{
						visitIter->RegretValues.clear();
						visitIter->RegretValues.resize(visitIter->VisVars.size() - 1);
						for (j = 1; j < visitIter->VisVars.size(); j++)
						{
							visitIter->RegretValues[j-1] = visitIter->VisVars[j].getDeltaObj() - 
							visitIter->VisVars[j-1].getDeltaObj();
						}
					}
				}//end if (visitIter->VisVars.size() > 0)
			
			}//end for visitIter

			//cout << "Before Actual Insertion" << endl;
			//printWeeklySchedule();
			//cout << endl << endl;
			/*
				for (visitIter = removedVisits.begin(); visitIter != removedVisits.end(); ++visitIter)
				{
					cout << "Visit->installation = " << visitIter->getOffshoreInst()->getInstName() << endl;
					cout << "VisVars.size() = " << visitIter->VisVars.size() << endl;
					if (visitIter->VisVars.size() > 0)
						cout << "RegretValues[0] = " << visitIter->RegretValues[0] << endl;
				}
				*/


			smallestInsertionCost = numeric_limits<double>::max();
			largestRegret = 0;

			j = 1;
			bestIdxInsert = 0;
			bestIdxRegret = 0;

			//Identify the insertion
			for (visitIter = routeVisits.begin() + 1; visitIter != routeVisits.end() - 1; ++visitIter)
			{
				
				if ( (visitIter->VisVars.size() == 1) && 
					(visitIter->RegretValues[0] < smallestInsertionCost) )
				{
					
					smallestInsertionCost = visitIter->RegretValues[0];
					bestIdxInsert = j;
				}
				else if ( (visitIter->VisVars.size() > 1) && 
					(visitIter->RegretValues[0] > largestRegret) )
				{
					
					largestRegret = visitIter->RegretValues[0];
					bestIdxRegret = j;
				}

				j++;
			}//end for visitIter

			//cout << "And here" << endl;

			if ( (largestRegret == 0) && (smallestInsertionCost == 
				numeric_limits<double>::max()) )
				insertionPossible = false;

			else if ( smallestInsertionCost < 
				numeric_limits<double>::max() )
			{
				/*
				cout << "BEST INSERTION: visit = " << routeVisits[bestIdxInsert].VisVars[0].
					getVarVisit()->getOffshoreInst()->getInstName() << endl;

				cout << "Inserting to route " << routeVisits[bestIdxInsert].VisVars[0].
					getRouteTo()->getRouteNum() << endl;

				cout << "Insertion cost = " << routeVisits[bestIdxInsert].VisVars[0].
					getVarVisit()->RegretValues[0] << endl;
					*/
			
				routeDays = (int)ceil( (routeVisits[bestIdxInsert].VisVars[0].getRouteTo()->computeRouteDuration()+ 
				routeVisits[bestIdxInsert].VisVars[0].getRouteTo()->getInstAtPos(0)->getLayTime()
				+ routeVisits[bestIdxInsert].VisVars[0].getRouteTo()->getRouteMinSlack() )/24);

				routeStartIdx = (int) ceil(routeVisits[bestIdxInsert].VisVars[0].getRouteTo()->getRouteStartTime()/24);
				routeEndIdx = routeStartIdx + routeDays - 1;

				//Update VesAvail
				if (routeVisits[bestIdxInsert].VisVars[0].getRouteDaysDelta() > 0)
				{
					for (j = 1; j <=routeVisits[bestIdxInsert].VisVars[0].getRouteDaysDelta(); j++)
						VesAvail[routeVisits[bestIdxInsert].VisVars[0].getRouteTo()->getRouteVes()->getID()]
								[routeEndIdx + j] = false;

				}

				routeVisits[bestIdxInsert].VisVars[0].getRouteTo()->insertInstalVisit(1, routeVisits[bestIdxInsert]);
				routeVisits[bestIdxInsert].VisVars[0].getRouteTo()->updateVisitVector();
				routeVisits[bestIdxInsert].VisVars[0].getRouteTo()->intelligentReorder();

				//Update visit day combination
				updateVisDayComb(routeVisits[bestIdxInsert].VisVars[0].getRouteFrom(),
					routeVisits[bestIdxInsert].VisVars[0].getRouteTo(), 
					routeVisits[bestIdxInsert].VisVars[0].getVarVisit() );

				
				//Update intermedRoutes
				for (routeIntermedIter = intermedRoutes.begin(); 
							routeIntermedIter->getRouteNum() != 
							routeVisits[bestIdxInsert].VisVars[0].getRouteTo()->getRouteNum();
							++routeIntermedIter)
							;
				*routeIntermedIter = *(routeVisits[bestIdxInsert].VisVars[0].getRouteTo());

				//Erase the visit from routeVisits
				
				routeIter->deleteInstalVisit(bestIdxInsert);
				routeIter->updateVisitVector();
				routeIter->intelligentReorder();

				//printWeeklySchedule();
				//cout << endl << endl;
				
			}//end else if

			else if (largestRegret > 0)
			{

				/*
				cout << "BEST REGRET INSERTION: visit = " << routeVisits[bestIdxRegret].VisVars[0].
					getVarVisit()->getOffshoreInst()->getInstName() << endl;

				cout << "Inserting to route " << routeVisits[bestIdxRegret].VisVars[0].
					getRouteTo()->getRouteNum() << endl;

				cout << "REGRET VALUE = " << routeVisits[bestIdxRegret].VisVars[0].
					getVarVisit()->RegretValues[0] << endl;
				*/	

				routeDays = (int)ceil( (routeVisits[bestIdxRegret].VisVars[0].getRouteTo()->computeRouteDuration()+ 
				routeVisits[bestIdxRegret].VisVars[0].getRouteTo()->getInstAtPos(0)->getLayTime()
				+ routeVisits[bestIdxRegret].VisVars[0].getRouteTo()->getRouteMinSlack() )/24);

				routeStartIdx = (int) ceil(routeVisits[bestIdxRegret].VisVars[0].getRouteTo()->getRouteStartTime()/24);
				routeEndIdx = routeStartIdx + routeDays - 1;

				//Update VesAvail before insertion
				if (routeVisits[bestIdxRegret].VisVars[0].getRouteDaysDelta() > 0)
				{
					for (j = 1; j <=routeVisits[bestIdxRegret].VisVars[0].getRouteDaysDelta(); j++)
						VesAvail[routeVisits[bestIdxRegret].VisVars[0].getRouteTo()->getRouteVes()->getID()]
								[routeEndIdx + j] = false;

				}

				routeVisits[bestIdxRegret].VisVars[0].getRouteTo()->insertInstalVisit(1, routeVisits[bestIdxRegret]);
				routeVisits[bestIdxRegret].VisVars[0].getRouteTo()->updateVisitVector();
				routeVisits[bestIdxRegret].VisVars[0].getRouteTo()->intelligentReorder();

				//Update visit day combination
				updateVisDayComb(routeVisits[bestIdxRegret].VisVars[0].getRouteFrom(),
					routeVisits[bestIdxRegret].VisVars[0].getRouteTo(), 
					routeVisits[bestIdxRegret].VisVars[0].getVarVisit() );

				//Update intermedRoutes
				for (routeIntermedIter = intermedRoutes.begin(); 
							routeIntermedIter->getRouteNum() != 
							routeVisits[bestIdxRegret].VisVars[0].getRouteTo()->getRouteNum();
							++routeIntermedIter)
							;
				*routeIntermedIter = *(routeVisits[bestIdxRegret].VisVars[0].getRouteTo());

				//Erase the visit from removedVisits
				routeIter->deleteInstalVisit(bestIdxRegret);
				routeIter->updateVisitVector();
				routeIter->intelligentReorder();
			}
			else
			{
				cout << "ERROR IN PERFORMING INSERTION - ReduceNumberOfRoutes()!!!" << endl;
			}

			//cout << "Olli Jokkinen" << endl;

		} while (insertionPossible);


		//store the cost reduction if we removed all visits
		if (routeIter->getVisitObjects().size() == 2)
		{
			routeRemovedNumber.push_back(routeIter->getRouteNum());
			costAfterRemove = computeSchedCost();

			if (routeIter->getRouteVes()->getVesselRoutes().size() == 1)
				costAfterRemove -= routeIter->getRouteVes()->getVesselCost();

			routeRemovalCost.push_back(costAfterRemove - costBeforeRemove);
		}

		//Restore the previous solution:
		//Restore the Routes vector
		for (l = 0; l < Routes.size(); l++)
		{
			Routes[l] = savedRoutes[l];
			intermedRoutes[l] = savedRoutes[l];
		}

		//cout << "Number of Visits = " << routeIter->getVisitObjects().size()-1 << endl;

		//Restore visit day combinations
		for (l = 1; l < routeRelocVisits.size()-1; l++)
		{
			//cout << "Inst " << routeRelocVisits[l].getOffshoreInst()->getInstName() << ": ";
			routeRelocVisits[l].getOffshoreInst()->CurVisitDayComb.clear();
			for (m = 0; m < routeRelocVisits[l].getOffshoreInst()->getVisitFreq(); m++)
			{
				routeRelocVisits[l].getOffshoreInst()->CurVisitDayComb.push_back(VisCombs[l][m]);
				//cout << VisCombs[l][m] << "\t";
			}
		}	

		// Restore VesAvail
		for (k = 0; k < VesAvail.size(); k++)
			for (i = 0; i < planHorizon; i++)
				VesAvail[k][i] = vesAvailability[k][i];

	}//end for routeIter

	if (routeRemovalCost.size() == 0)// no route can be removed
	{
		improvementFound = false;
		//cout << "No route can be removed" << endl;
	}

	else //there were routes which could be removed
	{
		bestCostRouteRemove = numeric_limits<double>::max();

		//Identify the best route to Remove
		for (l = 0; l < routeRemovalCost.size(); l++)
		{
			if (routeRemovalCost[l] < bestCostRouteRemove)
			{
				bestIdxRouteRemove = l;
				bestCostRouteRemove = routeRemovalCost[l];
			}
			
			//cout << "ROUTE " << routeRemovedNumber[l] << endl;
			//cout << "REMOVAL COST = " << routeRemovalCost[l] << endl << endl;
			//Routes[routeRemovedNumber[l]].printRoute();
		}

		//Remove all visits from the chosen route/////////////////////////
		for (routeIter = Routes.begin(); routeIter != Routes.end(); ++routeIter)
			if (routeIter->getRouteNum() == Routes[routeRemovedNumber[bestIdxRouteRemove]].getRouteNum() )
				break;

		//cout << "Removing the route number " << routeIter->getRouteNum() << endl;
		//routeIter->printRoute();

		//Update VesAvail and vesAvailability
		routeStartIndex = (int) ceil(routeIter->getRouteStartTime()/24);
		routeDays = (int) ceil ( (routeIter->computeRouteDuration() +
			routeIter->getInstAtPos(0)->getLayTime() + 
			routeIter->getRouteMinSlack() )/24);
		routeEndIndex = routeStartIndex + routeDays - 1;
		for (l = routeStartIndex; l <= routeEndIndex; l++)
		{
			VesAvail[routeIter->getRouteVes()->getID()][l]=true;
			vesAvailability[routeIter->getRouteVes()->getID()][l] = true;
		}

		do 
		{				
			routeVisits.clear();
			routeVisits = routeIter->getVisitObjects();
			insertionPossible = true;

			for (visitIter = routeVisits.begin()+1; visitIter != routeVisits.end()-1; ++visitIter)
			{
				visitIter->VisVars.clear();
				visitIter->RegretValues.clear();
				VisitVariation exchVar(&(*visitIter));

				//cout << "Let's see" << endl;

				//Evaluate and sort all possible insertions for a given visit
				for (routeRelocIter = Routes.begin(); routeRelocIter != Routes.end(); ++routeRelocIter)
				{
					routeDays = (int)ceil( (routeRelocIter->computeRouteDuration()+ 
					routeRelocIter->getInstAtPos(0)->getLayTime()
					+ routeRelocIter->getRouteMinSlack() )/24);

					routeStartIdx = (int) ceil(routeRelocIter->getRouteStartTime()/24);
					routeEndIdx = routeStartIdx + routeDays - 1;

					if ( (routeRelocIter->computeRouteDemand() + 
						visitIter->getOffshoreInst()->getWeeklyDemand()/visitIter->getOffshoreInst()->getVisitFreq()
						< routeRelocIter->getRouteVes()->getCapacity()) &&  
						(routeRelocIter->getVisitObjects().size() - 2 < routeRelocIter->getMaxVisits() ) &&
						!routeRelocIter->isInstOnRoute(visitIter->getOffshoreInst()) &&
						isDepartureSpreadEven(&(*routeIter), &(*routeRelocIter),&(*visitIter)) )
					{
						//Store the pre-insertion info
						routeToDurBefore = routeRelocIter->computeRouteDuration();
						routeDaysBefore = (int)ceil( (routeRelocIter->computeRouteDuration()+ 
							routeRelocIter->getInstAtPos(0)->getLayTime()
							+ routeRelocIter->getRouteMinSlack() )/24);
						routeSlackBefore = routeRelocIter->computeRouteSlack();
						schedCostBefore = computeSchedCost();
						//numRoutesBefore = Routes.size();

						//Do tentative insertion
						routeRelocIter->insertInstalVisit(1, *visitIter);
						routeRelocIter->updateVisitVector();
						routeRelocIter->intelligentReorder();
						routeToDurAfter = routeRelocIter->computeRouteDuration();

						if (routeToDurAfter < routeRelocIter->MAX_ROUTE_DUR + routeRelocIter->getRouteAcceptanceTime())
						{				
							routeDaysAfter = (int)ceil( (routeRelocIter->computeRouteDuration()+ 
							routeRelocIter->getInstAtPos(0)->getLayTime()
							+ routeRelocIter->getRouteMinSlack() )/24);

							daysDelta = routeDaysAfter - routeDaysBefore;

							for (j = 1; j <= daysDelta; j++)
							{
								if (!VesAvail[routeRelocIter->getRouteVes()->getID()]
								[routeEndIdx + j] )
								{
									insertionOverlaps = true;
									break;
								}

								if ( ( routeEndIdx + j > NUMBER_OF_DAYS) && 
									(!VesAvail[routeRelocIter->getRouteVes()->getID()]
								[(routeEndIdx+j)%NUMBER_OF_DAYS]) )
								{
									insertionOverlaps = true;
									break;
								}
							}

							if (!insertionOverlaps)
							{

								schedCostAfter = computeSchedCost();
								routeSlackAfter = routeRelocIter->computeRouteSlack();

								//Initialize VisitVariation objects
								VisitVariation visVar(&(*visitIter));
								visVar.setRouteTo(&(*routeRelocIter));
								visVar.setRouteFrom(&(*routeIter));
								visVar.setRouteToDurIncrease(routeToDurAfter - routeToDurBefore);
								visVar.setDeltaObj(schedCostAfter - schedCostBefore);
								visVar.setRouteDaysDelta(routeDaysAfter - routeDaysBefore);
								visVar.setRouteSlackDelta(routeSlackAfter - routeSlackBefore);

								visitIter->VisVars.push_back(visVar);
							
							//Use auxiliary objective???
							/*Auxiliary objective:
							- Schedule cost
							- Number of routes
							- Number of route days
							- Total slack
							*/
							}//end if
							insertionOverlaps = false;
						}//end if

						//Restore the affected route
						for (routeIntermedIter = intermedRoutes.begin(); 
							routeIntermedIter->getRouteNum() !=  routeRelocIter->getRouteNum();
							++routeIntermedIter)
							;
						*routeRelocIter = *routeIntermedIter;

					}//end if
			
				}//end for routeRelocIter

				//cout << "Aga" << endl;
				
				//Sort VisVars vector using insertion sort
				//Note - some may already be sorted from previous iterations
				if (visitIter->VisVars.size() > 0)
				{
					//cout << "visitIter->VisVars.size() = " << visitIter->VisVars.size()<< endl;
					for (j = 1; j < visitIter->VisVars.size(); j++)
					{
						exchVar = visitIter->VisVars[j]; 
						k = j;
						
						while ( ( (visitIter->VisVars[k-1].getDeltaObj() > exchVar.getDeltaObj())
							&& (visitIter->VisVars[k-1].getRouteTo()->getVisitObjects().size() > 2)
							&& (exchVar.getRouteTo()->getVisitObjects().size() > 2) )
							|| ( (visitIter->VisVars[k-1].getRouteTo()->getVisitObjects().size() == 2)
							&& (exchVar.getRouteTo()->getVisitObjects().size() > 2) )
							|| ( (visitIter->VisVars[k-1].getDeltaObj() > exchVar.getDeltaObj())
							&& (visitIter->VisVars[k-1].getRouteTo()->getVisitObjects().size() == 2)
							&& (exchVar.getRouteTo()->getVisitObjects().size() == 2) ) )
						{
							//cout << "k = " << k << endl;
							visitIter->VisVars[k] = visitIter->VisVars[k-1];
							k--;
							//cout << "Inside while" << endl;
							if (k == 0)
							{
								//cout << "before break" << endl;
								break;
							}
						}//end while
									
						//cout << "sorted" << endl;
						visitIter->VisVars[k] = exchVar;
						
					}//end for j

					//cout << "We are here" << endl;
					/*
					cout << "VisVars for " << visitIter->getOffshoreInst()->getInstName() << endl;
					for (j = 0; j < visitIter->VisVars.size(); j++)
					{
						cout << "\tRouteTo # of visits = " << visitIter->VisVars[j].getRouteTo()->getVisitObjects().size(); 
						
						cout << "  DeltaObj = " << visitIter->VisVars[j].getDeltaObj() << endl;
					}
					cout << endl;
					*/

					//Initialize regret vector
					if (visitIter->VisVars.size() == 1)
					{
						visitIter->RegretValues.clear();
						visitIter->RegretValues.resize(1);
						visitIter->RegretValues[0] = visitIter->VisVars[0].getDeltaObj();
					}
					else
					{
						visitIter->RegretValues.clear();
						visitIter->RegretValues.resize(visitIter->VisVars.size() - 1);
						for (j = 1; j < visitIter->VisVars.size(); j++)
						{
							visitIter->RegretValues[j-1] = visitIter->VisVars[j].getDeltaObj() - 
							visitIter->VisVars[j-1].getDeltaObj();
						}
					}
				}//end if (visitIter->VisVars.size() > 0)
			
			}//end for visitIter

			//cout << "Before Actual Insertion" << endl;
			//printWeeklySchedule();
			//cout << endl << endl;
			/*
				for (visitIter = removedVisits.begin(); visitIter != removedVisits.end(); ++visitIter)
				{
					cout << "Visit->installation = " << visitIter->getOffshoreInst()->getInstName() << endl;
					cout << "VisVars.size() = " << visitIter->VisVars.size() << endl;
					if (visitIter->VisVars.size() > 0)
						cout << "RegretValues[0] = " << visitIter->RegretValues[0] << endl;
				}
				*/


			smallestInsertionCost = numeric_limits<double>::max();
			largestRegret = 0;

			j = 1;
			bestIdxInsert = 0;
			bestIdxRegret = 0;

			//Identify the insertion
			for (visitIter = routeVisits.begin() + 1; visitIter != routeVisits.end() - 1; ++visitIter)
			{
				
				if ( (visitIter->VisVars.size() == 1) && 
					(visitIter->RegretValues[0] < smallestInsertionCost) )
				{
					
					smallestInsertionCost = visitIter->RegretValues[0];
					bestIdxInsert = j;
				}
				else if ( (visitIter->VisVars.size() > 1) && 
					(visitIter->RegretValues[0] > largestRegret) )
				{
					
					largestRegret = visitIter->RegretValues[0];
					bestIdxRegret = j;
				}

				j++;
			}//end for visitIter

			//cout << "And here" << endl;

			if ( (largestRegret == 0) && (smallestInsertionCost == 
				numeric_limits<double>::max()) )
				insertionPossible = false;

			else if ( smallestInsertionCost < 
				numeric_limits<double>::max() )
			{
				/*
				cout << "BEST INSERTION: visit = " << routeVisits[bestIdxInsert].VisVars[0].
					getVarVisit()->getOffshoreInst()->getInstName() << endl;

				cout << "Inserting to route " << routeVisits[bestIdxInsert].VisVars[0].
					getRouteTo()->getRouteNum() << endl;

				cout << "Insertion cost = " << routeVisits[bestIdxInsert].VisVars[0].
					getVarVisit()->RegretValues[0] << endl;
					*/
			
				routeDays = (int)ceil( (routeVisits[bestIdxInsert].VisVars[0].getRouteTo()->computeRouteDuration()+ 
				routeVisits[bestIdxInsert].VisVars[0].getRouteTo()->getInstAtPos(0)->getLayTime()
				+ routeVisits[bestIdxInsert].VisVars[0].getRouteTo()->getRouteMinSlack() )/24);

				routeStartIdx = (int) ceil(routeVisits[bestIdxInsert].VisVars[0].getRouteTo()->getRouteStartTime()/24);
				routeEndIdx = routeStartIdx + routeDays - 1;

				//Update VesAvail
				if (routeVisits[bestIdxInsert].VisVars[0].getRouteDaysDelta() > 0)
				{
					for (j = 1; j <=routeVisits[bestIdxInsert].VisVars[0].getRouteDaysDelta(); j++)
					{
						VesAvail[routeVisits[bestIdxInsert].VisVars[0].getRouteTo()->getRouteVes()->getID()]
								[routeEndIdx + j] = false;
						vesAvailability[routeVisits[bestIdxInsert].VisVars[0].getRouteTo()->getRouteVes()->getID()]
								[routeEndIdx + j] = false;
					}


				}

				routeVisits[bestIdxInsert].VisVars[0].getRouteTo()->insertInstalVisit(1, routeVisits[bestIdxInsert]);
				routeVisits[bestIdxInsert].VisVars[0].getRouteTo()->updateVisitVector();
				routeVisits[bestIdxInsert].VisVars[0].getRouteTo()->intelligentReorder();

				//Update visit day combination
				updateVisDayComb(routeVisits[bestIdxInsert].VisVars[0].getRouteFrom(),
					routeVisits[bestIdxInsert].VisVars[0].getRouteTo(), 
					routeVisits[bestIdxInsert].VisVars[0].getVarVisit() );

				
				//Update intermedRoutes
				for (routeIntermedIter = intermedRoutes.begin(); 
							routeIntermedIter->getRouteNum() != 
							routeVisits[bestIdxInsert].VisVars[0].getRouteTo()->getRouteNum();
							++routeIntermedIter)
							;
				*routeIntermedIter = *(routeVisits[bestIdxInsert].VisVars[0].getRouteTo());

				//Erase the visit from routeVisits
				
				//cout << "before 1" << endl;
				routeIter->deleteInstalVisit(bestIdxInsert);
				//cout << "after 1" << endl;
				routeIter->updateVisitVector();
				routeIter->intelligentReorder();

				//printWeeklySchedule();
				//cout << endl << endl;
				
			}//end else if

			else if (largestRegret > 0)
			{

				/*
				cout << "BEST REGRET INSERTION: visit = " << routeVisits[bestIdxRegret].VisVars[0].
					getVarVisit()->getOffshoreInst()->getInstName() << endl;

				cout << "Inserting to route " << routeVisits[bestIdxRegret].VisVars[0].
					getRouteTo()->getRouteNum() << endl;

				cout << "REGRET VALUE = " << routeVisits[bestIdxRegret].VisVars[0].
					getVarVisit()->RegretValues[0] << endl;
				*/	

				routeDays = (int)ceil( (routeVisits[bestIdxRegret].VisVars[0].getRouteTo()->computeRouteDuration()+ 
				routeVisits[bestIdxRegret].VisVars[0].getRouteTo()->getInstAtPos(0)->getLayTime()
				+ routeVisits[bestIdxRegret].VisVars[0].getRouteTo()->getRouteMinSlack() )/24);

				routeStartIdx = (int) ceil(routeVisits[bestIdxRegret].VisVars[0].getRouteTo()->getRouteStartTime()/24);
				routeEndIdx = routeStartIdx + routeDays - 1;

				//Update VesAvail before insertion
				if (routeVisits[bestIdxRegret].VisVars[0].getRouteDaysDelta() > 0)
				{
					for (j = 1; j <=routeVisits[bestIdxRegret].VisVars[0].getRouteDaysDelta(); j++)
					{
						VesAvail[routeVisits[bestIdxRegret].VisVars[0].getRouteTo()->getRouteVes()->getID()]
								[routeEndIdx + j] = false;
						vesAvailability[routeVisits[bestIdxRegret].VisVars[0].getRouteTo()->getRouteVes()->getID()]
									[routeEndIdx + j] = false;
					}

				}

				routeVisits[bestIdxRegret].VisVars[0].getRouteTo()->insertInstalVisit(1, routeVisits[bestIdxRegret]);
				routeVisits[bestIdxRegret].VisVars[0].getRouteTo()->updateVisitVector();
				routeVisits[bestIdxRegret].VisVars[0].getRouteTo()->intelligentReorder();

				//Update visit day combination
				updateVisDayComb(routeVisits[bestIdxRegret].VisVars[0].getRouteFrom(),
					routeVisits[bestIdxRegret].VisVars[0].getRouteTo(), 
					routeVisits[bestIdxRegret].VisVars[0].getVarVisit() );

				//Update intermedRoutes
				for (routeIntermedIter = intermedRoutes.begin(); 
							routeIntermedIter->getRouteNum() != 
							routeVisits[bestIdxRegret].VisVars[0].getRouteTo()->getRouteNum();
							++routeIntermedIter)
							;
				*routeIntermedIter = *(routeVisits[bestIdxRegret].VisVars[0].getRouteTo());

				//Erase the visit from removedVisits
				//cout << "before 2" << endl;
				routeIter->deleteInstalVisit(bestIdxRegret);
				//cout << "after 2" << endl;
				routeIter->updateVisitVector();
				routeIter->intelligentReorder();
			}
			else
			{
				cout << "ERROR IN PERFORMING INSERTION - ReduceNumberOfRoutes()!!!" << endl;
			}

			//cout << "Olli Jokkinen" << endl;

		} while (insertionPossible);

		//All visits removed - update the solution
		k = 0;
		for (routeIter = Routes.begin(); routeIter != Routes.end(); ++routeIter)
		{
				if (routeIter->getRouteNum() == Routes[routeRemovedNumber[bestIdxRouteRemove]].getRouteNum() )
					break;
				k++;
		}
		
		//cout << "before 3" << endl;
		Routes.erase(Routes.begin() + k);
		//cout << "after 3" << endl;

		intermedRoutes.clear();
		intermedRoutes.resize(Routes.size());
		savedRoutes.clear();
		savedRoutes.resize(Routes.size());

		setOnlyRoutes();

		for (l = 0; l < Routes.size(); l++) 
		{
			Routes[l].setRouteNum(l);
			intermedRoutes[l] = Routes[l];
			savedRoutes[l] = Routes[l];
		}

		//For each vessel clear and update vesselRoutes vector
		for (vesIter = SchedVessels.begin(); vesIter != SchedVessels.end(); ++vesIter)
		{
			vesIter->clearVesselRoutes();
			for (routeIter = Routes.begin(); 
				routeIter != Routes.end(); ++routeIter)
				if (vesIter->getID() == routeIter->getRouteVes()->getID() )
					vesIter->addVesselRoute(&(*routeIter));
			
			if (vesIter->getVesselRoutes().size() > 0)
				vesIter->setIsVesselUsed(true);
			else
				vesIter->setIsVesselUsed(false);
		}//end for
		
		improvementFound = true;
		//cout << "After removing the Route" << endl;
		//printWeeklySchedule();

	}//end else

	//Stop if we reached LB in terms of number of routes
	if ( Routes.size() == LB )
		break; //break out of while loop
}//end while (improvementFound)
	
//cout << "reduceNumberOfRoutes_END" << endl;

//cout << "After reducing the number of routes" << endl;
//cout << endl;
//printWeeklySchedule();

}

void WeeklySchedule::reduceRouteDurTotDays()
{
	//cout << "reduceRouteDurTotDays_BEGIN" << endl;
	//Redistributing a route visits between other routes
	//Making sure they are still evenly spread

	vector <Route>::iterator routeIter;
	vector <Route>::iterator routeRelocIter;
	vector <Route>::iterator routeIntermedIter;

	vector <Visit> routeVisits;
	vector <Visit> routeRelocVisits;
	vector <Visit>::iterator visitIter;

	vector <SupVes>::iterator vesIter;

	int i = 0, j = 1, k = 0, l, m;
	int routeStartIndex, routeEndIndex;

	vector <Route> savedRoutes; //additional vector of routes
	vector <Route> intermedRoutes;
	savedRoutes.resize(Routes.size());
	intermedRoutes.resize(Routes.size());

	vector < vector <int> > VisCombs;

	int LB = computeLB_numRoutes();

	for (l = 0; l < Routes.size(); l++)
	{
		savedRoutes[l] = Routes[l];
		intermedRoutes[l] = Routes[l];
	}

	double routeFromDurBefore;
	double routeToDurBefore, routeToDurAfter;
	double schedCostBefore, schedCostAfter;
	double routeSlackBefore, routeSlackAfter;

	int bestVarIdx;
	int routeDaysBefore, routeDaysAfter, daysDelta;
	int bestIdxInsert;
	double bestCostRouteReduce;
	int bestIdxRouteReduce, bestNumOnes;

	int routeStartIdx, routeEndIdx;
	int routeDays;

	bool insertionOverlaps = false;
	bool improvementFound = true;
	bool insertionPossible = true;

	vector <vector <bool> > vesAvailability; //local vessel availability vector

	vesAvailability.resize(VesAvail.size());

	int planHorizon = NUMBER_OF_DAYS + (int) ceil(
			( (double)Routes.begin()->MAX_ROUTE_DUR 
			+ Routes.begin()->getInstAtPos(0)->getLayTime() 
			+ Routes.begin()->getRouteMinSlack() )/24);

	for (k = 0; k < VesAvail.size(); k++)
		{
			vesAvailability[k].resize(planHorizon);
			for (i = 0; i < planHorizon; i++)
				vesAvailability[k][i] = VesAvail[k][i];
		}

	vector <double> routeReductionCost;
	vector <int> routeReducedNumber;
	double costAfterRemove, costBeforeRemove;

	//cout << "-----------Entering reduceRouteDurTotDays()----------" << endl;
	//cout << "Routes.size() = " << Routes.size() << endl << endl;
	//printWeeklySchedule();
	//Set visit Ids
	for (routeIter = Routes.begin(); routeIter != Routes.end(); ++routeIter)
		routeIter->setVisitIds();

	//printWeeklySchedule();

	bool endOfWeekReached;
	vector <int> numOnes;

	for (routeIter = Routes.begin(); routeIter != Routes.end(); ++routeIter)
	{//Calculate number of ones in VesAvail after every Route
		routeEndIndex = (int) ceil( (routeIter->getRouteEndTime() + 
			routeIter->getRouteMinSlack() - routeIter->getInstAtPos(0)->getLayTime())/24 );
		j = routeEndIndex + 1;
		k = 0;
		endOfWeekReached = false;

		if (j > NUMBER_OF_DAYS)
			endOfWeekReached = true;

		//cout << "j = " << j << endl;

		while ( (VesAvail[routeIter->getRouteVes()->getID()][j]) &&
			(!endOfWeekReached) )
		{
			k++;
			j++;
			if (j > NUMBER_OF_DAYS)
				endOfWeekReached = true;
		}

		//cout << "where" << endl;

		if (endOfWeekReached)
		{
			//If we reached the end of the week, start from the beginning again
			while (VesAvail[routeIter->getRouteVes()->getID()][j%NUMBER_OF_DAYS])
			{
				k++;
				j++;
			}
		}

		routeIter->setNumOnesAfterRoute(k);
		numOnes.push_back(k);
		//cout << "Route Number: " << routeIter->getRouteNum();
		//cout << " - number of ones after: " << routeIter->getNumOnesAfterRoute() << endl;
	}//end for routeIter
	
	int maxRouteDays = (int) ceil( ((double)Routes[0].MAX_ROUTE_DUR 
		+ Routes[0].getInstAtPos(0)->getLayTime())/24);

	//vector <VisitVariation>::iterator visVarIter;
	vector <VisitVariation> VisVariations;
	
	double routeFromDaysBefore, routeFromDaysAfter;
	double bestDeltaObj, bestRouDurDelta; 
	int bestRouteDaysDelta, bestVisVarSize, bestNumOnesAfterRoute;

while (improvementFound)
{
	routeReductionCost.clear();
	routeReducedNumber.clear();
	improvementFound = false;

	for (routeIter = Routes.begin(); routeIter != Routes.end(); ++routeIter)
	{
		routeFromDaysBefore = (int) ceil( (routeIter->computeRouteDuration() + routeIter->getRouteMinSlack()
			+ routeIter->getInstAtPos(0)->getLayTime() )/24);
		routeFromDaysAfter = routeFromDaysBefore;

		if ( routeFromDaysBefore == maxRouteDays )
		{
			costBeforeRemove = computeSchedCost();
			insertionPossible = true;
			//cout << "Route number " << routeIter->getRouteNum() << endl;
			//cout << "routeFromDaysBefore = " << routeFromDaysBefore << endl;

			routeRelocVisits.clear();
			routeRelocVisits = routeIter->getVisitObjects();

			VisCombs.clear();
			VisCombs.resize(routeIter->getVisitObjects().size());

			//Save the visit day combinations of installations on the route
			for (l = 1; l < routeIter->getVisitObjects().size()-1; l++)
				for (m = 0; m < routeRelocVisits[l].getOffshoreInst()->getVisitFreq(); m++)
					VisCombs[l].push_back(routeRelocVisits[l].getOffshoreInst()->CurVisitDayComb[m]);	

			do 
			{
				routeVisits.clear();
				routeVisits = routeIter->getVisitObjects();
				
				for (visitIter = routeVisits.begin()+1; visitIter != routeVisits.end()-1; ++visitIter)
				{
					visitIter->VisVars.clear();
					visitIter->RegretValues.clear();
					VisitVariation exchVar(&(*visitIter));

					//cout << "Let's see" << endl;

					//Evaluate and sort all possible insertions for a given visit
					for (routeRelocIter = Routes.begin(); routeRelocIter != Routes.end(); ++routeRelocIter)
					{
						routeDays = (int)ceil( (routeRelocIter->computeRouteDuration()+ 
						routeRelocIter->getInstAtPos(0)->getLayTime()
						+ routeRelocIter->getRouteMinSlack() )/24);

						routeStartIdx = (int) ceil(routeRelocIter->getRouteStartTime()/24);
						routeEndIdx = routeStartIdx + routeDays - 1;

						if ( (routeRelocIter->computeRouteDemand() + 
							visitIter->getOffshoreInst()->getWeeklyDemand()/visitIter->getOffshoreInst()->getVisitFreq()
							< routeRelocIter->getRouteVes()->getCapacity()) &&  
							(routeRelocIter->getVisitObjects().size() - 2 < routeRelocIter->getMaxVisits() ) &&
							!routeRelocIter->isInstOnRoute(visitIter->getOffshoreInst()) &&
							isDepartureSpreadEven(&(*routeIter), &(*routeRelocIter),&(*visitIter)) )
						{
							//Store the pre-insertion info
							routeToDurBefore = routeRelocIter->computeRouteDuration();
							routeDaysBefore = (int)ceil( (routeRelocIter->computeRouteDuration()+ 
								routeRelocIter->getInstAtPos(0)->getLayTime()
								+ routeRelocIter->getRouteMinSlack() )/24);
							routeSlackBefore = routeRelocIter->computeRouteSlack();
							schedCostBefore = computeSchedCost();
							//numRoutesBefore = Routes.size();

							//Do tentative insertion
							routeRelocIter->insertInstalVisit(1, *visitIter);
							routeRelocIter->updateVisitVector();
							routeRelocIter->intelligentReorder();
							routeToDurAfter = routeRelocIter->computeRouteDuration();

							if (routeToDurAfter < routeRelocIter->MAX_ROUTE_DUR + routeRelocIter->getRouteAcceptanceTime())
							{				
								routeDaysAfter = (int)ceil( (routeRelocIter->computeRouteDuration()+ 
								routeRelocIter->getInstAtPos(0)->getLayTime()
								+ routeRelocIter->getRouteMinSlack() )/24);

								daysDelta = routeDaysAfter - routeDaysBefore;

								/*
								for (j = 1; j <= daysDelta; j++)
								{
									if (!VesAvail[routeRelocIter->getRouteVes()->getID()]
									[routeEndIdx + j] )
									{
										insertionOverlaps = true;
										break;
									}

									if ( ( routeEndIdx + j > NUMBER_OF_DAYS) && 
										(!VesAvail[routeRelocIter->getRouteVes()->getID()]
									[(routeEndIdx+j)%NUMBER_OF_DAYS]) )
									{
										insertionOverlaps = true;
										break;
									}
								}
								*/
								if ( (daysDelta == 0) || 
									( (daysDelta == 1) && (routeRelocIter->getNumOnesAfterRoute() >= 1)
									&& (routeRelocIter->getNumOnesAfterRoute() <= 2) ) )
								{

									schedCostAfter = computeSchedCost();
									routeSlackAfter = routeRelocIter->computeRouteSlack();

									//Initialize VisitVariation objects
									VisitVariation visVar(&(*visitIter));
									visVar.setRouteTo(&(*routeRelocIter));
									visVar.setRouteFrom(&(*routeIter));
									visVar.setRouteToDurIncrease(routeToDurAfter - routeToDurBefore);
									visVar.setDeltaObj(schedCostAfter - schedCostBefore);
									visVar.setRouteDaysDelta(routeDaysAfter - routeDaysBefore);
									visVar.setRouteSlackDelta(routeSlackAfter - routeSlackBefore);
									visVar.setRouteDurationDelta(visVar.getRouteToDurIncrease() - 
										visVar.getRouteFromDurDecrease());
									visVar.setVisitIndex(visitIter->getIdNumber());

									visitIter->VisVars.push_back(visVar);
								
								//Use auxiliary objective???
								/*Auxiliary objective:
								- Schedule cost
								- Number of routes
								- Number of route days
								- Total slack
								*/
								}//end if

								//insertionOverlaps = false;
							}//end if

							//Restore the affected route
							for (routeIntermedIter = intermedRoutes.begin(); 
								routeIntermedIter->getRouteNum() !=  routeRelocIter->getRouteNum();
								++routeIntermedIter)
								;
							*routeRelocIter = *routeIntermedIter;

						}//end if
				
					}//end for routeRelocIter

					//cout << "Aga" << endl;
					
					//Sort VisVars vector using insertion sort
					//Note - some may already be sorted from previous iterations
					if (visitIter->VisVars.size() > 1)
					{
						//cout << "visitIter->VisVars.size() = " << visitIter->VisVars.size()<< endl;
						for (j = 1; j < visitIter->VisVars.size(); j++)
						{
							exchVar = visitIter->VisVars[j]; 
							k = j;

							//Multi-criteria sorting of variations
							while ( (visitIter->VisVars[k-1].getRouteDaysDelta() > exchVar.getRouteDaysDelta())
								|| ( (visitIter->VisVars[k-1].getRouteDaysDelta() == exchVar.getRouteDaysDelta())
								&& (visitIter->VisVars[k-1].getRouteDurationDelta() > exchVar.getRouteDurationDelta() )
								&& (exchVar.getRouteDaysDelta() == 0) )
								|| ( (visitIter->VisVars[k-1].getRouteDaysDelta() == exchVar.getRouteDaysDelta()) 
								&& (exchVar.getRouteDaysDelta() == 1) 
								&& (visitIter->VisVars[k-1].getRouteTo()->getNumOnesAfterRoute() > exchVar.getRouteTo()->getNumOnesAfterRoute()) )
								|| ( (visitIter->VisVars[k-1].getDeltaObj() > exchVar.getDeltaObj())
								&& (visitIter->VisVars[k-1].getRouteDaysDelta() == exchVar.getRouteDaysDelta())
								&& (visitIter->VisVars[k-1].getRouteDurationDelta() == exchVar.getRouteDurationDelta())
								&& (exchVar.getRouteDaysDelta() == 0) ) 
								|| ( (visitIter->VisVars[k-1].getRouteDaysDelta() == exchVar.getRouteDaysDelta()) 
								&& (exchVar.getRouteDaysDelta() == 1) 
								&& (visitIter->VisVars[k-1].getRouteTo()->getNumOnesAfterRoute() == exchVar.getRouteTo()->getNumOnesAfterRoute()) 
								&& (visitIter->VisVars[k-1].getRouteDurationDelta() > exchVar.getRouteDurationDelta()) )
								|| ( (visitIter->VisVars[k-1].getRouteDaysDelta() == exchVar.getRouteDaysDelta()) 
								&& (exchVar.getRouteDaysDelta() == 1) 
								&& (visitIter->VisVars[k-1].getRouteTo()->getNumOnesAfterRoute() == exchVar.getRouteTo()->getNumOnesAfterRoute() ) 
								&& (visitIter->VisVars[k-1].getRouteDurationDelta() == exchVar.getRouteDurationDelta()) 
								&& (visitIter->VisVars[k-1].getDeltaObj() > exchVar.getDeltaObj()) ) )
							{
								//cout << "k = " << k << endl;
								visitIter->VisVars[k] = visitIter->VisVars[k-1];
								k--;
								//cout << "Inside while" << endl;
								if (k == 0)
								{
									//cout << "before break" << endl;
									break;
								}
							}//end while
										
							
							visitIter->VisVars[k] = exchVar;
							
						}//end for j

						/*
						cout << "sorted" << endl;
						
						cout << "VisVars for " << visitIter->getOffshoreInst()->getInstName() << ":" << endl << endl;
						for (j = 0; j < visitIter->VisVars.size(); j++)
						{
							cout << "Variation " << j << ":" << endl;
							cout << "RouteDaysDelta = " << visitIter->VisVars[j].getRouteDaysDelta() << endl;
							cout << "NumOnesAfter = " << visitIter->VisVars[j].getRouteTo()->getNumOnesAfterRoute() << endl;
							cout << "RouteDurationDelta = " << visitIter->VisVars[j].getRouteDurationDelta() << endl;
							//cout << "\tRouteTo # of visits = " << visitIter->VisVars[j].getRouteTo()->getVisitObjects().size(); 
							cout << "  DeltaObj = " << visitIter->VisVars[j].getDeltaObj() << endl << endl;
						}
						cout << endl;
						*/
						

						/*
						//Initialize regret vector
						if (visitIter->VisVars.size() == 1)
						{
							visitIter->RegretValues.clear();
							visitIter->RegretValues.resize(1);
							visitIter->RegretValues[0] = visitIter->VisVars[0].getDeltaObj();
						}
						else
						{
							visitIter->RegretValues.clear();
							visitIter->RegretValues.resize(visitIter->VisVars.size() - 1);
							for (j = 1; j < visitIter->VisVars.size(); j++)
							{
								visitIter->RegretValues[j-1] = visitIter->VisVars[j].getDeltaObj() - 
								visitIter->VisVars[j-1].getDeltaObj();
							}
						}
						*/
					}//end if (visitIter->VisVars.size() > 1)
				
				}//end for visitIter

				//cout << "Before Actual Insertion" << endl;
				//printWeeklySchedule();
				//cout << endl << endl;
				/*
					for (visitIter = removedVisits.begin(); visitIter != removedVisits.end(); ++visitIter)
					{
						cout << "Visit->installation = " << visitIter->getOffshoreInst()->getInstName() << endl;
						cout << "VisVars.size() = " << visitIter->VisVars.size() << endl;
						if (visitIter->VisVars.size() > 0)
							cout << "RegretValues[0] = " << visitIter->RegretValues[0] << endl;
					}
					*/

				bestDeltaObj = numeric_limits<double>::max(); 
				bestRouDurDelta = numeric_limits<double>::max(); 
				bestRouteDaysDelta = 1; 
				bestVisVarSize = numeric_limits<int>::max();
				bestNumOnesAfterRoute = NUMBER_OF_DAYS;

				j = 1;
				bestIdxInsert = 0;

				//Identify the insertion
				for (visitIter = routeVisits.begin() + 1; visitIter != routeVisits.end() - 1; ++visitIter)
				{
					if ( ( visitIter->VisVars.size() < bestVisVarSize ) &&
						(visitIter->VisVars.size() > 0)
						&& (visitIter->VisVars[0].getRouteDaysDelta() == 0) )
					{
						bestVisVarSize = visitIter->VisVars.size();
						bestIdxInsert = j;
						bestRouteDaysDelta = visitIter->VisVars[0].getRouteDaysDelta();
						bestRouDurDelta = visitIter->VisVars[0].getRouteDurationDelta();
						bestDeltaObj = visitIter->VisVars[0].getDeltaObj();

						//cout << "a" << endl;
					}
					else if ( (visitIter->VisVars.size() == bestVisVarSize ) && 
						(visitIter->VisVars[0].getRouteDaysDelta() < bestRouteDaysDelta)
						&& (visitIter->VisVars.size() > 0))
					{
						
						bestVisVarSize = visitIter->VisVars.size();
						bestIdxInsert = j;
						bestRouteDaysDelta = visitIter->VisVars[0].getRouteDaysDelta();
						bestRouDurDelta = visitIter->VisVars[0].getRouteDurationDelta();
						bestDeltaObj = visitIter->VisVars[0].getDeltaObj();

						//cout << "b" << endl;
					}
					else if ( (visitIter->VisVars.size() == bestVisVarSize) 
						&& (visitIter->VisVars[0].getRouteDaysDelta() == bestRouteDaysDelta) 
						&& (visitIter->VisVars[0].getRouteDurationDelta() < bestRouDurDelta)
						&& (visitIter->VisVars[0].getRouteDaysDelta() == 0)
						&& (visitIter->VisVars.size() > 0) )
					{
						bestVisVarSize = visitIter->VisVars.size();
						bestIdxInsert = j;
						bestRouteDaysDelta = visitIter->VisVars[0].getRouteDaysDelta();
						bestRouDurDelta = visitIter->VisVars[0].getRouteDurationDelta();
						bestDeltaObj = visitIter->VisVars[0].getDeltaObj();

						//cout << "c" << endl;
					}
					else if ( (visitIter->VisVars.size() == bestVisVarSize) 
						&& (visitIter->VisVars[0].getRouteDaysDelta() == bestRouteDaysDelta) 
						&& (visitIter->VisVars[0].getRouteDurationDelta() == bestRouDurDelta)
						&& (visitIter->VisVars[0].getDeltaObj() < bestDeltaObj)
						&& (visitIter->VisVars[0].getRouteDaysDelta() == 0)
						&& (visitIter->VisVars.size() > 0) )
					{
						bestVisVarSize = visitIter->VisVars.size();
						bestIdxInsert = j;
						bestRouteDaysDelta = visitIter->VisVars[0].getRouteDaysDelta();
						bestRouDurDelta = visitIter->VisVars[0].getRouteDurationDelta();
						bestDeltaObj = visitIter->VisVars[0].getDeltaObj();

						//cout << "d" << endl;
					}
					/*
					else if ( (visitIter->VisVars.size() == bestVisVarSize) 
						&& (visitIter->VisVars[0].getRouteDaysDelta() == bestRouteDaysDelta)
						&& (visitIter->VisVars[0].getRouteTo()->getNumOnesAfterRoute() == 1)
						&& (visitIter->VisVars[0].getRouteDaysDelta() == 1)
						&& (visitIter->VisVars.size() > 0) )
					{
						bestVisVarSize = visitIter->VisVars.size();
						bestIdxInsert = j;
						bestRouteDaysDelta = visitIter->VisVars[0].getRouteDaysDelta();
						bestRouDurDelta = visitIter->VisVars[0].getRouteDurationDelta();
						bestDeltaObj = visitIter->VisVars[0].getDeltaObj();
						bestNumOnesAfterRoute = visitIter->VisVars[0].getRouteTo()->getNumOnesAfterRoute();

						cout << "e" << endl;
					}
					
					else if ( (visitIter->VisVars.size() == bestVisVarSize) 
						&& (visitIter->VisVars[0].getRouteDaysDelta() == bestRouteDaysDelta)
						&& (visitIter->VisVars[0].getRouteTo()->getNumOnesAfterRoute() == 1)
						&& (visitIter->VisVars[0].getRouteDaysDelta() == 1) 
						&& (visitIter->VisVars[0].getRouteDurationDelta() < bestRouDurDelta)
						&& (visitIter->VisVars.size() > 0) )
					{
						bestVisVarSize = visitIter->VisVars.size();
						bestIdxInsert = j;
						bestRouteDaysDelta = visitIter->VisVars[0].getRouteDaysDelta();
						bestRouDurDelta = visitIter->VisVars[0].getRouteDurationDelta();
						bestDeltaObj = visitIter->VisVars[0].getDeltaObj();
						//bestNumOnesAfterRoute = visitIter->VisVars[0].getRouteTo()->getNumOnesAfterRoute();
						//cout << "f" << endl;
					}
					else if ( (visitIter->VisVars.size() == bestVisVarSize) 
						&& (visitIter->VisVars[0].getRouteDaysDelta() == bestRouteDaysDelta)
						&& (visitIter->VisVars[0].getRouteTo()->getNumOnesAfterRoute() == 1)
						&& (visitIter->VisVars[0].getRouteDaysDelta() == 1) 
						&& (visitIter->VisVars[0].getRouteDurationDelta() == bestRouDurDelta)
						&& (visitIter->VisVars[0].getDeltaObj() < bestDeltaObj)
						&& (visitIter->VisVars.size() > 0) )
					{
						bestVisVarSize = visitIter->VisVars.size();
						bestIdxInsert = j;
						bestRouteDaysDelta = visitIter->VisVars[0].getRouteDaysDelta();
						bestRouDurDelta = visitIter->VisVars[0].getRouteDurationDelta();
						bestDeltaObj = visitIter->VisVars[0].getDeltaObj();
						//bestNumOnesAfterRoute = visitIter->VisVars[0].getRouteTo()->getNumOnesAfterRoute();

						//cout << "g" << endl;
					}			
					*/
					j++;
				}//end for visitIter

			//cout << "And here" << endl;

			if ( bestDeltaObj == numeric_limits<double>::max() )
				insertionPossible = false;
			else //Implement best insertion
			{
				/*
				cout << "ROUTE: " << endl;
				routeIter->printRoute();

				cout << "BEST INSERTION: visit = " << 
					routeVisits[bestIdxInsert].getOffshoreInst()->getInstName() << endl;
				//cout << "Inserting to route " << routeVisits[bestIdxInsert].VisVars[0].
					//getRouteTo()->getRouteNum() << endl;
				cout << "bestVisVarSize = " << routeVisits[bestIdxInsert].VisVars.size() << endl;
				cout << "bestRouteDaysDelta = " << routeVisits[bestIdxInsert].VisVars[0].getRouteDaysDelta() << endl;
				cout <<	"bestRouDurDelta = " << routeVisits[bestIdxInsert].VisVars[0].getRouteDurationDelta() << endl;
				cout <<	"bestDeltaObj = " <<routeVisits[bestIdxInsert].VisVars[0].getDeltaObj() << endl;
				cout << "bestNumOnesAfterRoute = " << routeVisits[bestIdxInsert].VisVars[0].getRouteTo()->getNumOnesAfterRoute() << endl;	
			*/
				routeDays = (int)ceil( (routeVisits[bestIdxInsert].VisVars[0].getRouteTo()->computeRouteDuration()+ 
				routeVisits[bestIdxInsert].VisVars[0].getRouteTo()->getInstAtPos(0)->getLayTime()
				+ routeVisits[bestIdxInsert].VisVars[0].getRouteTo()->getRouteMinSlack() )/24);

				routeStartIdx = (int) ceil(routeVisits[bestIdxInsert].VisVars[0].getRouteTo()->getRouteStartTime()/24);
				routeEndIdx = routeStartIdx + routeDays - 1;

				//Update VesAvail and decrement NumOnesAfterRoute
				if (routeVisits[bestIdxInsert].VisVars[0].getRouteDaysDelta() > 0)
				{
					for (j = 1; j <=routeVisits[bestIdxInsert].VisVars[0].getRouteDaysDelta(); j++)
						VesAvail[routeVisits[bestIdxInsert].VisVars[0].getRouteTo()->getRouteVes()->getID()]
								[routeEndIdx + j] = false;

					routeIter->setNumOnesAfterRoute(routeIter->getNumOnesAfterRoute() - 1);
				}

				routeVisits[bestIdxInsert].VisVars[0].getRouteTo()->insertInstalVisit(1, routeVisits[bestIdxInsert]);
				routeVisits[bestIdxInsert].VisVars[0].getRouteTo()->updateVisitVector();
				routeVisits[bestIdxInsert].VisVars[0].getRouteTo()->intelligentReorder();

				//Update visit day combination
				updateVisDayComb(routeVisits[bestIdxInsert].VisVars[0].getRouteFrom(),
					routeVisits[bestIdxInsert].VisVars[0].getRouteTo(), 
					routeVisits[bestIdxInsert].VisVars[0].getVarVisit() );

				
				//Update intermedRoutes
				for (routeIntermedIter = intermedRoutes.begin(); 
							routeIntermedIter->getRouteNum() != 
							routeVisits[bestIdxInsert].VisVars[0].getRouteTo()->getRouteNum();
							++routeIntermedIter)
							;
				*routeIntermedIter = *(routeVisits[bestIdxInsert].VisVars[0].getRouteTo());

				//Erase the visit from routeVisits
				
				routeIter->deleteInstalVisit(bestIdxInsert);
				routeIter->updateVisitVector();
				routeIter->intelligentReorder();

				//printWeeklySchedule();
				//cout << endl << endl;

				routeFromDaysAfter = (int) ceil( (routeIter->computeRouteDuration()
					+ routeIter->getRouteMinSlack() 
					+ routeIter->getInstAtPos(0)->getLayTime())/24);

				//cout << "routeFromDaysAfter = " << routeFromDaysAfter << endl;
				if (routeFromDaysAfter < routeFromDaysBefore)
					insertionPossible = false; // stop, we reduced the number of days
				
			}//end else

			//cout << "Olli Jokkinen" << endl;

		} while (insertionPossible);


		//store the cost reduction if we removed all visits
		if (routeFromDaysAfter < routeFromDaysBefore)
		{
			routeReducedNumber.push_back(routeIter->getRouteNum());
			costAfterRemove = computeSchedCost();
			routeReductionCost.push_back(costAfterRemove - costBeforeRemove);
		}

			//Restore the previous solution:
			//Restore the Routes vector
			for (l = 0; l < Routes.size(); l++)
			{
				Routes[l] = savedRoutes[l];
				intermedRoutes[l] = savedRoutes[l];

				//Restore numOnes
				Routes[l].setNumOnesAfterRoute(numOnes[l]);
			}

			//cout << "Number of Visits = " << routeIter->getVisitObjects().size()-1 << endl;

			//Restore visit day combinations
			for (l = 1; l < routeRelocVisits.size()-1; l++)
			{
				//cout << "Inst " << routeRelocVisits[l].getOffshoreInst()->getInstName() << ": ";
				routeRelocVisits[l].getOffshoreInst()->CurVisitDayComb.clear();
				for (m = 0; m < routeRelocVisits[l].getOffshoreInst()->getVisitFreq(); m++)
				{
					routeRelocVisits[l].getOffshoreInst()->CurVisitDayComb.push_back(VisCombs[l][m]);
					//cout << VisCombs[l][m] << "\t";
				}
			}	

			// Restore VesAvail
			for (k = 0; k < VesAvail.size(); k++)
				for (i = 0; i < planHorizon; i++)
					VesAvail[k][i] = vesAvailability[k][i];

		}//end if (routeDays == maxRouteDays)
	}//end for routeIter

	if (routeReductionCost.size() == 0)// no route can be removed
	{
		improvementFound = false;
		//cout << "No route can be removed" << endl;
	}

	else //there were routes which could be removed
	{
		bestCostRouteReduce = numeric_limits<double>::max();
		bestNumOnes = 0;

		//Identify the best route to Remove
		for (l = 0; l < routeReductionCost.size(); l++)
		{
			/*
			if (Routes[routeReducedNumber[l]].getNumOnesAfterRoute() > bestNumOnes)
			{
				bestIdxRouteReduce = l;
				bestCostRouteReduce = routeReductionCost[l];
			}
			else*/ 
			if ( routeReductionCost[l] < bestCostRouteReduce )
			{
				bestIdxRouteReduce = l;
				bestCostRouteReduce = routeReductionCost[l];
			}
			
			//cout << "ROUTE " << routeReducedNumber[l] << endl;
			//cout << "REDUCTION COST = " << routeReductionCost[l] << endl << endl;
			//Routes[routeReducedNumber[l]].printRoute();
		}

		//Remove several visits from the chosen route
		for (routeIter = Routes.begin(); routeIter != Routes.end(); ++routeIter)
			if (routeIter->getRouteNum() == Routes[routeReducedNumber[bestIdxRouteReduce]].getRouteNum() )
				break;

		//cout << "Removing the route number " << routeIter->getRouteNum() << endl;
		//routeIter->printRoute();

		//Update VesAvail and numOnes
		routeStartIndex = (int) ceil(routeIter->getRouteStartTime()/24);
		routeDays = (int) ceil( (routeIter->computeRouteDuration() +
			routeIter->getRouteMinSlack() + routeIter->getInstAtPos(0)->getLayTime())/24 );
		routeEndIndex = routeStartIndex + routeDays - 1;

		VesAvail[routeIter->getRouteVes()->getID()][routeEndIndex] = true;
		vesAvailability[routeIter->getRouteVes()->getID()][routeEndIndex] = true;

		routeIter->setNumOnesAfterRoute(routeIter->getNumOnesAfterRoute() + 1);
		
		do 
		{	
			insertionPossible = true;
			routeVisits.clear();
				routeVisits = routeIter->getVisitObjects();
				
				for (visitIter = routeVisits.begin()+1; visitIter != routeVisits.end()-1; ++visitIter)
				{
					visitIter->VisVars.clear();
					visitIter->RegretValues.clear();
					VisitVariation exchVar(&(*visitIter));

					//cout << "Let's see" << endl;

					//Evaluate and sort all possible insertions for a given visit
					for (routeRelocIter = Routes.begin(); routeRelocIter != Routes.end(); ++routeRelocIter)
					{
						routeDays = (int)ceil( (routeRelocIter->computeRouteDuration()+ 
						routeRelocIter->getInstAtPos(0)->getLayTime()
						+ routeRelocIter->getRouteMinSlack() )/24);

						routeStartIdx = (int) ceil(routeRelocIter->getRouteStartTime()/24);
						routeEndIdx = routeStartIdx + routeDays - 1;

						if ( (routeRelocIter->computeRouteDemand() + 
							visitIter->getOffshoreInst()->getWeeklyDemand()/visitIter->getOffshoreInst()->getVisitFreq()
							< routeRelocIter->getRouteVes()->getCapacity()) &&  
							(routeRelocIter->getVisitObjects().size() - 2 < routeRelocIter->getMaxVisits() ) &&
							!routeRelocIter->isInstOnRoute(visitIter->getOffshoreInst()) &&
							isDepartureSpreadEven(&(*routeIter), &(*routeRelocIter),&(*visitIter)) )
						{
							//Store the pre-insertion info
							routeToDurBefore = routeRelocIter->computeRouteDuration();
							routeDaysBefore = (int)ceil( (routeRelocIter->computeRouteDuration()+ 
								routeRelocIter->getInstAtPos(0)->getLayTime()
								+ routeRelocIter->getRouteMinSlack() )/24);
							routeSlackBefore = routeRelocIter->computeRouteSlack();
							schedCostBefore = computeSchedCost();
							//numRoutesBefore = Routes.size();

							//Do tentative insertion
							routeRelocIter->insertInstalVisit(1, *visitIter);
							routeRelocIter->updateVisitVector();
							routeRelocIter->intelligentReorder();
							routeToDurAfter = routeRelocIter->computeRouteDuration();

							if (routeToDurAfter < routeRelocIter->MAX_ROUTE_DUR + routeRelocIter->getRouteAcceptanceTime())
							{				
								routeDaysAfter = (int)ceil( (routeRelocIter->computeRouteDuration()+ 
								routeRelocIter->getInstAtPos(0)->getLayTime()
								+ routeRelocIter->getRouteMinSlack() )/24);

								daysDelta = routeDaysAfter - routeDaysBefore;

								/*
								for (j = 1; j <= daysDelta; j++)
								{
									if (!VesAvail[routeRelocIter->getRouteVes()->getID()]
									[routeEndIdx + j] )
									{
										insertionOverlaps = true;
										break;
									}

									if ( ( routeEndIdx + j > NUMBER_OF_DAYS) && 
										(!VesAvail[routeRelocIter->getRouteVes()->getID()]
									[(routeEndIdx+j)%NUMBER_OF_DAYS]) )
									{
										insertionOverlaps = true;
										break;
									}
								}
								*/
								if ( (daysDelta == 0) || 
									( (daysDelta == 1) && (routeRelocIter->getNumOnesAfterRoute() >= 1)
									&& (routeRelocIter->getNumOnesAfterRoute() <= 2) ) )
								{

									schedCostAfter = computeSchedCost();
									routeSlackAfter = routeRelocIter->computeRouteSlack();

									//Initialize VisitVariation objects
									VisitVariation visVar(&(*visitIter));
									visVar.setRouteTo(&(*routeRelocIter));
									visVar.setRouteFrom(&(*routeIter));
									visVar.setRouteToDurIncrease(routeToDurAfter - routeToDurBefore);
									visVar.setDeltaObj(schedCostAfter - schedCostBefore);
									visVar.setRouteDaysDelta(routeDaysAfter - routeDaysBefore);
									visVar.setRouteSlackDelta(routeSlackAfter - routeSlackBefore);
									visVar.setRouteDurationDelta(visVar.getRouteToDurIncrease() - 
										visVar.getRouteFromDurDecrease());
									visVar.setVisitIndex(visitIter->getIdNumber());

									visitIter->VisVars.push_back(visVar);
								
								//Use auxiliary objective???
								/*Auxiliary objective:
								- Schedule cost
								- Number of routes
								- Number of route days
								- Total slack
								*/
								}//end if

								//insertionOverlaps = false;
							}//end if

							//Restore the affected route
							for (routeIntermedIter = intermedRoutes.begin(); 
								routeIntermedIter->getRouteNum() !=  routeRelocIter->getRouteNum();
								++routeIntermedIter)
								;
							*routeRelocIter = *routeIntermedIter;

						}//end if
				
					}//end for routeRelocIter

					//cout << "Aga" << endl;
					
					//Sort VisVars vector using insertion sort
					//Note - some may already be sorted from previous iterations
					if (visitIter->VisVars.size() > 1)
					{
						//cout << "visitIter->VisVars.size() = " << visitIter->VisVars.size()<< endl;
						for (j = 1; j < visitIter->VisVars.size(); j++)
						{
							exchVar = visitIter->VisVars[j]; 
							k = j;

							//Multi-criteria sorting of variations
							while ( (visitIter->VisVars[k-1].getRouteDaysDelta() > exchVar.getRouteDaysDelta())
								|| ( (visitIter->VisVars[k-1].getRouteDaysDelta() == exchVar.getRouteDaysDelta())
								&& (visitIter->VisVars[k-1].getRouteDurationDelta() > exchVar.getRouteDurationDelta() )
								&& (exchVar.getRouteDaysDelta() == 0) )
								|| ( (visitIter->VisVars[k-1].getRouteDaysDelta() == exchVar.getRouteDaysDelta()) 
								&& (exchVar.getRouteDaysDelta() == 1) 
								&& (visitIter->VisVars[k-1].getRouteTo()->getNumOnesAfterRoute() > exchVar.getRouteTo()->getNumOnesAfterRoute()) )
								|| ( (visitIter->VisVars[k-1].getDeltaObj() > exchVar.getDeltaObj())
								&& (visitIter->VisVars[k-1].getRouteDaysDelta() == exchVar.getRouteDaysDelta())
								&& (visitIter->VisVars[k-1].getRouteDurationDelta() == exchVar.getRouteDurationDelta())
								&& (exchVar.getRouteDaysDelta() == 0) ) 
								|| ( (visitIter->VisVars[k-1].getRouteDaysDelta() == exchVar.getRouteDaysDelta()) 
								&& (exchVar.getRouteDaysDelta() == 1) 
								&& (visitIter->VisVars[k-1].getRouteTo()->getNumOnesAfterRoute() == exchVar.getRouteTo()->getNumOnesAfterRoute()) 
								&& (visitIter->VisVars[k-1].getRouteDurationDelta() > exchVar.getRouteDurationDelta()) )
								|| ( (visitIter->VisVars[k-1].getRouteDaysDelta() == exchVar.getRouteDaysDelta()) 
								&& (exchVar.getRouteDaysDelta() == 1) 
								&& (visitIter->VisVars[k-1].getRouteTo()->getNumOnesAfterRoute() == exchVar.getRouteTo()->getNumOnesAfterRoute() ) 
								&& (visitIter->VisVars[k-1].getRouteDurationDelta() == exchVar.getRouteDurationDelta()) 
								&& (visitIter->VisVars[k-1].getDeltaObj() > exchVar.getDeltaObj()) ) )
							{
								//cout << "k = " << k << endl;
								visitIter->VisVars[k] = visitIter->VisVars[k-1];
								k--;
								//cout << "Inside while" << endl;
								if (k == 0)
								{
									//cout << "before break" << endl;
									break;
								}
							}//end while
										
							
							visitIter->VisVars[k] = exchVar;
							
						}//end for j

						//cout << "sorted" << endl;
						/*
						cout << "VisVars for " << visitIter->getOffshoreInst()->getInstName() << endl;
						for (j = 0; j < visitIter->VisVars.size(); j++)
						{
							cout << "\tRouteTo # of visits = " << visitIter->VisVars[j].getRouteTo()->getVisitObjects().size(); 
							
							cout << "  DeltaObj = " << visitIter->VisVars[j].getDeltaObj() << endl;
						}
						cout << endl;
						*/

						/*
						//Initialize regret vector
						if (visitIter->VisVars.size() == 1)
						{
							visitIter->RegretValues.clear();
							visitIter->RegretValues.resize(1);
							visitIter->RegretValues[0] = visitIter->VisVars[0].getDeltaObj();
						}
						else
						{
							visitIter->RegretValues.clear();
							visitIter->RegretValues.resize(visitIter->VisVars.size() - 1);
							for (j = 1; j < visitIter->VisVars.size(); j++)
							{
								visitIter->RegretValues[j-1] = visitIter->VisVars[j].getDeltaObj() - 
								visitIter->VisVars[j-1].getDeltaObj();
							}
						}
						*/
					}//end if (visitIter->VisVars.size() > 1)
				
				}//end for visitIter

				//cout << "Before Actual Insertion" << endl;
				//printWeeklySchedule();
				//cout << endl << endl;
				/*
					for (visitIter = removedVisits.begin(); visitIter != removedVisits.end(); ++visitIter)
					{
						cout << "Visit->installation = " << visitIter->getOffshoreInst()->getInstName() << endl;
						cout << "VisVars.size() = " << visitIter->VisVars.size() << endl;
						if (visitIter->VisVars.size() > 0)
							cout << "RegretValues[0] = " << visitIter->RegretValues[0] << endl;
					}
					*/

				bestDeltaObj = numeric_limits<double>::max(); 
				bestRouDurDelta = numeric_limits<double>::max(); 
				bestRouteDaysDelta = 1; 
				bestVisVarSize = numeric_limits<int>::max();
				bestNumOnesAfterRoute = NUMBER_OF_DAYS;

				j = 1;
				bestIdxInsert = 0;

				//Identify the insertion
				for (visitIter = routeVisits.begin() + 1; visitIter != routeVisits.end() - 1; ++visitIter)
				{
					if ( ( visitIter->VisVars.size() < bestVisVarSize )
						&& (visitIter->VisVars.size() > 0)
						&& (visitIter->VisVars[0].getRouteDaysDelta() == 0) )
					{
						bestVisVarSize = visitIter->VisVars.size();
						bestIdxInsert = j;
						bestRouteDaysDelta = visitIter->VisVars[0].getRouteDaysDelta();
						bestRouDurDelta = visitIter->VisVars[0].getRouteDurationDelta();
						bestDeltaObj = visitIter->VisVars[0].getDeltaObj();

						//cout << "a" << endl;
					}
					else if ( (visitIter->VisVars.size() == bestVisVarSize ) && 
						(visitIter->VisVars[0].getRouteDaysDelta() < bestRouteDaysDelta)
						&& (visitIter->VisVars.size() > 0))
					{
						
						bestVisVarSize = visitIter->VisVars.size();
						bestIdxInsert = j;
						bestRouteDaysDelta = visitIter->VisVars[0].getRouteDaysDelta();
						bestRouDurDelta = visitIter->VisVars[0].getRouteDurationDelta();
						bestDeltaObj = visitIter->VisVars[0].getDeltaObj();

						//cout << "b" << endl;
					}
					else if ( (visitIter->VisVars.size() == bestVisVarSize) 
						&& (visitIter->VisVars[0].getRouteDaysDelta() == bestRouteDaysDelta) 
						&& (visitIter->VisVars[0].getRouteDurationDelta() < bestRouDurDelta)
						&& (visitIter->VisVars[0].getRouteDaysDelta() == 0)
						&& (visitIter->VisVars.size() > 0) )
					{
						bestVisVarSize = visitIter->VisVars.size();
						bestIdxInsert = j;
						bestRouteDaysDelta = visitIter->VisVars[0].getRouteDaysDelta();
						bestRouDurDelta = visitIter->VisVars[0].getRouteDurationDelta();
						bestDeltaObj = visitIter->VisVars[0].getDeltaObj();

						//cout << "c" << endl;
					}
					else if ( (visitIter->VisVars.size() == bestVisVarSize) 
						&& (visitIter->VisVars[0].getRouteDaysDelta() == bestRouteDaysDelta) 
						&& (visitIter->VisVars[0].getRouteDurationDelta() == bestRouDurDelta)
						&& (visitIter->VisVars[0].getRouteDurationDelta() == bestRouDurDelta)
						&& (visitIter->VisVars[0].getDeltaObj() < bestDeltaObj)
						&& (visitIter->VisVars[0].getRouteDaysDelta() == 0)
						&& (visitIter->VisVars.size() > 0) )
					{
						bestVisVarSize = visitIter->VisVars.size();
						bestIdxInsert = j;
						bestRouteDaysDelta = visitIter->VisVars[0].getRouteDaysDelta();
						bestRouDurDelta = visitIter->VisVars[0].getRouteDurationDelta();
						bestDeltaObj = visitIter->VisVars[0].getDeltaObj();

						//cout << "d" << endl;
					}
					/*
					else if ( (visitIter->VisVars.size() == bestVisVarSize) 
						&& (visitIter->VisVars[0].getRouteDaysDelta() == bestRouteDaysDelta)
						&& (visitIter->VisVars[0].getRouteTo()->getNumOnesAfterRoute() < bestNumOnesAfterRoute)
						&& (visitIter->VisVars[0].getRouteDaysDelta() == 1)
						&& (visitIter->VisVars.size() > 0) )
					{
						bestVisVarSize = visitIter->VisVars.size();
						bestIdxInsert = j;
						bestRouteDaysDelta = visitIter->VisVars[0].getRouteDaysDelta();
						bestRouDurDelta = visitIter->VisVars[0].getRouteDurationDelta();
						bestDeltaObj = visitIter->VisVars[0].getDeltaObj();
						bestNumOnesAfterRoute = visitIter->VisVars[0].getRouteTo()->getNumOnesAfterRoute();

						cout << "e" << endl;
					}
					else if ( (visitIter->VisVars.size() == bestVisVarSize) 
						&& (visitIter->VisVars[0].getRouteDaysDelta() == bestRouteDaysDelta)
						&& (visitIter->VisVars[0].getRouteTo()->getNumOnesAfterRoute() == 1)
						&& (visitIter->VisVars[0].getRouteDaysDelta() == 1) 
						&& (visitIter->VisVars[0].getRouteDurationDelta() < bestRouDurDelta)
						&& (visitIter->VisVars.size() > 0) )
					{
						bestVisVarSize = visitIter->VisVars.size();
						bestIdxInsert = j;
						bestRouteDaysDelta = visitIter->VisVars[0].getRouteDaysDelta();
						bestRouDurDelta = visitIter->VisVars[0].getRouteDurationDelta();
						bestDeltaObj = visitIter->VisVars[0].getDeltaObj();
						//bestNumOnesAfterRoute = visitIter->VisVars[0].getRouteTo()->getNumOnesAfterRoute();
						//cout << "f" << endl;
					}
					else if ( (visitIter->VisVars.size() == bestVisVarSize) 
						&& (visitIter->VisVars[0].getRouteDaysDelta() == bestRouteDaysDelta)
						&& (visitIter->VisVars[0].getRouteTo()->getNumOnesAfterRoute() == 1)
						&& (visitIter->VisVars[0].getRouteDaysDelta() == 1) 
						&& (visitIter->VisVars[0].getRouteDurationDelta() == bestRouDurDelta)
						&& (visitIter->VisVars[0].getDeltaObj() < bestDeltaObj)
						&& (visitIter->VisVars.size() > 0) )
					{
						bestVisVarSize = visitIter->VisVars.size();
						bestIdxInsert = j;
						bestRouteDaysDelta = visitIter->VisVars[0].getRouteDaysDelta();
						bestRouDurDelta = visitIter->VisVars[0].getRouteDurationDelta();
						bestDeltaObj = visitIter->VisVars[0].getDeltaObj();
						//bestNumOnesAfterRoute = visitIter->VisVars[0].getRouteTo()->getNumOnesAfterRoute();

						//cout << "g" << endl;
					}		
					*/
					j++;
				}//end for visitIter

			//cout << "And here" << endl;

			if ( bestDeltaObj == numeric_limits<double>::max() )
				insertionPossible = false;
			else //Implement best insertion
			{
				/*
				cout << "ROUTE: " << endl;
				routeIter->printRoute();

				cout << "BEST INSERTION: visit = " << 
					routeVisits[bestIdxInsert].getOffshoreInst()->getInstName() << endl;
				cout << "Inserting to route " << routeVisits[bestIdxInsert].VisVars[0].
					getRouteTo()->getRouteNum() << endl;
				cout << "bestVisVarSize = " << routeVisits[bestIdxInsert].VisVars.size() << endl;
				cout << "bestRouteDaysDelta = " << routeVisits[bestIdxInsert].VisVars[0].getRouteDaysDelta() << endl;
				cout <<	"bestRouDurDelta = " << routeVisits[bestIdxInsert].VisVars[0].getRouteDurationDelta() << endl;
				cout <<	"bestDeltaObj = " <<routeVisits[bestIdxInsert].VisVars[0].getDeltaObj() << endl;
				cout << "bestNumOnesAfterRoute = " << routeVisits[bestIdxInsert].VisVars[0].getRouteTo()->getNumOnesAfterRoute() << endl;	
			*/
				routeDays = (int)ceil( (routeVisits[bestIdxInsert].VisVars[0].getRouteTo()->computeRouteDuration()+ 
				routeVisits[bestIdxInsert].VisVars[0].getRouteTo()->getInstAtPos(0)->getLayTime()
				+ routeVisits[bestIdxInsert].VisVars[0].getRouteTo()->getRouteMinSlack() )/24);

				routeStartIdx = (int) ceil(routeVisits[bestIdxInsert].VisVars[0].getRouteTo()->getRouteStartTime()/24);
				routeEndIdx = routeStartIdx + routeDays - 1;

				//Update VesAvail and decrement NumOnesAfterRoute
				if (routeVisits[bestIdxInsert].VisVars[0].getRouteDaysDelta() > 0)
				{
					for (j = 1; j <=routeVisits[bestIdxInsert].VisVars[0].getRouteDaysDelta(); j++)
						VesAvail[routeVisits[bestIdxInsert].VisVars[0].getRouteTo()->getRouteVes()->getID()]
								[routeEndIdx + j] = false;

					for (j = 1; j <=routeVisits[bestIdxInsert].VisVars[0].getRouteDaysDelta(); j++)
						vesAvailability[routeVisits[bestIdxInsert].VisVars[0].getRouteTo()->getRouteVes()->getID()]
								[routeEndIdx + j] = false;


					routeIter->setNumOnesAfterRoute(routeIter->getNumOnesAfterRoute() - 1);
				}

				routeVisits[bestIdxInsert].VisVars[0].getRouteTo()->insertInstalVisit(1, routeVisits[bestIdxInsert]);
				routeVisits[bestIdxInsert].VisVars[0].getRouteTo()->updateVisitVector();
				routeVisits[bestIdxInsert].VisVars[0].getRouteTo()->intelligentReorder();

				//Update visit day combination
				updateVisDayComb(routeVisits[bestIdxInsert].VisVars[0].getRouteFrom(),
					routeVisits[bestIdxInsert].VisVars[0].getRouteTo(), 
					routeVisits[bestIdxInsert].VisVars[0].getVarVisit() );

				
				//Update intermedRoutes
				for (routeIntermedIter = intermedRoutes.begin(); 
							routeIntermedIter->getRouteNum() != 
							routeVisits[bestIdxInsert].VisVars[0].getRouteTo()->getRouteNum();
							++routeIntermedIter)
							;
				*routeIntermedIter = *(routeVisits[bestIdxInsert].VisVars[0].getRouteTo());

				//Erase the visit from routeVisits
				
				routeIter->deleteInstalVisit(bestIdxInsert);
				routeIter->updateVisitVector();
				routeIter->intelligentReorder();

				//printWeeklySchedule();
				//cout << endl << endl;

				routeFromDaysAfter = (int) ceil( (routeIter->computeRouteDuration()
					+ routeIter->getRouteMinSlack() 
					+ routeIter->getInstAtPos(0)->getLayTime())/24);

				if (routeFromDaysAfter < routeFromDaysBefore)
					insertionPossible = false; // stop, we reduced the number of days
				
			}//end else

			//cout << "Olli Jokkinen" << endl;

		} while (insertionPossible);

		//cout << "After while" << endl;

		intermedRoutes.clear();
		intermedRoutes.resize(Routes.size());
		savedRoutes.clear();
		savedRoutes.resize(Routes.size());
		numOnes.clear();
		numOnes.resize(Routes.size());

		for (l = 0; l < Routes.size(); l++) 
		{
			intermedRoutes[l] = Routes[l];
			savedRoutes[l] = Routes[l];
			numOnes[l] = Routes[l].getNumOnesAfterRoute();
		}

		
		improvementFound = true;
		//cout << "After removing the Route" << endl;
		//printWeeklySchedule();

	}//end else

}//end while (improvementFound)
	
//cout << "reduceRouteDurTotDays_END" << endl;
//cout << "At the end of ReduceRouteDurTot" << endl;
//printWeeklySchedule();
}

bool WeeklySchedule::isDepartureSpreadEven(Route *routeFrom, Route *routeTo, Visit *visit)
{
	vector <int> DayCombResult;
	DayCombResult.resize(visit->getOffshoreInst()->getVisitFreq());
	DayCombResult = visit->getOffshoreInst()->CurVisitDayComb;
	int removedDay = (int) ceil(routeFrom->getRouteStartTime()/24);
	int insertedDay = (int) ceil(routeTo->getRouteStartTime()/24);

	int i, j, k;
	bool dayInserted = false;

	//Keep the assigned visit day combination sorted
	for (i = 0; i < DayCombResult.size(); i++)
	{
		if (insertedDay == DayCombResult[i])
			return false;

		//cout << DayCombResult[i] << endl;
		if(insertedDay < DayCombResult[i])
		{
			DayCombResult.insert(DayCombResult.begin()+i, insertedDay);
			dayInserted = true;
			break;
		}
	}


	if (!dayInserted)
		DayCombResult.push_back(insertedDay);

	for (i = 0; i < visit->getOffshoreInst()->ObligatoryVisitDays.size(); i++)
		if  ((visit->getOffshoreInst()->ObligatoryVisitDays[i] == removedDay)
			&& (insertedDay != removedDay) )
			return false;

	for (i = 0; i < DayCombResult.size(); i++)
		if ( removedDay == DayCombResult[i] )
			DayCombResult.erase(DayCombResult.begin()+i);

	//Check if the visit day combination satisfy the "evenly spread" requirements

	vector <int> auxVec;
	int accumDiff = 0;
	int planHor = 0;
	bool ind1 = false, ind2 = false;
	int numVisits, day;

	//cout << "Installation " << visit->getOffshoreInst()->getInstName() << endl;
	//cout << "DayCombResult.size() = " << DayCombResult.size() << endl << endl;

	if ( (DayCombResult.size() == 1) || (DayCombResult.size() == 6) )
		return true;

	else if (DayCombResult.size() == 2)
	{
		//2 visits: at least one visit during 4 days
		//First indicator
		for (i = 1; i < DayCombResult.size(); i++)
			auxVec.push_back(DayCombResult[i] - DayCombResult[i-1]);

		/*
		if ( (DayCombResult[DayCombResult.size() - 1] == NUMBER_OF_DAYS - 1) && 
			(DayCombResult[0] == 1) )
				auxVec.push_back(1);
				*/

		for (i = 0; i < auxVec.size(); i++)
			if (auxVec[i] == 1)
				accumDiff++;

		if (accumDiff == visit->getOffshoreInst()->getVisitFreq() - 1)
			ind1 = false;
		else
			ind1 = true;


		//Second indicator
		planHor = 4;
		for (i = 1; i < NUMBER_OF_DAYS+1; i++)
		{
			numVisits = 0;
			for (j = 0; j < planHor; j++)
			{
				if ( (i+j) < NUMBER_OF_DAYS + 1)
					day = i + j;
				else
					day = (i+j)%(NUMBER_OF_DAYS+1) + 1;

				for (k = 0; k < DayCombResult.size(); k++)
					if (DayCombResult[k] == day)
						numVisits++;
			}//end for j

			if (numVisits == 0)
			{
				ind2 = true;
				break;
			}
		}//end for i
		
		if ( (ind1) && (!ind2) )
			return true;
		else 
			return false;

	}

	else if (DayCombResult.size() == 3)
	{
		//3 visits: at least one visit during 3 days
		//First indicator
		for (i = 1; i < DayCombResult.size(); i++)
			auxVec.push_back(DayCombResult[i] - DayCombResult[i-1]);

		/*
		if ( (DayCombResult[DayCombResult.size() - 1] == NUMBER_OF_DAYS - 1) && 
			(DayCombResult[0] == 1) )
				auxVec.push_back(1);
				*/

		for (i = 0; i < auxVec.size(); i++)
			if (auxVec[i] == 1)
				accumDiff++;

		if (accumDiff == visit->getOffshoreInst()->getVisitFreq() - 1)
			ind1 = false;
		else
			ind1 = true;


		//Second indicator
		planHor = 3;
		for (i = 1; i < NUMBER_OF_DAYS+1; i++)
		{
			numVisits = 0;
			for (j = 0; j < planHor; j++)
			{
				if ( (i+j) < NUMBER_OF_DAYS+1)
					day = i + j;
				else
					day = (i+j) % (NUMBER_OF_DAYS+1) + 1;

				for (k = 0; k < DayCombResult.size(); k++)
					if (DayCombResult[k] == day)
						numVisits++;
			}//end for j

			if (numVisits == 0)
			{
				ind2 = true;
				break;
			}
		}//end for i
		
		if ( (ind1) && (!ind2) )
			return true;
		else 
			return false;

	}
	else if (DayCombResult.size() == 4)
	{
		//4 visits: During a 4-day period at least two vessels
		//First indicator
		for (i = 1; i < DayCombResult.size(); i++)
			auxVec.push_back(DayCombResult[i] - DayCombResult[i-1]);

		/*
		if ( (DayCombResult[DayCombResult.size() - 1] == NUMBER_OF_DAYS - 1) && 
			(DayCombResult[0] == 1) )
				auxVec.push_back(1);
				*/

		for (i = 0; i < auxVec.size(); i++)
			if (auxVec[i] == 1)
				accumDiff++;

		if (accumDiff == visit->getOffshoreInst()->getVisitFreq() - 1)
			ind1 = false;
		else
			ind1 = true;

		//Second indicator
		planHor = 4;
		for (i = 1; i < NUMBER_OF_DAYS+1; i++)
		{
			numVisits = 0;
			for (j = 0; j < planHor; j++)
			{
				if ( (i+j) < NUMBER_OF_DAYS+1)
					day = i + j;
				else
					day = (i+j) % (NUMBER_OF_DAYS+1) + 1;

				for (k = 0; k < DayCombResult.size(); k++)
					if (DayCombResult[k] == day)
						numVisits++;
			}//end for j

			if (numVisits <= 1)
			{
				ind2 = true;
				break;
			}
		}//end for i
		
		if ( (ind1) && (!ind2) )
			return true;
		else 
			return false;
	}


	else if (DayCombResult.size() == 5)
	{
		//5 visits
		for (i = 1; i < DayCombResult.size(); i++)
			auxVec.push_back(DayCombResult[i] - DayCombResult[i-1]);

		/*
		if ( (DayCombResult[DayCombResult.size() - 1] == NUMBER_OF_DAYS - 1) && 
			(DayCombResult[0] == 1) )
				auxVec.push_back(1);
				*/

		for (i = 0; i < auxVec.size(); i++)
			if (auxVec[i] == 1)
				accumDiff++;

		if (accumDiff == visit->getOffshoreInst()->getVisitFreq() - 1)
			return false;
		else
			return true;
	}//end 5 visits

	else //DayCombResult of the wrong size
		cout << "Error in isDepartureSpreadEven 1111!!!" << endl << endl;
}

bool WeeklySchedule::isDepartureSpreadEven(Route *routeTo, Visit *visit)
{
	vector <int> DayCombResult;
	DayCombResult.resize(visit->getOffshoreInst()->CurVisitDayComb.size());
	DayCombResult = visit->getOffshoreInst()->CurVisitDayComb;
	int insertedDay = (int) ceil(routeTo->getRouteStartTime()/24);

	int i, j, k;
	bool dayInserted = false;

	//Keep the assigned visit day combination sorted
	for (i = 0; i < DayCombResult.size(); i++)
	{
		if (insertedDay == DayCombResult[i])
			return false;

		//cout << DayCombResult[i] << endl;
		if(insertedDay < DayCombResult[i])
		{
			DayCombResult.insert(DayCombResult.begin()+i, insertedDay);
			dayInserted = true;
			break;
		}
	}


	if (!dayInserted)
		DayCombResult.push_back(insertedDay);

	//if (DayCombResult.size() < visit->getOffshoreInst()->getVisitFreq() )
	//	return true;

	bool dayFound;

	if ( (DayCombResult.size() == visit->getOffshoreInst()->getVisitFreq())
		&& (visit->getOffshoreInst()->ObligatoryVisitDays.size() > 0) )
		for (i = 0; i < visit->getOffshoreInst()->ObligatoryVisitDays.size(); i++)
		{
			dayFound = false;
			for (j = 0; j < DayCombResult.size(); j++)
				if  (visit->getOffshoreInst()->ObligatoryVisitDays[i] == DayCombResult[j])
					dayFound = true;
				
			if (!dayFound)
				return false;
		}


	//Check if the visit day combination satisfy the "evenly spread" requirements

	vector <int> auxVec;
	int accumDiff = 0;
	int planHor = 0;
	bool ind1 = false, ind2 = false;
	int numVisits, day;

	if ( (DayCombResult.size() == 1) || (DayCombResult.size() == 6) )
		return true;
/*
	else if (DayCombResult.size() != visit->getOffshoreInst()->getVisitFreq() )
		return true;
*/
	else if (DayCombResult.size() == 2)
	{
		//2 visits: at least one visit during 4 days
		//First indicator
		for (i = 1; i < DayCombResult.size(); i++)
			auxVec.push_back(DayCombResult[i] - DayCombResult[i-1]);
		/*
		if ( (DayCombResult[DayCombResult.size() - 1] == NUMBER_OF_DAYS - 1) && 
			(DayCombResult[0] == 1) )
				auxVec.push_back(1);
				*/

		for (i = 0; i < auxVec.size(); i++)
			if (auxVec[i] == 1)
				accumDiff++;

		if (accumDiff == DayCombResult.size() - 1)
			ind1 = false;
		else
			ind1 = true;


		//Second indicator
		planHor = 4;
		for (i = 1; i < NUMBER_OF_DAYS+1; i++)
		{
			numVisits = 0;
			for (j = 0; j < planHor; j++)
			{
				if ( (i+j) < NUMBER_OF_DAYS + 1)
					day = i + j;
				else
					day = (i+j)%(NUMBER_OF_DAYS+1) + 1;

				for (k = 0; k < DayCombResult.size(); k++)
					if (DayCombResult[k] == day)
						numVisits++;
			}//end for j

			if (numVisits == 0)
			{
				ind2 = true;
				break;
			}
		}//end for i
		
		if ( (ind1) && (!ind2) )
			return true;
		else 
			return false;

	}

	else if (DayCombResult.size() == 3)
	{
		//3 visits: at least one visit during 3 days
		//First indicator
		for (i = 1; i < DayCombResult.size(); i++)
			auxVec.push_back(DayCombResult[i] - DayCombResult[i-1]);

		/*
		if ( (DayCombResult[DayCombResult.size() - 1] == NUMBER_OF_DAYS - 1) && 
			(DayCombResult[0] == 1) )
				auxVec.push_back(1);
				*/
		for (i = 0; i < auxVec.size(); i++)
			if (auxVec[i] == 1)
				accumDiff++;

		if (accumDiff == DayCombResult.size() - 1)
			ind1 = false;
		else
			ind1 = true;


		//Second indicator
		planHor = 3;
		for (i = 1; i < NUMBER_OF_DAYS+1; i++)
		{
			numVisits = 0;
			for (j = 0; j < planHor; j++)
			{
				if ( (i+j) < NUMBER_OF_DAYS+1)
					day = i + j;
				else
					day = (i+j) % (NUMBER_OF_DAYS+1) + 1;

				for (k = 0; k < DayCombResult.size(); k++)
					if (DayCombResult[k] == day)
						numVisits++;
			}//end for j

			if (numVisits == 0)
			{
				ind2 = true;
				break;
			}
		}//end for i
		
		if ( (ind1) && (!ind2) )
			return true;
		else 
			return false;

	}
	else if (DayCombResult.size() == 4)
	{
		//4 visits: During a 4-day period at least two vessels
		//First indicator
		for (i = 1; i < DayCombResult.size(); i++)
			auxVec.push_back(DayCombResult[i] - DayCombResult[i-1]);

		/*
		if ( (DayCombResult[DayCombResult.size() - 1] == NUMBER_OF_DAYS - 1) && 
			(DayCombResult[0] == 1) )
				auxVec.push_back(1);
				*/

		for (i = 0; i < auxVec.size(); i++)
			if (auxVec[i] == 1)
				accumDiff++;

		if (accumDiff == DayCombResult.size() - 1)
			ind1 = false;
		else
			ind1 = true;

		//Second indicator
		planHor = 4;
		for (i = 1; i < NUMBER_OF_DAYS+1; i++)
		{
			numVisits = 0;
			for (j = 0; j < planHor; j++)
			{
				if ( (i+j) < NUMBER_OF_DAYS+1)
					day = i + j;
				else
					day = (i+j) % (NUMBER_OF_DAYS+1) + 1;

				for (k = 0; k < DayCombResult.size(); k++)
					if (DayCombResult[k] == day)
						numVisits++;
			}//end for j

			if (numVisits <= 1)
			{
				ind2 = true;
				break;
			}
		}//end for i
		
		if ( (ind1) && (!ind2) )
			return true;
		else 
			return false;
	}


	else if (DayCombResult.size() == 5)
	{
		//5 visits
		for (i = 1; i < DayCombResult.size(); i++)
			auxVec.push_back(DayCombResult[i] - DayCombResult[i-1]);

		/*
		if ( (DayCombResult[DayCombResult.size() - 1] == NUMBER_OF_DAYS - 1) && 
			(DayCombResult[0] == 1) )
				auxVec.push_back(1);
				*/

		for (i = 0; i < auxVec.size(); i++)
			if (auxVec[i] == 1)
				accumDiff++;

		if (accumDiff == DayCombResult.size() - 1)
			return false;
		else
			return true;
	}//end 5 visits

	else //DayCombResult of the wrong size
		cout << "Error in isDepartureSpreadEven 2222!!!" << endl << endl;
}

void WeeklySchedule::updateVisDayComb(Route *routeFrom, Route *routeTo, Visit *visit)
{
	vector <int> DayCombResult;
	DayCombResult.resize(visit->getOffshoreInst()->getVisitFreq());
	DayCombResult = visit->getOffshoreInst()->CurVisitDayComb;
	int removedDay = (int) ceil(routeFrom->getRouteStartTime()/24);
	int insertedDay = (int) ceil(routeTo->getRouteStartTime()/24);

	int i;
	bool dayInserted = false;

	//Keep the assigned visit day combination sorted
	for (i = 0; i < DayCombResult.size(); i++)
	{
		if(insertedDay < DayCombResult[i])
		{
			DayCombResult.insert(DayCombResult.begin()+i, insertedDay);
			dayInserted = true;
			break;
		}
	}


	if (!dayInserted)
		DayCombResult.push_back(insertedDay);

	for (i = 0; i < DayCombResult.size(); i++)
		if ( removedDay == DayCombResult[i] )
			DayCombResult.erase(DayCombResult.begin()+i);

	visit->getOffshoreInst()->CurVisitDayComb.clear();

	for (i = 0; i < DayCombResult.size(); i++)
		visit->getOffshoreInst()->CurVisitDayComb.push_back(DayCombResult[i]);
	
	//cout << "End of UpdateVisDayComb" << endl;

}

void WeeklySchedule::relocateVisits() 
{
	//cout << "relocateVisits_BEGIN" << endl;
	vector <Route>::iterator routeIter;
	vector <Route>::iterator routeRelocIter;
	vector <Route>::iterator routeIntermedIter;

	vector <SupVes>::iterator vesIter;

	vector <Visit> routeVisits;
	vector <Visit> routeRelocVisits;
	vector <Visit>::iterator visitIter;

	int i = 0, j = 1, k = 0, l;
	int routeStartIndex, routeEndIndex;
	
	vector <Route> savedRoutes; //additional vector of routes
	vector <Route> intermedRoutes;
	savedRoutes.resize(Routes.size());
	intermedRoutes.resize(Routes.size());

	vector < vector <int> > VisCombs;

	int LB = computeLB_numRoutes();
	int totDays;
	int routeDays, routeDaysBefore, routeDaysAfter;
	int routeFromDaysBefore, routeFromDaysAfter;
	
	//cout << "BEFORE reduceRouteDurTotDays() Total route days = " << totDays << endl;
	//printWeeklySchedule();

	double routeFromDurBefore, routeFromDurAfter;
	double routeToDurBefore, routeToDurAfter;
	double schedCostBefore, schedCostAfter;
	double bestDeltaObj, bestRouDurDelta, bestRouteDaysDelta, bestRouteFromDaysDelta;
	int bestVarIdx;
	int bestVisVarSize;
	
	bool improvementFound = true;
	bool shouldReassign = false;
	bool numRoutesReduced;
	
	vector <vector <bool> > vesAvailability; //local vessel availability vector
	vesAvailability.resize(VesAvail.size());
	int planHorizon = NUMBER_OF_DAYS + (int) ceil(
			( (double)Routes.begin()->MAX_ROUTE_DUR 
			+ Routes.begin()->getInstAtPos(0)->getLayTime() 
			+ Routes.begin()->getRouteMinSlack() )/24);

	for (k = 0; k < VesAvail.size(); k++)
		{
			vesAvailability[k].resize(planHorizon);
			for (i = 0; i < planHorizon; i++)
				vesAvailability[k][i] = VesAvail[k][i];
		}

	//Set visit Ids
	for (routeIter = Routes.begin(); routeIter != Routes.end(); ++routeIter)
		routeIter->setVisitIds();

	//printWeeklySchedule();

	bool endOfWeekReached;
	for (routeIter = Routes.begin(); routeIter != Routes.end(); ++routeIter)
	{//Calculate number of ones in VesAvail after every Route
		routeEndIndex = (int) ceil( (routeIter->getRouteEndTime() + 
			routeIter->getRouteMinSlack() - routeIter->getInstAtPos(0)->getLayTime())/24 );
		j = routeEndIndex + 1;
		k = 0;
		endOfWeekReached = false;

		if (j > NUMBER_OF_DAYS)
			endOfWeekReached = true;

		//cout << "j = " << j << endl;

		while ( (VesAvail[routeIter->getRouteVes()->getID()][j]) &&
			(!endOfWeekReached) )
		{
			k++;
			j++;
			if (j > NUMBER_OF_DAYS)
				endOfWeekReached = true;
		}

		//cout << "where" << endl;

		if (endOfWeekReached)
		{
			//If we reached the end of the week, start from the beginning again
			while (VesAvail[routeIter->getRouteVes()->getID()][j%NUMBER_OF_DAYS])
			{
				k++;
				j++;
			}
		}

		routeIter->setNumOnesAfterRoute(k);
		//cout << "Route Number: " << routeIter->getRouteNum();
		//cout << " - number of ones after: " << routeIter->getNumOnesAfterRoute() << endl;
	}//end for routeIter
	
	int maxRouteDays = (int) ceil( ((double)Routes[0].MAX_ROUTE_DUR 
		+ Routes[0].getInstAtPos(0)->getLayTime())/24);

	vector <VisitVariation>::iterator visVarIter;
	vector <VisitVariation> VisVariations;

	for (l = 0; l < Routes.size(); l++)
	{
		savedRoutes[l] = Routes[l];
		intermedRoutes[l] = Routes[l];
		totDays += (int)ceil( (Routes[l].computeRouteDuration()+
			Routes[l].getInstAtPos(0)->getLayTime() + Routes[l].getRouteMinSlack())/24);
	}


while (improvementFound)
{
	improvementFound = false;
	numRoutesReduced = false;
	i = 0;  k = 0;

	//cout << "Here" << endl;
	VisVariations.clear();

	for (routeIter = Routes.begin(); routeIter != Routes.end(); ++routeIter)
		if ( ( (int)ceil( (routeIter->computeRouteDuration()+ 
			routeIter->getInstAtPos(0)->getLayTime())/24) == maxRouteDays)
			/*&& (routeIter->getNumOnesAfterRoute() <= 4) 
			&& (routeIter->getNumOnesAfterRoute() >= 1)*/ )
			//Only consider 3-day routes with between one and four 1's after them
	{
		
		routeDays = (int)ceil( (routeIter->computeRouteDuration()+ routeIter->getInstAtPos(0)->getLayTime()
			 + routeIter->getRouteMinSlack() )/24);
	
		/*routeRelocVisits.clear();
		routeRelocVisits = routeIter->getVisitObjects();

		VisCombs.clear();
		VisCombs.resize(routeIter->getVisitObjects().size());

		Save the visit day combinations of installations on the route
		for (l = 1; l < routeIter->getVisitObjects().size()-1; l++)
			for (m = 0; m < routeRelocVisits[l].getOffshoreInst()->getVisitFreq(); m++)
				VisCombs[l].push_back(routeRelocVisits[l].getOffshoreInst()->CurVisitDayComb[m]);

		//insertMore = true;*/

		
		//cout << "yep" << endl;
		routeVisits.clear();
		routeVisits = routeIter->getVisitObjects();

		j = 1;

		for (visitIter = routeVisits.begin()+1; visitIter != routeVisits.end()-1; ++visitIter)
		{
			k = 0;
			for (routeRelocIter = Routes.begin(); routeRelocIter != Routes.end(); ++routeRelocIter)
			{
				//do not reinsert in the same route, check load feasibility
				//if the installation is already on the route and evenly spread
				if ( (routeIter != routeRelocIter)
				&& (routeRelocIter->computeRouteDemand() + visitIter->getOffshoreInst()->getWeeklyDemand()
				/ visitIter->getOffshoreInst()->getVisitFreq() < routeRelocIter->getRouteVes()->getCapacity())
				&& ( !routeRelocIter->isInstOnRoute( (visitIter->getOffshoreInst()) ) ) 
				&& (isDepartureSpreadEven(&(*routeIter), &(*routeRelocIter), &(*visitIter)) ) 
				&& (routeRelocIter->getVisitObjects().size() - 2 < routeRelocIter->getMaxVisits())
				/*&& (routeRelocIter->getNumOnesAfterRoute() <= 3)*/ )
				{
					
					schedCostBefore = computeSchedCost();
					/*
					cout << "Routes Before!!!" << endl;
					cout << "RouteFrom: " << endl;
					routeIter->printRoute();
					cout << "RouteTo: " << endl;
					routeRelocIter->printRoute();
					*/
					//printWeeklySchedule();

					routeToDurBefore = routeRelocIter->computeRouteDuration();
					routeDaysBefore = ceil( (routeToDurBefore + routeRelocIter->getRouteMinSlack() +
						routeRelocIter->getInstAtPos(0)->getLayTime())/24 ) ; 
					routeRelocIter->insertInstalVisit(1, *visitIter);
					routeRelocIter->updateVisitVector();
					routeRelocIter->intelligentReorder();
					routeToDurAfter = routeRelocIter->computeRouteDuration();
					routeDaysAfter = ceil( (routeToDurAfter + routeRelocIter->getRouteMinSlack() +
							routeRelocIter->getInstAtPos(0)->getLayTime())/24 );

					if ( (routeToDurAfter + routeRelocIter->getRouteMinSlack() < routeRelocIter->MAX_ROUTE_DUR + routeRelocIter->getRouteAcceptanceTime() )
						&& ( (routeDaysBefore >= routeDaysAfter) || (routeRelocIter->getNumOnesAfterRoute() >= 1) ) )
					{	
						routeFromDurBefore = routeIter->computeRouteDuration();
						routeFromDaysBefore = ceil ( (routeFromDurBefore + routeIter->getRouteMinSlack()
							+ routeIter->getInstAtPos(0)->getLayTime() )/24);
						routeIter->deleteInstalVisit( visitIter->getIdNumber() );
						routeIter->updateVisitVector();
						routeIter->intelligentReorder();
						routeFromDurAfter = routeIter->computeRouteDuration();
						routeFromDaysAfter = ceil ( (routeFromDurAfter + routeIter->getRouteMinSlack()
							+ routeIter->getInstAtPos(0)->getLayTime() )/24);

						if (routeFromDurAfter > routeIter->MIN_ROUTE_DUR + routeIter->getRouteAcceptanceTime())
						{

							schedCostAfter = computeSchedCost();
							/*
							cout << "Routes AFTER!!!" << endl;
							cout << "RouteFrom: " << endl;
							routeIter->printRoute();
							cout << "RouteTo: " << endl;
							routeRelocIter->printRoute();
							*/
							//printWeeklySchedule();
							
							//Initialize VisitVariation objects
							VisitVariation visVar(&(*visitIter));
							visVar.setRouteFrom(&(*routeIter));
							visVar.setRouteTo(&(*routeRelocIter));
							visVar.setRouteFromDurDecrease(routeFromDurBefore - routeFromDurAfter);
							visVar.setRouteToDurIncrease(routeToDurAfter - routeToDurBefore);
							visVar.setRouteDurationDelta(visVar.getRouteToDurIncrease() - visVar.getRouteFromDurDecrease());
							visVar.setDeltaObj(schedCostAfter - schedCostBefore);
							visVar.setRouteDaysDelta(routeDaysAfter - routeDaysBefore);
							visVar.setRouteFromDaysDelta(routeFromDaysAfter - routeFromDaysBefore);
							visVar.setVisitIndex(visitIter->getIdNumber());
							visVar.setVarVisit(&(*visitIter));

							visitIter->VisVars.push_back(visVar);
							VisVariations.push_back(visVar);
						}
						//Restore affected routes.
						for (routeIntermedIter = intermedRoutes.begin(); 
							routeIntermedIter->getRouteNum() !=  routeIter->getRouteNum();
							++routeIntermedIter)
							;
						*routeIter = *routeIntermedIter;

						//routeIter->setVisitIds();
								
					}
					
					for (routeIntermedIter = intermedRoutes.begin(); 
						routeIntermedIter->getRouteNum() !=  routeRelocIter->getRouteNum();
						++routeIntermedIter)
						;
					*routeRelocIter = *routeIntermedIter;

					//routeRelocIter->setVisitIds();
					
				}//end if
					
				k++;
			}//end for routeRelocIter
			j++;	
		}//end for visitIter
		i++;
	}//end for routeIter
	
	bestDeltaObj = 0; 
	bestRouDurDelta = numeric_limits<double>::max(); 
	bestRouteDaysDelta = 1; 
	bestRouteFromDaysDelta = 0;
	bestVisVarSize = numeric_limits<int>::max();
	bestVarIdx = 0;
	
	l = 0;
	//cout << "VisVariations.size() = " << VisVariations.size() << endl;

	//Update best Variation here
	if (VisVariations.size() > 0)
	{
		for (l = 0; l < VisVariations.size(); l++)
		{
			/*
			if ( (VisVariations[l].getRouteFrom()->getVisitAtPos(VisVariations[l].getVisitIndex()).VisVars.size() < bestVisVarSize)
				&& (VisVariations[l].getDeltaObj() < bestDeltaObj ) )
			{
				bestVarIdx = l;
				bestVisVarSize = VisVariations[l].getRouteFrom()->getVisitAtPos(VisVariations[l].getVisitIndex()).VisVars.size();
				bestRouteFromDaysDelta = VisVariations[l].getRouteFromDaysDelta();
				bestRouteDaysDelta = VisVariations[l].getRouteDaysDelta();
				bestRouDurDelta = VisVariations[l].getRouteDurationDelta();
				bestDeltaObj = VisVariations[l].getDeltaObj();
			}
			else if ( (VisVariations[l].getRouteFrom()->getVisitAtPos(VisVariations[l].getVisitIndex()).VisVars.size() == bestVisVarSize)
				&& (VisVariations[l].getDeltaObj() < bestDeltaObj ) )
			{
				bestVarIdx = l;
				bestVisVarSize = VisVariations[l].getRouteFrom()->getVisitAtPos(VisVariations[l].getVisitIndex()).VisVars.size();
				bestRouteFromDaysDelta = VisVariations[l].getRouteFromDaysDelta();
				bestRouteDaysDelta = VisVariations[l].getRouteDaysDelta();
				bestRouDurDelta = VisVariations[l].getRouteDurationDelta();
				bestDeltaObj = VisVariations[l].getDeltaObj();	
			}

			*/

			if (VisVariations[l].getDeltaObj() < bestDeltaObj)
			{
				bestVarIdx = l;
				bestVisVarSize = VisVariations[l].getRouteFrom()->getVisitAtPos(VisVariations[l].getVisitIndex()).VisVars.size();
				bestRouteFromDaysDelta = VisVariations[l].getRouteFromDaysDelta();
				bestRouteDaysDelta = VisVariations[l].getRouteDaysDelta();
				bestRouDurDelta = VisVariations[l].getRouteDurationDelta();
				bestDeltaObj = VisVariations[l].getDeltaObj();	
			}

			//First-accept
			//if (bestDeltaObj < 0 )
			//	break;
				

			l++;
		}//end for

		//cout << "After for" << endl;
		if (VisVariations[bestVarIdx].getDeltaObj() < 0)
			improvementFound = true;

		if (improvementFound) // Implement the best relocation
		{
			/*
			cout << "bestVarIdx = "<< bestVarIdx << endl;

			cout << "Relocating Installation " << VisVariations[bestVarIdx].getRouteFrom()->getVisitAtPos(VisVariations[bestVarIdx].getVisitIndex() ).getOffshoreInst()->getInstName() << endl;
			cout << "VisVariations[bestVarIdx].getVisitIndex() = " << VisVariations[bestVarIdx].getVisitIndex() << endl;
			cout << "Best insertion from route " << VisVariations[bestVarIdx].getRouteFrom()->getRouteStartTime() 
				<< " to route " << VisVariations[bestVarIdx].getRouteTo()->getRouteStartTime() << endl;

			cout << "DeltaObj = " << VisVariations[bestVarIdx].getDeltaObj() << endl;
			cout << "RouteFromDurDecrease = " << VisVariations[bestVarIdx].getRouteFromDurDecrease() << endl;
			cout << "RouteToDurIncrease = " << VisVariations[bestVarIdx].getRouteToDurIncrease() << endl;

			cout << "Printing the routes BEFORE insertions" << endl << endl;

			VisVariations[bestVarIdx].getRouteFrom()->printRoute();
			VisVariations[bestVarIdx].getRouteTo()->printRoute();
			*/

			//Update VesAvail for RouteTo	
			if (VisVariations[bestVarIdx].getRouteDaysDelta() > 0)
			{
				//cout << "AAA" << endl;
				routeEndIndex = (int) ceil( (VisVariations[bestVarIdx].getRouteTo()->getRouteEndTime() 
				+ VisVariations[bestVarIdx].getRouteTo()->getRouteMinSlack()
				- VisVariations[bestVarIdx].getRouteTo()->getInstAtPos(0)->getLayTime() )/24 ); 

				for (l = 1; l <= VisVariations[bestVarIdx].getRouteDaysDelta(); l++)
					VesAvail[VisVariations[bestVarIdx].getRouteTo()->getRouteVes()->getID()]
							[routeEndIndex + l] = false;

				//Decrement numOnesAfterRoute
				l = VisVariations[bestVarIdx].getRouteTo()->getNumOnesAfterRoute();
				VisVariations[bestVarIdx].getRouteTo()->setNumOnesAfterRoute(l - 1);
			}

			//Update VesAvail for RouteFrom
			if (VisVariations[bestVarIdx].getRouteFromDaysDelta() < 0)
			{
				//cout << "BBB" << endl;
				routeEndIndex = (int) ceil( (VisVariations[bestVarIdx].getRouteFrom()->getRouteEndTime() 
					+ VisVariations[bestVarIdx].getRouteTo()->getRouteMinSlack()
					- VisVariations[bestVarIdx].getRouteFrom()->getInstAtPos(0)->getLayTime() )/24 ); 

				VesAvail[VisVariations[bestVarIdx].getRouteFrom()->getRouteVes()->getID()]
					[routeEndIndex] = true;

				//Increment numOnesAfter routeFrom
				VisVariations[bestVarIdx].getRouteFrom()->setNumOnesAfterRoute(VisVariations[bestVarIdx].getRouteFrom()->getNumOnesAfterRoute() + 1);
			}

			//Update visit day combination
			updateVisDayComb(VisVariations[bestVarIdx].getRouteFrom(),
			VisVariations[bestVarIdx].getRouteTo(), 
			&VisVariations[bestVarIdx].getRouteFrom()->getVisitAtPos(VisVariations[bestVarIdx].getVisitIndex()) );

			//Implement best relocation
			VisVariations[bestVarIdx].getRouteTo()->insertInstalVisit(VisVariations[bestVarIdx].getVisitIndex(),
				VisVariations[bestVarIdx].getRouteFrom() );
			//cout << "Before implementing" << endl;
			VisVariations[bestVarIdx].getRouteFrom()->deleteInstalVisit( VisVariations[bestVarIdx].getVisitIndex() );

			
			VisVariations[bestVarIdx].getRouteTo()->updateVisitVector();
			VisVariations[bestVarIdx].getRouteFrom()->updateVisitVector();

			VisVariations[bestVarIdx].getRouteTo()->intelligentReorder();
			VisVariations[bestVarIdx].getRouteFrom()->intelligentReorder();

			VisVariations[bestVarIdx].getRouteTo()->setVisitIds();
			VisVariations[bestVarIdx].getRouteFrom()->setVisitIds();

			//j++;

			/*
			cout << endl;

			cout << "New visit day combination for the installation " << 
				VisVariations[bestVarIdx].getRouteFrom()->getVisitAtPos(VisVariations[bestVarIdx].getVisitIndex()).getOffshoreInst()->getInstName()
				<< " is ";
			for (l = 0; l < VisVariations[bestVarIdx].getRouteFrom()->getVisitAtPos(VisVariations[bestVarIdx].getVisitIndex()).getOffshoreInst()->CurVisitDayComb.size(); l++)
				cout << VisVariations[bestVarIdx].getRouteFrom()->getVisitAtPos(VisVariations[bestVarIdx].getVisitIndex()).getOffshoreInst()->CurVisitDayComb[l] << " ";

			cout << endl;
			
			cout << "Printing the routes after insertions" << endl << endl;

			
			VisVariations[bestVarIdx].getRouteFrom()->printRoute();
			VisVariations[bestVarIdx].getRouteTo()->printRoute();
			*/

			i = 0;
			//Update Intermediate route vector
			for (routeIntermedIter = intermedRoutes.begin(); 
				routeIntermedIter->getRouteNum() !=  
				VisVariations[bestVarIdx].getRouteFrom()->getRouteNum();
				++routeIntermedIter)
					i++;
			*routeIntermedIter = *(VisVariations[bestVarIdx].getRouteFrom());

			for (routeIntermedIter = intermedRoutes.begin(); 
				routeIntermedIter->getRouteNum() !=  
				VisVariations[bestVarIdx].getRouteTo()->getRouteNum();
				++routeIntermedIter)
				;
			*routeIntermedIter = *(VisVariations[bestVarIdx].getRouteTo());
		}//end if (improvementFound)

		//If all visits that were relocated successfully
		//Update relevant data structures
		if (VisVariations[bestVarIdx].getRouteFrom()->getVisitObjects().size() == 2) //only base at the beginning and the end
		{
			cout << "In ReduceRouteDurTotDays erasing route number " << 
			VisVariations[bestVarIdx].getRouteFrom()->getRouteNum() << endl; 

			//Update the solution
			routeStartIndex = (int) ceil(VisVariations[bestVarIdx].getRouteFrom()->getRouteStartTime()/24);
			routeEndIndex = routeStartIndex + 1;

			for (l = routeStartIndex; l <= routeEndIndex; l++)
				VesAvail[routeIter->getRouteVes()->getID()][l] = true;

			//routeIntermedIter->getRouteVes()->eraseVesselRoute(routeIntermedIter->getRouteNum());
			intermedRoutes.erase(intermedRoutes.begin()+i);
			Routes.erase(Routes.begin()+i);

			savedRoutes.clear();
			savedRoutes.resize(Routes.size());
			for (l = 0; l < Routes.size(); l++) 
			{
				savedRoutes[l] = Routes[l];
			}

			//For each vessel clear and update vesselRoutes vector
			for (vesIter = SchedVessels.begin(); vesIter != SchedVessels.end(); ++vesIter)
					{
						if (vesIter->getVesselRoutes().size() > 0)
						{
							vesIter->clearVesselRoutes();
							for (routeIter = Routes.begin(); 
								routeIter != Routes.end(); ++routeIter)
								if (vesIter->getID() == routeIter->getRouteVes()->getID() )
									vesIter->addVesselRoute(&(*routeIter));
						}
						else
							vesIter->setIsVesselUsed(false);
					}//end for

			numRoutesReduced = true;
			break;
				
		}//end if (routeIter->getVisitObjects().size() == 2)
	}//end if (VisVariations.size() > 0)

//cout << "After relocateVisits() schedule cost = " << computeSchedCost() << endl;
}// end while (improvementFound)

//cout << "relocateVisits_END" << endl;
}


void WeeklySchedule::writeScheduleToFile(int iter, int LNSiter)
{
	const char* fileName;
	fileName = "C:/Aliaksandr Shyshou/Supply Vessels 2008-2010/Multi base/MongstadInst_MBPSVPP_Heuristic/output.txt";
	ofstream solFile(fileName, ios::app);

	solFile << "Iteration " << iter << ": LNS iteration " << LNSiter << endl;
	solFile << "Schedule cost = " << computeSchedCost() << endl << endl;

	solFile << "Number of routes = " << Routes.size() << endl;
	vector <Route>::iterator it;
	int i, k, l, totNumVis = 0;
	vector <OffshoreInst>::iterator instIter;

	solFile << "Vessel availability vector" << endl;

	for (k = 0; k < VesAvail.size(); k++)
	{
		for (i = 0; i < VesAvail[k].size(); i++)
			solFile << VesAvail[k][i] << " ";

		solFile << endl;
	}

	//Print visit day combinations
	solFile << "Installations: Visit day combinations" << endl;
	for (instIter = SchedInstals.begin(); instIter != SchedInstals.end(); ++instIter)
	{
		solFile << instIter->getInstName() << ": ";
		for (l = 0; l < instIter->CurVisitDayComb.size(); l++)
			solFile << instIter->CurVisitDayComb[l] << " ";

		solFile << endl;
	}

	vector < Visit >::iterator iter;
	vector <Visit> Visits;

	for (it = Routes.begin(); it != Routes.end(); ++it)
	{
		solFile << "____________" << it->getRouteVes()->getName() << " id = "
			<< it->getRouteVes()->getID() << "_____________" << endl;
		solFile << "Route number " << it->getRouteNum();
		it->getOnlyRoute() ? solFile << " Single " : solFile << " Multiple ";
		solFile << endl;
		solFile << "Route DEMAND = " << it->computeRouteDemand() << endl;
		Visits.clear();
		Visits = it->getVisitObjects();
		totNumVis+= (Visits.size() - 2);
		
		for (iter = Visits.begin(); iter != Visits.end(); ++iter)
		{
			solFile << iter->getOffshoreInst()->getInstName() << " ";
			solFile << "Arrival = " << iter->getVisitStart() << " ";
			solFile << "Departure = " << iter->getVisitEnd() << " ";
			solFile << "WaitTime = " << iter->getVisitWaitTime();
			solFile << endl;
		}

	solFile << endl;
	}

	solFile << "TOTAL VISITS = " << totNumVis << endl;
}

void WeeklySchedule::minimizeTotalSlack()
{
	vector <Route>::iterator routeIter;
	vector <Route>::iterator routeRelocIter;
	vector <Route>::iterator routeIntermedIter;

	vector <Visit> routeVisits;
	vector <Visit>::iterator visitIter;

	vector <SupVes>::iterator vesIter;

	int i, j, k, l;
	int routeStartIndex, routeEndIndex;

	vector <Route> intermedRoutes; //additional vector of routes
	vector <Route> savedRoutes; //to restore routes if infeasible
	intermedRoutes.resize(Routes.size());
	savedRoutes.resize(Routes.size());

	vector < vector <int> > VisCombs;

	for (l = 0; l < Routes.size(); l++)
	{
		intermedRoutes[l] = Routes[l];
		savedRoutes[l] = Routes[l];
	}

	double routeToSlackBefore, routeToSlackAfter;
	double routeFromSlackBefore, routeFromSlackAfter;
	double totalSlackBefore, totalSlackAfter;
	double schedCostBefore, schedCostAfter;

	double routeToDurBefore, routeToDurAfter;
	double routeFromDurBefore, routeFromDurAfter;

	double bestDeltaObj, bestDeltaSlack;
	int bestVarIdx;

	int routeDays;

	bool improvementFound = true;
	bool numRoutesReduced;

	vector <vector <bool> > vesAvailability; //local vessel availability vector
	vesAvailability.resize(VesAvail.size());
	int planHorizon = NUMBER_OF_DAYS + (int) ceil(
			( (double)Routes.begin()->MAX_ROUTE_DUR 
			+ Routes.begin()->getInstAtPos(0)->getLayTime() 
			+ Routes.begin()->getRouteMinSlack() )/24);

	for (k = 0; k < VesAvail.size(); k++)
		{
			vesAvailability[k].resize(planHorizon);
			for (i = 0; i < planHorizon; i++)
				vesAvailability[k][i] = VesAvail[k][i];
		}

	//cout << "BEFORE minimizeTotalSlack()" << endl;
	//cout << "ScheduleCost = " << computeSchedCost() << endl;
	//cout << "Total slack = " << computeTotalSlack() << endl;

while (improvementFound)
{
	//cout << "Routes.size() = " << Routes.size() << endl;
	//cout << "intermedRoutes.size() = " << intermedRoutes.size() << endl;
	//printWeeklySchedule();

	improvementFound = false;
	numRoutesReduced = false;
	
	for (routeIter = Routes.begin(); routeIter != Routes.end(); ++routeIter)
	{//Try to relocate (some of the) route visits
	
		//cout << "routeIter->getRouteNumber() = " << routeIter->getRouteNum() << endl;

		
		routeVisits.clear();
		routeVisits = routeIter->getVisitObjects();
		VisCombs.clear();
		VisCombs.resize(routeIter->getVisitObjects().size());
	
		j=1;
		routeDays = (int) ceil( (routeIter->computeRouteDuration() + 
						routeIter->getInstAtPos(0)->getLayTime() +
						routeIter->getRouteMinSlack() )/24 );

		for (visitIter = routeVisits.begin()+1; visitIter != routeVisits.end()-1; ++visitIter)
		{
			
			VisVariations.clear();
			
			bestDeltaObj = numeric_limits<double>::max();
			bestDeltaSlack = numeric_limits<double>::max();
	

			for (routeRelocIter = Routes.begin(); routeRelocIter != Routes.end(); ++routeRelocIter)
			{
				//do not reinsert in the same route, check load feasibility
				//if the installation is already on the route and even spread
				//cout << "routeRelocIter->getRouteNumber = " << routeRelocIter->getRouteNum() << endl;

				
				if ( (routeIter != routeRelocIter)
				&& (routeRelocIter->computeRouteDemand() + visitIter->getOffshoreInst()->getWeeklyDemand()
				/ visitIter->getOffshoreInst()->getVisitFreq() < routeRelocIter->getRouteVes()->getCapacity())
				&& ( !routeRelocIter->isInstOnRoute( (visitIter->getOffshoreInst()) ) ) 
				&& (isDepartureSpreadEven(&(*routeIter), &(*routeRelocIter), &(*visitIter)) ) 
				&& (routeRelocIter->getVisitObjects().size() - 2 < routeRelocIter->getMaxVisits()) )
				{
					//printWeeklySchedule();
					schedCostBefore = computeSchedCost();
					totalSlackBefore = computeTotalSlack();

					routeToDurBefore = routeRelocIter->computeRouteDuration();
					routeToSlackBefore = routeRelocIter->computeRouteSlack();

					routeRelocIter->insertInstalVisit(1, *visitIter);
					routeRelocIter->updateVisitVector();
					routeRelocIter->intelligentReorder();
					routeToDurAfter = routeRelocIter->computeRouteDuration();
					routeToSlackAfter = routeRelocIter->computeRouteSlack();

					if ( (routeToDurAfter < routeRelocIter->MAX_ROUTE_DUR + routeRelocIter->getRouteAcceptanceTime())
						&& ( ceil( (routeToDurBefore + routeRelocIter->getRouteMinSlack() +
						routeRelocIter->getInstAtPos(0)->getLayTime())/24 )
							>= ceil( (routeToDurAfter + routeRelocIter->getRouteMinSlack() +
							routeRelocIter->getInstAtPos(0)->getLayTime())/24 ) ) )
					{	
						routeFromDurBefore = routeIter->computeRouteDuration();
						routeFromSlackBefore = routeIter->computeRouteSlack();
						routeIter->deleteInstalVisit( j );
						routeIter->updateVisitVector();
						routeIter->intelligentReorder();
						routeFromDurAfter = routeIter->computeRouteDuration();
						routeFromSlackAfter = routeIter->computeRouteSlack();

						if ( (routeFromDurAfter > routeIter->MIN_ROUTE_DUR + 
							routeIter->getRouteAcceptanceTime() ) && 
							( ceil( (routeFromDurBefore + routeRelocIter->getRouteMinSlack() +
						routeRelocIter->getInstAtPos(0)->getLayTime())/24 )
							> ceil( (routeFromDurAfter + routeRelocIter->getRouteMinSlack() +
							routeRelocIter->getInstAtPos(0)->getLayTime())/24 ) ) )
						{
							schedCostAfter = computeSchedCost();
							totalSlackAfter = computeTotalSlack();
							
							//cout << "Sched After!!!" << endl;
							//printWeeklySchedule();
							
							//Initialize VisitVariation objects
							VisitVariation visVar(&(*visitIter));
							visVar.setRouteFrom(&(*routeIter));
							visVar.setRouteTo(&(*routeRelocIter));
							visVar.setRouteFromDurDecrease(routeFromDurBefore - routeFromDurAfter);
							visVar.setRouteToDurIncrease(routeToDurAfter - routeToDurBefore);
							visVar.setTotalSlackDelta(totalSlackAfter - totalSlackBefore);
							visVar.setDeltaObj(schedCostAfter - schedCostBefore);

							VisVariations.push_back(visVar);
						}

						//Restore affected routes.
						for (routeIntermedIter = intermedRoutes.begin(); 
							routeIntermedIter->getRouteNum() !=  routeIter->getRouteNum();
							++routeIntermedIter)
							;
						*routeIter = *routeIntermedIter;
					}//end if
					
					for (routeIntermedIter = intermedRoutes.begin(); 
						routeIntermedIter->getRouteNum() !=  routeRelocIter->getRouteNum();
						++routeIntermedIter)
						;
					*routeRelocIter = *routeIntermedIter;
					
				}//end if
					
			}//end for routeRelocIter
			
			//No feasible insertion found - go on to the next routeIter
			if (VisVariations.size() == 0)
			{
				//cout << "No insertion found" << endl;
				j++;
				continue; // continue to the next visitIter
			}
			else // Identify best relocation (in terms of obj. function)
			{
				for (l = 0; l < VisVariations.size(); l++)
					if (VisVariations[l].getTotalSlackDelta() < bestDeltaSlack)
					{
						bestDeltaSlack = VisVariations[l].getTotalSlackDelta();
						bestVarIdx = l;
						//cout << "Best DeltaObj = " << bestDeltaObj << endl;
					}
				
			
				/*
				cout << "Relocating Installation " << VisVariations[bestVarIdx].getVarVisit()->getOffshoreInst()->getInstName() << endl;
				
				cout << "OLD visit day combination for the installation " << visitIter->getOffshoreInst()->getInstName()
					<< " is ";
				for (l = 0; l < visitIter->getOffshoreInst()->CurVisitDayComb.size(); l++)
					cout << visitIter->getOffshoreInst()->CurVisitDayComb[l] << " ";

				cout << endl;

				cout << "Best insertion from route " << VisVariations[bestVarIdx].getRouteFrom()->getRouteStartTime() 
					<< " to route " << VisVariations[bestVarIdx].getRouteTo()->getRouteStartTime() << endl;

				cout << "DeltaObj = " << VisVariations[bestVarIdx].getDeltaObj() << endl;
				cout << "RouteFromDurDecrease = " << VisVariations[bestVarIdx].getRouteFromDurDecrease() << endl;
				cout << "RouteToDurIncrease = " << VisVariations[bestVarIdx].getRouteToDurIncrease() << endl;

				cout << "Position of visit on the route is " << visitIter->getIdNumber() << endl;
				cout << "Printing the routes BEFORE insertions" << endl << endl;

				VisVariations[bestVarIdx].getRouteFrom()->printRoute();
				VisVariations[bestVarIdx].getRouteTo()->printRoute();

				*/

				//Implement best relocation if it reduces the cost
				//cout << "Improvement in relocateVisits()" << endl;
				improvementFound = true;

				VisVariations[bestVarIdx].getRouteTo()->insertInstalVisit(1, *visitIter);
				VisVariations[bestVarIdx].getRouteFrom()->deleteInstalVisit( j );

				VisVariations[bestVarIdx].getRouteTo()->updateVisitVector();
				VisVariations[bestVarIdx].getRouteFrom()->updateVisitVector();

				VisVariations[bestVarIdx].getRouteTo()->intelligentReorder();
				VisVariations[bestVarIdx].getRouteFrom()->intelligentReorder();

				//j++;

				//Update visit day combination
				updateVisDayComb(VisVariations[bestVarIdx].getRouteFrom(),
				VisVariations[bestVarIdx].getRouteTo(), &(*visitIter) );
				/*
				cout << endl;
				cout << "New visit day combination for the installation " << visitIter->getOffshoreInst()->getInstName()
					<< " is ";
				for (l = 0; l < visitIter->getOffshoreInst()->CurVisitDayComb.size(); l++)
					cout << visitIter->getOffshoreInst()->CurVisitDayComb[l] << " ";

				cout << endl;

				cout << "Printing the routes after insertions" << endl << endl;
				
				VisVariations[bestVarIdx].getRouteFrom()->printRoute();
				VisVariations[bestVarIdx].getRouteTo()->printRoute();
				*/

				//Set visit sequential numbers
				VisVariations[bestVarIdx].getRouteFrom()->setVisitIds();
				VisVariations[bestVarIdx].getRouteTo()->setVisitIds();
				

				//Update intermediate route vectors
				for (routeIntermedIter = intermedRoutes.begin(); 
					routeIntermedIter->getRouteNum() !=  
					VisVariations[bestVarIdx].getRouteFrom()->getRouteNum();
					++routeIntermedIter)
						;
				*routeIntermedIter = *(VisVariations[bestVarIdx].getRouteFrom());

				for (routeIntermedIter = intermedRoutes.begin(); 
					routeIntermedIter->getRouteNum() !=  
					VisVariations[bestVarIdx].getRouteTo()->getRouteNum();
					++routeIntermedIter)
					;
				*routeIntermedIter = *(VisVariations[bestVarIdx].getRouteTo());

				//Check if we could reduce the number of routes
				if (VisVariations[bestVarIdx].getRouteFrom()->getVisitObjects().size() == 2)
				{
					cout << "Erasing route number " << 
						VisVariations[bestVarIdx].getRouteFrom()->getRouteNum() 
						<< endl; 

					i = 0;
					//Update the solution
					for (routeIntermedIter = intermedRoutes.begin(); 
						routeIntermedIter->getRouteNum() !=  
						VisVariations[bestVarIdx].getRouteFrom()->getRouteNum();
						++routeIntermedIter)
						i++;

					routeStartIndex = (int) ceil(routeIntermedIter->getRouteStartTime()/24);

					routeEndIndex = routeStartIndex + routeDays - 1;
					

					for (l = routeStartIndex; l <= routeEndIndex; l++)
						VesAvail[routeIntermedIter->getRouteVes()->getID()][l]=true;

					//routeIntermedIter->getRouteVes()->eraseVesselRoute(routeIntermedIter->getRouteNum());
					intermedRoutes.erase(intermedRoutes.begin()+i);
					Routes.erase(Routes.begin()+i);
					
					numRoutesReduced = true;

					//reassignVesselsToRoutes();
					
				}
				//cout << "Before break 1" << endl;
				else if ( (int)ceil( (routeIter->computeRouteDuration() + routeIter->getInstAtPos(0)->getLayTime()
						+ routeIter->getRouteMinSlack() )/24) < routeDays )
				{
					/*
					cout << "Improvement in reduceRouteDurTotDays()" << endl;
					cout << "Old route days = " << routeDays << endl;
					cout << "New route days = " << (int)ceil( (routeIter->computeRouteDuration() + routeIter->getInstAtPos(0)->getLayTime()
					+ routeIter->getRouteMinSlack() )/24) << endl;
					cout << "Route number = " << routeIter->getRouteNum() << endl;
					cout << "VESSEL: "<< routeIter->getRouteVes()->getName() << endl;
					routeIter->printRoute();
					*/

					//Update the solution
					routeStartIndex = (int) ceil(routeIter->getRouteStartTime()/24);
					routeEndIndex = (int) ceil ((routeIter->getRouteEndTime() - routeIter->getInstAtPos(0)->getLayTime() 
						+ routeIter->getRouteMinSlack() )/24);
				
					VesAvail[routeIter->getRouteVes()->getID()][routeEndIndex+1]=true;
				}//end else if

				break;
			}//end else
		}//end for visitIter

		if (numRoutesReduced)
		{
			//cout << "before break 2" << endl;
			break;	
		}
	}//end for routeIter

	if (numRoutesReduced)
	{
		//For each vessel clear and update vesselRoutes vector
		for (vesIter = SchedVessels.begin(); vesIter != SchedVessels.end(); ++vesIter)
				{
					if (vesIter->getVesselRoutes().size() > 0)
					{
						vesIter->clearVesselRoutes();
						for (routeIter = Routes.begin(); 
							routeIter != Routes.end(); ++routeIter)
							if (vesIter->getID() == routeIter->getRouteVes()->getID() )
								vesIter->addVesselRoute(&(*routeIter));
					}
					else
						vesIter->setIsVesselUsed(false);
				}//end for
	}//end if

}//end while (improvementFound)

//cout << "After minimizeTotalSlack()" << endl;
//cout << "ScheduleCost = " << computeSchedCost()<<endl;
//cout << "Total slack = " << computeTotalSlack() << endl << endl;

}

void WeeklySchedule::goLNS(int iter, double *curBestCost, int curRestart,WeeklySchedule *bs,
						   vector <WeeklySchedule>* trialScheds, double schedMargin,
						    vector <double>* cost, vector <double>* startTime,
							vector <double>* vesselCap, vector <int>* numVessels)
{
	int i, j, k, l, m;
	WeeklySchedule bestSched;
	int numVisitsRemove, numRoutesRemove;
	double schedCost;
	static double bestSchedCost;
	bestSchedCost = *curBestCost;
	
	vector <Route>::iterator routeIter;
	vector <Route>::iterator routeRelocIter;
	vector <Route>::iterator routeIntermedIter;
	vector <SupVes>::iterator vesIter;

	vector <Route> intermedRoutes;
	vector <Route> savedRoutes;

	vector <Visit> routeVisits;
	
	vector <Visit>::iterator visitIter;

	int routeIdx, visitIdx;
	int routeStartIdx, routeEndIdx;
	int routeDays, newRouteDays;

	double smallestInsertionCost, largestRegret;

	intermedRoutes.resize(Routes.size());
	savedRoutes.resize(Routes.size());

	for (k=0; k < Routes.size(); k++)
	{
		intermedRoutes[k] = Routes[k];
		savedRoutes[k] = Routes[k];
	}

	bool insertionPossible;
	bool insertionOverlaps = false;
	double localCostBefore, localCostAfter;
	
	double routeToDurBefore, routeToDurAfter;
	double schedCostBefore, schedCostAfter;
	int routeDaysBefore, routeDaysAfter, daysDelta, bestRouteDaysDelta;

	double routeSlackBefore, routeSlackAfter;
	//int numRoutesBefore, numRoutesAfter;
	int bestIdxRegret, bestIdxInsert;
	double totNumZeroes, numVesselsUsed;

	bool reduceNumVesselsSuccessful = false;
	int LB = computeLB_numRoutes();

	/*
	int planHorizon = NUMBER_OF_DAYS + (int) ceil(
			( (double)Routes.begin()->MAX_ROUTE_DUR 
			+ Routes.begin()->getInstAtPos(0)->getLayTime() 
			+ Routes.begin()->getRouteMinSlack() )/24);
	*/

	if (SchedInstals.size() <= 6)
		numRoutesRemove = 1;
	else
		numRoutesRemove = 2;

	bool isolDayFound;

	int randCounter;
	bool cycle;
	
	//writeScheduleToFile();
	//Do LNS iterations
	for (i = 0; i < iter; i++)
	{
		//printWeeklySchedule();

		//cout << "LNS Iteration " << i << endl;
		//cout << "Before Deletions" << endl;
		//printWeeklySchedule();
		
		
		/*
		if (i % 50 == 0) 
		{
			cout << "LNS iter " << i << endl;
			cout << "removedVisits.size() = " << removedVisits.size() << endl;
			for (visitIter = removedVisits.begin(); visitIter != removedVisits.end(); ++visitIter)
				cout << "Visit->installation = " << visitIter->getOffshoreInst()->getInstName() << endl;

			printWeeklySchedule();
		//}*/
		

		insertionPossible = true;

		
		//numRoutesRemove = rand() % (Routes.size() - Routes.size()/4) + 1;

		l = 0; m = 0;
		for (j = 0; j < numRoutesRemove; j++)
		{
			/*m = l;
			for (k = l; k < Routes.size(); k ++)
				if (!Routes[k].isRouteDurFeasible() )
				{
					l = k + 1;
					break;
				}

			if ( l > m)
				routeIdx = k;
			else
			{*/
				routeIdx = rand() % Routes.size();

				randCounter = 0;
				cycle = false;

			while (Routes[routeIdx].getVisitObjects().size() <= 3)
			{
				routeIdx = rand() % Routes.size();
				randCounter++;
				if (randCounter > 50)
				{
					cycle = true;
					break;
				}
				//cout << "Are we here" << endl;
			}
					
			//}

			if (cycle)
				break; //out of for j
	
			//cout << "yep" << endl;

			routeDays = (int)ceil( (Routes[routeIdx].computeRouteDuration()+ 
				Routes[routeIdx].getInstAtPos(0)->getLayTime()
				+ Routes[routeIdx].getRouteMinSlack() )/24);

			routeStartIdx = (int) ceil(Routes[routeIdx].getRouteStartTime()/24);
			routeEndIdx = routeStartIdx + routeDays - 1;

			numVisitsRemove = (rand() % ( Routes[routeIdx].getVisitObjects().size() - 2 - 1 ) ) + 1;
			//numVisitsRemove = 1;
			visitIdx = (rand() % (Routes[routeIdx].getVisitObjects().size() - 2) ) + 1;

			//cout << "Erasing from route " << routeIdx << endl;
			//cout << "numVisitsRemove = " << numVisitsRemove << endl;
			//cout << "visitIdx = " << visitIdx << endl;
			if (numVisitsRemove == (Routes[routeIdx].getVisitObjects().size() - 2))
			{
				//cout << "We get here" << endl;
				for (k = 0; k < numVisitsRemove; k++)
				{
					removedVisits.push_back(Routes[routeIdx].getVisitAtPos(visitIdx));
					Routes[routeIdx].getVisitAtPos(visitIdx).getOffshoreInst()->
						RemoveDayFromVisDayComb( (int) ceil(Routes[routeIdx].getRouteStartTime()/24) );
					Routes[routeIdx].deleteInstalVisit(visitIdx);

					if (visitIdx == (Routes[routeIdx].getVisitObjects().size() - 1) )
						visitIdx = 1;
				}

				Routes[routeIdx].updateVisitVector();

				for (k = routeStartIdx + 1; k <= routeEndIdx; k++)
					VesAvail[Routes[routeIdx].getRouteVes()->getID()][k] = true;

				//Update intermedRoutes
				for (routeIntermedIter = intermedRoutes.begin(); 
							routeIntermedIter->getRouteNum() != 
							Routes[routeIdx].getRouteNum();
							++routeIntermedIter)
							;
				*routeIntermedIter = Routes[routeIdx];

				//Keep empty routes for possible insertions

				//Routes.erase(Routes.begin() + routeIdx);
				//intermedRoutes.erase(intermedRoutes.begin() + routeIdx);

				//For each vessel clear and update vesselRoutes vector
				/*
				for (vesIter = SchedVessels.begin(); vesIter != SchedVessels.end(); ++vesIter)
				{
					if (vesIter->getVesselRoutes().size() > 0)
					{
						vesIter->clearVesselRoutes();
						for (routeIter = Routes.begin(); 
							routeIter != Routes.end(); ++routeIter)
							if (vesIter->getID() == routeIter->getRouteVes()->getID() )
								vesIter->addVesselRoute(&(*routeIter));
					}
					else
						vesIter->setIsVesselUsed(false);
				}//end for
				*/
			}
			else
			{
				//Delete numVisitsRemove visits starting from visitIdx 
				//cout<< "before removal" << endl;
				//Routes[routeIdx].printRoute();
				
				for (k = 0; k < numVisitsRemove; k++)
				{
					removedVisits.push_back(Routes[routeIdx].getVisitAtPos(visitIdx));
					Routes[routeIdx].getVisitAtPos(visitIdx).getOffshoreInst()->
						RemoveDayFromVisDayComb( (int) ceil(Routes[routeIdx].getRouteStartTime()/24) );
					
					Routes[routeIdx].deleteInstalVisit(visitIdx);
				

					if (visitIdx == (Routes[routeIdx].getVisitObjects().size() - 1) )
						visitIdx = 1;
				}
				
				Routes[routeIdx].intelligentReorder();
				//cout<< "after removal" << endl;
				//Routes[routeIdx].printRoute();

				newRouteDays = (int)ceil( (Routes[routeIdx].computeRouteDuration()+ 
				Routes[routeIdx].getInstAtPos(0)->getLayTime()
				+ Routes[routeIdx].getRouteMinSlack() )/24);
				
		
				for (k = routeStartIdx + newRouteDays; k <= routeEndIdx; k++)
					VesAvail[Routes[routeIdx].getRouteVes()->getID()][k] = true;

				//Update intermedRoutes
				for (routeIntermedIter = intermedRoutes.begin(); 
							routeIntermedIter->getRouteNum() != 
							Routes[routeIdx].getRouteNum();
							++routeIntermedIter)
							;
				*routeIntermedIter = Routes[routeIdx];


			}//end else
			
		}//end for j

		if (cycle)
			break; //out of for i
		/*
		cout << "After Deletions" << endl;
		//printWeeklySchedule();
		cout << "removedVisits.size() = " << removedVisits.size() << endl;
		for (visitIter = removedVisits.begin(); visitIter != removedVisits.end(); ++visitIter)
			cout << "Visit->installation = " << visitIter->getOffshoreInst()->getInstName() << endl;
		*/

		/*
		for (visitIter = removedVisits.begin(); visitIter != removedVisits.end(); ++visitIter)
		{
			cout << "Visit->installation = " << visitIter->getOffshoreInst()->getInstName() << endl;
		}
		*/

		//Create extra voyages here
		
		if (reduceNumVesselsSuccessful)
		{
			//cout << "inside if" << endl;
			if (createVoyages() )
				reduceNumVesselsSuccessful = false;

			//cout << "after createVoyages" << endl;

			if (Routes.size() > intermedRoutes.size())
			{
				intermedRoutes.clear();
				intermedRoutes.resize(Routes.size());

				for (l = 0; l < Routes.size(); l++)
					intermedRoutes[l] = Routes[l];
			}
		}

		insertionPossible = true;

		//cout << "before insertion loop" << endl;

		while ( insertionPossible && (removedVisits.size() > 0) )
		{
			for (visitIter = removedVisits.begin(); visitIter != removedVisits.end(); ++visitIter)
			{
				visitIter->VisVars.clear();
				visitIter->RegretValues.clear();
				VisitVariation exchVar(&(*visitIter));

				//cout << "Let's see" << endl;

				//Evaluate and sort all possible insertions for a given visit
				for (routeIter = Routes.begin(); routeIter != Routes.end(); ++routeIter)
				{
					routeDays = (int)ceil( (routeIter->computeRouteDuration()+ 
					routeIter->getInstAtPos(0)->getLayTime()
					+ routeIter->getRouteMinSlack() )/24);

					routeStartIdx = (int) ceil(routeIter->getRouteStartTime()/24);
					routeEndIdx = routeStartIdx + routeDays - 1;

					if ( (routeIter->computeRouteDemand() + 
						visitIter->getOffshoreInst()->getWeeklyDemand()/visitIter->getOffshoreInst()->getVisitFreq()
						< routeIter->getRouteVes()->getCapacity()) &&  
						(routeIter->getVisitObjects().size() - 2 < routeIter->getMaxVisits() ) &&
						!routeIter->isInstOnRoute(visitIter->getOffshoreInst()) &&
						isDepartureSpreadEven(&(*routeIter),&(*visitIter)) )
					{
						//Store the pre-insertion info
						routeToDurBefore = routeIter->computeRouteDuration();
						routeDaysBefore = (int)ceil( (routeIter->computeRouteDuration()+ 
							routeIter->getInstAtPos(0)->getLayTime()
							+ routeIter->getRouteMinSlack() )/24);
						routeSlackBefore = routeIter->computeRouteSlack();
						schedCostBefore = computeSchedCost();
						//numRoutesBefore = Routes.size();

						//Do tentative insertion
						routeIter->insertInstalVisit(1, *visitIter);
						routeIter->updateVisitVector();
						routeIter->intelligentReorder();
						routeToDurAfter = routeIter->computeRouteDuration();

						if (routeToDurAfter < routeIter->MAX_ROUTE_DUR + routeIter->getRouteAcceptanceTime())
						{				
							routeDaysAfter = (int)ceil( (routeIter->computeRouteDuration()+ 
							routeIter->getInstAtPos(0)->getLayTime()
							+ routeIter->getRouteMinSlack() )/24);

							daysDelta = routeDaysAfter - routeDaysBefore;

							if (daysDelta > 0)
								for (j = 1; j <= daysDelta; j++)
								{
									if (!VesAvail[routeIter->getRouteVes()->getID()]
									[routeEndIdx + j] )
									{
										insertionOverlaps = true;
										break;
									}

									if ( ( routeEndIdx + j > NUMBER_OF_DAYS) && 
										(!VesAvail[routeIter->getRouteVes()->getID()]
									[(routeEndIdx+j)%NUMBER_OF_DAYS]) )
									{
										insertionOverlaps = true;
										break;
									}
								}

							if (!insertionOverlaps)
							{

								schedCostAfter = computeSchedCost();
								routeSlackAfter = routeIter->computeRouteSlack();

								//Initialize VisitVariation objects
								VisitVariation visVar(&(*visitIter));
								visVar.setRouteTo(&(*routeIter));
								visVar.setRouteToDurIncrease(routeToDurAfter - routeToDurBefore);
								visVar.setDeltaObj(schedCostAfter - schedCostBefore);
								visVar.setRouteDaysDelta(routeDaysAfter - routeDaysBefore);
								visVar.setRouteSlackDelta(routeSlackAfter - routeSlackBefore);

								visitIter->VisVars.push_back(visVar);
							
							//Use auxiliary objective???
							/*Auxiliary objective:
							- Schedule cost
							- Number of routes
							- Number of route days
							- Total slack
							*/
							}//end if
							insertionOverlaps = false;
						}//end if

						//Restore the affected route
						for (routeIntermedIter = intermedRoutes.begin(); 
							routeIntermedIter->getRouteNum() !=  routeIter->getRouteNum();
							++routeIntermedIter)
							;
						*routeIter = *routeIntermedIter;

					}//end if
			
				}//end for routeIter

				//cout << "Aga" << endl;
				
				//Sort VisVars vector using insertion sort
				//Note - some may already be sorted from previous iterations
				if (visitIter->VisVars.size() > 0)
				{
					//cout << "visitIter->VisVars.size() = " << visitIter->VisVars.size()<< endl;
					for (j = 1; j < visitIter->VisVars.size(); j++)
					{
						exchVar = visitIter->VisVars[j]; 
						k = j;
						
							while ( ( (visitIter->VisVars[k-1].getDeltaObj() >= exchVar.getDeltaObj())
								&& (visitIter->VisVars[k-1].getRouteDaysDelta() >= exchVar.getRouteDaysDelta() )
								&& (visitIter->VisVars[k-1].getRouteTo()->getVisitObjects().size() > 2)
								&& (exchVar.getRouteTo()->getVisitObjects().size() > 2) )
								|| ( (visitIter->VisVars[k-1].getRouteTo()->getVisitObjects().size() == 2)
								&& (exchVar.getRouteTo()->getVisitObjects().size() > 2) )
								|| ( (visitIter->VisVars[k-1].getDeltaObj() >= exchVar.getDeltaObj())
								&& (visitIter->VisVars[k-1].getRouteDaysDelta() >= exchVar.getRouteDaysDelta() )
								&& (visitIter->VisVars[k-1].getRouteTo()->getVisitObjects().size() == 2)
								&& (exchVar.getRouteTo()->getVisitObjects().size() == 2) ) )
							{
								//cout << "k = " << k << endl;
								visitIter->VisVars[k] = visitIter->VisVars[k-1];
								k--;
								//cout << "Inside while" << endl;
								if (k == 0)
								{
									//cout << "before break" << endl;
									break;
								}
							}//end while
									
						//cout << "sorted" << endl;
						visitIter->VisVars[k] = exchVar;
						
					}//end for j

					/*
					cout << "VisVars for " << visitIter->getOffshoreInst()->getInstName() << endl;
					for (j = 0; j < visitIter->VisVars.size(); j++)
					{
						cout << "\tRouteTo # of visits = " << visitIter->VisVars[j].getRouteTo()->getVisitObjects().size(); 
						
						cout << "  DeltaObj = " << visitIter->VisVars[j].getDeltaObj() << endl;
					}
					cout << endl;
					*/

					//Initialize regret vector
					if (visitIter->VisVars.size() == 1)
					{
						visitIter->RegretValues.resize(1);
						visitIter->RegretValues[0] = visitIter->VisVars[0].getDeltaObj();
					}
					else
					{
						visitIter->RegretValues.resize(visitIter->VisVars.size() - 1);
						for (j = 1; j < visitIter->VisVars.size(); j++)
						{
							visitIter->RegretValues[j-1] = visitIter->VisVars[j].getDeltaObj() - 
							visitIter->VisVars[j-1].getDeltaObj();
						}
					}
				}//end if (visitIter->VisVars.size() > 0)
			
			}//end for visitIter

			//cout << "Before Actual Insertion" << endl;
			//printWeeklySchedule();
			//cout << endl << endl;
			/*
				for (visitIter = removedVisits.begin(); visitIter != removedVisits.end(); ++visitIter)
				{
					cout << "Visit->installation = " << visitIter->getOffshoreInst()->getInstName() << endl;
					cout << "VisVars.size() = " << visitIter->VisVars.size() << endl;
					if (visitIter->VisVars.size() > 0)
						cout << "RegretValues[0] = " << visitIter->RegretValues[0] << endl;
				}
				*/


			smallestInsertionCost = numeric_limits<double>::max();
			largestRegret = 0;

			j = 0;
			bestIdxInsert = 0;
			bestIdxRegret = 0;
			bestRouteDaysDelta = 1;

			//Identify the insertion
			for (visitIter = removedVisits.begin(); visitIter != removedVisits.end(); ++visitIter)
			{
				
				if ( (visitIter->VisVars.size() == 1) 
					&& (visitIter->VisVars[0].getRouteDaysDelta() <= bestRouteDaysDelta )
					&& (visitIter->RegretValues[0] <= smallestInsertionCost) )
				{
					
					smallestInsertionCost = visitIter->RegretValues[0];
					bestIdxInsert = j;
					bestRouteDaysDelta = visitIter->VisVars[0].getRouteDaysDelta();
				}
				else if ( (visitIter->VisVars.size() > 1) 
					&& (visitIter->VisVars[0].getRouteDaysDelta() <= bestRouteDaysDelta)
					&& (visitIter->RegretValues[0] >= largestRegret) )
				{
					
					largestRegret = visitIter->RegretValues[0];
					bestIdxRegret = j;
					bestRouteDaysDelta = visitIter->VisVars[0].getRouteDaysDelta();
				}

				j++;
			}

			if ( (largestRegret == 0) && (smallestInsertionCost == 
				numeric_limits<double>::max()) )
				insertionPossible = false;

			else if ( smallestInsertionCost < 
				numeric_limits<double>::max() )
			{
				/*
				cout << "BEST INSERTION: visit = " << removedVisits[bestIdxInsert].VisVars[0].
					getVarVisit()->getOffshoreInst()->getInstName() << endl;

				cout << "Inserting to route " << removedVisits[bestIdxInsert].VisVars[0].
					getRouteTo()->getRouteNum() << endl;

				cout << "Insertion cost = " << removedVisits[bestIdxInsert].VisVars[0].
					getVarVisit()->RegretValues[0] << endl;
					*/
			
				routeDays = (int)ceil( (removedVisits[bestIdxInsert].VisVars[0].getRouteTo()->computeRouteDuration()+ 
				removedVisits[bestIdxInsert].VisVars[0].getRouteTo()->getInstAtPos(0)->getLayTime()
				+ removedVisits[bestIdxInsert].VisVars[0].getRouteTo()->getRouteMinSlack() )/24);

				routeStartIdx = (int) ceil(removedVisits[bestIdxInsert].VisVars[0].getRouteTo()->getRouteStartTime()/24);
				routeEndIdx = routeStartIdx + routeDays - 1;

				//Update VesAvail
				if (removedVisits[bestIdxInsert].VisVars[0].getRouteDaysDelta() > 0)
				{
					for (j = 1; j <=removedVisits[bestIdxInsert].VisVars[0].getRouteDaysDelta(); j++)
						VesAvail[removedVisits[bestIdxInsert].VisVars[0].getRouteTo()->getRouteVes()->getID()]
								[routeEndIdx + j] = false;

				}

				removedVisits[bestIdxInsert].VisVars[0].getRouteTo()->insertInstalVisit(1, removedVisits[bestIdxInsert]);
				removedVisits[bestIdxInsert].VisVars[0].getRouteTo()->updateVisitVector();
				removedVisits[bestIdxInsert].VisVars[0].getRouteTo()->intelligentReorder();

				
				//Update curVisDayComb
				removedVisits[bestIdxInsert].VisVars[0].getVarVisit()->getOffshoreInst()->InsertDayToVisDayComb(
					(int) ceil(removedVisits[bestIdxInsert].VisVars[0].getRouteTo()->getRouteStartTime()/24) );

				//Update intermedRoutes
				for (routeIntermedIter = intermedRoutes.begin(); 
							routeIntermedIter->getRouteNum() != 
							removedVisits[bestIdxInsert].VisVars[0].getRouteTo()->getRouteNum();
							++routeIntermedIter)
							;
				*routeIntermedIter = *(removedVisits[bestIdxInsert].VisVars[0].getRouteTo());

				//Erase the visit from removedVisits
				removedVisits.erase(removedVisits.begin() + bestIdxInsert);

				//printWeeklySchedule();
				//cout << endl << endl;
				
			}//end else if

			else if (largestRegret > 0)
			{

				/*
				cout << "BEST REGRET INSERTION: visit = " << removedVisits[bestIdxRegret].VisVars[0].
					getVarVisit()->getOffshoreInst()->getInstName() << endl;

				cout << "Inserting to route " << removedVisits[bestIdxRegret].VisVars[0].
					getRouteTo()->getRouteNum() << endl;

				cout << "REGRET VALUE = " << removedVisits[bestIdxRegret].VisVars[0].
					getVarVisit()->RegretValues[0] << endl;
					*/

				routeDays = (int)ceil( (removedVisits[bestIdxRegret].VisVars[0].getRouteTo()->computeRouteDuration()+ 
				removedVisits[bestIdxRegret].VisVars[0].getRouteTo()->getInstAtPos(0)->getLayTime()
				+ removedVisits[bestIdxRegret].VisVars[0].getRouteTo()->getRouteMinSlack() )/24);

				routeStartIdx = (int) ceil(removedVisits[bestIdxRegret].VisVars[0].getRouteTo()->getRouteStartTime()/24);
				routeEndIdx = routeStartIdx + routeDays - 1;

				//Update VesAvail before insertion
				if (removedVisits[bestIdxRegret].VisVars[0].getRouteDaysDelta() > 0)
				{
					for (j = 1; j <=removedVisits[bestIdxRegret].VisVars[0].getRouteDaysDelta(); j++)
						VesAvail[removedVisits[bestIdxRegret].VisVars[0].getRouteTo()->getRouteVes()->getID()]
								[routeEndIdx + j] = false;

				}

				removedVisits[bestIdxRegret].VisVars[0].getRouteTo()->insertInstalVisit(1, removedVisits[bestIdxRegret]);
				removedVisits[bestIdxRegret].VisVars[0].getRouteTo()->updateVisitVector();
				removedVisits[bestIdxRegret].VisVars[0].getRouteTo()->intelligentReorder();

				//Update curVisDayComb
				removedVisits[bestIdxRegret].VisVars[0].getVarVisit()->getOffshoreInst()->InsertDayToVisDayComb(
					(int) ceil(removedVisits[bestIdxRegret].VisVars[0].getRouteTo()->getRouteStartTime()/24) );

				//Update intermedRoutes
				for (routeIntermedIter = intermedRoutes.begin(); 
							routeIntermedIter->getRouteNum() != 
							removedVisits[bestIdxRegret].VisVars[0].getRouteTo()->getRouteNum();
							++routeIntermedIter)
							;
				*routeIntermedIter = *(removedVisits[bestIdxRegret].VisVars[0].getRouteTo());

				//Erase the visit from removedVisits
				removedVisits.erase(removedVisits.begin() + bestIdxRegret);
			}
			else
			{
				cout << "ERROR IN PERFORMING INSERTION - goLNS()!!!" << endl;
			}

			//cout << "Olli Jokkinen" << endl;

		}//end while

		
		if (removedVisits.size() == 0)
		{
			clearEmptyRoutes();

			if (Routes.size() < intermedRoutes.size())
			{
				intermedRoutes.clear();
				intermedRoutes.resize(Routes.size());
				for (k = 0; k < Routes.size(); k++)
				{
					intermedRoutes[k] = Routes[k];
					//savedRoutes[k] = Routes[k];
				}
				
			}
		}
	
		if ( (removedVisits.size() == 0) && isSchedFeasible() )
		{
			//cout << "Inside IF" << endl;
			//cout << "removedVisits.size() = " << removedVisits.size() << endl;
			//cout << "isSchedFeasible() = " << isSchedFeasible() << endl;
			//cout << "Schedule cost = " << computeSchedCost() << endl;
			//printWeeklySchedule();

			/*
			if ( schedCost < bestSchedCost )
			{
				cout << "schedCost = " << schedCost << endl;
				cout << "bestSchedCost = " << bestSchedCost << endl;
				cout << "*curBestCost = " << *curBestCost << endl;
				*curBestCost = schedCost;
				bestSchedCost = schedCost;
				//printWeeklySchedule();
				writeScheduleToFile();
			}	

			*/

			//Just to enter the loop
			
			localCostAfter = 0;
			localCostBefore = 1;

			while (localCostAfter < localCostBefore)
			{
				
				localCostBefore = computeSchedCost();
				//cout << "localCostBefore = " << localCostBefore << endl;

				reduceNumberOfRoutes();
				//cout << "1";
				/*cout << "after reduceNumberOfRoutes Ves Avail: " << endl;
				for (k = 0; k < VesAvail.size(); k++)
				{
					for (j = 0; j < VesAvail[k].size(); j++)
						cout << VesAvail[k][j] << " ";

					cout << endl;
				}
				*/

				reassignVesselsToRoutes();
				//cout << "2";

				/*
				cout << "after reassignVessels Ves Avail: " << endl;
				for (k = 0; k < VesAvail.size(); k++)
				{
					for (j = 0; j < VesAvail[k].size(); j++)
						cout << VesAvail[k][j] << " ";

					cout << endl;
				}
		*/
				reduceRouteDurTotDays();
				//cout << "3";
				reassignVesselsToRoutes();
				//cout << "4";
				relocateVisits();
				//cout << "5";
				reassignVesselsToRoutes();
				//cout << "6";

				isolateVoyage();	
				
				localCostAfter = computeSchedCost();
				//cout << "localCostAfter = " << localCostAfter << endl;
			}
			//cout << "After local loop" << endl;
			

			/*
			if (Routes.size() < intermedRoutes.size())
			{
				intermedRoutes.clear();
				savedRoutes.clear();
				intermedRoutes.resize(Routes.size());
				savedRoutes.resize(Routes.size());
			}*/

			//Check if solutiong improves the best found so far
			schedCost = computeSchedCost();
			setIsolRouteNull();

			if ( schedCost < *curBestCost )
			{
				cout << "schedCost = " << schedCost << endl;
				cout << "bestSchedCost = " << bestSchedCost << endl;
				cout << "*curBestCost = " << *curBestCost << endl;
				*curBestCost = schedCost;
				bestSchedCost = schedCost;
				//printWeeklySchedule();

				writeScheduleToFile(curRestart, i);

				*bs = *this;
				//if (outerIter > 0)
				//	iter += 10;
			}	
			else if ( ( schedCost <= *curBestCost ) && 
				(getIsolRoute() != NULL) )
			{
				cout << "schedCost = " << schedCost << endl;
				cout << "bestSchedCost = " << bestSchedCost << endl;
				cout << "*curBestCost = " << *curBestCost << endl;
				*curBestCost = schedCost;
				bestSchedCost = schedCost;
				//printWeeklySchedule();

				//writeScheduleToFile(curRestart, i);

				*bs = *this;
				//if (outerIter > 0)
				//	iter += 10;
			}	
			
			if ( ( (schedCost - *curBestCost)/(*curBestCost) <= schedMargin ) 
				&& (getIsolRoute() != NULL) )
			{
				//cout << "Yo" << endl;

				m = 0;
				for (j = 0; j < SchedVessels.size(); j++)
					if (SchedVessels[j].getIsVesselUsed())
						m++;

				if (trialScheds->size() == 0)
				{
					trialScheds->push_back(*this);
					//trialScheds->operator [](trialScheds->size()-1).updateRelationalInfo();
					//trialScheds->operator [](trialScheds->size()-1).setIsolRoute();
					cost->push_back(schedCost);
					startTime->push_back(getIsolRoute()->getRouteStartTime());
					vesselCap->push_back(getIsolRoute()->getRouteVes()->getCapacity());
					numVessels->push_back(m);

					cout << "trialScheds->size () = " << trialScheds->size() << endl;
				}
				else
				{

					isolDayFound = false;

					for (k = 0; k < trialScheds->size(); k++)
					{
						if ( (startTime->operator [](k) == getIsolRoute()->getRouteStartTime())
							&& (numVessels->operator [](k) > m) )
						{

							trialScheds->operator [](k) = *this;
							//trialScheds->operator [](k).updateRelationalInfo();
							//trialScheds->operator [](k).setIsolRoute();
							cost->operator [](k) = schedCost;
							vesselCap->operator [](k) = getIsolRoute()->getRouteVes()->getCapacity();
							numVessels->operator [](k) = m;

							isolDayFound = true;
						}
						else if ( (startTime->operator [](k) == getIsolRoute()->getRouteStartTime())
							&& (numVessels->operator [](k) == m)
							&& (vesselCap->operator [](k) < getIsolRoute()->getRouteVes()->getCapacity() ) )
						{
							trialScheds->operator [](k) = *this;
							//trialScheds->operator [](k).updateRelationalInfo();
							//trialScheds->operator [](k).setIsolRoute();
							cost->operator [](k) = schedCost;
							vesselCap->operator [](k) = getIsolRoute()->getRouteVes()->getCapacity();

							isolDayFound = true;
						}
						else if ( (startTime->operator [](k) == getIsolRoute()->getRouteStartTime())
							&& (numVessels->operator [](k) == m)
							&& (vesselCap->operator [](k) == getIsolRoute()->getRouteVes()->getCapacity()) 
							&& (cost->operator [](k) > schedCost) )
						{
							trialScheds->operator [](k) = *this;
							//trialScheds->operator [](k).updateRelationalInfo();
							//trialScheds->operator [](k).setIsolRoute();
							cost->operator [](k) = schedCost;
							isolDayFound = true;
						}
						else if (startTime->operator [](k) == getIsolRoute()->getRouteStartTime())
						{
							isolDayFound = true;
						}
					}//end for k

					if (!isolDayFound)
					{
						trialScheds->push_back(*this);
						cost->push_back(schedCost);
						startTime->push_back(getIsolRoute()->getRouteStartTime());
						vesselCap->push_back(getIsolRoute()->getRouteVes()->getCapacity());
						numVessels->push_back(m);
						//trialScheds->operator [](trialScheds->size()-1).updateRelationalInfo();
						//trialScheds->operator [](trialScheds->size()-1).setIsolRoute();
					}

					
				}//end else
				//printWeeklySchedule();
				//writeScheduleToFile(curRestart, i);

			}	

			totNumZeroes = 0;
			numVesselsUsed = 0;
			
			for (k = 0; k < VesAvail.size(); k++)
			{
				for (j = 0; j < VesAvail[k].size(); j++)
					if (!VesAvail[k][j])
						totNumZeroes++;

				if (SchedVessels[k].getIsVesselUsed())
					numVesselsUsed++;
			}

			//cout << "totNumZeroes = " << totNumZeroes << endl;
			//cout << "numVesselsUsed = " << numVesselsUsed << endl;

			if ( (int)ceil(totNumZeroes/NUMBER_OF_DAYS) < numVesselsUsed)
			{
				reduceNumVesselsSuccessful = reduceNumberOfVessels();

				if (Routes.size() >= LB + 1)
					reduceNumVesselsSuccessful = false;
			}

			if (Routes.size() < intermedRoutes.size())
			{
				intermedRoutes.clear();
				//savedRoutes.clear();
				intermedRoutes.resize(Routes.size());
				//savedRoutes.resize(Routes.size());
			}

			//Keep the changes even if the solution cost is worse
			for (k = 0; k < Routes.size(); k++)
			{
				intermedRoutes[k] = Routes[k];
				//savedRoutes[k] = Routes[k];
			}
		}//end if

	}//end for i
	//cout << "At the end of LNS schedCost = " << computeSchedCost() << endl;
	//printWeeklySchedule();

	/*
	cout << "Printing trialScheds " << endl;
	for (k = 0; k < trialScheds->size(); k++)
	{
		//cout << "IsolRouteStartTime = " << trialScheds->operator [](k).getIsolRoute()->getRouteStartTime() << endl;
		//cout << "SchedCost = " << trialScheds->operator [](k).computeSchedCost() << endl;
		trialScheds->operator [](k).printWeeklySchedule();
	}
	*/
	cout << "trialSches->size () = " << trialScheds->size() << endl;
}

bool WeeklySchedule::reduceNumberOfVessels()
{
	
	//cout << "reduceNumberOfVessels_BEGIN" << endl;

	int i, j, k;
	vector <SupVes>::iterator vesPointIter;
	vector <SupVes>::iterator vesIter;

	bool isAssignmentFeasible;
	bool endOfWeekOverlap;
	bool imprFound = true;

	vector <Route*>::iterator routeIter;
	vector <Route*> vesRoutes;

	int routeEndIdx, routeStartIdx;
	double rouDur;

	vector < vector <bool> > isRouteReassigned;
	vector <vector <SupVes*> > assignToVessel;
	vector <int> numZeroes;
	vector <int> numZeroesReduction;

	
	numZeroes.resize(SchedVessels.size());
	numZeroesReduction.resize(SchedVessels.size());

	vector <vector <bool> > vesAvailability; //local vessel availability vector
	vesAvailability.resize(VesAvail.size());

	int planHorizon = NUMBER_OF_DAYS + (int) ceil(
			( (double)Routes.begin()->MAX_ROUTE_DUR 
			+ Routes.begin()->getInstAtPos(0)->getLayTime() 
			+ Routes.begin()->getRouteMinSlack() )/24);
	
	for (k = 0; k < VesAvail.size(); k++)
		{
			vesAvailability[k].resize(planHorizon);
			for (i = 0; i < planHorizon; i++)
				vesAvailability[k][i] = VesAvail[k][i];	
		}

	int bestNumZeroesResulting = NUMBER_OF_DAYS;
	double bestVesselCap = SchedVessels[0].getCapacity();
	int best_index;

	
	//cout << "Before reassignments" << endl;
		//cout << "Size of vesRoutes =" << vesRoutes.size() << endl << endl;
	/*
	cout << "VesAvail is: " << endl;
	for (k = 0; k < VesAvail.size(); k++)
			{
				for (i = 0; i < VesAvail[k].size(); i++)
					cout << VesAvail[k][i] << " ";
				cout << endl;
			}
	cout << endl;
	*/
	
	while (imprFound)
	{

		for (k = 0; k < VesAvail.size(); k++)
		{
			numZeroesReduction[k] = 0;
			numZeroes[k] = 0;
			for (i = 0; i < planHorizon; i++)
				if (!VesAvail[k][i])
					numZeroes[k]++;
		}

		isRouteReassigned.clear();
		assignToVessel.clear();
		isRouteReassigned.resize(SchedVessels.size());
		assignToVessel.resize(SchedVessels.size());

		//Reassignment of vessels part
		for (vesPointIter = SchedVessels.begin(); vesPointIter != SchedVessels.end(); ++vesPointIter)
		{
			if (vesPointIter->getIsVesselUsed())
			{
				vesRoutes.clear();
				vesRoutes = vesPointIter->getVesselRoutes();
				isRouteReassigned[vesPointIter->getID()].clear();
				isRouteReassigned[vesPointIter->getID()].resize(vesRoutes.size());
				assignToVessel[vesPointIter->getID()].clear();
				assignToVessel[vesPointIter->getID()].resize(vesRoutes.size());

				j=0;
				
				for (routeIter = vesRoutes.begin(); routeIter != vesRoutes.end(); ++routeIter)
				{
					routeStartIdx = (int) ceil((*routeIter)->getRouteStartTime()/24);
					//cout << "RouteStartIdx = " << routeStartIdx << endl;

					routeEndIdx = (int) ceil( ( (*routeIter)->getRouteEndTime() - 
									(*routeIter)->getInstAtPos(0)->getLayTime() +
									(*routeIter)->getRouteMinSlack() )/24 );
					//cout<< "RouteEndIdx = " << routeEndIdx << endl;

					rouDur = (*routeIter)->getRouteEndTime() - (*routeIter)->getRouteStartTime();


					//Try to assign route to a different vessel
					for (vesIter = SchedVessels.begin(); vesIter != SchedVessels.end(); ++vesIter)
						if ( (vesIter->getIsVesselUsed()) && (vesIter != vesPointIter) )
						{
							isAssignmentFeasible = true;
							endOfWeekOverlap = false;

							for (i = routeStartIdx; i <= routeEndIdx; i++)
							{
								if(!vesAvailability[vesIter->getID()][i])
								{
									isAssignmentFeasible = false;
									break;
								}

								if ( (i <= planHorizon - (NUMBER_OF_DAYS+1))
									&& (!vesAvailability[vesIter->getID()][i + NUMBER_OF_DAYS]) )
									{
										isAssignmentFeasible = false;
										break;
									}
							}

							//Additional check to see if the route overlaps 
							//with the first route next week
							if (routeEndIdx >= (NUMBER_OF_DAYS+1) )
								for (i = 0; i <= routeEndIdx % (NUMBER_OF_DAYS+1); i++)
									if(!vesAvailability[vesIter->getID()][i+1])
									{
										endOfWeekOverlap = true;
										break;
									}
							//Can we reassign the route to another vessel?
							if ((vesIter->getCapacity() >= (*routeIter)->computeRouteDemand()) && 
								(isAssignmentFeasible) && (!endOfWeekOverlap) )
							{
								//Assign vessel to the route
								isRouteReassigned[vesPointIter->getID()][j] = true;
								assignToVessel[vesPointIter->getID()][j] = &(*vesIter);

								//cout << "Route " << (*routeIter)->getRouteNum() <<
								//	" can be reassigned &&&&&&&" << endl;

								numZeroesReduction[vesPointIter->getID()] +=
									(routeEndIdx - routeStartIdx + 1);

								/*
								cout << "numZeroesReduction = " << 
									numZeroesReduction[vesPointIter->getID()] << endl;
								cout << "numZeroesResulting = " << (numZeroes[vesPointIter->getID()] -
									numZeroesReduction[vesPointIter->getID()]) << endl;
									*/

								//Update vessel availability vector
								for (i = routeStartIdx; i <= routeEndIdx; i++)
									vesAvailability[vesIter->getID()][i] = false;

								break; //no need to assign the same route to several vessels
							}//end if 
							else
							{
								isRouteReassigned[vesPointIter->getID()][j] = false;
								assignToVessel[vesPointIter->getID()][j] = NULL;
							}
						}// end if (vesIter != vesPointIter) and end for (vesIter)
						j++;		
				}//end for (routeIter)

				//Determine which is the best vessel to be removed
				if (numZeroes[vesPointIter->getID()] - numZeroesReduction[vesPointIter->getID()] 
				< bestNumZeroesResulting)	
				{
					bestNumZeroesResulting = numZeroes[vesPointIter->getID()] - numZeroesReduction[vesPointIter->getID()];
					bestVesselCap = vesPointIter->getCapacity();
					best_index = vesPointIter->getID();
				}
				else if ( (numZeroes[vesPointIter->getID()] - numZeroesReduction[vesPointIter->getID()] 
				== bestNumZeroesResulting)
					&& (vesPointIter->getCapacity() > bestVesselCap) )
				{
					bestNumZeroesResulting = numZeroes[vesPointIter->getID()] - numZeroesReduction[vesPointIter->getID()];
					bestVesselCap = vesPointIter->getCapacity();
					best_index = vesPointIter->getID();
				}



				vesAvailability = VesAvail;
				/*			
				for (k = 0; k < VesAvail.size(); k++)
				{
					for (i = 0; i < VesAvail[k].size(); i++)
						cout << VesAvail[k][i] << " ";
					cout << endl;
				}
				cout << endl;
				*/
			}//end if
		
		}//end for (vesPointIter)

		//Do reassignments (if there are any)
		for (vesIter = SchedVessels.begin(); vesIter != SchedVessels.end(); ++vesIter)
			if (vesIter->getID() == best_index)
				break;

		
		vesRoutes.clear();
		vesRoutes = vesIter->getVesselRoutes();

		imprFound = false;

		for (j = 0; j < isRouteReassigned[best_index].size(); j++)
			if (isRouteReassigned[best_index][j])
			{
				//cout << "Route " << vesRoutes[j]->getRouteNum() << " is reassigned!!" << endl;
				vesRoutes[j]->setRouteVessel(assignToVessel[best_index][j]);
				//add route to "vesselRoutes" vector
				assignToVessel[best_index][j]->addVesselRoute(vesRoutes[j]); 

				//Erase the vessel route from vesIter vesselRoutes
				vesIter->eraseVesselRoute(vesRoutes[j]->getRouteNum());

				routeStartIdx = (int) ceil(vesRoutes[j]->getRouteStartTime()/24);
					//cout << "RouteStartIdx = " << routeStartIdx << endl;

				routeEndIdx = (int) ceil( ( vesRoutes[j]->getRouteEndTime() - 
						vesRoutes[j]->getInstAtPos(0)->getLayTime() +
						vesRoutes[j]->getRouteMinSlack() )/24 );

				//Update VesAvail
				for (k = routeStartIdx; k <= routeEndIdx; k++)
				{
					VesAvail[vesIter->getID()][k] = true;
					VesAvail[assignToVessel[best_index][j]->getID()][k] = false;
				}
				imprFound = true;
				cout << "IMPROVEMENT FOUND!!!!!!" << endl;
				cout << "bestNumZeroesResulting = " << bestNumZeroesResulting << endl;
				cout << "best_index = " << best_index << endl;
				
				vesAvailability = VesAvail;
			}
	}//end while (imprFound)

	vesRoutes.clear();
	vesRoutes = vesIter->getVesselRoutes();

	
	/*cout << "After reassignments" << endl;
	//cout << "Size of vesRoutes =" << vesRoutes.size() << endl;
	cout << "VesAvail is: " << endl;
	for (k = 0; k < VesAvail.size(); k++)
	{
		for (i = 0; i < VesAvail[k].size(); i++)
			cout << VesAvail[k][i] << " ";
		cout << endl;
	}
	cout << endl;
	*/

	///////////////////////////////
	vector <Visit> routeVisits;
	vector <Visit>::iterator visIter;
	vector <Route>::iterator rouIter;
	bool allMulti = true;
	vector <int> routeNumbers;
	//Remove the routes which were not reassigned
	//and put the visits into removedVisits bank

	//cout << "Routes to be removed: " << endl;
	for (routeIter = vesRoutes.begin(); routeIter != vesRoutes.end(); ++routeIter)
	{
		//cout << "Route " << (*routeIter)->getRouteNum() << endl;
		
		if ((*routeIter)->getOnlyRoute())
		{
			allMulti = false;
			break;
		}
	}
	
		
	if ( /*(vesRoutes.size() == 1) &&*/ (allMulti) )
	{
		routeNumbers.resize(vesRoutes.size());
		i = 0;
		cout << "Routes to be removed: " << endl;
		for (routeIter = vesRoutes.begin(); routeIter != vesRoutes.end(); ++routeIter)
		{
			routeNumbers[i] = (*routeIter)->getRouteNum();
			cout << "Route " << routeNumbers[i] << endl;
			i++;
		}
		
		i = 0;
		for (routeIter = vesRoutes.begin(); routeIter != vesRoutes.end(); ++routeIter)
		{

			k = 0;
			for (rouIter = Routes.begin(); rouIter != Routes.end(); ++rouIter)
			{
					if (rouIter->getRouteNum() == routeNumbers[i] )
						break;
					k++;
			}

			routeVisits.clear();
			routeVisits = rouIter->getVisitObjects();

			routeStartIdx = (int) ceil(rouIter->getRouteStartTime()/24);
					//cout << "RouteStartIdx = " << routeStartIdx << endl;

			routeEndIdx = (int) ceil( ( rouIter->getRouteEndTime() - 
				rouIter->getInstAtPos(0)->getLayTime() +
				rouIter->getRouteMinSlack() )/24 );

			for (visIter = routeVisits.begin()+1; visIter != routeVisits.end() - 1; ++visIter)
			{
				removedVisits.push_back(*visIter);
				visIter->getOffshoreInst()->RemoveDayFromVisDayComb( (int) ceil(rouIter->getRouteStartTime()/24) );
			}


			//Delete the route
			//cout << "After removing Route " << routeNumbers[i] << endl;
			Routes.erase(Routes.begin() + k);

			//Update VesAvail
			for (j = routeStartIdx; j <= routeEndIdx; j++)
			{
				VesAvail[vesIter->getID()][j] = true;
			}

			//printWeeklySchedule();
			i++;

		}//end for routeIter

		for (k = 0; k < Routes.size(); k++) 
			Routes[k].setRouteNum(k);

		setOnlyRoutes();

		//For each vessel clear and update vesselRoutes vector
		for (vesIter = SchedVessels.begin(); vesIter != SchedVessels.end(); ++vesIter)
		{
			vesIter->clearVesselRoutes();
			for (rouIter = Routes.begin(); rouIter != Routes.end(); ++rouIter)
				if (vesIter->getID() == rouIter->getRouteVes()->getID() )
					vesIter->addVesselRoute(&(*rouIter));
			
			if (vesIter->getVesselRoutes().size() > 0)
				vesIter->setIsVesselUsed(true);
			else
				vesIter->setIsVesselUsed(false);
		}//end for

	}//end if

	if (removedVisits.size() > 0)
	{
		cout << "AFTER!!! reduceNumberOfVessels() " << endl;
		cout << "removedVisits.size() = " << removedVisits.size() << endl;
		for (visIter = removedVisits.begin(); visIter != removedVisits.end(); ++visIter)
			cout << "Visit->installation = " << visIter->getOffshoreInst()->getInstName() << endl;

		return true;
	}
	//cout << "reduceNumberOfVessels_END" << endl;
	//cout << "end of reduce number of vessels" << endl;
	//printWeeklySchedule();
	return false;
}

void WeeklySchedule::updateRelationalInfo()
{
	vector<SupVes>::iterator vesIter;
	vector<Route>::iterator rouIter;

	for (vesIter = SchedVessels.begin(); vesIter != SchedVessels.end(); ++vesIter)
		{
			vesIter->clearVesselRoutes();
			for (rouIter = Routes.begin(); rouIter != Routes.end(); ++rouIter)
				if (vesIter->getName() == rouIter->getRouteVes()->getName() )
					vesIter->addVesselRoute(&(*rouIter));
			
			if (vesIter->getVesselRoutes().size() > 0)
				vesIter->setIsVesselUsed(true);
			else
				vesIter->setIsVesselUsed(false);
		}//end for

	setOnlyRoutes();
}
bool WeeklySchedule::createVoyages()
{
	//Check if there is an available slot for an extra voyage
	//cout << "createVoyages_BEGIN" << endl;
	vector <SupVes>::iterator vesIter;
	int i, j, k;
	int numRoutes;
	vector <Route>::iterator routeIter;
	int routeEndIndex, routeDays, minRouteDays;

	vector <double> potentialRouteStartTimes;
	vector <SupVes *> potentialVessels;

	bool endOfWeekReached;

	bool moreRoutes = true;
	bool returnValue = false;
	double bestRouteStartTime;

	while (moreRoutes)
	{
		moreRoutes = false;

		potentialRouteStartTimes.clear();
		potentialVessels.clear();

		//cout << "Before for routeIter" << endl;
		for (routeIter = Routes.begin(); routeIter != Routes.end(); ++routeIter)
		{//Calculate number of ones in VesAvail after every Route
			routeDays = (int) ceil( (routeIter->computeRouteDuration() +
				routeIter->getRouteMinSlack() + routeIter->getInstAtPos(0)->getLayTime() )/24 );
			minRouteDays = (int) ceil( (routeIter->MIN_ROUTE_DUR + 0.0001 +
				routeIter->getRouteMinSlack() + routeIter->getInstAtPos(0)->getLayTime() )/24 );

			routeEndIndex = (int) ceil( (routeIter->getRouteEndTime() + 
				routeIter->getRouteMinSlack() - routeIter->getInstAtPos(0)->getLayTime())/24 );
			j = routeEndIndex + 1;
			k = 0;
			endOfWeekReached = false;

			//cout << "routeIter->getRouteVes() = " <<
			//	routeIter->getRouteVes()->getName() << endl;

			if (j > NUMBER_OF_DAYS)
				endOfWeekReached = true;

			//cout << "j = " << j << endl;

			while ( (VesAvail[routeIter->getRouteVes()->getID()][j]) &&
				(!endOfWeekReached) )
			{
				k++;
				j++;
				if (j > NUMBER_OF_DAYS)
					endOfWeekReached = true;
			}

			//cout << "where" << endl;

			if (endOfWeekReached)
			{
				//If we reached the end of the week, start from the beginning again
				while (VesAvail[routeIter->getRouteVes()->getID()][j%NUMBER_OF_DAYS])
				{
					k++;
					j++;
				}
			}

			//routeIter->setNumOnesAfterRoute(k);
			//numOnes.push_back(k);

			if ( (routeEndIndex + 1 != NUMBER_OF_DAYS)
				&& (routeDays >= minRouteDays)
				&& (k >= minRouteDays ) 
				 )
				//create route here
			{
				numRoutes = 0;
				for (i = 0; i < Routes.size(); i++)
					if (Routes[i].getRouteStartTime() == ((routeEndIndex % NUMBER_OF_DAYS)*24 + 16) )
						numRoutes++;
				if (numRoutes <= 1)
				{
					potentialRouteStartTimes.push_back( (routeEndIndex % NUMBER_OF_DAYS)*24 + 16);
					potentialVessels.push_back(routeIter->getRouteVes() );
				}
			}
			else if ( (routeEndIndex + 1 == NUMBER_OF_DAYS)
				&& (routeDays >= minRouteDays)
				&& (k - 1 >= minRouteDays ) )
			{
				numRoutes = 0;
				for (i = 0; i < Routes.size(); i++)
					if (Routes[i].getRouteStartTime() == 16 )
						numRoutes++;

				if (numRoutes <= 1)
				{
					potentialRouteStartTimes.push_back(16);
					potentialVessels.push_back(routeIter->getRouteVes() );
				}
			}
			
			//cout << "Route Number: " << routeIter->getRouteNum();
			//cout << " - number of ones after: " << routeIter->getNumOnesAfterRoute() << endl;
		}//end for routeIter

		if (potentialRouteStartTimes.size() > 0)
		{
			returnValue = true;
			moreRoutes = true;
			bestRouteStartTime = 136;
		
			//Choose the best route
			for (j = 0; j < potentialRouteStartTimes.size(); j++)
			{
				if ( (potentialRouteStartTimes[j] == 16)
					|| (potentialRouteStartTimes[j] == 136) )
				{
					i = j;
					break;
				}
				else if ( (potentialRouteStartTimes[j] != 16)
					&& (potentialRouteStartTimes[j] != 136) 
					&& (potentialRouteStartTimes[j] < bestRouteStartTime) )
				{
					i = j;
					bestRouteStartTime = potentialRouteStartTimes[j];
				}
			}
			
			
			Route newRoute(0, potentialRouteStartTimes[i]);
			newRoute.setRouteVessel(potentialVessels[i]);

			newRoute.addRouteInstal( &SchedInstals[0] );
			newRoute.addVisit( Visit( &SchedInstals[0] ));
			newRoute.addRouteInstal( &SchedInstals[0] );
			newRoute.addVisit( Visit( &SchedInstals[0] ));
				
			//Set route parameters
			newRoute.setRouteAcceptanceTime(Instance->getAcceptTime());
			newRoute.setRouteMinSlack(Instance->getMinSlack());
			newRoute.setRouteLoadFactor(Instance->getLoadFactor());
			newRoute.setMaxVisits(Instance->getMaxInst());
			newRoute.setMinVisits(Instance->getMinInst());	

			newRoute.updateVisitVector();

			cout << "Route created" << endl;
			cout << "StartTime = " << potentialRouteStartTimes[i]<< endl;
			cout << "RouteVessel is " << potentialVessels[i]->getName() << endl;

			j = 0;
			//Insert route into Routes
			for (routeIter = Routes.begin(); routeIter != Routes.end(); ++routeIter)
			{
				if (routeIter->getRouteStartTime() == potentialRouteStartTimes[i])
					break;
				j++;
			}

			Routes.insert(Routes.begin() + j, newRoute);

			//Update VesAvail
			VesAvail[potentialVessels[i]->getID()][(int) ceil(potentialRouteStartTimes[i]/24)] = false;

			//Set single/multiple attributes
			setOnlyRoutes();

			//Update route numbers
			for (k = 0; k < Routes.size(); k++)
				Routes[k].setRouteNum(k);

			//Update vesRoutes for potentialVessels[i]
			for (vesIter = SchedVessels.begin(); vesIter != SchedVessels.end(); ++vesIter)
			{

				vesIter->clearVesselRoutes();

				for (routeIter = Routes.begin(); routeIter != Routes.end(); ++routeIter)
					if (vesIter->getID() == routeIter->getRouteVes()->getID() )
						vesIter->addVesselRoute(&(*routeIter));

				if (vesIter->getVesselRoutes().size() == 0)
					vesIter->setIsVesselUsed(false);
				else
					vesIter->setIsVesselUsed(true);
			}

			

			/*
			cout << "VesAvail is" << endl;
			for (k = 0; k < VesAvail.size(); k++)
			{
				for (j = 0; j < VesAvail[k].size(); j++)
					cout << VesAvail[k][j] << " ";
				cout << endl;
			}
			cout << endl;
			*/
		} //end if

	}//end while

	//cout << "End of createVoyages()" << endl;
	//cout << "createVoyages_END" << endl;

	if (returnValue)
	{
		return true;
	}
	else
		return false;
}
void WeeklySchedule::setOnlyRoutes()
{
	bool only;
	int startIdx, endIdx;
	int i, j;

	for (i = 0; i < Routes.size(); i++)
	{
		only = true;

		if (i - (getDepPerDay()-1) < 0)
			startIdx = 0;
		else
			startIdx = i - (getDepPerDay() - 1);

		if (i + (getDepPerDay()-1) > (Routes.size() - 1) )
			endIdx = Routes.size() - 1;
		else
			endIdx = i + (getDepPerDay()-1);

		//cout << "startIdx = " << startIdx << endl;
		//cout << "endIdx = " << endIdx << endl;

		for (j = startIdx; j <= endIdx; j++)
			if ( (Routes[j].getRouteStartTime() == Routes[i].getRouteStartTime())
				&& (i != j) )
				only = false;

		Routes[i].setOnlyRoute(only);
	}
}

bool WeeklySchedule::isSchedFeasible()
{
	vector<Route>::iterator routeIter;
	double routeDuration;
	bool feasible = true;

	for (routeIter = Routes.begin(); routeIter != Routes.end(); ++routeIter)
	{
		routeDuration = routeIter->computeRouteDuration();
		if ( (routeDuration > routeIter->MAX_ROUTE_DUR + routeIter->getRouteAcceptanceTime()) 
			|| (routeDuration < routeIter->MIN_ROUTE_DUR + routeIter->getRouteAcceptanceTime()) )
			feasible = false;
	}

	return feasible;
}

void WeeklySchedule::clearEmptyRoutes()
{

	//cout << "Before clearEmptyRoutes" << endl;
	//cout << "Number of routes = " << Routes.size() << endl;

	vector <SupVes>::iterator vesIter;
	int i, k;
	vector <Route>::iterator routeIter;
	
	bool moreToClear = true;
	bool wasCleared = false;
	
	while (moreToClear)
	{
		moreToClear = false;
		i = 0;
		for (routeIter = Routes.begin(); routeIter != Routes.end(); ++routeIter)
		{
			if ( (routeIter->getVisitObjects().size() == 2) 
				&& (!routeIter->getOnlyRoute()) )
			{
				k = (int) ceil(routeIter->getRouteStartTime()/24);
				
				VesAvail[routeIter->getRouteVes()->getID()][k] = true;

				cout << "In clear Empty routes!!!" << endl;
				//printWeeklySchedule();
				cout << "Erasing route " << Routes[i].getRouteNum() << endl;
				Routes[i].printRoute();

				Routes.erase(Routes.begin() + i);
				moreToClear = true;
				wasCleared = true;
						
				break;
			}

			i++;
		}//end for

		if (moreToClear)
		{
			for (k=0; k <Routes.size(); k++)
					Routes[k].setRouteNum(k);

			setOnlyRoutes();

			for (vesIter = SchedVessels.begin(); vesIter != SchedVessels.end(); ++vesIter)
			{
					vesIter->clearVesselRoutes();
					for (routeIter = Routes.begin(); 
						routeIter != Routes.end(); ++routeIter)
						if (vesIter->getID() == routeIter->getRouteVes()->getID() )
							vesIter->addVesselRoute(&(*routeIter));
				
				if (vesIter->getVesselRoutes().size() > 0)
					vesIter->setIsVesselUsed(true);
				else
					vesIter->setIsVesselUsed(false);
			}//end for
		}//end if

	}//end while
				
	if (wasCleared)
	{
		cout << "After clearEmptyRoutes" << endl;
		printWeeklySchedule();
		//cout << "Number of routes = " << Routes.size() << endl << endl;
	}
}

void WeeklySchedule::isolateVoyage()
{
	int i, j, k;
	vector <SupVes>::iterator vesPointIter;
	vector <SupVes>::iterator vesIter;

	bool isAssignmentFeasible;
	bool endOfWeekOverlap;
	
	vector <Route*>::iterator routeIter;
	vector <Route*> vesRoutes;

	int routeEndIdx, routeStartIdx;
	double rouDur;

	vector <vector <bool> > vesAvailability; //local vessel availability vector
	vesAvailability.resize(VesAvail.size());

	int planHorizon = NUMBER_OF_DAYS + (int) ceil(
			( (double)Routes.begin()->MAX_ROUTE_DUR 
			+ Routes.begin()->getInstAtPos(0)->getLayTime() 
			+ Routes.begin()->getRouteMinSlack() )/24);
	
	for (k = 0; k < VesAvail.size(); k++)
		{
			vesAvailability[k].resize(planHorizon);
			for (i = 0; i < planHorizon; i++)
				vesAvailability[k][i] = VesAvail[k][i];	
		}

	
	//cout << "Before reassignments" << endl;
		//cout << "Size of vesRoutes =" << vesRoutes.size() << endl << endl;
	
	/*
	cout << "AT THE BEGIN" << endl;

	for (k = 0; k < VesAvail.size(); k++)
	{
		for (i = 0; i < VesAvail[k].size(); i++)
			cout << VesAvail[k][i] << " ";
		cout << endl;
	}
	cout << endl;
	*/

	//SWAP REASSIGNMENTS
	bool swapImprovement = true;
	bool endOfWeekReached;
	vector <int> numConsecZeroes;
	vector <Route*> anotherVesRoutes;
	vector <Route*>::iterator routePointIter;
	int anotherRouteStartIdx, anotherRouteEndIdx;
	int value;

	//calculate numConsecZeroes
	numConsecZeroes.clear();
	numConsecZeroes.resize(SchedVessels.size());

	for (vesPointIter = SchedVessels.begin(); vesPointIter != SchedVessels.end(); ++vesPointIter)
		if (vesPointIter->getIsVesselUsed())
		{	
			j = 1;
			k = 0;
			value = 0;
			endOfWeekReached = false;
			while (!endOfWeekReached)
			{

				while (VesAvail[vesPointIter->getID()][j])
					//skip 1's
					j++;

				while (!VesAvail[vesPointIter->getID()][j])
				{
					j++;
					k++;

					if (j > NUMBER_OF_DAYS)
						endOfWeekReached = true;
				}

				if (k > value)
					value = k;

				//cout << "where" << endl;

				//if we reached the end of the week we know the value
				if (endOfWeekReached)
					numConsecZeroes[vesPointIter->getID()] = value;
				else
				{
					k = 0;
					//skip the ones
					while (VesAvail[vesPointIter->getID()][j])
					{
						j++;
						if (j >= NUMBER_OF_DAYS)
						{
							endOfWeekReached = true;
							break;
						}
					}

					if (!endOfWeekReached)
					{
						//Count zeroes
						while (!VesAvail[vesPointIter->getID()][j])
						{
							j++;
							k++;

							if (j > NUMBER_OF_DAYS)
								endOfWeekReached = true;
						}

						if (endOfWeekReached)
						{
							while (!VesAvail[vesPointIter->getID()][j % NUMBER_OF_DAYS])
							{
								j++;
								k++;
							}

							if (k > value)
								value = k;
						}
						else if (k > value)
							value = k;
					}//end if (!endOfWeekReached)

					numConsecZeroes[vesPointIter->getID()] = value;

					//Make sure we reach the end of the week
					if (!endOfWeekReached)
						while (VesAvail[vesPointIter->getID()][j])
						{
							j++;
							if (j >= NUMBER_OF_DAYS)
							{
								endOfWeekReached = true;
								break;
							}
						}
				}//end else
			}//end while (!endOfWeekReached)
		}//end for (vesPointIter)
	
	swapImprovement = true;

	bool shouldPrint = false;

	while (swapImprovement)
	{
		
		swapImprovement = false;

		
		for (vesPointIter = SchedVessels.begin(); vesPointIter != SchedVessels.end(); ++vesPointIter)
			if (vesPointIter->getIsVesselUsed())
			{
				vesRoutes.clear();
				vesRoutes.resize(vesPointIter->getVesselRoutes().size());
				vesRoutes = vesPointIter->getVesselRoutes();

				j=0;

				for (routeIter = vesRoutes.begin(); routeIter != vesRoutes.end(); ++routeIter)
				{
					routeStartIdx = (int) ceil((*routeIter)->getRouteStartTime()/24);
					

					routeEndIdx = (int) ceil( ( (*routeIter)->getRouteEndTime() - 
									(*routeIter)->getInstAtPos(0)->getLayTime() +
									(*routeIter)->getRouteMinSlack() )/24 );
					

					rouDur = (*routeIter)->getRouteEndTime() - (*routeIter)->getRouteStartTime();

					//Make vessel available
					for (i = routeStartIdx; i <= routeEndIdx; i++)
						VesAvail[vesPointIter->getID()][i] = true;

					/*
					if (shouldPrint)
					{
						cout << "vesPointIter = " << vesPointIter->getName() << endl;
						cout << "RouteStartIdx = " << routeStartIdx << endl;
						cout<< "RouteEndIdx = " << routeEndIdx << endl;
						for (k = 0; k < VesAvail.size(); k++)
						{
							for (i = 0; i < VesAvail[k].size(); i++)
								cout << VesAvail[k][i] << " ";
							cout << endl;
						}
						cout << endl;
					}
					*/

					//Try to assign route to a different vessel
					for (vesIter = SchedVessels.begin(); vesIter != SchedVessels.end(); ++vesIter)
						if ( (vesIter->getIsVesselUsed()) && (vesIter->getID() > vesPointIter->getID()) )
						{
							anotherVesRoutes.clear();
							anotherVesRoutes.resize(vesIter->getVesselRoutes().size());
							anotherVesRoutes = vesIter->getVesselRoutes();

							for (routePointIter = anotherVesRoutes.begin(); routePointIter != anotherVesRoutes.end(); ++routePointIter)
							{
								anotherRouteStartIdx = (int) ceil( (*routePointIter)->getRouteStartTime()/24 );

								anotherRouteEndIdx = (int) ceil( ( (*routePointIter)->getRouteEndTime()
									- (*routePointIter)->getInstAtPos( (*routePointIter)->getVisitObjects().size() - 1)->getOpeningTime()
									+ (*routePointIter)->getRouteMinSlack() )/24 );

								for (i = anotherRouteStartIdx; i <= anotherRouteEndIdx; i++)
									VesAvail[vesIter->getID()][i] = true;

								/*
								if (shouldPrint)
								{
									
									cout << "vesIter = " << vesIter->getName() << endl;
									cout << "anotherRouteStartIdx = " << anotherRouteStartIdx << endl;
									cout<< "anotherRouteEndIdx = " << anotherRouteEndIdx << endl;
									for (k = 0; k < VesAvail.size(); k++)
									{
										for (i = 0; i < VesAvail[k].size(); i++)
											cout << VesAvail[k][i] << " ";
										cout << endl;
									}
									cout << endl;
								}
								*/
								//SWAP MOVES

								//First voyage to second vessel
								isAssignmentFeasible = true;
								endOfWeekOverlap = false;

								for (i = routeStartIdx; i <= routeEndIdx; i++)
								{
									if(!VesAvail[vesIter->getID()][i])
									{
										isAssignmentFeasible = false;
										break;
									}

									if ( (i <= planHorizon - (NUMBER_OF_DAYS+1))
										&& (!VesAvail[vesIter->getID()][i + NUMBER_OF_DAYS]) )
										{
											isAssignmentFeasible = false;
											break;
										}
								}

								//Additional check to see if the route overlaps 
								//with the first route next week
								if (routeEndIdx >= (NUMBER_OF_DAYS+1) )
									for (i = 0; i <= routeEndIdx % (NUMBER_OF_DAYS+1); i++)
										if(!VesAvail[vesIter->getID()][i+1])
										{
											endOfWeekOverlap = true;
											break;
										}

								//Second voyage to first vessel
								for (i = anotherRouteStartIdx; i <= anotherRouteEndIdx; i++)
								{
									if(!VesAvail[vesPointIter->getID()][i])
									{
										isAssignmentFeasible = false;
										break;
									}

									if ( (i <= planHorizon - (NUMBER_OF_DAYS+1))
										&& (!VesAvail[vesPointIter->getID()][i + NUMBER_OF_DAYS]) )
										{
											isAssignmentFeasible = false;
											break;
										}
								}

								//Additional check to see if the route overlaps 
								//with the first route next week
								if (anotherRouteEndIdx >= (NUMBER_OF_DAYS+1) )
									for (i = 0; i <= anotherRouteEndIdx % (NUMBER_OF_DAYS+1); i++)
										if(!VesAvail[vesPointIter->getID()][i+1])
										{
											endOfWeekOverlap = true;
											break;
										}

								//If we are feasible and improving 
								//Implement the reassignment and evaluate it
								if ( (isAssignmentFeasible) && (!endOfWeekOverlap)
									&& (vesIter->getCapacity() > (*routeIter)->computeRouteDemand() )
									&& (vesPointIter->getCapacity() > (*routePointIter)->computeRouteDemand() )
									)
								{
									//Do tentative reassignments
									for (i = routeStartIdx; i <= routeEndIdx; i++)
										VesAvail[vesIter->getID()][i] = false;
									for (i = anotherRouteStartIdx; i <= anotherRouteEndIdx; i++)
										VesAvail[vesPointIter->getID()][i] = false;

									/*	
									if (shouldPrint)
									{
										cout << "AFTER TEntative REASSignments" << endl;

										for (k = 0; k < VesAvail.size(); k++)
										{
											for (i = 0; i < VesAvail[k].size(); i++)
												cout << VesAvail[k][i] << " ";
											cout << endl;
										}
										cout << endl;

										cout << "vesIter->getID() = " << vesIter->getID() << endl
											<< "NumConsecZeroes = " << computeConsecZeroes(vesIter->getID()) << endl << endl;

										cout << "vesPointIter->getID() = " << vesPointIter->getID() << endl
											<< "NumConsecZeroes = " << computeConsecZeroes(vesPointIter->getID()) << endl;
									}
									*/


									if ( ( (numConsecZeroes[vesIter->getID()] < computeConsecZeroes(vesIter->getID()) )
										&& (numConsecZeroes[vesPointIter->getID()] <= computeConsecZeroes(vesPointIter->getID()) ) )
										|| ( (numConsecZeroes[vesIter->getID()] <= computeConsecZeroes(vesIter->getID()) )
										&& (numConsecZeroes[vesPointIter->getID()] < computeConsecZeroes(vesPointIter->getID()) ) ) )
										 
									{
										swapImprovement = true;
										cout << "SWAP IMPROVEMENT!!!!" << endl;

										//Do actual reassignments
										numConsecZeroes[vesIter->getID()] = computeConsecZeroes(vesIter->getID());
										numConsecZeroes[vesPointIter->getID()] = computeConsecZeroes(vesPointIter->getID());

										(*routePointIter)->setRouteVessel( &(*vesPointIter) );
										(*routeIter)->setRouteVessel( &(*vesIter) );

										/*
										vesIter->addVesselRoute(*routeIter);
										vesIter->eraseVesselRoute((*routePointIter)->getRouteNum());
										vesPointIter->addVesselRoute(*routePointIter);
										vesPointIter->eraseVesselRoute((*routeIter)->getRouteNum());
										*/
										for (k = 0; k < VesAvail.size(); k++)
										{
											for (i = 0; i < VesAvail[k].size(); i++)
												cout << VesAvail[k][i] << " ";
											cout << endl;
										}
										cout << endl;

										updateRelationalInfo();
										setIsolRoute();
										
										

										//vesAvailability = VesAvail;

										break;
									}
									else //Update VesAvail accordingly
									{
										for (i = routeStartIdx; i <= routeEndIdx; i++)
											VesAvail[vesIter->getID()][i] = true;
										for (i = anotherRouteStartIdx; i <= anotherRouteEndIdx; i++)
											VesAvail[vesPointIter->getID()][i] = true;
										for (i = anotherRouteStartIdx; i <= anotherRouteEndIdx; i++)
											VesAvail[vesIter->getID()][i] = false;
									}

								}//end if
								else
								{
									for (i = anotherRouteStartIdx; i <= anotherRouteEndIdx; i++)
											VesAvail[vesIter->getID()][i] = false;
								}

								
							}//end for routePointIter

							if (swapImprovement)
								break; //out of vesIter
								
						}//end if 
					//end for (vesIter)

					if (!swapImprovement)
						for (i = routeStartIdx; i <= routeEndIdx; i++)
							VesAvail[vesPointIter->getID()][i] = false;
					else
					{
						break; //out of for routeIter
					}

				}//end for routeIter

				if (swapImprovement)
					break; //out of for vesPointIter

			}// end if
		//end for (vesPointIter)

		/*
		if (swapImprovement)
		{
			cout << "before end while" << endl;
			for (k = 0; k < VesAvail.size(); k++)
			{
				for (i = 0; i < VesAvail[k].size(); i++)
					cout << VesAvail[k][i] << " ";
				cout << endl;
			}
			cout << endl;
			shouldPrint = true;
		}
		*/
	}//end while (swapImprovement)


	vector < vector <bool> > isRouteReassigned;
	vector <vector <SupVes*> > assignToVessel;
	vector <int> numZeroes;
	vector <int> numZeroesReduction;
	
	numZeroes.resize(SchedVessels.size());
	numZeroesReduction.resize(SchedVessels.size());

	int bestNumZeroesResulting = NUMBER_OF_DAYS;
	double bestVesselCap = SchedVessels[0].getCapacity();
	int best_index;


	
	//ONE-WAY REASSIGNMENTS
	bool imprFound = true;
	
	while (imprFound)
	{
		best_index = -1;

		for (k = 0; k < VesAvail.size(); k++)
		{
			numZeroesReduction[k] = 0;
			numZeroes[k] = 0;
			for (i = 0; i < planHorizon; i++)
				if (!VesAvail[k][i])
					numZeroes[k]++;

			if ((numZeroes[k] < bestNumZeroesResulting) && (numZeroes[k] > 0))
				bestNumZeroesResulting = numZeroes[k];
		}

		//cout << "bestNumZeroesResulting = " << bestNumZeroesResulting << endl;
		isRouteReassigned.clear();
		assignToVessel.clear();
		isRouteReassigned.resize(SchedVessels.size());
		assignToVessel.resize(SchedVessels.size());

		//Reassignment of vessels part
		for (vesPointIter = SchedVessels.begin(); vesPointIter != SchedVessels.end(); ++vesPointIter)
		{
			if (vesPointIter->getIsVesselUsed())
			{
				vesRoutes.clear();
				vesRoutes = vesPointIter->getVesselRoutes();
				isRouteReassigned[vesPointIter->getID()].clear();
				isRouteReassigned[vesPointIter->getID()].resize(vesRoutes.size());
				assignToVessel[vesPointIter->getID()].clear();
				assignToVessel[vesPointIter->getID()].resize(vesRoutes.size());

				j=0;
				
				for (routeIter = vesRoutes.begin(); routeIter != vesRoutes.end(); ++routeIter)
				{
					routeStartIdx = (int) ceil((*routeIter)->getRouteStartTime()/24);
					//cout << "RouteStartIdx = " << routeStartIdx << endl;

					routeEndIdx = (int) ceil( ( (*routeIter)->getRouteEndTime() - 
									(*routeIter)->getInstAtPos(0)->getLayTime() +
									(*routeIter)->getRouteMinSlack() )/24 );
					//cout<< "RouteEndIdx = " << routeEndIdx << endl;

					rouDur = (*routeIter)->getRouteEndTime() - (*routeIter)->getRouteStartTime();


					//Try to assign route to a different vessel
					for (vesIter = SchedVessels.begin(); vesIter != SchedVessels.end(); ++vesIter)
						if ( (vesIter->getIsVesselUsed()) && (vesIter != vesPointIter) )
						{
							isAssignmentFeasible = true;
							endOfWeekOverlap = false;

							for (i = routeStartIdx; i <= routeEndIdx; i++)
							{
								if(!vesAvailability[vesIter->getID()][i])
								{
									isAssignmentFeasible = false;
									break;
								}

								if ( (i <= planHorizon - (NUMBER_OF_DAYS+1))
									&& (!vesAvailability[vesIter->getID()][i + NUMBER_OF_DAYS]) )
									{
										isAssignmentFeasible = false;
										break;
									}
							}

							//Additional check to see if the route overlaps 
							//with the first route next week
							if (routeEndIdx >= (NUMBER_OF_DAYS+1) )
								for (i = 0; i <= routeEndIdx % (NUMBER_OF_DAYS+1); i++)
									if(!vesAvailability[vesIter->getID()][i+1])
									{
										endOfWeekOverlap = true;
										break;
									}
							//Can we reassign the route to another vessel?
							if ((vesIter->getCapacity() >= (*routeIter)->computeRouteDemand()) && 
								(isAssignmentFeasible) && (!endOfWeekOverlap) )
							{
								//Assign vessel to the route
								isRouteReassigned[vesPointIter->getID()][j] = true;
								assignToVessel[vesPointIter->getID()][j] = &(*vesIter);

								//cout << "Route " << (*routeIter)->getRouteNum() <<
								//	" can be reassigned &&&&&&&" << endl;

								numZeroesReduction[vesPointIter->getID()] +=
									(routeEndIdx - routeStartIdx + 1);


								//Update vessel availability vector
								for (i = routeStartIdx; i <= routeEndIdx; i++)
									vesAvailability[vesIter->getID()][i] = false;

								break; //no need to assign the same route to several vessels
							}//end if 
							else
							{
								isRouteReassigned[vesPointIter->getID()][j] = false;
								assignToVessel[vesPointIter->getID()][j] = NULL;
							}
						}// end if (vesIter != vesPointIter) and end for (vesIter)
						j++;		
				}//end for (routeIter)

				//Determine which is the best vessel to be removed
				if (numZeroes[vesPointIter->getID()] - numZeroesReduction[vesPointIter->getID()] 
				< bestNumZeroesResulting)	
				{
					bestNumZeroesResulting = numZeroes[vesPointIter->getID()] - numZeroesReduction[vesPointIter->getID()];
					bestVesselCap = vesPointIter->getCapacity();
					best_index = vesPointIter->getID();
				}
				else if ( (numZeroes[vesPointIter->getID()] - numZeroesReduction[vesPointIter->getID()] 
				== bestNumZeroesResulting)
					&& (vesPointIter->getCapacity() > bestVesselCap) )
				{
					bestNumZeroesResulting = numZeroes[vesPointIter->getID()] - numZeroesReduction[vesPointIter->getID()];
					bestVesselCap = vesPointIter->getCapacity();
					best_index = vesPointIter->getID();
				}

				vesAvailability = VesAvail;
			
			}//end if
		
		}//end for (vesPointIter)

		
		//Do reassignments (if there are any)
		imprFound = false;

		if (best_index >= 0)
		{
			for (vesIter = SchedVessels.begin(); vesIter != SchedVessels.end(); ++vesIter)
				if (vesIter->getID() == best_index)
					break;

			
			vesRoutes.clear();
			vesRoutes = vesIter->getVesselRoutes();


			for (j = 0; j < isRouteReassigned[best_index].size(); j++)
				if (isRouteReassigned[best_index][j])
				{
					//cout << "Route " << vesRoutes[j]->getRouteNum() << " is reassigned!!" << endl;
					vesRoutes[j]->setRouteVessel(assignToVessel[best_index][j]);
					//add route to "vesselRoutes" vector
					assignToVessel[best_index][j]->addVesselRoute(vesRoutes[j]); 

					//Erase the vessel route from vesIter vesselRoutes
					vesIter->eraseVesselRoute(vesRoutes[j]->getRouteNum());

					routeStartIdx = (int) ceil(vesRoutes[j]->getRouteStartTime()/24);
					//cout << "RouteStartIdx = " << routeStartIdx << endl;

					routeEndIdx = (int) ceil( ( vesRoutes[j]->getRouteEndTime() - 
							vesRoutes[j]->getInstAtPos(0)->getLayTime() +
							vesRoutes[j]->getRouteMinSlack() )/24 );

					//Update VesAvail
					for (k = routeStartIdx; k <= routeEndIdx; k++)
					{
						VesAvail[vesIter->getID()][k] = true;
						VesAvail[assignToVessel[best_index][j]->getID()][k] = false;
					}
					
				}
			
			vesAvailability = VesAvail;

			updateRelationalInfo();
			setIsolRoute();		

			imprFound = true;
			//cout << "IMPROVEMENT FOUND!!!!!!" << endl;
			//cout << "bestNumZeroesResulting = " << bestNumZeroesResulting << endl;
			//cout << "best_index = " << best_index << endl;

		}//end if (best_index > 0)

	}//end while (imprFound)

	//setIsolRoute();

	
	/*
	cout << "AT THE END" << endl;

	for (k = 0; k < VesAvail.size(); k++)
	{
		for (i = 0; i < VesAvail[k].size(); i++)
			cout << VesAvail[k][i] << " ";
		cout << endl;
	}
	cout << endl;
	*/
}
bool WeeklySchedule::isolateVoyageBetween(int start, int end, int isolVesID, vector <double> startTimes)
{
	int i, j, k;
	vector <SupVes>::iterator vesPointIter;
	vector <SupVes>::iterator vesIter;

	bool isAssignmentFeasible;
	bool endOfWeekOverlap;
	

	vector <Route*>::iterator routeIter;
	vector <Route*> vesRoutes;

	int routeEndIdx, routeStartIdx;
	double rouDur;

	vector <vector <bool> > vesAvailability; //local vessel availability vector
	vesAvailability.resize(VesAvail.size());

	int planHorizon = NUMBER_OF_DAYS + (int) ceil(
			( (double)Routes.begin()->MAX_ROUTE_DUR 
			+ Routes.begin()->getInstAtPos(0)->getLayTime() 
			+ Routes.begin()->getRouteMinSlack() )/24);
	
	bool wasIsolated = false;

	for (k = 0; k < VesAvail.size(); k++)
		{
			vesAvailability[k].resize(planHorizon);
			for (i = 0; i < planHorizon; i++)
				vesAvailability[k][i] = VesAvail[k][i];	
		}

	//cout << "AT THE BEGIN of IsolateBetween" << endl;
	//cout << "isolBegin = " << start << endl
	//		<< "isolEnd = " << end << endl;

	/*
	for (k = 0; k < VesAvail.size(); k++)
	{
		for (i = 0; i < VesAvail[k].size(); i++)
			cout << VesAvail[k][i] << " ";
		cout << endl;
	}
	cout << endl;
	*/
	

	//SWAP REASSIGNMENTS
	bool swapImprovement = false;
	bool endOfWeekReached;
	vector <int> numConsecZeroes;
	vector <Route*> anotherVesRoutes;
	vector <Route*>::iterator routePointIter;
	int anotherRouteStartIdx, anotherRouteEndIdx;
	int value;

	//Locate isolVessel
	for (vesPointIter = SchedVessels.begin(); vesPointIter != SchedVessels.end(); ++vesPointIter)
		if (vesPointIter->getID() == isolVesID)
			break;
	
	
	
	vesRoutes.clear();
	vesRoutes.resize(vesPointIter->getVesselRoutes().size());
	vesRoutes = vesPointIter->getVesselRoutes();

	j=0;

	if (vesRoutes.size() == 1)
		for (routeIter = vesRoutes.begin(); routeIter != vesRoutes.end(); ++routeIter)
		{
			routeStartIdx = (int) ceil((*routeIter)->getRouteStartTime()/24);
			//cout << "RouteStartIdx = " << routeStartIdx << endl;

			routeEndIdx = (int) ceil( ( (*routeIter)->getRouteEndTime() - 
							(*routeIter)->getInstAtPos(0)->getLayTime() +
							(*routeIter)->getRouteMinSlack() )/24 );
			//cout<< "RouteEndIdx = " << routeEndIdx << endl;

			rouDur = (*routeIter)->getRouteEndTime() - (*routeIter)->getRouteStartTime();

			//Make vessel available
			for (i = routeStartIdx; i <= routeEndIdx; i++)
				VesAvail[vesPointIter->getID()][i] = true;

			//Try to assign route to a different vessel
			for (vesIter = SchedVessels.begin(); vesIter != SchedVessels.end(); ++vesIter)
				if ( (vesIter->getIsVesselUsed()) && (vesIter->getID() != vesPointIter->getID()) )
				{
					anotherVesRoutes.clear();
					anotherVesRoutes.resize(vesIter->getVesselRoutes().size());
					anotherVesRoutes = vesIter->getVesselRoutes();

					for (routePointIter = anotherVesRoutes.begin(); routePointIter != anotherVesRoutes.end(); ++routePointIter)
					{
						wasIsolated = false;

						
						for (i = 0; i < startTimes.size(); i++)
							if (startTimes[i] == (*routePointIter)->getRouteStartTime())
								wasIsolated = true;

						/*
						cout << "The route " << (*routePointIter)->getRouteStartTime() <<" ";
						if (wasIsolated)
							cout << "was isolated before" << endl;
						else
							cout << "was NOT isolated before" << endl;
						*/

						if (!wasIsolated)
						{
							anotherRouteStartIdx = (int) ceil( (*routePointIter)->getRouteStartTime()/24 );

							anotherRouteEndIdx = (int) ceil( ( (*routePointIter)->getRouteEndTime()
								- (*routePointIter)->getInstAtPos( (*routePointIter)->getVisitObjects().size() - 1)->getOpeningTime()
								+ (*routePointIter)->getRouteMinSlack() )/24 );

							//If another route satisfies start and end conditions, try to swap
							if ( (anotherRouteStartIdx >= start) && (anotherRouteEndIdx <= end) )
							{//SWAP MOVES
								for (i = anotherRouteStartIdx; i <= anotherRouteEndIdx; i++)
									VesAvail[vesIter->getID()][i] = true;

								//First voyage to second vessel
								isAssignmentFeasible = true;
								endOfWeekOverlap = false;

								for (i = routeStartIdx; i <= routeEndIdx; i++)
								{
									if(!VesAvail[vesIter->getID()][i])
									{
										isAssignmentFeasible = false;
										break;
									}

									if ( (i <= planHorizon - (NUMBER_OF_DAYS+1))
										&& (!VesAvail[vesIter->getID()][i + NUMBER_OF_DAYS]) )
										{
											isAssignmentFeasible = false;
											break;
										}
								}

								//Additional check to see if the route overlaps 
								//with the first route next week
								if (routeEndIdx >= (NUMBER_OF_DAYS+1) )
									for (i = 0; i <= routeEndIdx % (NUMBER_OF_DAYS+1); i++)
										if(!VesAvail[vesIter->getID()][i+1])
										{
											endOfWeekOverlap = true;
											break;
										}

								//Second voyage to first vessel
								for (i = anotherRouteStartIdx; i <= anotherRouteEndIdx; i++)
								{
									if(!VesAvail[vesPointIter->getID()][i])
									{
										isAssignmentFeasible = false;
										break;
									}

									if ( (i <= planHorizon - (NUMBER_OF_DAYS+1))
										&& (!VesAvail[vesPointIter->getID()][i + NUMBER_OF_DAYS]) )
										{
											isAssignmentFeasible = false;
											break;
										}
								}

								//Additional check to see if the route overlaps 
								//with the first route next week
								if (anotherRouteEndIdx >= (NUMBER_OF_DAYS+1) )
									for (i = 0; i <= anotherRouteEndIdx % (NUMBER_OF_DAYS+1); i++)
										if(!VesAvail[vesPointIter->getID()][i+1])
										{
											endOfWeekOverlap = true;
											break;
										}

								//If we are feasible
								//Implement the reassignment and evaluate it
								if ( (isAssignmentFeasible) && (!endOfWeekOverlap)
									&& (vesIter->getCapacity() > (*routeIter)->computeRouteDemand() )
									&& (vesPointIter->getCapacity() > (*routePointIter)->computeRouteDemand() )
									)
								{
									//Do tentative reassignments
									for (i = routeStartIdx; i <= routeEndIdx; i++)
										VesAvail[vesIter->getID()][i] = false;
									for (i = anotherRouteStartIdx; i <= anotherRouteEndIdx; i++)
										VesAvail[vesPointIter->getID()][i] = false;

									
									swapImprovement = true;
									cout << "SWAP IMPROVEMENT!!!!" << endl;

									(*routePointIter)->setRouteVessel( &(*vesPointIter) );
									(*routeIter)->setRouteVessel( &(*vesIter) );

									vesIter->eraseVesselRoute((*routePointIter)->getRouteNum());
									vesIter->addVesselRoute(*routeIter);
									
									vesPointIter->eraseVesselRoute((*routeIter)->getRouteNum());
									vesPointIter->addVesselRoute(*routePointIter);

									setIsolRoute();
									updateRelationalInfo();
									
									for (k = 0; k < VesAvail.size(); k++)
									{
										for (i = 0; i < VesAvail[k].size(); i++)
											cout << VesAvail[k][i] << " ";
										cout << endl;
									}
									cout << endl;
																
									return swapImprovement;

									//vesAvailability = VesAvail;

									
								}//end if
								else
									for (i = anotherRouteStartIdx; i <= anotherRouteEndIdx; i++)
										VesAvail[vesIter->getID()][i] = false;
							}// end if ( (anotherRouteStartIdx >= start) && (anotherRouteEndIdx <= end) )
						}//end if (!wasIsolated)
					}//end for routePointIter
						
				}//end if 
			//end for (vesIter)

			if (!swapImprovement)
				for (i = routeStartIdx; i <= routeEndIdx; i++)
					VesAvail[vesPointIter->getID()][i] = false;
		}//end for routeIter
	//end if (vesRoutes.size() == 1)


	//cout << "After swap part" << endl;

	/*
	for (k = 0; k < VesAvail.size(); k++)
	{
		for (i = 0; i < VesAvail[k].size(); i++)
			cout << VesAvail[k][i] << " ";
		cout << endl;
	}
	cout << endl;
*/

	//ONE-WAY REASSIGNMENTS
	vector < vector <bool> > isRouteReassigned;
	vector <vector <SupVes*> > assignToVessel;
	
	double bestVesselCap = SchedVessels[0].getCapacity();
	int best_index = -1;

	bool imprFound;	


	isRouteReassigned.clear();
	assignToVessel.clear();
	isRouteReassigned.resize(SchedVessels.size());
	assignToVessel.resize(SchedVessels.size());

	

	//Reassignment of vessels part
	for (vesIter = SchedVessels.begin(); vesIter != SchedVessels.end(); ++vesIter)
	{
		imprFound = true;
		wasIsolated = false;

		if ( (vesIter->getIsVesselUsed()) && (vesIter->getID() != vesPointIter->getID()) )
		{
			vesRoutes.clear();
			vesRoutes = vesIter->getVesselRoutes();
			isRouteReassigned[vesIter->getID()].clear();
			isRouteReassigned[vesIter->getID()].resize(vesRoutes.size());
			assignToVessel[vesIter->getID()].clear();
			assignToVessel[vesIter->getID()].resize(vesRoutes.size());

			j=0;

			//cout << "vesRoutes.size() = " << vesRoutes.size() << endl;
			
			for (routeIter = vesRoutes.begin(); routeIter != vesRoutes.end(); ++routeIter)
			{
				routeStartIdx = (int) ceil((*routeIter)->getRouteStartTime()/24);
				//cout << "RouteStartIdx = " << routeStartIdx << endl;

				routeEndIdx = (int) ceil( ( (*routeIter)->getRouteEndTime() - 
								(*routeIter)->getInstAtPos(0)->getLayTime() +
								(*routeIter)->getRouteMinSlack() )/24 );
				//cout<< "RouteEndIdx = " << routeEndIdx << endl;

				rouDur = (*routeIter)->getRouteEndTime() - (*routeIter)->getRouteStartTime();


				//Try to assign route to an isolated vessel
				isAssignmentFeasible = true;
				endOfWeekOverlap = false;

				for (i = routeStartIdx; i <= routeEndIdx; i++)
				{
					if(!vesAvailability[vesPointIter->getID()][i])
					{
						isAssignmentFeasible = false;
						break;
					}

					if ( (i <= planHorizon - (NUMBER_OF_DAYS+1))
						&& (!vesAvailability[vesPointIter->getID()][i + NUMBER_OF_DAYS]) )
						{
							isAssignmentFeasible = false;
							break;
						}
				}

				//Additional check to see if the route overlaps 
				//with the first route next week
				if (routeEndIdx >= (NUMBER_OF_DAYS+1) )
					for (i = 0; i <= routeEndIdx % (NUMBER_OF_DAYS+1); i++)
						if(!vesAvailability[vesPointIter->getID()][i+1])
						{
							endOfWeekOverlap = true;
							break;
						}

				//cout << "donde" << endl;
				//Can we reassign the route to another vessel?
				if ((vesPointIter->getCapacity() >= (*routeIter)->computeRouteDemand()) && 
					(isAssignmentFeasible) && (!endOfWeekOverlap) )
				{
					
					//Assign vessel to the route
					isRouteReassigned[vesIter->getID()][j] = true;
					assignToVessel[vesIter->getID()][j] = &(*vesPointIter);

					//cout << "Route " << (*routeIter)->getRouteNum() <<
					//	" can be reassigned &&&&&&&" << endl;

					/*
					numZeroesReduction[vesPointIter->getID()] +=
						(routeEndIdx - routeStartIdx + 1);

					
					cout << "numZeroesReduction = " << 
						numZeroesReduction[vesPointIter->getID()] << endl;
					cout << "numZeroesResulting = " << (numZeroes[vesPointIter->getID()] -
						numZeroesReduction[vesPointIter->getID()]) << endl;
						*/

					
					//Update vessel availability vector
					for (i = routeStartIdx; i <= routeEndIdx; i++)
						vesAvailability[vesPointIter->getID()][i] = false;

					
					for (i = routeStartIdx; i <= routeEndIdx; i++)
						vesAvailability[vesIter->getID()][i] = true;

					//break; //no need to assign the same route to several vessels
				}//end if 
				else
				{
					isRouteReassigned[vesIter->getID()][j] = false;
					assignToVessel[vesIter->getID()][j] = NULL;
				}
			
				
				j++;		
			}//end for (routeIter)

			//cout << "Here" << endl;

			//Check if we are successful in isolating vesIter
			for (i = 1; i < start; i++)
				if (!vesAvailability[vesIter->getID()][i])
					imprFound = false;

			for (i = end + 1; i < planHorizon; i++)
				if (!vesAvailability[vesIter->getID()][i])
					imprFound = false;

			
			if (imprFound)
				for (j = 0; j < isRouteReassigned[vesIter->getID()].size(); j++)
					if (!isRouteReassigned[vesIter->getID()][j]) 
						for (i = 0; i < startTimes.size(); i++)
							if (vesRoutes[j]->getRouteStartTime() == startTimes[i])
								wasIsolated = true;


			//Determine which is the best vessel to be removed
			if ( imprFound && (vesIter->getCapacity() > bestVesselCap)
				&& (!wasIsolated) )
			{
				bestVesselCap = vesPointIter->getCapacity();
				best_index = vesPointIter->getID();
			}

			vesAvailability = VesAvail;
			/*			
			for (k = 0; k < VesAvail.size(); k++)
			{
				for (i = 0; i < VesAvail[k].size(); i++)
					cout << VesAvail[k][i] << " ";
				cout << endl;
			}
			cout << endl;
			*/
		}//end if
	
	}//end for (vesIter)

	//cout << "aqui" << endl;

	//Do reassignments (if there are any)
	if (best_index >= 0)
	{
		for (vesIter = SchedVessels.begin(); vesIter != SchedVessels.end(); ++vesIter)
			if (vesIter->getID() == best_index)
				break;

	
		vesRoutes.clear();
		vesRoutes = vesIter->getVesselRoutes();

		imprFound = false;

		for (j = 0; j < isRouteReassigned[best_index].size(); j++)
			if (isRouteReassigned[best_index][j])
			{
				//cout << "Route " << vesRoutes[j]->getRouteNum() << " is reassigned!!" << endl;
				vesRoutes[j]->setRouteVessel(assignToVessel[best_index][j]);
				//add route to "vesselRoutes" vector
				assignToVessel[best_index][j]->addVesselRoute(vesRoutes[j]); 

				//Erase the vessel route from vesIter vesselRoutes
				vesIter->eraseVesselRoute(vesRoutes[j]->getRouteNum());

				routeStartIdx = (int) ceil(vesRoutes[j]->getRouteStartTime()/24);
					//cout << "RouteStartIdx = " << routeStartIdx << endl;

				routeEndIdx = (int) ceil( ( vesRoutes[j]->getRouteEndTime() - 
						vesRoutes[j]->getInstAtPos(0)->getLayTime() +
						vesRoutes[j]->getRouteMinSlack() )/24 );

				//Update VesAvail
				for (k = routeStartIdx; k <= routeEndIdx; k++)
				{
					VesAvail[vesIter->getID()][k] = true;
					VesAvail[assignToVessel[best_index][j]->getID()][k] = false;
				}
				imprFound = true;
				cout << "IMPROVEMENT FOUND!!!!!!" << endl;
				cout << "best_index = " << best_index << endl;
				
				//vesAvailability = VesAvail;		
			}

		setIsolRoute();
		updateRelationalInfo();

		return true;
	}// end if
	else
	{
		cout << "At the end of IsolateBetween" << endl;
		return false;
	}

}

void WeeklySchedule::makeMirrorScheduleSmall(WeeklySchedule *otherSched)
{
	//
	vector <int> beg;
	vector <int> end;
	bool beginning = false;
	bool ending = false;

	//Beginning day and ending day of an isolated voyage from otherSched
	int isolVoyBeg;
	int i, j, k;

	for (i = 0; i < otherSched->VesAvail.size(); i++)
	{
		k = 0;
		isolVoyBeg = -1;

		for (j = 0; j < otherSched->VesAvail[i].size(); j++)
			if (!otherSched->VesAvail[i][j])
			{
				k++;
				if (isolVoyBeg < 0)
					isolVoyBeg = j;
			}

		if (k <= (int) ceil( (otherSched->getRoutePointer(0)->getRouteMaxDur()
			+ otherSched->getRoutePointer(0)->getInstAtPos(0)->getLayTime() )/24 ) )
			break;
	}


	vector <OffshoreInst>::iterator it;

	if (Routes.size() == 3)
	{
		if (isolVoyBeg <= 3)
		{
			for (it = SchedInstals.begin(); it != SchedInstals.end(); ++it)
				if (it->getVisitFreq() == 3)
				{
					it->ObligatoryVisitDays.push_back(2);
					it->ObligatoryVisitDays.push_back(5);
					it->ObligatoryVisitDays.push_back(6);
				}
		}
		else
		{
			for (it = SchedInstals.begin(); it != SchedInstals.end(); ++it)
				if (it->getVisitFreq() == 3)
				{
					it->ObligatoryVisitDays.push_back(1);
					it->ObligatoryVisitDays.push_back(2);
					it->ObligatoryVisitDays.push_back(5);
				}
		}	
	}
	else if (Routes.size() == 4)
	{
		if (isolVoyBeg <= 3)
		{
			for (it = SchedInstals.begin(); it != SchedInstals.end(); ++it)
				if (it->getVisitFreq() == 4)
				{
					it->ObligatoryVisitDays.push_back(1);
					it->ObligatoryVisitDays.push_back(3);
					it->ObligatoryVisitDays.push_back(5);
					it->ObligatoryVisitDays.push_back(6);
				}
		}
		else
		{
			for (it = SchedInstals.begin(); it != SchedInstals.end(); ++it)
				if (it->getVisitFreq() == 4)
				{
					it->ObligatoryVisitDays.push_back(1);
					it->ObligatoryVisitDays.push_back(2);
					it->ObligatoryVisitDays.push_back(4);
					it->ObligatoryVisitDays.push_back(6);
				}
		}	
	}
	else if (Routes.size() == 5)
	{
		if (isolVoyBeg <= 3)
		{
			for (it = SchedInstals.begin(); it != SchedInstals.end(); ++it)
				if (it->getVisitFreq() == 5)
				{
					it->ObligatoryVisitDays.push_back(1);
					it->ObligatoryVisitDays.push_back(2);
					it->ObligatoryVisitDays.push_back(4);
					it->ObligatoryVisitDays.push_back(5);
					it->ObligatoryVisitDays.push_back(6);
				}
		}
		else
		{
			for (it = SchedInstals.begin(); it != SchedInstals.end(); ++it)
				if (it->getVisitFreq() == 5)
				{
					it->ObligatoryVisitDays.push_back(1);
					it->ObligatoryVisitDays.push_back(2);
					it->ObligatoryVisitDays.push_back(3);
					it->ObligatoryVisitDays.push_back(5);
					it->ObligatoryVisitDays.push_back(6);
				}
		}	
	}	
}

int WeeklySchedule::computeConsecZeroes(int vesID)
{
	//calculate numConsecZeroes
	int j , k, value;
	bool endOfWeekReached;

	j = 1;
	k = 0;
	value = 0;
	endOfWeekReached = false;

	while (!endOfWeekReached)
	{

		while (VesAvail[vesID][j])
			//skip 1's
			j++;

		while (!VesAvail[vesID][j])
		{
			j++;
			k++;

			if (j > NUMBER_OF_DAYS)
				endOfWeekReached = true;
		}

		if (k > value)
			value = k;

		//cout << "where" << endl;

		//if we reached the end of the week we know the value
		if (!endOfWeekReached)
		{
			k = 0;
			//skip the ones
			while (VesAvail[vesID][j])
			{
				j++;
				if (j >= NUMBER_OF_DAYS)
				{
					endOfWeekReached = true;
					break;
				}
			}

			if (!endOfWeekReached)
			{
				//Count zeroes
				while (!VesAvail[vesID][j])
				{
					j++;
					k++;

					if (j > NUMBER_OF_DAYS)
						endOfWeekReached = true;
				}

				if (endOfWeekReached)
				{
					while (!VesAvail[vesID][j % NUMBER_OF_DAYS])
					{
						j++;
						k++;
					}

					if (k > value)
						value = k;
				}
				else if (k > value)
					value = k;
			}//end if (!endOfWeekReached)

			//Make sure we reach the end of the week
			if (!endOfWeekReached)
				while (VesAvail[vesID][j])
				{
					j++;
					if (j >= NUMBER_OF_DAYS)
					{
						endOfWeekReached = true;
						break;
					}
				}
		}//end else
	}//end while (!endOfWeekReached)

	return value;
}
void WeeklySchedule::setIsolRoute()
{
	vector <Route>::iterator routeIter;
	vector <SupVes>::iterator vesIter;
	vector <Route>::iterator rouIter;

	int j;
	for (vesIter = SchedVessels.begin(); vesIter != SchedVessels.end(); ++vesIter)
		if (vesIter->getIsVesselUsed())
	{
		j = 0;
		for (routeIter = Routes.begin(); routeIter != Routes.end(); ++routeIter)
			if (routeIter->getRouteVes()->getID() == vesIter->getID())
		{
			j++;
		}

		if (j == 1)
		{
			for (rouIter = Routes.begin(); rouIter != Routes.end(); ++rouIter)
				if (rouIter->getRouteVes()->getID() == vesIter->getID())
				{
					isolRoute = &(*rouIter);
					isolRoute->getRouteVes()->clearVesselRoutes();
					isolRoute->getRouteVes()->addVesselRoute(isolRoute);
					break;
				}
		}

	}
}

void WeeklySchedule::setIsolRouteNull()
{
	vector <Route>::iterator routeIter;
	vector <SupVes>::iterator vesIter;
	vector <Route>::iterator rouIter;

	int j;
	for (vesIter = SchedVessels.begin(); vesIter != SchedVessels.end(); ++vesIter)
		if (vesIter->getIsVesselUsed())
	{
		j = 0;
		for (routeIter = Routes.begin(); routeIter != Routes.end(); ++routeIter)
			if (routeIter->getRouteVes()->getID() == vesIter->getID())
		{
			j++;
		}

		if (j == 1)
		{
			for (rouIter = Routes.begin(); rouIter != Routes.end(); ++rouIter)
				if (rouIter->getRouteVes()->getID() == vesIter->getID())
				{
					isolRoute = &(*rouIter);
					isolRoute->getRouteVes()->clearVesselRoutes();
					isolRoute->getRouteVes()->addVesselRoute(isolRoute);
					return;
				}
		}

	}

	isolRoute = NULL;
}

void WeeklySchedule::multiLNS(int iter, double *curBestCost, WeeklySchedule *otherSched, 
		WeeklySchedule *bs, Route* otherBaseRoute)
{
	int i, j, k, l, m, p;
	
	int numVisitsRemove, numRoutesRemove;
	double schedCost;
	static double bestSchedCost;
	bestSchedCost = *curBestCost;
	
	vector <Route>::iterator routeIter;
	vector <Route>::iterator routeRelocIter;
	vector <Route>::iterator routeIntermedIter;
	vector <SupVes>::iterator vesIter;

	vector <Route> intermedRoutes;
	vector <Route> savedRoutes;

	vector <Visit> routeVisits;
	vector <Visit>::iterator visitIter;

	vector <Visit*> Visits;
	vector <Visit*>::iterator visIter;

	int routeIdx, visitIdx;
	int routeStartIdx, routeEndIdx;
	int routeDays, newRouteDays;

	double smallestInsertionCost, largestRegret;

	intermedRoutes.resize(Routes.size());
	savedRoutes.resize(Routes.size());

	synchrVisitInstalConnection();

	for (k=0; k < Routes.size(); k++)
	{
		intermedRoutes[k] = Routes[k];
		savedRoutes[k] = Routes[k];
	}

	vector <OffshoreInst>::iterator instalIter;

	//Relational info
	for (routeIter = intermedRoutes.begin(); routeIter != intermedRoutes.end(); ++routeIter)
	{
		Visits.clear();
		Visits = routeIter->getVisitPointers();

		for (visIter = Visits.begin(); visIter != Visits.end(); ++visIter)
			for (instalIter = SchedInstals.begin(); instalIter != SchedInstals.end(); ++instalIter)
				if ( instalIter->getInstName() == (*visIter)->getOffshoreInst()->getInstName() )
					(*visIter)->setOffshoreInst(&(*instalIter));
	} // end for routeIter

	

	bool insertionPossible;
	bool insertionOverlaps = false;
	double localCostBefore, localCostAfter;
	
	double routeToDurBefore, routeToDurAfter;
	double schedCostBefore, schedCostAfter;
	int routeDaysBefore, routeDaysAfter, daysDelta, bestRouteDaysDelta;

	double routeSlackBefore, routeSlackAfter;
	//int numRoutesBefore, numRoutesAfter;
	int bestIdxRegret, bestIdxInsert;
	
	bool reduceNumVesselsSuccessful = false;
	int LB = computeLB_numRoutes();

	/*
	int planHorizon = NUMBER_OF_DAYS + (int) ceil(
			( (double)Routes.begin()->MAX_ROUTE_DUR 
			+ Routes.begin()->getInstAtPos(0)->getLayTime() 
			+ Routes.begin()->getRouteMinSlack() )/24);
	*/

	//cout << "SchedInstals.size() = " << SchedInstals.size() << endl;

	if (SchedInstals.size() <= 6)
		numRoutesRemove = 1;
	else
		numRoutesRemove = 2;

	
	//otherBaseRoute->printRoute();

	vector <Route*> vesRoutes; 
	vector <Route*>::iterator rouIter;

	//writeScheduleToFile();
	//Do LNS iterations
	for (i = 0; i < iter; i++)
	{
		//printWeeklySchedule();

		//cout << "LNS Iteration " << i << endl;

		/*
		for (vesIter = SchedVessels.begin(); vesIter != SchedVessels.end(); ++vesIter)
			if (vesIter->getIsVesselUsed())
			{
				vesRoutes.clear();
				vesRoutes = vesIter->getVesselRoutes();
				cout <<"Routes of " << vesIter->getName() << endl;

				for (rouIter = vesRoutes.begin(); rouIter != vesRoutes.end(); ++rouIter)
					(*rouIter)->printRoute();
			}

		cout << "SchedInstals.size() = " << SchedInstals.size() << endl;
		for (p = 0; p < SchedInstals.size(); p++)
			cout << "Installation " << SchedInstals[p].getInstName() << ": "
				<< "Dist.size() = " << SchedInstals[p].Dist.size() << endl;

		//cout << "Before Deletions" << endl;
		//printWeeklySchedule();
		*/
		
		/*
		if (i % 50 == 0) 
		{
			cout << "LNS iter " << i << endl;
			cout << "removedVisits.size() = " << removedVisits.size() << endl;
			for (visitIter = removedVisits.begin(); visitIter != removedVisits.end(); ++visitIter)
				cout << "Visit->installation = " << visitIter->getOffshoreInst()->getInstName() << endl;

			printWeeklySchedule();
		//}*/
		
		insertionPossible = true;
		
		//numRoutesRemove = rand() % (Routes.size() - Routes.size()/4) + 1;

		l = 0; m = 0;
		for (j = 0; j < numRoutesRemove; j++)
		{
			/*m = l;
			for (k = l; k < Routes.size(); k ++)
				if (!Routes[k].isRouteDurFeasible() )
				{
					l = k + 1;
					break;
				}

			if ( l > m)
				routeIdx = k;
			else
			{*/
				routeIdx = rand() % Routes.size();

			p = 0;

			while (Routes[routeIdx].getVisitObjects().size() <= 3)
			{
				routeIdx = rand() % Routes.size();
				//cout << "Are we here" << endl;
				p++;

				//if (p > 50)
				//	cout << "infinite loop" << endl;
			}
					
			//}


			//cout << "otherBaseRoute = " << otherBaseRoute << endl;
			//cout << "&Routes[routeIdx] = " << &Routes[routeIdx] << endl;

			routeDays = (int)ceil( (Routes[routeIdx].computeRouteDuration()+ 
				Routes[routeIdx].getInstAtPos(0)->getLayTime()
				+ Routes[routeIdx].getRouteMinSlack() )/24);

			routeStartIdx = (int) ceil(Routes[routeIdx].getRouteStartTime()/24);
			routeEndIdx = routeStartIdx + routeDays - 1;

			numVisitsRemove = (rand() % ( Routes[routeIdx].getVisitObjects().size() - 2 - 1 ) ) + 1;
			//numVisitsRemove = 1;
			visitIdx = (rand() % (Routes[routeIdx].getVisitObjects().size() - 2) ) + 1;

			//cout << "Erasing from route " << routeIdx << endl;
			//cout << "numVisitsRemove = " << numVisitsRemove << endl;
			//cout << "visitIdx = " << visitIdx << endl;
			if (numVisitsRemove == (Routes[routeIdx].getVisitObjects().size() - 2))
			{
				//cout << "We get here" << endl;
				for (k = 0; k < numVisitsRemove; k++)
				{
					removedVisits.push_back(Routes[routeIdx].getVisitAtPos(visitIdx));
					Routes[routeIdx].getVisitAtPos(visitIdx).getOffshoreInst()->
						RemoveDayFromVisDayComb( (int) ceil(Routes[routeIdx].getRouteStartTime()/24) );
					Routes[routeIdx].deleteInstalVisit(visitIdx);

					if (visitIdx == (Routes[routeIdx].getVisitObjects().size() - 1) )
						visitIdx = 1;
				}

				Routes[routeIdx].updateVisitVector();

				if (&Routes[routeIdx] == otherBaseRoute)
					for (k = routeStartIdx + 1; k <= routeEndIdx; k++)
						otherSched->VesAvail[Routes[routeIdx].getRouteVes()->getID()][k] = true;
				else
					for (k = routeStartIdx + 1; k <= routeEndIdx; k++)
						VesAvail[Routes[routeIdx].getRouteVes()->getID()][k] = true;

				synchrVisitInstalConnection();

				//Update intermedRoutes
				for (routeIntermedIter = intermedRoutes.begin(); 
							routeIntermedIter->getRouteNum() != 
							Routes[routeIdx].getRouteNum();
							++routeIntermedIter)
							;
				*routeIntermedIter = Routes[routeIdx];

				//Keep empty routes for possible insertions

				//Routes.erase(Routes.begin() + routeIdx);
				//intermedRoutes.erase(intermedRoutes.begin() + routeIdx);

				//For each vessel clear and update vesselRoutes vector
				/*
				for (vesIter = SchedVessels.begin(); vesIter != SchedVessels.end(); ++vesIter)
				{
					if (vesIter->getVesselRoutes().size() > 0)
					{
						vesIter->clearVesselRoutes();
						for (routeIter = Routes.begin(); 
							routeIter != Routes.end(); ++routeIter)
							if (vesIter->getID() == routeIter->getRouteVes()->getID() )
								vesIter->addVesselRoute(&(*routeIter));
					}
					else
						vesIter->setIsVesselUsed(false);
				}//end for
				*/
			}
			else
			{
				//Delete numVisitsRemove visits starting from visitIdx 

				//cout<< "before removal" << endl;
				//Routes[routeIdx].printRoute();
				//cout << "numVisitsRemove = " << numVisitsRemove << endl;
				//cout << "visitIdx = " << visitIdx << endl; 
				
				for (k = 0; k < numVisitsRemove; k++)
				{
					removedVisits.push_back(Routes[routeIdx].getVisitAtPos(visitIdx));
					//cout << "VisDayComb for visit being removed: " 
					//	<< Routes[routeIdx].getVisitAtPos(visitIdx).getOffshoreInst()->getInstName()  
					//	<< endl;

					//for (p = 0; p < Routes[routeIdx].getVisitAtPos(visitIdx).getOffshoreInst()->CurVisitDayComb.size(); p++)
					//	cout << Routes[routeIdx].getVisitAtPos(visitIdx).getOffshoreInst()->CurVisitDayComb[p] << " ";
					//cout << endl;

					Routes[routeIdx].getVisitAtPos(visitIdx).getOffshoreInst()->
						RemoveDayFromVisDayComb( (int) ceil(Routes[routeIdx].getRouteStartTime()/24) );
					
					Routes[routeIdx].deleteInstalVisit(visitIdx);
					
					if (visitIdx == (Routes[routeIdx].getVisitObjects().size() - 1) )
						visitIdx = 1;
				}
				
				Routes[routeIdx].intelligentReorder();

				//cout<< "after removal" << endl;
				//Routes[routeIdx].printRoute();

				newRouteDays = (int)ceil( (Routes[routeIdx].computeRouteDuration()+ 
				Routes[routeIdx].getInstAtPos(0)->getLayTime()
				+ Routes[routeIdx].getRouteMinSlack() )/24);
				
		
				if (&Routes[routeIdx] == otherBaseRoute )
					for (k = routeStartIdx + newRouteDays; k <= routeEndIdx; k++)
						otherSched->VesAvail[Routes[routeIdx].getRouteVes()->getID()][k] = true;
				else
					for (k = routeStartIdx + newRouteDays; k <= routeEndIdx; k++)
						VesAvail[Routes[routeIdx].getRouteVes()->getID()][k] = true;


				synchrVisitInstalConnection();

				//Update intermedRoutes
				for (routeIntermedIter = intermedRoutes.begin(); 
							routeIntermedIter->getRouteNum() != 
							Routes[routeIdx].getRouteNum();
							++routeIntermedIter)
							;
				*routeIntermedIter = Routes[routeIdx];


			}//end else
			
		}//end for j

		/*
		cout << "After Deletions" << endl;
		//printWeeklySchedule();
		cout << "removedVisits.size() = " << removedVisits.size() << endl;
		for (visitIter = removedVisits.begin(); visitIter != removedVisits.end(); ++visitIter)
			cout << "Visit->installation = " << visitIter->getOffshoreInst()->getInstName() << endl;
		*/

		/*
		for (visitIter = removedVisits.begin(); visitIter != removedVisits.end(); ++visitIter)
		{
			cout << "Visit->installation = " << visitIter->getOffshoreInst()->getInstName() << endl;
		}
		*/

		//Create extra voyages here
		
		insertionPossible = true;

		//cout << "before insertion loop" << endl;

		while ( insertionPossible && (removedVisits.size() > 0) )
		{
			for (visitIter = removedVisits.begin(); visitIter != removedVisits.end(); ++visitIter)
			{
				visitIter->VisVars.clear();
				visitIter->RegretValues.clear();
				VisitVariation exchVar(&(*visitIter));

				//cout << "Let's see" << endl;

				//Evaluate and sort all possible insertions for a given visit
				for (routeIter = Routes.begin(); routeIter != Routes.end(); ++routeIter)
				{
					routeDays = (int)ceil( (routeIter->computeRouteDuration()+ 
					routeIter->getInstAtPos(0)->getLayTime()
					+ routeIter->getRouteMinSlack() )/24);

					routeStartIdx = (int) ceil(routeIter->getRouteStartTime()/24);
					routeEndIdx = routeStartIdx + routeDays - 1;

					//cout << "banda" << endl;

					if ( (routeIter->computeRouteDemand() + 
						visitIter->getOffshoreInst()->getWeeklyDemand()/visitIter->getOffshoreInst()->getVisitFreq()
						< routeIter->getRouteVes()->getCapacity()) &&  
						(routeIter->getVisitObjects().size() - 2 < routeIter->getMaxVisits() ) &&
						!routeIter->isInstOnRoute(visitIter->getOffshoreInst()) &&
						isDepartureSpreadEven(&(*routeIter),&(*visitIter)) )
					{
						//cout << "yupi" << endl;
						//Store the pre-insertion info
						routeToDurBefore = routeIter->computeRouteDuration();
						routeDaysBefore = (int)ceil( (routeIter->computeRouteDuration()+ 
							routeIter->getInstAtPos(0)->getLayTime()
							+ routeIter->getRouteMinSlack() )/24);
						//cout << "1"; 
						routeSlackBefore = routeIter->computeRouteSlack();
						//cout << "2"; 

						/*
						cout << "SchedInstals.size() = " << SchedInstals.size() << endl;
						for (p = 0; p < SchedInstals.size(); p++)
							cout << "Installation " << SchedInstals[p].getInstName() << ": "
								<< "Dist.size() = " << SchedInstals[p].Dist.size() << endl;

								*/
						synchrVisitInstalConnection();
						schedCostBefore = computeSchedCost();
						
						//numRoutesBefore = Routes.size();
						
						
						//Do tentative insertion
						routeIter->insertInstalVisit(1, *visitIter);
						routeIter->updateVisitVector();
						//cout << "3"; 
						routeIter->intelligentReorder();
						routeToDurAfter = routeIter->computeRouteDuration();
						//cout << "4"; 

						

						if (routeToDurAfter < routeIter->MAX_ROUTE_DUR + routeIter->getRouteAcceptanceTime())
						{		
							//cout << "Aida" << endl;

							routeDaysAfter = (int)ceil( (routeIter->computeRouteDuration()+ 
							routeIter->getInstAtPos(0)->getLayTime()
							+ routeIter->getRouteMinSlack() )/24);

							daysDelta = routeDaysAfter - routeDaysBefore;

							//cout << "routeIter is " << endl;
							//routeIter->printRoute();

							if (daysDelta > 0)
								for (j = 1; j <= daysDelta; j++)
								{
									//cout << "routeEndIdx + j = " << (routeEndIdx + j) << endl;

									if ((&(*routeIter) != otherBaseRoute))
									{
										if (!VesAvail[routeIter->getRouteVes()->getID()]
										[routeEndIdx + j] )
										{
											insertionOverlaps = true;
											break;
										}
									}
									else if ( (&(*routeIter) == otherBaseRoute)
										&& (!otherSched->VesAvail[routeIter->getRouteVes()->getID()]
									[routeEndIdx + j] ) )
									{
										insertionOverlaps = true;
										break;
									}


									if (&(*routeIter) != otherBaseRoute)
									{
										if ( ( routeEndIdx + j > NUMBER_OF_DAYS) && 
											(!VesAvail[routeIter->getRouteVes()->getID()]
										[(routeEndIdx+j)%NUMBER_OF_DAYS])  )
										{
											insertionOverlaps = true;
											break;
										}
									}
									else if ( (&(*routeIter) == otherBaseRoute)
										&& ( routeEndIdx + j > otherSched->NUMBER_OF_DAYS) && 
										(!otherSched->VesAvail[routeIter->getRouteVes()->getID()]
									[(routeEndIdx+j)%NUMBER_OF_DAYS]) )
									{
										insertionOverlaps = true;
										break;
									}
								}
							//cout << "Oido" << endl;

							if (!insertionOverlaps)
							{
								synchrVisitInstalConnection();
								schedCostAfter = computeSchedCost();
								routeSlackAfter = routeIter->computeRouteSlack();

								//Initialize VisitVariation objects
								VisitVariation visVar(&(*visitIter));
								visVar.setRouteTo(&(*routeIter));
								visVar.setRouteToDurIncrease(routeToDurAfter - routeToDurBefore);
								visVar.setDeltaObj(schedCostAfter - schedCostBefore);
								visVar.setRouteDaysDelta(routeDaysAfter - routeDaysBefore);
								visVar.setRouteSlackDelta(routeSlackAfter - routeSlackBefore);

								visitIter->VisVars.push_back(visVar);
							
							//Use auxiliary objective???
							/*Auxiliary objective:
							- Schedule cost
							- Number of routes
							- Number of route days
							- Total slack
							*/
							}//end if
							insertionOverlaps = false;
						}//end if
						//cout << "Carla" << endl;

						//Restore the affected route
						for (routeIntermedIter = intermedRoutes.begin(); 
							routeIntermedIter->getRouteNum() !=  routeIter->getRouteNum();
							++routeIntermedIter)
							;
						*routeIter = *routeIntermedIter;

					}//end if
			
				}//end for routeIter

				//cout << "Aga" << endl;
				
				//Sort VisVars vector using insertion sort
				//Note - some may already be sorted from previous iterations
				if (visitIter->VisVars.size() > 0)
				{
					//cout << "visitIter->VisVars.size() = " << visitIter->VisVars.size()<< endl;
					for (j = 1; j < visitIter->VisVars.size(); j++)
					{
						exchVar = visitIter->VisVars[j]; 
						k = j;
						
							while ( ( (visitIter->VisVars[k-1].getDeltaObj() >= exchVar.getDeltaObj())
								&& (visitIter->VisVars[k-1].getRouteDaysDelta() >= exchVar.getRouteDaysDelta() )
								&& (visitIter->VisVars[k-1].getRouteTo()->getVisitObjects().size() > 2)
								&& (exchVar.getRouteTo()->getVisitObjects().size() > 2) )
								|| ( (visitIter->VisVars[k-1].getRouteTo()->getVisitObjects().size() == 2)
								&& (exchVar.getRouteTo()->getVisitObjects().size() > 2) )
								|| ( (visitIter->VisVars[k-1].getDeltaObj() >= exchVar.getDeltaObj())
								&& (visitIter->VisVars[k-1].getRouteDaysDelta() >= exchVar.getRouteDaysDelta() )
								&& (visitIter->VisVars[k-1].getRouteTo()->getVisitObjects().size() == 2)
								&& (exchVar.getRouteTo()->getVisitObjects().size() == 2) ) )
							{
								//cout << "k = " << k << endl;
								visitIter->VisVars[k] = visitIter->VisVars[k-1];
								k--;
								//cout << "Inside while" << endl;
								if (k == 0)
								{
									//cout << "before break" << endl;
									break;
								}
							}//end while
									
						//cout << "sorted" << endl;
						visitIter->VisVars[k] = exchVar;
						
					}//end for j

					/*				
					cout << "VisVars for " << visitIter->getOffshoreInst()->getInstName() << endl;
					for (j = 0; j < visitIter->VisVars.size(); j++)
					{
						cout << "\tRouteTo # of visits = " << visitIter->VisVars[j].getRouteTo()->getVisitObjects().size(); 
						
						cout << "  DeltaObj = " << visitIter->VisVars[j].getDeltaObj() << endl;
					}
					cout << endl;
					*/

					//Initialize regret vector
					if (visitIter->VisVars.size() == 1)
					{
						visitIter->RegretValues.resize(1);
						visitIter->RegretValues[0] = visitIter->VisVars[0].getDeltaObj();
					}
					else
					{
						visitIter->RegretValues.resize(visitIter->VisVars.size() - 1);
						for (j = 1; j < visitIter->VisVars.size(); j++)
						{
							visitIter->RegretValues[j-1] = visitIter->VisVars[j].getDeltaObj() - 
							visitIter->VisVars[j-1].getDeltaObj();
						}
					}
				}//end if (visitIter->VisVars.size() > 0)
			
			}//end for visitIter

			//cout << "Before Actual Insertion" << endl;

			//printWeeklySchedule();
			//cout << endl << endl;
			/*
				for (visitIter = removedVisits.begin(); visitIter != removedVisits.end(); ++visitIter)
				{
					cout << "Visit->installation = " << visitIter->getOffshoreInst()->getInstName() << endl;
					cout << "VisVars.size() = " << visitIter->VisVars.size() << endl;
					if (visitIter->VisVars.size() > 0)
						cout << "RegretValues[0] = " << visitIter->RegretValues[0] << endl;
				}
				*/


			smallestInsertionCost = numeric_limits<double>::max();
			largestRegret = 0;

			j = 0;
			bestIdxInsert = 0;
			bestIdxRegret = 0;
			bestRouteDaysDelta = 1;

			//Identify the insertion
			for (visitIter = removedVisits.begin(); visitIter != removedVisits.end(); ++visitIter)
			{
				
				if ( (visitIter->VisVars.size() == 1) 
					&& (visitIter->VisVars[0].getRouteDaysDelta() <= bestRouteDaysDelta )
					&& (visitIter->RegretValues[0] <= smallestInsertionCost) )
				{
					
					smallestInsertionCost = visitIter->RegretValues[0];
					bestIdxInsert = j;
					bestRouteDaysDelta = visitIter->VisVars[0].getRouteDaysDelta();
				}
				else if ( (visitIter->VisVars.size() > 1) 
					&& (visitIter->VisVars[0].getRouteDaysDelta() <= bestRouteDaysDelta)
					&& (visitIter->RegretValues[0] >= largestRegret) )
				{
					
					largestRegret = visitIter->RegretValues[0];
					bestIdxRegret = j;
					bestRouteDaysDelta = visitIter->VisVars[0].getRouteDaysDelta();
				}

				j++;
			}

			if ( (largestRegret == 0) && (smallestInsertionCost == 
				numeric_limits<double>::max()) )
				insertionPossible = false;

			else if ( smallestInsertionCost < 
				numeric_limits<double>::max() )
			{
				/*
				cout << "BEST INSERTION: visit = " << removedVisits[bestIdxInsert].VisVars[0].
					getVarVisit()->getOffshoreInst()->getInstName() << endl;

				cout << "Inserting to route " << removedVisits[bestIdxInsert].VisVars[0].
					getRouteTo()->getRouteNum() << endl;

				cout << "Insertion cost = " << removedVisits[bestIdxInsert].VisVars[0].
					getVarVisit()->RegretValues[0] << endl;
					*/
			
				routeDays = (int)ceil( (removedVisits[bestIdxInsert].VisVars[0].getRouteTo()->computeRouteDuration()+ 
				removedVisits[bestIdxInsert].VisVars[0].getRouteTo()->getInstAtPos(0)->getLayTime()
				+ removedVisits[bestIdxInsert].VisVars[0].getRouteTo()->getRouteMinSlack() )/24);

				routeStartIdx = (int) ceil(removedVisits[bestIdxInsert].VisVars[0].getRouteTo()->getRouteStartTime()/24);
				routeEndIdx = routeStartIdx + routeDays - 1;

				//Update VesAvail
				if (removedVisits[bestIdxInsert].VisVars[0].getRouteDaysDelta() > 0)
				{
					if (removedVisits[bestIdxInsert].VisVars[0].getRouteTo() == otherBaseRoute )
						for (j = 1; j <=removedVisits[bestIdxInsert].VisVars[0].getRouteDaysDelta(); j++)
								otherSched->VesAvail[removedVisits[bestIdxInsert].VisVars[0].getRouteTo()->getRouteVes()->getID()]
										[routeEndIdx + j] = false;
					else
						for (j = 1; j <= removedVisits[bestIdxInsert].VisVars[0].getRouteDaysDelta(); j++)
							VesAvail[removedVisits[bestIdxInsert].VisVars[0].getRouteTo()->getRouteVes()->getID()]
									[routeEndIdx + j] = false;

				}

				removedVisits[bestIdxInsert].VisVars[0].getRouteTo()->insertInstalVisit(1, removedVisits[bestIdxInsert]);
				removedVisits[bestIdxInsert].VisVars[0].getRouteTo()->updateVisitVector();
				removedVisits[bestIdxInsert].VisVars[0].getRouteTo()->intelligentReorder();

				
				//Update curVisDayComb
				removedVisits[bestIdxInsert].VisVars[0].getVarVisit()->getOffshoreInst()->InsertDayToVisDayComb(
					(int) ceil(removedVisits[bestIdxInsert].VisVars[0].getRouteTo()->getRouteStartTime()/24) );

				//Update intermedRoutes
				for (routeIntermedIter = intermedRoutes.begin(); 
							routeIntermedIter->getRouteNum() != 
							removedVisits[bestIdxInsert].VisVars[0].getRouteTo()->getRouteNum();
							++routeIntermedIter)
							;
				*routeIntermedIter = *(removedVisits[bestIdxInsert].VisVars[0].getRouteTo());

				//Erase the visit from removedVisits
				removedVisits.erase(removedVisits.begin() + bestIdxInsert);

				//printWeeklySchedule();
				//cout << endl << endl;
				
			}//end else if

			else if (largestRegret > 0)
			{

				/*
				cout << "BEST REGRET INSERTION: visit = " << removedVisits[bestIdxRegret].VisVars[0].
					getVarVisit()->getOffshoreInst()->getInstName() << endl;

				cout << "Inserting to route " << removedVisits[bestIdxRegret].VisVars[0].
					getRouteTo()->getRouteNum() << endl;

				cout << "REGRET VALUE = " << removedVisits[bestIdxRegret].VisVars[0].
					getVarVisit()->RegretValues[0] << endl;
					*/

				routeDays = (int)ceil( (removedVisits[bestIdxRegret].VisVars[0].getRouteTo()->computeRouteDuration()+ 
				removedVisits[bestIdxRegret].VisVars[0].getRouteTo()->getInstAtPos(0)->getLayTime()
				+ removedVisits[bestIdxRegret].VisVars[0].getRouteTo()->getRouteMinSlack() )/24);

				routeStartIdx = (int) ceil(removedVisits[bestIdxRegret].VisVars[0].getRouteTo()->getRouteStartTime()/24);
				routeEndIdx = routeStartIdx + routeDays - 1;

				//Update VesAvail before insertion
				//cout << "before VesAvail" << endl;
				if (removedVisits[bestIdxRegret].VisVars[0].getRouteDaysDelta() > 0)
				{
					if (removedVisits[bestIdxRegret].VisVars[0].getRouteTo() == otherBaseRoute)
						for (j = 1; j <= removedVisits[bestIdxRegret].VisVars[0].getRouteDaysDelta(); j++)
							otherSched->VesAvail[removedVisits[bestIdxRegret].VisVars[0].getRouteTo()->getRouteVes()->getID()]
									[routeEndIdx + j] = false;
					else
						for (j = 1; j <=removedVisits[bestIdxRegret].VisVars[0].getRouteDaysDelta(); j++)
							VesAvail[removedVisits[bestIdxRegret].VisVars[0].getRouteTo()->getRouteVes()->getID()]
									[routeEndIdx + j] = false;

				}
				//cout << "after VesAvail" << endl;

				removedVisits[bestIdxRegret].VisVars[0].getRouteTo()->insertInstalVisit(1, removedVisits[bestIdxRegret]);
				removedVisits[bestIdxRegret].VisVars[0].getRouteTo()->updateVisitVector();
				removedVisits[bestIdxRegret].VisVars[0].getRouteTo()->intelligentReorder();

				//Update curVisDayComb
				removedVisits[bestIdxRegret].VisVars[0].getVarVisit()->getOffshoreInst()->InsertDayToVisDayComb(
					(int) ceil(removedVisits[bestIdxRegret].VisVars[0].getRouteTo()->getRouteStartTime()/24) );

				//Update intermedRoutes
				for (routeIntermedIter = intermedRoutes.begin(); 
							routeIntermedIter->getRouteNum() != 
							removedVisits[bestIdxRegret].VisVars[0].getRouteTo()->getRouteNum();
							++routeIntermedIter)
							;
				*routeIntermedIter = *(removedVisits[bestIdxRegret].VisVars[0].getRouteTo());

				//Erase the visit from removedVisits
				removedVisits.erase(removedVisits.begin() + bestIdxRegret);
			}
			else
			{
				cout << "ERROR IN PERFORMING INSERTION - goLNS()!!!" << endl;
			}

			//cout << "Olli Jokkinen" << endl;

		}//end while
	
		if ( (removedVisits.size() == 0) && isSchedFeasible() )
		{
			//cout << "Inside IF" << endl;
			//cout << "removedVisits.size() = " << removedVisits.size() << endl;
			//cout << "isSchedFeasible() = " << isSchedFeasible() << endl;
			//cout << "Schedule cost = " << computeSchedCost() << endl;
			//printWeeklySchedule();

			/*
			if ( schedCost < bestSchedCost )
			{
				cout << "schedCost = " << schedCost << endl;
				cout << "bestSchedCost = " << bestSchedCost << endl;
				cout << "*curBestCost = " << *curBestCost << endl;
				*curBestCost = schedCost;
				bestSchedCost = schedCost;
				//printWeeklySchedule();
				writeScheduleToFile();
			}	

			*/

			synchrVisitInstalConnection();

			/*
			//Just to enter the loop

			localCostAfter = 0;
			localCostBefore = 1;
			while (localCostAfter < localCostBefore)
			{
				
				localCostBefore = computeSchedCost();
				//cout << "localCostBefore = " << localCostBefore << endl;

				//reduceNumberOfRoutes();


				//cout << "1";
				

				//reassignVesselsToRoutes();


				//cout << "2";

			
				//reduceRouteDurTotDays();
				//cout << "3";
				//reassignVesselsToRoutes();
				//cout << "4";
				relocateVisits();
				//cout << "5";
				reassignVesselsToRoutes();
				//cout << "6";

				//isolateVoyage();
				synchrVisitInstalConnection();
				localCostAfter = computeSchedCost();
				//cout << "localCostAfter = " << localCostAfter << endl;
			}
		
			//cout << "After local loop" << endl;
			

			
			if (Routes.size() < intermedRoutes.size())
			{
				intermedRoutes.clear();
				savedRoutes.clear();
				intermedRoutes.resize(Routes.size());
				savedRoutes.resize(Routes.size());
			}
			*/

			reassignVesselsToRoutes();

			//Check if solutiong improves the best found so far
			schedCost = computeSchedCost();
			schedCost += otherSched->computeSchedCost();

			//cout << "cost in multiLNS = " << schedCost << endl;

			if ( schedCost < *curBestCost )
			{
				
				cout << "best MultiSchedCost in multiLNS = " << schedCost << endl;
				*curBestCost = schedCost;
				bestSchedCost = schedCost;
				//printWeeklySchedule();

				//writeScheduleToFile(2708, i);

				*bs = *this;
				//if (outerIter > 0)
				//	iter += 10;
			}	
			
			
			if (Routes.size() < intermedRoutes.size())
			{
				intermedRoutes.clear();
				//savedRoutes.clear();
				intermedRoutes.resize(Routes.size());
				//savedRoutes.resize(Routes.size());
			}

			//Keep the changes even if the solution cost is worse
			for (k = 0; k < Routes.size(); k++)
			{
				intermedRoutes[k] = Routes[k];
				//savedRoutes[k] = Routes[k];
			}
		}//end if

		synchrVisitInstalConnection();

	}//end for i
	//cout << "At the end of LNS schedCost = " << computeSchedCost() << endl;
	//printWeeklySchedule();

	/*
	cout << "Printing trialScheds " << endl;
	for (k = 0; k < trialScheds->size(); k++)
	{
		//cout << "IsolRouteStartTime = " << trialScheds->operator [](k).getIsolRoute()->getRouteStartTime() << endl;
		//cout << "SchedCost = " << trialScheds->operator [](k).computeSchedCost() << endl;
		trialScheds->operator [](k).printWeeklySchedule();
	}
	*/

}

int WeeklySchedule::calcNumLNSiter()
{
		if (Instance->getPlatforms().size() <= 6)
			return 5;
		else if ( (Instance->getPlatforms().size() >= 7)
			&& (Instance->getPlatforms().size() < 10) )
			return 10;
		else if ( (Instance->getPlatforms().size() >= 10)
			&& (Instance->getPlatforms().size() <= 13) )
			return 50;
		else
			return 75;
}
void WeeklySchedule::synchrVisitInstalConnection()
{
	
	vector <OffshoreInst>::iterator instalIter;

	vector <Visit*> routeVisits;
	vector <Visit*>::iterator visIter;

	vector <Route>::iterator routeIter;

	for (routeIter = Routes.begin(); routeIter != Routes.end(); ++routeIter)
	{
		routeVisits.clear();
		routeVisits = routeIter->getVisitPointers();

		for (visIter = routeVisits.begin(); visIter != routeVisits.end(); ++visIter)
			for (instalIter = SchedInstals.begin(); instalIter != SchedInstals.end(); ++instalIter)
				if ( instalIter->getInstName() == (*visIter)->getOffshoreInst()->getInstName() )
					(*visIter)->setOffshoreInst(&(*instalIter));
	} // end for routeIter

}
int WeeklySchedule::calcNumVesselsUsed()
{
	vector <SupVes>::iterator vesIter;
	int count = 0;

	for (vesIter = SchedVessels.begin(); vesIter != SchedVessels.end(); ++vesIter)
		if (vesIter->getIsVesselUsed())
			count++;

	return count;
}