/* Definition of a Supply Vessel class */

#pragma once
#include "stdafx.h"

class Route;

class SupVes
{
	//private members of the Supply vessel class
	double capacity;
	double speed;
	double FCCosts;
	double FCSailing;	
	double FCBase;	
	double FCInstallation;	
	double start;
	string name;
	int id;
	bool isUsed;
	double vesCost;
	vector < Route* > vesselRoutes;
	double baseTime;
	double installationTime;
	double sailingTime;

public:
	//Constructors
	SupVes(void);
	SupVes (string VesName, double VesCap, double VesSpeed, 
		double Costs, double Sailing, double Base,
		double Installation, double VesStart, double CostVessel);

	//get methods
	double getCapacity() const {return capacity;}
	double getSpeed() const {return speed;}
	double getFCCosts() const {return FCCosts;}
	double getFCSailing() const {return FCSailing;}
	double getFCBase() const {return FCBase;}
	double getFCInstallation() const {return FCInstallation;}
	double getStart() const {return start;}
	string getName() const {return name;}
	bool getIsVesselUsed() const {return isUsed;}
	vector <Route*> getVesselRoutes() const {return vesselRoutes;}
	double getVesselCost() const {return vesCost;}
	double getBaseTime() const {return baseTime;}
	double getInstallationTime() const {return installationTime;}
	double getSailingTime() const {return sailingTime;}
	int getID() const {return id;}

	//set methods
	void setID(int);
	void setIsVesselUsed(bool Use) {isUsed = Use;}
	void addVesselRoute (Route* vesRoute) {vesselRoutes.push_back(vesRoute);}
	void eraseVesselRoute(int RouteNumb);

	void clearVesselRoutes () {vesselRoutes.clear();}
	void setBaseTime(double baTime) {baseTime = baTime;}
	void setInstallationTime(double insTime) {installationTime = insTime;}
	void setSailingTime (double saTime) {sailingTime = saTime;}

};