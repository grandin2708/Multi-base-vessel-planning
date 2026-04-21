// Timer.h: interface for the Timer class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"

/*
#if !defined(AFX_TIMER_H__47663DE2_9114_415B_A1DE_8E1AB78EF767__INCLUDED_)
#define AFX_TIMER_H__47663DE2_9114_415B_A1DE_8E1AB78EF767__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
*/
#include <ctime>
//#include <windows.h>


/// Class for timing the program execution.
class Timer  
{
public:
	Timer();
	virtual ~Timer();

  /// 
  void startTimer();
  ///
  void stopTimer();
  ///
  double continueTimer();
  ///
  double getElapsedSeconds();
  ///
  void setTimeLimit(int seconds);
  ///
  bool hasPassedLimit();
  ///
  void pause(int seconds);


private:
  
  bool started_;
  bool stopped_;
  
  clock_t start_;
  clock_t limit_;
  clock_t stop_;

};

//#endif // !defined(AFX_MDMKPTIMER_H__47663DE2_9114_415B_A1DE_8E1AB78EF767__INCLUDED_)