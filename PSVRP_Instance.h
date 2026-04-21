//Instance data for PSVRP

#pragma once
#include "stdafx.h"
#include "SupVes.h"
#include "OffshoreInst.h"


class PSVRP_Instance
{
	vector < OffshoreInst > Platforms;
	vector < SupVes > Vessels;
	int minInst;
	int maxInst;
	double loadFactor;
	double minSlack;
	double acceptanceTime;
	string instanceName;
	double OPlat;
	double OPlon;

public:
	PSVRP_Instance(void);
	bool ReadInstance (string BaseName, string NNodes); //Read instance data
	void PrintInstance();
	vector < OffshoreInst > getPlatforms() const {return Platforms;}
	vector < SupVes > getVessels() const;

	int getMaxInst() {return maxInst;}
	int getMinInst() {return minInst;}
	double getLoadFactor() {return loadFactor;}
	double getMinSlack() {return minSlack;}
	double getAcceptTime() {return acceptanceTime;}
	string getInstanceName() {return instanceName;}
	double getOPlat() {return OPlat;}
	double getOPlon() {return OPlon;}
	double getEarthRadius() {return 3440.07019148119;}


	

};
