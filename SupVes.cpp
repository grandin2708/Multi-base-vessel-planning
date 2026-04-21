#include "StdAfx.h"
#include "SupVes.h"
#include "Route.h"


SupVes::SupVes(void)
{
}

SupVes::SupVes(string VesName, double VesCap, double VesSpeed, 
		double Costs, double Sailing, double Base,
		double Installation, double VesStart, double CostVessel){

			capacity = VesCap;
			speed = VesSpeed;
			FCCosts = Costs;
			FCSailing = Sailing;	
			FCBase = Base;	
			FCInstallation = Installation;	
			start = VesStart;
			name = VesName;
			vesCost = CostVessel;
}

void SupVes::setID(int ID) {
	if (ID >= 0)
		this->id = ID;
	else
		cout << "Error in SetID SupVes" << endl;
}

void SupVes::eraseVesselRoute(int RouteNumb)
{
	int i = 0;

	vector <Route*>::iterator it;

	for (it = vesselRoutes.begin(); it != vesselRoutes.end(); ++it)
	{
		if ((*it)->getRouteNum() == RouteNumb)
			break;
		
		i++;
	}

	
	//cout << "In eraseVesselRoute i = " << i << endl;
	//(*(vesselRoutes.begin() + i))->printRoute();
	//cout << "vesselRoutes.size() = " << vesselRoutes.size() << endl;

	vesselRoutes.erase(vesselRoutes.begin() + i);

	//cout << "After erase vesselRoutes.size() = " << vesselRoutes.size() << endl;
	//cout << "Remaining route" << endl << endl;
	//vesselRoutes[0]->printRoute();
	}