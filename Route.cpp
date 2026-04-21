#include "StdAfx.h"
#include "Route.h"

Route::Route(void)
{
}

Route::Route (int num, double stTime)
{
	routeNum = num;
	routeStartTime = stTime;
}

void Route::addRouteInstal (OffshoreInst *someInst)
{
	RouteInstals.push_back ( someInst );
}

void Route::addVisit(Visit vis)
{
	RouteVisits.push_back(vis);
}


OffshoreInst* Route::getInstAtPos (int pos)
{
	//cout << "pos = " << pos << endl;
	if (pos >= 0 && pos < RouteInstals.size() ) 
		return RouteInstals[pos];
	else
	{
		cout << "Bad index in getInstAtPos!!!" << endl;
		cout << "Index = " << pos << endl;
		cout << "Route number = " << getRouteNum() << endl;
		printRoute();
		cout << "RouteInstals.size() = " << RouteInstals.size() << endl;
		
		return NULL;
	}
}

vector < Visit* > Route::getVisitPointers()
{
	vector <Visit>::iterator visIter;
	vector <Visit*> visPointers;

	for (visIter = RouteVisits.begin(); visIter != RouteVisits.end(); ++visIter)
		visPointers.push_back(&(*visIter));

	return visPointers;
}

double Route::computeRouteLength()
{
	double length;
	int i;
	length = 0;
	for (i = 0; i < RouteInstals.size()-1; i++)	
		length += RouteInstals[i]->Dist[RouteInstals[i+1]->getSeqNumb()];

	return length;
}
	
double Route::computeRouteDuration()
{
	return (getRouteEndTime() - getRouteStartTime() ); //RouteVes->getSpeed());
}

double Route::computeRouteDemand()
{
	double load = 0;
	int k;
	for (k = 1; k < RouteInstals.size()-1; k++)
		load += (RouteInstals[k]->getWeeklyDemand()) / 
			(RouteInstals[k]->getVisitFreq());

	return load;
}	

double Route::computeRouteSlack()
{
	double slack = 0;
	slack = MAX_ROUTE_DUR - computeRouteDuration();

	while (slack > 24)
		slack -= 24;

	return slack;
}


bool Route::isLoadFeasible()
{
	double load = 0;
	int k;
	for (k = 1; k < RouteInstals.size()-1; k++)
		load += (RouteInstals[k]->getWeeklyDemand()) / 
			(RouteInstals[k]->getVisitFreq());

	return (RouteVes->getCapacity() >= load);
}

bool Route::isNumInstFeasible()
{
	return (RouteVisits.size()-2 <= maxVisits);
}

bool Route::isRouteDurFeasible()
{
	return (routeEndTime - routeStartTime < MAX_ROUTE_DUR + routeAcceptanceTime);
}

bool Route::isFeasible()
{
	return (isLoadFeasible() && isNumInstFeasible() && isRouteDurFeasible());
}

void Route::cheapInsertSeq()
{
	//Go through the route and reinsert each installation
	//into the best position
	int i, j, pos, seq_num;
	double cost, bestCost, length; 

	for (i = 1; i < RouteInstals.size() - 1; i++)
	{
		cost = 0;
		bestCost = numeric_limits<double>::max();
		seq_num = RouteInstals[i]->getSeqNumb();
		length = computeRouteLength();

		//Determine the best position of the platform
		for (j = 0; j < RouteInstals.size()-1; j++)
		{
				cost = RouteInstals[j]->Dist[RouteInstals[i]->getSeqNumb()]+
				RouteInstals[j+1]->Dist[RouteInstals[i]->getSeqNumb()];
				if (cost < bestCost)
				{
					bestCost = cost;
					pos = j; //insert after j
				}
		}
		//Do the insertion if the position is different
		if (pos + 1 != i)
		{
			RouteInstals.insert(RouteInstals.begin() + pos + 1, RouteInstals[i]);
			//Remove the duplicate
			if (RouteInstals[i]->getSeqNumb() == seq_num)
				RouteInstals.erase(RouteInstals.begin() + i);
			else if (RouteInstals[i+1]->getSeqNumb() == seq_num)
				RouteInstals.erase(RouteInstals.begin() + i + 1);
		}

		//Check if we are worsening
		if (computeRouteLength() > length)
		{//reverse the changes
			if (RouteInstals[pos+1]->getSeqNumb() == seq_num)
			{
				if (i > pos + 1)
				{
					RouteInstals.insert(RouteInstals.begin() + i + 1, RouteInstals[pos + 1]);
					RouteInstals.erase(RouteInstals.begin() + pos + 1);
				}
				else
				{
					RouteInstals.insert(RouteInstals.begin() + i, RouteInstals[pos + 1]);
					RouteInstals.erase(RouteInstals.begin() + pos + 2);
				}

			}
			else if (RouteInstals[pos]->getSeqNumb() == seq_num)
			{
				if (i > pos)
				{
					RouteInstals.insert(RouteInstals.begin() + i + 1, RouteInstals[pos]);
					RouteInstals.erase(RouteInstals.begin() + pos);
				}
				else
				{
					RouteInstals.insert(RouteInstals.begin() + i, RouteInstals[pos]);
					RouteInstals.erase(RouteInstals.begin() + pos + 1);
				}
		
			}
		} //end if
	}//end for i
}

void Route::printRoute()
{
	/*
	vector < OffshoreInst* >::iterator it;
	for (it = RouteInstals.begin(); it != RouteInstals.end(); ++it)
		cout << (*it)->getInstName() << " ";
	cout << endl;
*/
	cout << "Vessel = " << RouteVes->getName() << endl;
	vector < Visit >::iterator iter;
	for (iter = RouteVisits.begin(); iter != RouteVisits.end(); ++iter)
	{
		cout << iter->getOffshoreInst()->getInstName() << " ";
		cout << "Arrival = " << iter->getVisitStart() << " ";
		cout << "Departure = " << iter->getVisitEnd() << " ";
		cout << "WaitTime = " << iter->getVisitWaitTime();
		cout << endl;
	}

	/*
	if (isFeasible())
		cout << "The route is feasible: ";
	else
		cout << "The route is INFEASIBLE: ";

	if (isLoadFeasible())
		cout << "Load feasible, ";
	else
	{
		cout << "Load INFEASIBLE, ";
		cout << "Route load = " << computeRouteDemand()
			<< ", Vessel capacity = " << RouteVes->getCapacity() << endl;
	}
	if (isNumInstFeasible())
		cout << "NumInst feasible, ";
	else
		cout << "NumInst INFEASIBLE, ";

	if (isRouteDurFeasible())
		cout << "Route Duration feasible." << endl;
	else
		cout << "Route Duration INFEASIBLE." << endl;
*/
	cout << endl;
}

void Route::updateVisitVector()
{
	vector < Visit >::iterator iter;
	double arrival, departure, waitClock;
	double arrTime;
	arrival = 0; departure = 0; waitClock = 0;

	for (iter = RouteVisits.begin(); iter != RouteVisits.end(); ++iter)
	{
		if (iter == RouteVisits.begin())//base at the beginning
		{
			//(*iter)->setVisitStart(routeStartTime - 8);
			iter->setVisitEnd(routeStartTime); //leave when the route starts
			iter->setVisitWaitTime(0);
		}
		else if (iter == RouteVisits.end() - 1)//base at the end
		{
			arrival = (iter-1)->getVisitEnd() + 
				iter->getOffshoreInst()->Dist[(iter-1)->getOffshoreInst()->getSeqNumb()]
			/ 12 ;// RouteVes->getSpeed();
			iter->setVisitStart(arrival);

			setRouteEndTime(arrival); // set route end time
			iter->setVisitWaitTime(0);
		}
		else //the platforms
		{
			arrival = (iter-1)->getVisitEnd() + 
				iter->getOffshoreInst()->Dist[(iter-1)->getOffshoreInst()->getSeqNumb()]
			/ 12; // RouteVes->getSpeed();
			arrTime = ((arrival/24) - floor(arrival/24))*24;
			iter->setVisitStart(arrival);

			if (iter->getOffshoreInst()->getClosingTime() == 24)
			{//24-hour open
				
				departure = arrival + iter->getOffshoreInst()->getLayTime();
				iter->setVisitEnd(departure);
				iter->setVisitWaitTime(0);
			}
			//Check if we are able to complete within TW
			else if ( ( arrTime + iter->getOffshoreInst()->getLayTime() <
				iter->getOffshoreInst()->getClosingTime() + getRouteAcceptanceTime() ) &&
				(arrTime + getRouteAcceptanceTime() > iter->getOffshoreInst()->getOpeningTime()) )
			{
				departure = arrival + iter->getOffshoreInst()->getLayTime();
				iter->setVisitEnd(departure);
				iter->setVisitWaitTime(0);
			}

			else if ( arrTime + iter->getOffshoreInst()->getLayTime() >
				iter->getOffshoreInst()->getClosingTime() + getRouteAcceptanceTime() )//Cannot complete within TW
			{
				departure = ceil(arrival/24)*24 + 
					iter->getOffshoreInst()->getOpeningTime() +
					iter->getOffshoreInst()->getLayTime();
				iter->setVisitEnd(departure);

				waitClock = departure - arrival - 
					iter->getOffshoreInst()->getLayTime();
				iter->setVisitWaitTime(waitClock);
				
			}

			else if (arrTime < iter->getOffshoreInst()->getOpeningTime())
			{
				departure = floor(arrival/24)*24 + 
					iter->getOffshoreInst()->getOpeningTime() +
					iter->getOffshoreInst()->getLayTime();
				iter->setVisitEnd(departure);

				waitClock = departure - arrival - 
					iter->getOffshoreInst()->getLayTime();
				iter->setVisitWaitTime(waitClock);
			}
			else
			{
				cout << "SOMETHING is wrong in updateVisitObjects()" << endl;
				cout << "arrTime = " << arrTime << " arrival = " << arrival << endl;
			}
		}//end else
	}//end for

	//update visit positions on the route
	int i; 
	for (i = 0; i < RouteVisits.size(); i++)
		RouteVisits[i].setIdNumber(i);
}

void Route::setVisitIds()
{
	int i; 
	for (i= 0; i < RouteVisits.size(); i++)
		RouteVisits[i].setIdNumber(i);
}

void Route::synchrRouteInstalVisits()
{
	RouteVisits.clear();
	vector <OffshoreInst *>::iterator it;

	for (it = RouteInstals.begin(); it != RouteInstals.end(); ++it)
		RouteVisits.push_back(Visit(*it));
}

void Route::synchrRouteVisitInstals()
{
	RouteInstals.clear();
	vector <Visit>::iterator it;

	for (it = RouteVisits.begin(); it != RouteVisits.end(); ++it)
		RouteInstals.push_back(it->getOffshoreInst());
}

void Route::deleteInstalVisit(int delPos)
{
	//cout << "delPos = " << delPos << endl;
	//cout << "RouteInstals.size() = " << RouteInstals.size() << endl;
	//cout << "RouteVisits.size() = " << RouteVisits.size() << endl;
	//cout << "Route Number = " << getRouteNum() << endl << endl;
	RouteInstals.erase(RouteInstals.begin() + delPos);
	RouteVisits.erase(RouteVisits.begin() + delPos);
}

void Route::insertInstalVisit(int insPos, Visit vis)
{
	RouteInstals.insert(RouteInstals.begin() + insPos, vis.getOffshoreInst());
	RouteVisits.insert(RouteVisits.begin() + insPos, vis);
}

void Route::insertInstalVisit(int visPos, Route *routeFrom)
{
	RouteInstals.insert(RouteInstals.begin() + 1, routeFrom->getVisitAtPos(visPos).getOffshoreInst());
	RouteVisits.insert(RouteVisits.begin() + 1, routeFrom->getVisitAtPos(visPos));
}

bool Route::isInstOnRoute(OffshoreInst *inst)
{
	vector<OffshoreInst *>::iterator it;
	for (it = RouteInstals.begin() + 1; it != RouteInstals.end()-1; it++)
		if (inst->getSeqNumb() == (*it)->getSeqNumb())
			return true;

	return false;
}

void Route::intelligentReorder()
{
	//Try to reposition the installations whose waiting time is > 0
	//in the hope of reducing the route duration.
	vector < Visit >::iterator visitIter;
	vector < OffshoreInst * >::iterator instIter;
	vector < Visit > Visits;

	double routeDur = 0, newRouteDur = 0; 
	int i, j;
	bool improvementFound = true;

while (improvementFound)
{
	improvementFound = false;
	for (i = 1; i < RouteInstals.size() - 1; i++)
			for (j = 0; j < RouteInstals.size()-1; j++)
			{
				if (j + 1 != i)//Do the insertion if the position is different
				{
					//cout << "Before reinsertion" << endl;
					//printRoute();

					routeDur = computeRouteDuration();
					RouteInstals.insert(RouteInstals.begin() + j + 1, RouteInstals[i]);
					//Remove the duplicate
					if (j + 1 > i)
						RouteInstals.erase(RouteInstals.begin() + i);
					else // i > j + 1
						RouteInstals.erase(RouteInstals.begin() + i + 1);
					
					synchrRouteInstalVisits();
					updateVisitVector();
					newRouteDur = computeRouteDuration();

					//cout << "After reinsertion" << endl;
					//printRoute();

					if (newRouteDur < routeDur)
					{
						improvementFound = true;
						//cout << "IMPROVEMENT FOUND!!!" << endl << endl;
						break;
					}
					else //reverse the changes
					{
						if (j + 1 > i)
						{
							RouteInstals.insert(RouteInstals.begin() + i, RouteInstals[j]);
							RouteInstals.erase(RouteInstals.begin() + j + 1);
						}
						else // i > j + 1
						{
							RouteInstals.insert(RouteInstals.begin() + i + 1, RouteInstals[j + 1]);
							RouteInstals.erase(RouteInstals.begin() + j + 1);
						}

						synchrRouteInstalVisits();
						updateVisitVector();
					}//end else

				}//end if (j+1 != 1)
			}//end for

}//end while
}

ostream& operator << (ostream& os, Route &route)
{
	os.width(20);
	os << "Start time = " << route.getRouteStartTime();

	return os;
}