#ifndef CCARCONTROLLER_H
#define CCARCONTROLLER_H

//------------------------------------------------------------------------
//
//	Name: CCarController.h
//
//  Author: Morten Schnack 2013 (mosch@itu.dk)
//
//  Desc: Controller class for NEAT Cars  
//
//------------------------------------------------------------------------
#include <vector>
#include <sstream>
#include <string>
#include <windows.h>

#include "CCar.h"
#include "CParams.h"
#include "Cga.h"

using namespace std;

class CCarController
{

private:

	//storage for the entire population of chromosomes
	Cga*                 m_pPop;

	//array of cars
	vector<CCar> m_vecCars;

  //array of best cars from last generation (used for
  //display purposes when 'B' is pressed by the user)
  vector<CCar> m_vecBestCars;

	int					         m_NumCars;

  //vertex buffer for objects
  vector<SPoint>       m_ObjectsVB;

	//stores the average fitness per generation 
	vector<double>		   m_vecAvFitness;

	//stores the best fitness per generation
	vector<double>		   m_vecBestFitness;

  //best fitness ever
  double               m_dBestFitness;

	//pens we use for the stats
	HPEN				m_RedPen;
	HPEN				m_BluePen;
	HPEN				m_GreenPen;
  HPEN        m_GreyPenDotted;
  HPEN        m_RedPenDotted;
	HPEN				m_OldPen;

  HBRUSH      m_RedBrush;
  HBRUSH      m_BlueBrush;
	
	//local copy of the handle to the application window
	HWND				m_hwndMain;

  //local copy of the  handle to the info window
  HWND        m_hwndInfo;

	//toggles the speed at which the simulation runs
	bool				m_bFastRender;

  //when set, renders the best performers from the 
  //previous generaion.
  bool        m_bRenderBest;
	
	//cycles per generation
	int					m_iTicks;

	//generation counter
	int					m_iGenerations;

  //local copy of the client window dimensions
  int         m_cxClient, m_cyClient;

  //this is the car who's memory cells are displayed
  int         m_iViewThisCar;

  void   PlotStats(HDC surface)const;
  
  void   RenderCars(HDC &surface, vector<CCar> &cars);
  
  void   RenderSensors (HDC &surface, vector<CCar> &cars);

public:

	CCarController(HWND hwndMain, int cxClient, int cyClient);

	~CCarController();

	void		Render(HDC &surface);

  //renders the phenotypes of the four best performers from
  //the previous generation
  void    RenderNetworks(HDC &surface);
	
	bool		Update();


	//-------------------------------------accessor methods
	bool		        FastRender()const{return m_bFastRender;}
	void		        FastRender(bool arg){m_bFastRender = arg;}
	void		        FastRenderToggle(){m_bFastRender = !m_bFastRender;}

  bool            RenderBest()const{return m_bRenderBest;}
  void            RenderBestToggle(){m_bRenderBest = !m_bRenderBest;}

  void            PassInfoHandle(HWND hnd){m_hwndInfo = hnd;}
  
  vector<double>  GetFitnessScores()const;

  void            ViewBest(int val)
  {
    if ( (val>4) || (val< 1) )
    {
      return;
    }

    m_iViewThisCar = val-1;
  }
};


#endif
	
