//Class Visit definitions

#pragma once
#include "OffshoreInst.h"
#include "SupVes.h"
#include "stdafx.h"
#include "VisitVariation.h"


class Visit
{
private:
	OffshoreInst* installation;
	SupVes* vessel;
	double visitStart;
	double visitEnd;
	double visitWaitTime; //because of TW
	int idNumber;

public:
	Visit(void);
	Visit(OffshoreInst* offshoreInstal);

	vector <VisitVariation> VisVars; //vector of possible insertions
	vector <double> RegretValues;

	OffshoreInst* getOffshoreInst() {return installation;}
	SupVes* getSupVes() {return vessel;}
	double getVisitStart() const {return visitStart;}
	double getVisitEnd() const {return visitEnd;}
	double getVisitWaitTime() const {return visitWaitTime;}
	int getIdNumber() const {return idNumber;}

	void setOffshoreInst(OffshoreInst* inst) {installation = inst;}
	void setSupVes(SupVes* ves) {*vessel = *ves;}
	void setVisitStart (double stTime) {visitStart = stTime;}
	void setVisitEnd (double enTime) {visitEnd = enTime;}
	void setVisitWaitTime (double waTime) {visitWaitTime = waTime;}
	void setIdNumber(int num) {idNumber = num;}
};
