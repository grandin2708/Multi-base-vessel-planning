#pragma once

class Visit;
class Route;

class VisitVariation
{
	//private members
	Route* routeFrom;
	Route* routeTo;
	Visit* varVisit; //pointer to a visit being relocated
	int insertionPos;
	int removalPos;
	double deltaObj;
	double routeToDurIncrease;
	double routeFromDurDecrease;
	double totalSlackDelta;
	double routeSlackDelta;
	double routeDurationDelta;
	int routeDaysDelta;
	int routeFromDaysDelta;
	int visitIndex;

public:
	VisitVariation(void);
	VisitVariation(Visit *varVis);

	Route* getRouteFrom() {return routeFrom;}
	Route* getRouteTo() {return routeTo;}
	Visit* getVarVisit() {return varVisit;}
	int getInsertionPos() {return insertionPos;}
	int getRemovalPos() {return removalPos;}
	double getDeltaObj() {return deltaObj;}
	double getRouteToDurIncrease() {return routeToDurIncrease;}
	double getRouteFromDurDecrease() {return routeFromDurDecrease;}
	double getTotalSlackDelta() {return totalSlackDelta;}
	double getRouteSlackDelta() {return routeSlackDelta;}
	double getRouteDurationDelta() {return routeDurationDelta;}
	int getRouteDaysDelta() {return routeDaysDelta;}
	int getRouteFromDaysDelta(){return routeFromDaysDelta;}
	int getVisitIndex() {return visitIndex;}
	
	void setRouteFrom(Route* rouFr) {routeFrom = rouFr;}
	void setRouteTo(Route* rouTo) {routeTo = rouTo;}
	void setInsertionPos (int insPos) {insertionPos = insPos;}
	void setRemovalPos (int remPos) {removalPos = remPos;}
	void setDeltaObj (double delObj) {deltaObj = delObj;}
	void setRouteToDurIncrease (double rouIncr) {routeToDurIncrease = rouIncr;}
	void setRouteFromDurDecrease (double rouDecr) {routeFromDurDecrease = rouDecr;}
	void setTotalSlackDelta (double delta) {totalSlackDelta = delta;}
	void setRouteSlackDelta (double rouSlDelta) {routeSlackDelta = rouSlDelta;}
	void setRouteDurationDelta(double rouDurDelta) {routeDurationDelta = rouDurDelta;}
	void setRouteDaysDelta (int rouDaysDelta) {routeDaysDelta = rouDaysDelta;}
	void setRouteFromDaysDelta (int rouFrDaDelta) {routeFromDaysDelta = rouFrDaDelta;}
	void setVisitIndex (int idx) {visitIndex = idx;}
	void setVarVisit (Visit *vis) {varVisit = vis;}
};
