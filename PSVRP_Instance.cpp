#include "stdafx.h"
#include "PSVRP_Instance.h"
#include "OffshoreInst.h"


PSVRP_Instance::PSVRP_Instance(void)
{
}

bool PSVRP_Instance::ReadInstance(string BaseName, string NNodes) 
{
	instanceName = BaseName;
	//Read instance
	string vesFilename("Vessel"+BaseName+".txt"), 
		instFilename("Installation"+BaseName+"_"+NNodes+".txt");

	ifstream VesInput(vesFilename.c_str());
	ifstream InstInput(instFilename.c_str());

	string aStr, ves1stLineStr("VesselCost");
	

	int i,j,k;
	
	//Ignore the first line of the vessel input file
	do
		VesInput >> aStr;
	while (ves1stLineStr.compare(aStr) != 0);
	
	//Read and initialize a vector of SupVes objects
	string stName;
	double cap;
	double sp;
	double FCCos;
	double FCSail;	
	double FCBas;	
	double FCInstal;	
	double sta;
	double costVes;
	
	while (VesInput >> stName >> cap >> sp >> FCCos 
		>> FCSail >> FCBas >> FCInstal >> sta >> costVes)

			Vessels.push_back( SupVes(stName, cap, sp, FCCos, 
			FCSail, FCBas, FCInstal, sta, costVes));

	VesInput.close();

	

	//Assign integer ids to vessels, starting with 1 (not 0)
	vector < SupVes >::iterator iter;
	i=0;
	for (iter = Vessels.begin(); iter != Vessels.end(); ++iter)
	{
		iter->setID(i);
		i++;
	}


	//Read and initialize installations and other data
	InstInput >> aStr >> minInst >> aStr >> maxInst >> aStr >> loadFactor
		>> aStr >> acceptanceTime >> aStr >> minSlack >> aStr 
		>> OPlat >> OPlon;

	//Ignore one line of the Installations input file
	string inst1stLineStr("LonDec");
	do
		InstInput >> aStr;
	while (inst1stLineStr.compare(aStr) != 0);

	string insName;
	double WeekDemand;
	int VisFreq;
	double LaTime;
	int ClustSize;
	int OpenTime;
	int ClosTime;
	double lat, lon;

	//Initialize installation objects
	int sequentNum = 0;
	while (InstInput >> insName >> OpenTime >> ClosTime >>
		WeekDemand >> VisFreq >> LaTime >> ClustSize >> lat >> lon)

			Platforms.push_back( OffshoreInst(insName, OpenTime, ClosTime,
			WeekDemand*loadFactor, VisFreq, LaTime, ClustSize, lat, lon));
			
	InstInput.close();

	/*Initialize a vector of possible visit day combinations for each frequency*/
	vector <int> NumVDComb;
	NumVDComb.push_back(0); NumVDComb.push_back(6); NumVDComb.push_back(5);
	NumVDComb.push_back(8); NumVDComb.push_back(6); NumVDComb.push_back(4); 
	NumVDComb.push_back(1); NumVDComb.push_back(0); NumVDComb.push_back(0);

	

	//Read in visit day combinations for each offshore installation
	ifstream VisDayCombInput("VisDayCombs.txt");
	vector < OffshoreInst >::iterator it;
	i = 0;
	string stNumVisLine;
	int visitDay;

	if (!VisDayCombInput)
		cout << "Error opening VisDayCombs file!!!"<< endl << endl;
	else 
	{
		
		for (it = Platforms.begin(); it != Platforms.end(); ++it)
		{
			
			it->setSeqNumb(i);
			it->setNumVisDayComb(NumVDComb[it->getVisitFreq()]);
			cout << it->getInstName() << endl;

			switch (it->getVisitFreq()){
				case 1: stNumVisLine = "1:";
					break;
				case 2: stNumVisLine = "2:";
					break;
				case 3: stNumVisLine = "3:";
					break;
				case 4: stNumVisLine = "4:";
					break;
				case 5: stNumVisLine = "5:";
					break;
				case 6: stNumVisLine = "6:";
					break;

				default: 
					stNumVisLine = "0:";
					break;
			}

			//Find appropriate line in the input file
			if (stNumVisLine != "0:")
			{
				do
					VisDayCombInput >> aStr;
				while ( stNumVisLine.compare(aStr) != 0);

				//cout << aStr << endl;

				//Read in the data
				it->VisitDayCombs.resize(it->getNumVisDayComb());
				for (k=0; k < it->VisitDayCombs.size(); k++)
				{
					for (j=0; j < it->getVisitFreq(); j++)
					{
						VisDayCombInput >> visitDay;
						it->VisitDayCombs[k].push_back(visitDay);
					}
				}
			} //end if

			++i;
			VisDayCombInput.seekg (0, ios::beg);
		} // end for

	} // end else

	VisDayCombInput.close();

	//Calculate distances
	double distance;

	for (i = 0; i < Platforms.size(); i++)
		for (j = 0; j < Platforms.size(); j++)
			if (i == j)
				Platforms[i].Dist.push_back(0);
			
			else if ( ((i == 0) || (j == 0)) && (i!=j) ) 
			{//Through offshore point

				//Distance to OP
				distance = getEarthRadius()*acos( sin(Platforms[i].getLatitude()*M_PI/180)*sin(OPlat*M_PI/180)
					+ cos(Platforms[i].getLatitude()*M_PI/180)*cos(OPlat*M_PI/180)
					* cos( (Platforms[i].getLongitude() - OPlon)*M_PI/180 ) );

				//Distance from OP
				distance += getEarthRadius()*acos (sin(OPlat*M_PI/180) * sin(Platforms[j].getLatitude()*M_PI/180)
					+ cos(OPlat*M_PI/180) * cos(Platforms[j].getLatitude()*M_PI/180)
					* cos ( (OPlon - Platforms[j].getLongitude())*M_PI/180) ); 

				Platforms[i].Dist.push_back(distance);
			}
			else
			{//Regularly
				distance = getEarthRadius()*acos( sin(Platforms[i].getLatitude()*M_PI/180) * sin(Platforms[j].getLatitude()*M_PI/180)
					+ cos(Platforms[i].getLatitude()*M_PI/180) * cos(Platforms[j].getLatitude()*M_PI/180)
					* cos( (Platforms[i].getLongitude() - Platforms[j].getLongitude())*M_PI/180 ) );
				
				Platforms[i].Dist.push_back(distance);
			}
	

	/*
	//Read distance vectors
	string distFileName("Dist"+BaseName+".txt");
	ifstream DistInput(distFileName.c_str());
	
	string c ("#");;

	//Read in the values for each platform
	for (it = Platforms.begin(); it != Platforms.end(); ++it)
	{
		for (i = 0; i < Platforms.size(); i++)
		{
			DistInput >> distance;
			it->Dist.push_back(distance);
		}

		//Proceed to next line
		do
			DistInput >> aStr;
		while (c.compare(aStr) != 0);

	}
	*/

	return true;
}

void PSVRP_Instance::PrintInstance()
{
	int i,j;

	cout << "Instance name is "<< instanceName << endl;
	cout << "minInst = " << minInst << endl;
	cout << "maxInst = " << maxInst << endl;
	cout << "loadFactor = " << loadFactor << endl;
	cout << "acceptanceTime = " << acceptanceTime << endl;
	cout << "minSlack = " << minSlack << endl << endl;

	vector < OffshoreInst >::iterator it;
	cout << "Installations:" << endl;
	for (it = Platforms.begin(); it != Platforms.end(); ++it)
	{
		cout << it->getSeqNumb() << ":" << (*it).getInstName() << endl;
		cout << "Demand = " << (*it).getWeeklyDemand() << endl;
		cout << "Visit day combinations" << endl;
		for (i=0; i < it->getNumVisDayComb(); i++)
		{
			cout << (it->VisitDayCombs[i]).size() << ": ";
			for (j = 0; j < it->getVisitFreq(); j++)
				cout << it->VisitDayCombs[i][j] << " ";
			cout << endl;
		}
	}

	/*
	int i;
	for (i=0; i < Platforms.size(); i++)
		cout << Platforms[i].getInstName()<<endl;
		*/
	//Distance and time information
	for (it = Platforms.begin(); it != Platforms.end(); ++it)
	{
		cout << it->getInstName() << ": Distance" << endl; 
		for (i = 0; i < Platforms.size(); i++)
			cout << i << ":\t" << it->Dist[i] << endl;
	}
	
	vector < SupVes >::iterator iter;
	cout << endl << endl << "Vessels:" << endl;

	for (iter = Vessels.begin(); iter != Vessels.end(); ++iter)
		cout << iter->getID() << ":" << (*iter).getName() << endl;	

}

vector < SupVes > PSVRP_Instance::getVessels() const
{
	return Vessels;
	/*
	vector < SupVes* > supVesPointers;
	vector <SupVes>::iterator it;

	for (it = Vessels.begin(); it != Vessels.end(); it++)
		supVesPointers.push_back(&it);

	return supVesPointers; */
}
