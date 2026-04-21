//Route class definitions

#pragma once
#include "OffshoreInst.h"
#include "Visit.h"
#include "stdafx.h"

class SupVes;

class Route
{
private:
	double routeStartTime;
	double routeEndTime;
	vector < OffshoreInst * > RouteInstals; //installations visited along the route
	vector < Visit > RouteVisits;
	SupVes *RouteVes;
	int routeNum;
	bool onlyRoute; //shows whether the voyage is the only one for its departure day

	//Set these values when initializing weekly schedule
	int minVisits;
	int maxVisits;
	double routeMaxDur;
	double routeLoadFactor;
	double routeMinSlack;
	double routeAcceptanceTime;
	int numOnesAfterRoute;

public:
	Route(void);
	Route (int num, double stTime);

	//get methods
	int getRouteNum() {return routeNum;}
	double getRouteStartTime() const {return routeStartTime;}
	double getRouteEndTime() const {return routeEndTime;}
	SupVes * getRouteVes() const {return RouteVes;}
	OffshoreInst *getInstAtPos (int pos);
	double getRouteMaxDur() {return routeMaxDur;}
	double getRouteLoadFactor() {return routeLoadFactor;}
	double getRouteMinSlack() {return routeMinSlack;}
	double getRouteAcceptanceTime() {return routeAcceptanceTime;}
	int getMinVisits() {return minVisits;}
	int getMaxVisits() {return maxVisits;}
	int getNumOnesAfterRoute() {return numOnesAfterRoute;}
	Visit getVisitAtPos (int pos) {return RouteVisits[pos];}
	vector < Visit > getVisitObjects() {return RouteVisits;}
	vector < Visit* > getVisitPointers();
	vector <OffshoreInst*> getRouteInstalPointers() {return RouteInstals;}
	bool getOnlyRoute() {return onlyRoute;}

	static const int MAX_ROUTE_DUR = 64;
	static const int MIN_ROUTE_DUR = 16;

	//set methods
	void setRouteStartTime(double stTime) {routeStartTime = stTime;}
	void setRouteEndTime (double enTime) {routeEndTime = enTime;}
	void setRouteVessel (SupVes *ves) {RouteVes = ves;}
	void setRouteMaxDur (double dur) {routeMaxDur = dur;}
	void setRouteLoadFactor (double fact) {routeLoadFactor = fact;}
	void setRouteMinSlack (double slack) {routeMinSlack = slack;}
	void setRouteAcceptanceTime (double time) {routeAcceptanceTime = time;}
	void setMinVisits (int minVis) {minVisits = minVis;}
	void setMaxVisits (int maxVis) {maxVisits = maxVis;}
	void setNumOnesAfterRoute (int numOnes) {numOnesAfterRoute = numOnes;}
	void setRouteNum (int num) {routeNum = num;}
	void setVisitIds();
	void setOnlyRoute(bool val) {onlyRoute = val;}


	//To add an installation to RouteInstals vector
	void addRouteInstal (OffshoreInst *someInst);
	void addVisit (Visit vis);
	void setRouteNumber(int num){routeNum = num;}

	//Delete-insert visit methods
	void deleteInstalVisit(int delPos);
	void insertInstalVisit(int insPos, Visit vis);
	void insertInstalVisit(int visPos, Route *routeFrom); 

	//Compute route characteristics
	double computeRouteLength();
	double computeRouteDuration();
	double computeRouteDemand();
	double computeRouteSlack();

	//Feasibility methods
	bool isLoadFeasible();
	bool isNumInstFeasible();
	bool isRouteDurFeasible();
	bool isFeasible();

	bool isInstOnRoute(OffshoreInst* inst);

	//reordering methods
	void cheapInsertSeq();
	void updateVisitVector();
	void synchrRouteInstalVisits();
	void synchrRouteVisitInstals();
	void intelligentReorder();

	void printRoute();
};