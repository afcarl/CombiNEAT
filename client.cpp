#include "client.h"
#include "stdio.h"
#include <iostream>
/***************************************************************************

file                 : client.cpp
copyright            : (C) 2007 Daniele Loiacono

***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
/* Uncomment the following lines under windows */
#define WIN32 // maybe not necessary because already define
#define __DRIVER_CLASS__ SimpleDriver     // put here the name of your driver class
#define __DRIVER_INCLUDE__ "SimpleDriver.h" // put here the filename of your driver h\\eader

#include "CParams.h"

// 0 = show only stats and network, 1 = show debug windows, 2 = start TORCS (but don't show debug windows), 3 = start TORCS and show debug windows
#define GRAPHICS 0

#ifdef WIN32
#include <WinSock.h>
#else
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include __DRIVER_INCLUDE__

/*** defines for UDP *****/
#define UDP_MSGLEN 1000
#define UDP_CLIENT_TIMEUOT 1000000
//#define __UDP_CLIENT_VERBOSE__
/************************/

#ifdef WIN32
typedef sockaddr_in tSockAddrIn;
#define CLOSE(x) closesocket(x)
#define INVALID(x) x == INVALID_SOCKET
#else
typedef int SOCKET;
typedef struct sockaddr_in tSockAddrIn;
#define CLOSE(x) close(x)
#define INVALID(x) x < 0
#endif

class __DRIVER_CLASS__;
typedef __DRIVER_CLASS__ tDriver;


using namespace std;

//void parse_args(int argc, char *argv[], char *hostName, unsigned int &serverPort, char *id, unsigned int &maxEpisodes,unsigned int &maxSteps,
//		bool &noise, double &noiseAVG, double &noiseSTD, long &seed, char *trackName, BaseDriver::tstage &stage);
void parse_args(int argc, char *argv[], char *hostName, unsigned int &serverPort, char *id, unsigned int &maxEpisodes,
	unsigned int &maxSteps, char *trackName, BaseDriver::tstage &stage);

int runCar(CNeuralNet* brain)
{
	//Alloc Console
	//print some stuff to the console
	//make sure to include #include "stdio.h"
	//note, you must use the #include <iostream>/ using namespace std
	//to use the iostream... #incldue "iostream.h" didn't seem to work
	//in my VC 6
	if(GRAPHICS == 1 || GRAPHICS == 3)
	{
		AllocConsole();
		freopen("conin$","r",stdin);
		freopen("conout$","w",stdout);
		freopen("conout$","w",stderr);
		printf("Debugging Window:\n");
	}

	SOCKET socketDescriptor;
	int numRead;

	char hostName[1000];
	unsigned int serverPort;
	char id[1000];
	unsigned int maxEpisodes;
	unsigned int maxSteps;
	//    bool noise;
	//    double noiseAVG;
	//    double noiseSTD;
	//    long seed;
	char trackName[1000];
	BaseDriver::tstage stage;

	tSockAddrIn serverAddress;
	struct hostent *hostInfo;
	struct timeval timeVal;
	fd_set readSet;
	char buf[UDP_MSGLEN];
#ifdef WIN32 
	/* WinSock Startup */

	WSADATA wsaData={0};
	WORD wVer = MAKEWORD(2,2);
	int nRet = WSAStartup(wVer,&wsaData);

	if(nRet == SOCKET_ERROR)
	{
		std::cout << "Failed to init WinSock library" << std::endl;
		exit(1);
	}
#endif

	//    parse_args(argc,argv,hostName,serverPort,id,maxEpisodes,maxSteps,noise,noiseAVG,noiseSTD,seed,trackName,stage);

	char *argv [] = { { "" } };
	parse_args(0,argv,hostName,serverPort,id,maxEpisodes,maxSteps,trackName,stage);

	//    if (seed>0)
	//    	srand(seed);
	//    else
	//    	srand(time(NULL));

	maxSteps = CParams::iNumTicks;
	maxEpisodes = 1;

	hostInfo = gethostbyname(hostName);
	if (hostInfo == NULL)
	{
		std::cout << "Error: problem interpreting host: " << hostName << "\n";
		exit(1);
	}

	// Print command line option used
	std::cout << "***********************************" << endl;

	std::cout << "HOST: "   << hostName    << endl;

	std::cout << "PORT: " << serverPort  << endl;

	std::cout << "ID: "   << id     << endl;

	std::cout << "MAX_STEPS: " << maxSteps << endl; 

	std::cout << "MAX_EPISODES: " << maxEpisodes << endl;

	//    if (seed>0)
	//    	std::cout << "SEED: " << seed << endl;

	std::cout << "TRACKNAME: " << trackName << endl;

	if (stage == BaseDriver::WARMUP)
		std::cout << "STAGE: WARMUP" << endl;
	else if (stage == BaseDriver::QUALIFYING)
		std::cout << "STAGE:QUALIFYING" << endl;
	else if (stage == BaseDriver::RACE)
		std::cout << "STAGE: RACE" << endl;
	else
		std::cout << "STAGE: UNKNOWN" << endl;

	std::cout << "***********************************" << endl;
	// Create a socket (UDP on IPv4 protocol)
	socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
	if (INVALID(socketDescriptor))
	{
		cerr << "cannot create socket\n";
		exit(1);
	}

	// Set some fields in the serverAddress structure.
	serverAddress.sin_family = hostInfo->h_addrtype;
	memcpy((char *) &serverAddress.sin_addr.s_addr,
		hostInfo->h_addr_list[0], hostInfo->h_length);
	serverAddress.sin_port = htons(serverPort);

	tDriver d;
	strcpy(d.trackName,trackName);
	d.stage = stage;

	// Launch server
	// Notice /c causes cmd to close after executing the command
	if(GRAPHICS == 0)
		ShellExecute(0, "open", "cmd.exe", "/c \"cd C:\\Program Files (x86)\\torcs\\ & wtorcs.exe -t 10000 -r race_config.xml\"", 0, SW_HIDE);
	else if(GRAPHICS == 1)
		ShellExecute(0, "open", "cmd.exe", "/c \"cd C:\\Program Files (x86)\\torcs\\ & wtorcs.exe -t 10000 -r race_config.xml\"", 0, SW_SHOWNORMAL);
	else if(GRAPHICS >= 2)
		ShellExecute(0, "open", "cmd.exe", "/k \"cd C:\\Program Files (x86)\\torcs\\ & wtorcs.exe -t 10000\"", 0, SW_SHOWNORMAL);
	// Give server time to start up, before trying to connect with client
	//Sleep(800);

	bool shutdownClient=false;
	unsigned long curEpisode=0;
	do
	{
		/***********************************************************************************
		************************* UDP client identification ********************************
		***********************************************************************************/
		do
		{
			// Initialize the angles of rangefinders
			float angles[19];
			d.init(angles);
			string initString = SimpleParser::stringify(string("init"),angles,19);
			cout << "Sending id to server: " << id << endl;
			initString.insert(0,id);
			cout << "Sending init string to the server: " << initString << endl;
			if (sendto(socketDescriptor, initString.c_str(), initString.length(), 0,
				(struct sockaddr *) &serverAddress,
				sizeof(serverAddress)) < 0)
			{
				cerr << "cannot send data ";
				CLOSE(socketDescriptor);
				exit(1);
			}

			// wait until answer comes back, for up to UDP_CLIENT_TIMEUOT micro sec
			FD_ZERO(&readSet);
			FD_SET(socketDescriptor, &readSet);
			timeVal.tv_sec = 0;
			timeVal.tv_usec = UDP_CLIENT_TIMEUOT;

			if (select(socketDescriptor+1, &readSet, NULL, NULL, &timeVal))
			{
				// Read data sent by the solorace server
				memset(buf, 0x0, UDP_MSGLEN);  // Zero out the buffer.
				numRead = recv(socketDescriptor, buf, UDP_MSGLEN, 0);
				if (numRead < 0)
				{
					cerr << "didn't get response from server...";
				}
				else
				{
					cout << "Received: " << buf << endl;

					if (strcmp(buf,"***identified***")==0)
						break;
				}
			}

		}  while(1);

		unsigned long currentStep=0; 

		while(1)
		{
			// wait until answer comes back, for up to UDP_CLIENT_TIMEUOT micro sec
			FD_ZERO(&readSet);
			FD_SET(socketDescriptor, &readSet);
			timeVal.tv_sec = 0;
			timeVal.tv_usec = UDP_CLIENT_TIMEUOT;

			if (select(socketDescriptor+1, &readSet, NULL, NULL, &timeVal))
			{
				// Read data sent by the solorace server
				memset(buf, 0x0, UDP_MSGLEN);  // Zero out the buffer.
				numRead = recv(socketDescriptor, buf, UDP_MSGLEN, 0);
				if (numRead < 0)
				{
					cerr << "didn't get response from server?";
					CLOSE(socketDescriptor);
					exit(1);
				}

#ifdef __UDP_CLIENT_VERBOSE__
				cout << "Received: " << buf << endl;
#endif

				if (strcmp(buf,"***shutdown***")==0)
				{
					d.onShutdown();
					shutdownClient = true;
					cout << "Client Shutdown" << endl;
					break;
				}

				if (strcmp(buf,"***restart***")==0)
				{
					d.onRestart();
					cout << "Client Restart" << endl;
					break;
				}
				/**************************************************
				* Compute The Action to send to the solorace sever
				**************************************************/
				if ( (++currentStep) != maxSteps)
				{
					string action = d.drive(string(buf), brain);
					memset(buf, 0x0, UDP_MSGLEN);
					sprintf(buf,"%s",action.c_str());
				}
				else
					break;
					//sprintf (buf, "(meta 1)");

				if (sendto(socketDescriptor, buf, strlen(buf)+1, 0,
					(struct sockaddr *) &serverAddress,
					sizeof(serverAddress)) < 0)
				{
					cerr << "cannot send data ";
					CLOSE(socketDescriptor);
					exit(1);
				}
#ifdef __UDP_CLIENT_VERBOSE__
				else
					cout << "Sending " << buf << endl;
#endif
			}
			else
			{
				cout << "** Server did not respond in 1 second.\n";
			}
		}
	} while(shutdownClient==false && ( (++curEpisode) != maxEpisodes) );

	if (shutdownClient==false)
		d.onShutdown();
	CLOSE(socketDescriptor);
#ifdef WIN32
	WSACleanup();
#endif
	return d.distRaced - d.damage + 10000;
}

//void parse_args(int argc, char *argv[], char *hostName, unsigned int &serverPort, char *id, unsigned int &maxEpisodes,
//		  unsigned int &maxSteps,bool &noise, double &noiseAVG, double &noiseSTD, long &seed, char *trackName, BaseDriver::tstage &stage)
void parse_args(int argc, char *argv[], char *hostName, unsigned int &serverPort, char *id, unsigned int &maxEpisodes,
	unsigned int &maxSteps, char *trackName, BaseDriver::tstage &stage)
{
	int		i;

	// Set default values
	maxEpisodes=0;
	maxSteps=0;
	if(GRAPHICS >= 2)
		serverPort=3001;
	else
		serverPort=3002;
	strcpy(hostName,"localhost");
	strcpy(id,"SCR");
	//    noise=false;
	//    noiseAVG=0;
	//    noiseSTD=0.05;
	//    seed=0;
	strcpy(trackName,"unknown");
	stage=BaseDriver::UNKNOWN;


	i = 1;
	while (i < argc)
	{
		if (strncmp(argv[i], "host:", 5) == 0)
		{
			sprintf(hostName, "%s", argv[i]+5);
			i++;
		}
		else if (strncmp(argv[i], "port:", 5) == 0)
		{
			sscanf(argv[i],"port:%d",&serverPort);
			i++;
		}
		else if (strncmp(argv[i], "id:", 3) == 0)
		{
			sprintf(id, "%s", argv[i]+3);
			i++;
		}
		else if (strncmp(argv[i], "maxEpisodes:", 12) == 0)
		{
			sscanf(argv[i],"maxEpisodes:%ud",&maxEpisodes);
			i++;
		}
		else if (strncmp(argv[i], "maxSteps:", 9) == 0)
		{
			sscanf(argv[i],"maxSteps:%ud",&maxSteps);
			i++;
		}
		//    	else if (strncmp(argv[i], "seed:", 5) == 0)
		//    	{
		//    	    	sscanf(argv[i],"seed:%ld",&seed);
		//    	    	i++;
		//    	}
		else if (strncmp(argv[i], "track:", 6) == 0)
		{
			sscanf(argv[i],"track:%s",trackName);
			i++;
		}
		else if (strncmp(argv[i], "stage:", 6) == 0)
		{
			int temp;
			sscanf(argv[i],"stage:%d",&temp);
			stage = (BaseDriver::tstage) temp;
			i++;
			if (stage<BaseDriver::WARMUP || stage > BaseDriver::RACE)
				stage = BaseDriver::UNKNOWN;
		}
		else {
			i++;		/* ignore bad args */
		}
	}
}
