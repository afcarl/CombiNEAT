#ifndef CCAR_H
#define CCAR_H

//------------------------------------------------------------------------
//
//	Name: CCar.h
//
//  Author: Morten Schnack 2013 (mosch@itu.dk)
//
//  Desc: Class to create a car object 
//
//------------------------------------------------------------------------
#include <vector>
#include <math.h>

#include "phenotype.h"
#include "CParams.h"

using namespace std;

class CCar
{

private:
	// its brain
	CNeuralNet*  m_pItsBrain; 

	//to store output from the ANN
	double			m_leftRightTrack, m_brakeAccelerateTrack;

	//the car's fitness score. 
	double			m_dFitness;


public:
	CCar();

	//updates the ANN with information from the car enviroment
	bool			Update(vector<SPoint> &objects);

	void			Reset();

	void      EndOfRunCalculations();

	void      RenderStats(HDC surface);

	void      DrawNet(HDC &surface, int cxLeft, int cxRight, int cyTop, int cyBot)
	{
		m_pItsBrain->DrawNet(surface, cxLeft, cxRight, cyTop, cyBot);
	}


	//-------------------accessor functions
	float			        Fitness()const{return m_dFitness;}

	void              InsertNewBrain(CNeuralNet* brain){m_pItsBrain = brain;}

};


#endif