//Weekly schedule class definitions

#pragma once
#include "Route.h"
#include "PSVRP_Instance.h"
#include "stdafx.h"
#include "VisitVariation.h"


class WeeklySchedule
{
	PSVRP_Instance* Instance; //Instance object
	int numberOfRoutes; //number of routes performed during the week
	int numberOfVessels; //number of vessels used during the week
	vector < Route > Routes;
	vector < SupVes > SchedVessels;
	vector < OffshoreInst > SchedInstals;
	vector < VisitVariation > VisVariations;
	int departuresPerDay;
	int totNumVis;
	Route *isolRoute;
	int schedID;

public:
	WeeklySchedule(void); 
	WeeklySchedule(PSVRP_Instance* Inst); //construct initial solution here
	WeeklySchedule(WeeklySchedule* sched);
	WeeklySchedule(PSVRP_Instance* Inst, int maxDep); 

	PSVRP_Instance * getInstance() {return Instance;}

	void initSchedule();

	int getSchedID(){return schedID;}
	void setSchedID(int num) {schedID = num;}

	void setDepPerDay(int num) {departuresPerDay  = num;}
	int getDepPerDay () {return departuresPerDay;}
	int getTotNumVis() {return totNumVis;}
	void setTotNumVis(int val) {totNumVis = val;}
	void setRoute (int idx, Route rout) {Routes[idx] = rout;}

	Route* getIsolRoute() {return isolRoute;}
	void setIsolRoute();
	void setIsolRouteNull();
	void setIsolRoute(Route* routePtr) {isolRoute = routePtr;}

	int getRoutesSize() {return Routes.size();}
	int getSchedVesselsSize() {return SchedVessels.size();}
	int getSchedInstalsSize() {return SchedInstals.size();}

	Route* getRoutePointer(int idx) {return &Routes[idx];}
	SupVes* getSchedVesselPointer(int idx) {return &SchedVessels[idx];}
	OffshoreInst* getSchedInstalPointer (int idx) {return &SchedInstals[idx];}

	vector <Route*> getRoutePointers();
	vector <SupVes*> getSchedVesselsPointers();
	vector <OffshoreInst*> getSchedInstalsPointers();
	
	int computeLB_numRoutes();
	double computeSchedCost();
	bool schedFeasible();

	vector <SupVes> getSchedVessels() {return SchedVessels;}

	Route * getRoute(int routeNum);
	
	static const int NUMBER_OF_DAYS = 7;
	
	vector < vector < bool > > VesAvail; // vessel availability throughout the week
	vector < vector < int > > VisDayAssign; // assignment of visit days 2d-vector
	vector <Visit> removedVisits; // vector of removed visits from goLNS()

	void addVessel(SupVes ves){SchedVessels.push_back(ves);}
	void addInstallation (OffshoreInst inst) {SchedInstals.push_back(inst);}
	void addVisitVariation (VisitVariation visVar) {VisVariations.push_back(visVar);}
	//A procedure to come close to LB in terms of number of routes

	void reassignVesselsToRoutes(); //reducing number of vessels used
	void reduceNumberOfRoutes();
	void relocateVisits();
	bool assignVessels(); //initial vessel assignment

	void reassignIsolatedVoyage(); //assign isolatedVoyage to a vessel with the largest feasible capacity

	bool isDepartureSpreadEven(Route* routeFrom, Route* routeTo, Visit *visit);
	bool isDepartureSpreadEven(Route *routeTo, Visit *visit);
	bool isSchedFeasible();

	void updateVisDayComb(Route *routeFrom, Route *routeTo, Visit *visit);
	
	void printWeeklySchedule();
	void saveSchedule (WeeklySchedule &sched);
	void writeScheduleToFile(int iter, int LNSiter);
	void clearEmptyRoutes();
	void setOnlyRoutes();
	void updateRelationalInfo();

	void isolateVoyage(); //reassigns vessels to voyages to isolate a single voyage
	void makeMirrorScheduleSmall(WeeklySchedule *otherSched); //affects the schedule through visit day combination changes
	void makeMirrorScheduleLarge(WeeklySchedule *otherSched); //removes/recreates voyages to make a "mirror" schedule
	int computeConsecZeroes (int vesID); //computes consecutive 0's for a vessel with vesID
	bool isolateVoyageBetween(int start, int end, int isolVesID, vector <double> startTimes); //isolate voyage between start and end day

	double computeTotalSlack();
	int computeDurTotDays();
	void minimizeTotalSlack();
	void reduceRouteDurTotDays();
	void goLNS (int iter, double *curBestCost, int curRestart, WeeklySchedule *bs, 
		vector <WeeklySchedule>* trialScheds, double schedMargin, 
		vector <double>* cost, vector <double>* startTime, 
		vector <double>* vesselCap, vector <int>* numVessels);

	void multiLNS (int iter, double *bestCost, WeeklySchedule *otherSched, 
		WeeklySchedule *bs, Route* otherBaseRoute); //Post-optimization LNS

	int calcNumLNSiter();
	void synchrVisitInstalConnection();
	int calcNumVesselsUsed();

	bool reduceNumberOfVessels(); //reducing number of vessels used
	bool createVoyages(); //create extra voyages


};