#include "StdAfx.h"
#include "GeneralSchedule.h"

GeneralSchedule::GeneralSchedule(void)
{
}

GeneralSchedule::GeneralSchedule(int numberWeeks, int numberBases)
{
	numWeeks = numberWeeks;
	numBases = numberBases;
	schedVector.resize(numWeeks);
}

void GeneralSchedule::makeSchedule()
{
	int i, j, k;

	vector <Route*>::iterator routeIter;
	vector <Route*> vesRoutes;
	//Construct and optimize schedules here
	for (i = 0; i < potentialScheds->size(); i++)
	{
		potentialScheds->operator [](i).updateInterBaseInfo();
		potentialScheds->operator [](i).updateVisitInstalConnection();
		potentialScheds->operator [](i).setMultiSchedID(i);
		
		//potentialScheds->operator [](i).printMultiSchedule();
		
		//Pointing at the WRONG objects!!!
		
		//cout << "For MultiSchedule Relational Info" << i << ": " << endl;
		//potentialScheds[i].printMultiRelationalInfo();
		/*cout << "Shared vessel is: " << endl;
		potentialScheds[i].getSharedVessel()->getName();

		vesRoutes.clear();
		vesRoutes = potentialScheds[i].getSharedVessel()->getVesselRoutes();

		cout << "Its Routes are: " << endl;
		for (routeIter = vesRoutes.begin(); routeIter != vesRoutes.end(); ++routeIter)
			(*routeIter)->printRoute();	
		*/
	}

	vesRoutes.clear();

	vector <MultiSchedule>::iterator multSchedIter;
	vector <MultiSchedule>::iterator otherMultSchedIter;
	Route *firstSchedEarlierRoute, *firstSchedLaterRoute;
	Route *secondSchedEarlierRoute, *secondSchedLaterRoute;
	Route *firstSchedAnotherBaseRoute, *secondSchedAnotherBaseRoute, *anotherBaseRoute;

	MultiSchedule localSched, otherLocalSched;

	double bestGenSchedCost = numeric_limits<double>::max();
	double genCost;
	bool overlap;
	vector <Route*> schedRoutes;
	vector <Route*> interBaseRoutes;
	vector <Route*> otherInterBaseRoutes;
	int firstIdxOptimize, secondIdxOptimize;
	double bestMultiCost;
	bool counter;

	WeeklySchedule firstWeekBestSched, secondWeekBestSched;

	vector <SupVes*> ScheduleVessels;
	vector <SupVes*>::iterator vesIter;

	vector< vector <bool> > schedVesselsUsed;
	vector< vector <bool> > otherSchedVesselsUsed;



	bool sameVesselsUsed;

	for (multSchedIter = potentialScheds->begin(); multSchedIter != potentialScheds->end(); ++multSchedIter)
	{
	
		localSched = *multSchedIter;
		counter = false;
		
		schedVesselsUsed.clear();
		schedVesselsUsed.resize(multSchedIter->Schedules.size());

		for (i = 0; i < multSchedIter->Schedules.size(); i++)
		{
			ScheduleVessels.clear();
			ScheduleVessels = multSchedIter->Schedules[i].getSchedVesselsPointers();

			j = 0;
			schedVesselsUsed[i].resize(ScheduleVessels.size());

			for (vesIter = ScheduleVessels.begin(); vesIter != ScheduleVessels.end(); ++vesIter)
			{
				schedVesselsUsed[i][j] = (*vesIter)->getIsVesselUsed();
				j++;
			}
		}


		for(otherMultSchedIter = potentialScheds->begin(); otherMultSchedIter != potentialScheds->end(); ++otherMultSchedIter)
		{

			otherSchedVesselsUsed.clear();
			otherSchedVesselsUsed.resize(otherMultSchedIter->Schedules.size());

			for (i = 0; i < otherMultSchedIter->Schedules.size(); i++)
			{
				ScheduleVessels.clear();
				ScheduleVessels = otherMultSchedIter->Schedules[i].getSchedVesselsPointers();

				j = 0;
				otherSchedVesselsUsed[i].resize(ScheduleVessels.size());

				for (vesIter = ScheduleVessels.begin(); vesIter != ScheduleVessels.end(); ++vesIter)
				{
					otherSchedVesselsUsed[i][j] = (*vesIter)->getIsVesselUsed();
					j++;
				}
			}

			sameVesselsUsed = true;
			for (i = 0; i < schedVesselsUsed.size(); i++)
				for (j = 0; j < schedVesselsUsed[i].size(); j++)
					if (schedVesselsUsed[i][j] != otherSchedVesselsUsed[i][j])
					{
						sameVesselsUsed = false;
						break;
					}

			if ( (multSchedIter->getMultiSchedID() < otherMultSchedIter->getMultiSchedID())
				&& sameVesselsUsed)
			{

				
				otherLocalSched  = *otherMultSchedIter;

				//Set route pointers for FIRST multiSchedule
				if (!counter)
				{
					//cout << "yes we are inside" << endl;

					schedRoutes.clear();
					schedRoutes = multSchedIter->Schedules[0].getRoutePointers();

					interBaseRoutes.clear();

					for (routeIter = schedRoutes.begin(); routeIter != schedRoutes.end(); ++routeIter)
						if ( (*routeIter)->getInstAtPos(0)->getInstName() != 
							(*routeIter)->getInstAtPos((*routeIter)->getVisitObjects().size() - 1)->getInstName() )
							interBaseRoutes.push_back(*routeIter);

					schedRoutes.clear();
					schedRoutes = multSchedIter->Schedules[1].getRoutePointers();

					for (routeIter = schedRoutes.begin(); routeIter != schedRoutes.end(); ++routeIter)
						if ( (*routeIter)->getInstAtPos(0)->getInstName() != 
							(*routeIter)->getInstAtPos((*routeIter)->getVisitObjects().size() - 1)->getInstName() )
							interBaseRoutes.push_back(*routeIter);

					
					multSchedIter->setSharedVessel(interBaseRoutes[0]->getRouteVes());

					//multSchedIter->printMultiSchedule();
					//cout << "At the end of inside" << endl;
				}
				//Set route pointers for SECOND multiSchedule
				schedRoutes.clear();
				schedRoutes = otherMultSchedIter->Schedules[0].getRoutePointers();

				otherInterBaseRoutes.clear();

				for (routeIter = schedRoutes.begin(); routeIter != schedRoutes.end(); ++routeIter)
					if ( (*routeIter)->getInstAtPos(0)->getInstName() != 
						(*routeIter)->getInstAtPos((*routeIter)->getVisitObjects().size() - 1)->getInstName() )
						otherInterBaseRoutes.push_back(*routeIter);

				schedRoutes.clear();
				schedRoutes = otherMultSchedIter->Schedules[1].getRoutePointers();

				for (routeIter = schedRoutes.begin(); routeIter != schedRoutes.end(); ++routeIter)
					if ( (*routeIter)->getInstAtPos(0)->getInstName() != 
						(*routeIter)->getInstAtPos((*routeIter)->getVisitObjects().size() - 1)->getInstName() )
						otherInterBaseRoutes.push_back(*routeIter);

				otherMultSchedIter->setSharedVessel(otherInterBaseRoutes[0]->getRouteVes());


				//cout << "Here" << endl;
				//cout << "interBaseRoutes.size() = " << interBaseRoutes.size() << endl;
				//cout << "otherInterBaseRoutes.size() = " << otherInterBaseRoutes.size() << endl;

				//Check for potential synergy
				if ( ( (interBaseRoutes[0]->getRouteStartTime() > interBaseRoutes[1]->getRouteStartTime() )
					&& (otherInterBaseRoutes[0]->getRouteStartTime() <
				otherInterBaseRoutes[1]->getRouteStartTime())
				&& (multSchedIter->getSharedVessel()->getName() == otherMultSchedIter->getSharedVessel()->getName()) )
				|| ( ( interBaseRoutes[0]->getRouteStartTime() < interBaseRoutes[1]->getRouteStartTime() )
					&& (otherInterBaseRoutes[0]->getRouteStartTime() >
				otherInterBaseRoutes[1]->getRouteStartTime())
				&& (multSchedIter->getSharedVessel()->getName() == otherMultSchedIter->getSharedVessel()->getName()) ) )
				{
					//Do stuff here
					cout << "BIWEEKLY POTENTIAL" << endl;
					firstWeekMultiSchedule = &(*multSchedIter);
					secondWeekMultiSchedule = &(*otherMultSchedIter);

					//printGeneralSchedule();

					/*
					firstWeekMultiSchedule->updateInterBaseInfo();
					secondWeekMultiSchedule->updateInterBaseInfo();

					firstWeekMultiSchedule->updateVisitInstalConnection();
					secondWeekMultiSchedule->updateVisitInstalConnection();

					firstWeekMultiSchedule->synchrScheduleVector();
					secondWeekMultiSchedule->synchrScheduleVector();
					*/

					if (!counter)
					{
						if (interBaseRoutes[0]->getRouteStartTime()
							< interBaseRoutes[1]->getRouteStartTime())
						{
							firstSchedEarlierRoute = interBaseRoutes[0];
							firstSchedLaterRoute = interBaseRoutes[1];
							firstIdxOptimize = 1;
						}
						else
						{
							firstSchedEarlierRoute = interBaseRoutes[1];
							firstSchedLaterRoute = interBaseRoutes[0];
							firstIdxOptimize = 0;
						}
					}

					if (otherInterBaseRoutes[0]->getRouteStartTime()
						< otherInterBaseRoutes[1]->getRouteStartTime())
					{
						secondSchedEarlierRoute = otherInterBaseRoutes[0];
						secondSchedLaterRoute = otherInterBaseRoutes[1];
						secondIdxOptimize = 1;
					}
					else
					{
						secondSchedEarlierRoute = otherInterBaseRoutes[1];
						secondSchedLaterRoute = otherInterBaseRoutes[0];
						secondIdxOptimize = 0;
					}

					
					//Set anotherBaseRoute pointers
					ScheduleVessels.clear();
					ScheduleVessels = firstWeekMultiSchedule->Schedules[firstIdxOptimize].getSchedVesselsPointers();

					for (vesIter = ScheduleVessels.begin(); vesIter != ScheduleVessels.end(); ++vesIter)
					{
						if ( (*vesIter)->getName() == firstSchedLaterRoute->getRouteVes()->getName() )
						{
							anotherBaseRoute = secondSchedLaterRoute;
							break;
						}
						anotherBaseRoute = firstSchedLaterRoute;
					}

					if (anotherBaseRoute == firstSchedLaterRoute)
					{
						firstSchedAnotherBaseRoute = firstSchedLaterRoute;
						secondSchedAnotherBaseRoute = secondSchedEarlierRoute;
					}
					else
					{
						firstSchedAnotherBaseRoute = firstSchedEarlierRoute;
						secondSchedAnotherBaseRoute = secondSchedLaterRoute;
					}
					//


					//Try doing modifications and see whether it is feasible
					//First schedule
					//cout << "FirstSchedLaterRoute before" << endl;
					//firstSchedLaterRoute->printRoute();

					if (!counter)
					{
						firstSchedLaterRoute->deleteInstalVisit(firstSchedLaterRoute->getVisitObjects().size() - 1);
						firstSchedLaterRoute->addRouteInstal(firstSchedLaterRoute->getInstAtPos(0));
						firstSchedLaterRoute->addVisit(Visit(firstSchedLaterRoute->getInstAtPos(0)));
						firstSchedLaterRoute->updateVisitVector();
						firstSchedLaterRoute->intelligentReorder();

						firstSchedEarlierRoute->getInstAtPos(firstSchedEarlierRoute->getVisitObjects().size() - 1)->setInstName(
							secondSchedEarlierRoute->getInstAtPos(0)->getInstName());
						//cout << "AFTER" << endl;
						//firstSchedLaterRoute->printRoute();
						/*
						cout << "Beginning of route" << endl;
						cout << "Latitude = " << firstSchedEarlierRoute->getInstAtPos(0)->getLatitude() << endl;
						cout << "Longitude = " << firstSchedEarlierRoute->getInstAtPos(0)->getLongitude() << endl;
						cout << "InstName = " << firstSchedEarlierRoute->getInstAtPos(0)->getInstName() << endl;

						cout << "At the end of interbase route" << endl;
						cout << "Latitude = " << firstSchedEarlierRoute->getInstAtPos(firstSchedEarlierRoute->getVisitObjects().size() - 1)->getLatitude() << endl;
						cout << "Longitude = " << firstSchedEarlierRoute->getInstAtPos(firstSchedEarlierRoute->getVisitObjects().size() - 1)->getLongitude() << endl;
						cout << "InstName = " << firstSchedEarlierRoute->getInstAtPos(firstSchedEarlierRoute->getVisitObjects().size() - 1)->getInstName() << endl;

						*/
						//updateInterBaseInfo();

						//cout << "secondSchedLaterRoute before" << endl;
						//secondSchedLaterRoute->printRoute();
					}

					secondSchedEarlierRoute->getInstAtPos(secondSchedEarlierRoute->getVisitObjects().size() - 1)->setInstName(
						firstSchedEarlierRoute->getInstAtPos(0)->getInstName());

					//Second Schedule
					secondSchedLaterRoute->addRouteInstal(secondSchedLaterRoute->getInstAtPos(0));
					secondSchedLaterRoute->addVisit(Visit(secondSchedLaterRoute->getInstAtPos(0)));
					secondSchedLaterRoute->deleteInstalVisit(secondSchedLaterRoute->getVisitObjects().size() - 2);
					secondSchedLaterRoute->updateVisitVector();
					secondSchedLaterRoute->intelligentReorder();

					//cout << "AFTER" << endl;
					//secondSchedLaterRoute->printRoute();

					/*
					cout << "FirstSchedEARLIER Route========" << endl;
					firstSchedEarlierRoute->printRoute();
					cout << "FirstSchedLATER Route==========" << endl;
					firstSchedLaterRoute->printRoute();

					cout << "SecondSchedEARLIER Route" << endl;
					secondSchedEarlierRoute->printRoute();
					cout << "SecondSchedLATER Route========" << endl;
					secondSchedLaterRoute->printRoute();
					*/

					overlap = true;

					//cout << "AFTER MODIFICATIONS" << endl;
					//printGeneralSchedule();

					//Check if the resulting voyages overlap
					if ( ( firstSchedLaterRoute->getRouteEndTime() + firstSchedLaterRoute->getRouteMinSlack()
						< 168 + secondSchedEarlierRoute->getRouteStartTime()
						- secondSchedEarlierRoute->getInstAtPos(0)->getLayTime() )
						&& (secondSchedLaterRoute->getRouteEndTime() + secondSchedLaterRoute->getRouteMinSlack() 
						< 168 + firstSchedEarlierRoute->getRouteStartTime() - 
						firstSchedEarlierRoute->getInstAtPos(0)->getLayTime() ) )

						overlap = false;

					if (!overlap) 
					{
						//Post-optimize affected schedules			
						firstWeekBestSched = firstWeekMultiSchedule->Schedules[firstIdxOptimize];
						bestMultiCost = firstWeekMultiSchedule->computeMultiSchedCost();

						cout << "Before FIRST week multiLNS bestMultiCost = " << bestMultiCost << endl;

						//firstWeekMultiSchedule->writeMultiScheduleToFile(999);
						if (!counter)
						{
							firstWeekMultiSchedule->Schedules[firstIdxOptimize].multiLNS(
								firstWeekMultiSchedule->Schedules[firstIdxOptimize].calcNumLNSiter(),
								&bestMultiCost, &firstWeekMultiSchedule->Schedules[firstIdxOptimize ? 0 : 1],
								&firstWeekBestSched, firstSchedAnotherBaseRoute);

							
							firstWeekMultiSchedule->Schedules[firstIdxOptimize] = firstWeekBestSched;
						}
						cout << "After FIRST week multiLNS bestMultiCost = " << bestMultiCost << endl;
						
						//firstWeekMultiSchedule->updateInterBaseInfo();
						//firstWeekMultiSchedule->updateVisitInstalConnection();


						secondWeekBestSched = secondWeekMultiSchedule->Schedules[secondIdxOptimize];
						bestMultiCost = secondWeekMultiSchedule->computeMultiSchedCost();

						cout << "Before SECOND week multiLNS bestMultiCost = " << bestMultiCost << endl;

						//secondWeekMultiSchedule->writeMultiScheduleToFile(888);
						secondWeekMultiSchedule->Schedules[secondIdxOptimize].multiLNS(
							secondWeekMultiSchedule->Schedules[secondIdxOptimize].calcNumLNSiter(),
							&bestMultiCost, &secondWeekMultiSchedule->Schedules[secondIdxOptimize ? 0 : 1],
							&secondWeekBestSched, secondSchedAnotherBaseRoute);
						
						secondWeekMultiSchedule->Schedules[secondIdxOptimize] = secondWeekBestSched;
						cout << "After SECOND week multiLNS bestMultiCost = " << bestMultiCost << endl;
						

						//secondWeekMultiSchedule->updateInterBaseInfo();
						//secondWeekMultiSchedule->updateVisitInstalConnection();


						//End of postLNS//

						genCost = computeGeneralSchedCost();
						if (genCost < bestGenSchedCost)
						{
							bestGenSchedCost = genCost;
							writeGeneralScheduleToFile();
							cout << "Best general cost = " << genCost << endl;
						}

					}//end if (!overlap)
					counter = true;
				}//end if (Bi-Weekly potential)
				

				//cout << "after if" << endl;
				*otherMultSchedIter = otherLocalSched;
				otherMultSchedIter->updateInterBaseInfo();
				otherMultSchedIter->updateVisitInstalConnection();

			}//end if (sameVesselsUsed)

		}//end for(otherMultSchedIter)

		*multSchedIter = localSched;
		multSchedIter->updateInterBaseInfo();
		multSchedIter->updateVisitInstalConnection();

	}//end for (multSchedIter)
}

void GeneralSchedule::printGeneralSchedule()
{
	//vector <MultiSchedule>::iterator multSchedIter;
	cout << "Week 1 schedule" << endl;
	firstWeekMultiSchedule->printMultiSchedule();
	cout << "Week 2 schedule" << endl;
	secondWeekMultiSchedule->printMultiSchedule();
}
void GeneralSchedule::writeGeneralScheduleToFile()
{
	const char* fileName;
	fileName = "C:/Aliaksandr Shyshou/Supply Vessels 2008-2010/Multi base/MongstadInst_MBPSVPP_Heuristic/output.txt";
	ofstream solFile(fileName, ios::app);
	solFile << "================================================================" << endl;
	solFile << "General OUTPUT" << endl;
	solFile << "General schedule COST = " << computeGeneralSchedCost() << endl;
	
	solFile << "Week 1 schedule" << endl;
	firstWeekMultiSchedule->writeMultiScheduleToFile(1983);
	solFile << "Week 2 schedule" << endl;
	secondWeekMultiSchedule->writeMultiScheduleToFile(1983);
		
	solFile << "================================================================" << endl;
}
double GeneralSchedule::computeGeneralSchedCost()
{
	double cost;

	cost = firstWeekMultiSchedule->computeMultiSchedCost();
	cost += secondWeekMultiSchedule->computeMultiSchedCost();

	return cost;
}