#include "CCarController.h"

//---------------------------------------constructor---------------------
//
//	initialize the cars, their brains and the GA factory
//
//-----------------------------------------------------------------------
CCarController::CCarController(HWND hwndMain,
	int  cxClient,
	int  cyClient): m_NumCars(CParams::iPopSize), 
	m_bFastRender(false),
	m_bRenderBest(false),
	m_iTicks(0),
	m_hwndMain(hwndMain),
	m_hwndInfo(NULL),
	m_iGenerations(0),
	m_cxClient(cxClient),
	m_cyClient(cyClient),
	m_iViewThisCar(0)
{

	//let's create the cars
	for (int i=0; i<m_NumCars; ++i)
	{
		m_vecCars.push_back(CCar());
	}

	//and the vector of cars which will hold the best performing cars
	for (int i=0; i<CParams::iNumBestSweepers; ++i)
	{
		m_vecBestCars.push_back(CCar());
	}

	m_pPop = new Cga(CParams::iPopSize,
		CParams::iNumInputs,
		CParams::iNumOutputs);

	//create the phenotypes
	vector<CNeuralNet*> pBrains = m_pPop->CreatePhenotypes();

	//assign the phenotypes
	for (int i=0; i<m_NumCars; i++)
	{   
		m_vecCars[i].InsertNewBrain(pBrains[i]);
	}

	//create a pen for the graph drawing
	m_BluePen        = CreatePen(PS_SOLID, 1, RGB(0, 0, 255));
	m_RedPen         = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
	m_GreenPen       = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
	m_GreyPenDotted  = CreatePen(PS_DOT, 1, RGB(100, 100, 100));
	m_RedPenDotted   = CreatePen(PS_DOT, 1, RGB(200, 0, 0));

	m_OldPen	= NULL;

	//and the brushes
	m_BlueBrush = CreateSolidBrush(RGB(0,0,244));
	m_RedBrush  = CreateSolidBrush(RGB(150,0,0));
}

//--------------------------------------destructor-------------------------------------
//
//--------------------------------------------------------------------------------------
CCarController::~CCarController()
{
	if (m_pPop)
	{
		delete m_pPop;
	}

	DeleteObject(m_BluePen);
	DeleteObject(m_RedPen);
	DeleteObject(m_GreenPen);
	DeleteObject(m_OldPen);
	DeleteObject(m_GreyPenDotted);
	DeleteObject(m_RedPenDotted);
	DeleteObject(m_BlueBrush);
	DeleteObject(m_RedBrush);
}


//-------------------------------------Update---------------------------------------
//
//	This is the main workhorse. The entire simulation is controlled from here.
//
//--------------------------------------------------------------------------------------
bool CCarController::Update()
{
	// evaluate the cars one by one, each running NUM_TICKS cycles
	
	//first add up each cars fitness scores
		for (int swp=0; swp<m_vecCars.size(); ++swp)
		{
			m_vecCars[swp].EndOfRunCalculations();
		}

	//We have completed another generation so now we need to run the GA

	//increment the generation counter
	++m_iGenerations;

	//reset cycles
	m_iTicks = 0;

	//perform an epoch and grab the new brains
	vector<CNeuralNet*> pBrains = m_pPop->Epoch(GetFitnessScores());

	//insert the new  brains back into the sweepers and reset their
		//positions
		// also sum fitness values, so we can report average of this generation
		float avgFitness = 0;
		float bestFitness = 0;
		int i = 0;
		for (i=0; i<m_NumCars; ++i)
		{
			float sweeperFitness = m_vecCars[i].Fitness();
			if(sweeperFitness > bestFitness) bestFitness = sweeperFitness;
			avgFitness += sweeperFitness;			

			m_vecCars[i].InsertNewBrain(pBrains[i]);
			m_vecCars[i].Reset();
		}

		avgFitness /= i;

		// Write average fitness and best fitness for this generation out to be appended to a file
		std::ofstream outfile;
		outfile.open("fitnessValues.txt", std::ios_base::app);
		if(m_iGenerations == 1) {
			outfile << "         -----------\n"; // Signify new run
			outfile << "\"Generation\";\"Best\";\"Avg\"\n";
		}
		else
		{
			outfile << string("\"" + itos(m_iGenerations-1) + "\"");
			outfile << string(";");
			outfile << string("\"") << bestFitness << string("\"");
			outfile << string(";");
			outfile << string("\"") << avgFitness << string("\"");
			outfile << "\n";
		}

	//grab the NNs of the best performers from the last generation
	vector<CNeuralNet*> pBestBrains = m_pPop->GetBestPhenotypesFromLastGeneration();

	//put them into our record of the best sweepers & save them to file
	for (int i=0; i<m_vecBestCars.size(); ++i)
	{
		m_vecBestCars[i].InsertNewBrain(pBestBrains[i]);

		m_vecBestCars[i].Reset();

		// Index 0 is the best genome, index 1 the second best and so on

		string text = string("g") + to_string((long double) (m_iGenerations-1)) + string(".") + to_string((long double) i) + string(".genome");

		//const char *text = "test";//m_iGenerations-1+"."+i; //+"Test";
		m_pPop->WriteGenome(text.c_str(), i);
	}

	//this will call WM_PAINT which will render our scene
	InvalidateRect(m_hwndInfo, NULL, TRUE);
	UpdateWindow(m_hwndInfo);
	return true;
}

//---------------------------------- RenderNetworks ----------------------
//
//  Renders the best four phenotypes from the previous generation
//------------------------------------------------------------------------
void CCarController::RenderNetworks(HDC &surface)
{
	if (m_iGenerations < 1)
	{
		return;
	}

	//draw the network of the best 4 genomes. First get the dimensions of the 
	//info window
	RECT rect;
	GetClientRect(m_hwndInfo, &rect);

	int	cxInfo = rect.right;
	int	cyInfo = rect.bottom;

	//now draw the m_cars best networks
	for(int i = 0; i < CParams::iNumBestSweepers; i++)
	{
		if(i == 0) m_vecBestCars[0].DrawNet(surface, 0, cxInfo/2, cyInfo/2, 0);
		else if(i == 1) m_vecBestCars[1].DrawNet(surface, cxInfo/2, cxInfo, cyInfo/2, 0);
		else if(i == 2) m_vecBestCars[2].DrawNet(surface, 0, cxInfo/2, cyInfo, cyInfo/2);
		else if(i == 3) m_vecBestCars[3].DrawNet(surface, cxInfo/2, cxInfo, cyInfo, cyInfo/2);
	}
}

//------------------------------------Render()--------------------------------------
//
//----------------------------------------------------------------------------------
void CCarController::Render(HDC &surface)
{
	// only render stats
	PlotStats(surface);

	RECT sr;
	sr.top    = m_cyClient-50;
	sr.bottom = m_cyClient;
	sr.left   = 0;
	sr.right  = m_cxClient;

	//render the species chart
	m_pPop->RenderSpeciesInfo(surface, sr);
}
//------------------------- RenderSweepers -------------------------------
//
//  given a vector of sweepers this function renders them to the screen
//------------------------------------------------------------------------
void CCarController::RenderCars(HDC &surface, vector<CCar> &cars)
{
	return;
	/*
	for (int i=0; i<sweepers.size(); ++i)
	{

		//if they have crashed into an obstacle draw
		if ( sweepers[i].Collided())
		{
			SelectObject(surface, m_RedPen);
		}

		else
		{
			SelectObject(surface, m_BluePen);
		}

		//grab the sweeper vertices
		vector<SPoint> sweeperVB = m_SweeperVB;

		//transform the vertex buffer
		sweepers[i].WorldTransform(sweeperVB, sweepers[i].Scale());

		//draw the sweeper left track
		MoveToEx(surface, (int)sweeperVB[0].x, (int)sweeperVB[0].y, NULL);

		for (int vert=1; vert<4; ++vert)
		{
			LineTo(surface, (int)sweeperVB[vert].x, (int)sweeperVB[vert].y);
		}

		LineTo(surface, (int)sweeperVB[0].x, (int)sweeperVB[0].y);

		//draw the sweeper right track
		MoveToEx(surface, (int)sweeperVB[4].x, (int)sweeperVB[4].y, NULL);

		for (int vert=5; vert<8; ++vert)
		{
			LineTo(surface, (int)sweeperVB[vert].x, (int)sweeperVB[vert].y);
		}

		LineTo(surface, (int)sweeperVB[4].x, (int)sweeperVB[4].y);

		MoveToEx(surface, (int)sweeperVB[8].x, (int)sweeperVB[8].y, NULL);
		LineTo(surface, (int)sweeperVB[9].x, (int)sweeperVB[9].y);

		MoveToEx(surface, (int)sweeperVB[10].x, (int)sweeperVB[10].y, NULL);

		for (int vert=11; vert<16; ++vert)
		{
			LineTo(surface, (int)sweeperVB[vert].x, (int)sweeperVB[vert].y);
		}
	}//next sweeper
	*/
}

//----------------------------- RenderSensors ----------------------------
//
//  renders the sensors of a given vector of sweepers
//------------------------------------------------------------------------
void CCarController::RenderSensors(HDC &surface, vector<CCar> &cars)
{
	/*
	//render the sensors
	for (int i=0; i<cars.size(); ++i)
	{
		//grab each sweepers sensor data
		vector<SPoint> tranSensors    = sweepers[i].Sensors();
		vector<double> SensorReadings = sweepers[i].SensorReadings();
		vector<double> MemoryReadings = sweepers[i].MemoryReadings();

		for (int sr=0; sr<tranSensors.size(); ++sr)
		{
			if (SensorReadings[sr] > 0)
			{
				SelectObject(surface, m_RedPen);
			}

			else
			{
				SelectObject(surface, m_GreyPenDotted);
			}

			//make sure we clip the drawing of the sensors or we will get
			//unwanted artifacts appearing
			if (!((fabs(sweepers[i].Position().x - tranSensors[sr].x) >
				(CParams::dSensorRange+1))||
				(fabs(sweepers[i].Position().y - tranSensors[sr].y) >
				(CParams::dSensorRange+1))))
			{

				MoveToEx(surface,
					(int)sweepers[i].Position().x,
					(int)sweepers[i].Position().y,
					NULL);

				LineTo(surface, (int)tranSensors[sr].x, (int)tranSensors[sr].y);

				//render the cell sensors
				RECT rect;
				rect.left  = (int)tranSensors[sr].x - 2;
				rect.right = (int)tranSensors[sr].x + 2;
				rect.top   = (int)tranSensors[sr].y - 2;
				rect.bottom= (int)tranSensors[sr].y + 2;

				if (MemoryReadings[sr] < 0)
				{

					FillRect(surface, &rect, m_BlueBrush);
				}

				else
				{
					FillRect(surface, &rect, m_RedBrush);
				}

			}
		}
	}
	*/
}


//--------------------------PlotStats-------------------------------------
//
//  Given a surface to draw on this function displays some simple stats 
//------------------------------------------------------------------------
void CCarController::PlotStats(HDC surface)const
{
	string s = "Generation:              " + itos(m_iGenerations);
	TextOut(surface, 5, 25, s.c_str(), s.size());

	s = "Num Species:          " + itos(m_pPop->NumSpecies());
	TextOut(surface, 5, 45, s.c_str(), s.size());

	s = "Best Fitness so far: " + ftos(m_pPop->BestEverFitness());
	TextOut(surface, 5, 5, s.c_str(), s.size());
}

//------------------------------- GetFitnessScores -----------------------
//
//  returns a std::vector containing the genomes fitness scores
//------------------------------------------------------------------------
vector<double> CCarController::GetFitnessScores()const
{
	vector<double> scores;

	for (int i=0; i<m_vecCars.size(); ++i)
	{
		scores.push_back(m_vecCars[i].Fitness());
	}
	return scores;
}