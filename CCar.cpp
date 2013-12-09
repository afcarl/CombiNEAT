#include "CCar.h"
#include "client.h"

//-----------------------------------constructor-------------------------
//
//-----------------------------------------------------------------------
CCar::CCar():
	m_leftRightTrack(0),
	m_brakeAccelerateTrack(0),
	m_dFitness(0)                      
{
}

//-----------------------------Reset()------------------------------------
//
//	Resets the car's fitness
//
//------------------------------------------------------------------------
void CCar::Reset()
{
	//reset the fitness
	m_dFitness = 0;
}

//-------------------------------Update()--------------------------------
//
//	First we take sensor readings and feed these into the car's brain.
//
//	The inputs are:
//	
//  The readings from the car sensors
//
//	We receive two outputs from the brain.. lTrack & rTrack.
//	So given a force for each track we calculate the resultant rotation 
//	and acceleration and apply to current velocity vector.
//
//-----------------------------------------------------------------------
bool CCar::Update(vector<SPoint> &objects)
{
	//this will store all the inputs for the NN
	vector<double> inputs;	

	//grab sensor readings

	//input sensors into net
	//inputs.push_back(m_vecdSensors[sr]);

	//inputs.push_back(m_vecFeelers[sr]);

	//update the brain and get feedback
	vector<double> output = m_pItsBrain->Update(inputs, CNeuralNet::active);

	//make sure there were no errors in calculating the 
	//output
	if (output.size() < CParams::iNumOutputs) 
	{
		return false;
	}

	//assign the outputs to the sweepers left & right tracks
	m_leftRightTrack = output[0];
	m_brakeAccelerateTrack = output[1];

	//calculate steering forces
	return true;
}

//------------------------- EndOfRunCalculations() -----------------------
//
//------------------------------------------------------------------------
void CCar::EndOfRunCalculations()
{
	// Calculate fitness
	char *argv[] = { "", ""};
	m_dFitness = runCar(m_pItsBrain);
}