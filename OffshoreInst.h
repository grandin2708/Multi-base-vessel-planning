//Offshore installation class

#pragma once

#include "stdafx.h"

class OffshoreInst
{
private:
	string instName;
	double WeeklyDemand;
	int VisitFreq;
	double LayTime;
	int ClusterSize;
	int OpeningTime;
	int ClosingTime;
	int seqNumb;
	int numVisDayComb;
	double latitude;
	double longitude;

public:
	OffshoreInst(void);
	OffshoreInst (string theName, int OpenTime, int CloseTime, double WeekDem, 
		int VisFreq, double LayDur, int ClustSize, double lat, double lon);

	double getWeeklyDemand() const {return WeeklyDemand;}
	int getVisitFreq() const {return VisitFreq;}
	double getLayTime() const {return LayTime;}
	int getClusterSize() const {return ClusterSize;}
	int getOpeningTime() const {return OpeningTime;}
	int getClosingTime() const {return ClosingTime;}
	string getInstName() const {return instName;}
	int getSeqNumb() const {return seqNumb;}
	int getNumVisDayComb() const {return numVisDayComb;}
	double getLatitude() const {return latitude;}
	double getLongitude() const {return longitude;}

	void setInstName(string name){instName = name;}
	//vectors made public for easier access
	vector < vector <int> > VisitDayCombs; 
	vector <double> Dist; //a vector of distances to other installations
	vector <int> CurVisitDayComb;
	vector <int> ObligatoryVisitDays;//a vector of days when the installation has to be visited

	void RemoveDayFromVisDayComb (int day);
	void InsertDayToVisDayComb (int day);

	void setSeqNumb(int num)
	{
	if (num >= 0)
		seqNumb = num;
	else
		cout << "Err SeqNumb" << endl;
	}

	void setNumVisDayComb(int numb) 
	{
		if (numb >= 0)
			numVisDayComb = numb;
		else
			cout << "Err numVisDayComb" << endl;
	}
			
};
