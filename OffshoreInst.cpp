#include "stdafx.h"
#include "OffshoreInst.h"

OffshoreInst::OffshoreInst(void)
{
}

OffshoreInst::OffshoreInst(string theName, int OpenTime, int CloseTime, double WeekDem, 
		int VisFreq, double LayDur, int ClustSize, double lat, double lon) {
	WeeklyDemand = WeekDem;
	VisitFreq = VisFreq;
	LayTime = LayDur;
	ClusterSize = ClustSize;
	OpeningTime = OpenTime;
	ClosingTime = CloseTime;
	instName = theName;
	latitude = lat;
	longitude = lon;
}

void OffshoreInst::RemoveDayFromVisDayComb(int day)
{
	int i;

	//cout << "CurVisitDayComb" << endl;
	for (i = 0; i < CurVisitDayComb.size(); i++)
	{
		//cout << CurVisitDayComb[i] << "\t";

		if (CurVisitDayComb[i] == day)
			break;
	}

	/*
	cout << endl << "Inst Name = " << getInstName() << endl;
	cout << "CurVisitDayComb.size() = " << CurVisitDayComb.size() << endl;
	cout << "Day = " << day << endl;
	cout << "i = " << i << endl;
	*/
	CurVisitDayComb.erase(CurVisitDayComb.begin() + i);
}

void OffshoreInst::InsertDayToVisDayComb(int day)
{
	int i;
	bool dayInserted = false;


	//Keep the assigned visit day combination sorted
	for (i = 0; i < CurVisitDayComb.size(); i++)
		if(day < CurVisitDayComb[i])
		{
			CurVisitDayComb.insert(CurVisitDayComb.begin() + i, day);
			dayInserted = true;
			break;
		}

	if (!dayInserted)
		CurVisitDayComb.push_back(day);

}

		