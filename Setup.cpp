/*
 * Setup.cpp
 *
 *  Created on: 23 Sep 2015
 *      Author: becker
 */

#include "Setup.h"
#include <iostream>
#include "fstream"
#include <string>
#include <climits>
#include <cmath>

using namespace std;


void parseConfigurationFile(const string &filename,HostParameters &hostParamters ///< comtains parameters only relevant for the host
		,Parameters &khParameters) {///< contain parameters relevant for the host and the kernels
	ifstream infile;
	infile.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
	hostParamters.saveDatails=false;
	hostParamters.maxTimeSteps=INT_MAX;
	hostParamters.useOPENCL=true;
	hostParamters.targetDevice=0;
	hostParamters.targetPlatform=0;
	khParameters.number_of_systems=1;
	try {
		infile.open(filename, std::ifstream::in);
		infile.exceptions ( std::ifstream::badbit );
		string keyword;
		while(!infile.eof()) {
			infile >> keyword;
			if(keyword[0]=='#') getline(infile,keyword); //ignore comment
			else
			{
				if(!infile.fail())
					evaluateKeyWord(infile,keyword,hostParamters,khParameters);
			}

		}
		infile.close();
	} catch (ifstream::failure &e) {
		cerr << "Error during read configuration file \n" << e.what() << endl;
	}
	khParameters.damping = 2 * khParameters.damping
			* sqrt(khParameters.springConstant * khParameters.mass);
	khParameters.stampAcceleration *= khParameters.inverseMass;
}

void evaluateKeyWord(ifstream &infile ///< stream with parameter file
		, string keyword, ///< keyword
		HostParameters &hostParamters, Parameters &khParamters) {
	infile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try {
		if (keyword == "USEOPENCL") {
			infile >> hostParamters.useOPENCL;
		} else if (keyword== "CLTARGET") {
			infile >> hostParamters.targetPlatform;
			infile >> hostParamters.targetDevice;
		} else if (keyword == "PNUMBER") {
			infile >> khParamters.numberOfParticles;
		} else if (keyword == "PMASS") {
			infile >> khParamters.mass;
			khParamters.inverseMass=1.0f/khParamters.mass;
		} else if (keyword == "PRADIUS") {
			infile >> khParamters.radius;
			khParamters.diameter = 2 * khParamters.radius;
		} else if (keyword == "TIMESTEP") {
			infile >> khParamters.timestep;
			khParamters.timestepSq = khParamters.timestep
					* khParamters.timestep;
		} else if (keyword == "SPRINGCONST") {
			infile >> khParamters.springConstant;
		} else if (keyword == "NORMALDAMPING") {
			infile >> khParamters.damping;
		} else if (keyword == "VISKOSITY") {
			infile >> khParamters.viskosity;
		} else if (keyword == "NUMSYS"){
			infile >> khParamters.number_of_systems;
		} else if (keyword == "SYSTEMSIZE") {
			double width, hight;
			infile >> width;
			infile >> hight;
			khParamters.leftWallofset = 0;
			khParamters.leftWall=0;
			khParamters.rightWallOffset = (int) width;
			khParamters.rightWall=width - (int)width;
			khParamters.upperWall = hight / 2;
			khParamters.lowerWall = -khParamters.upperWall;
		} else if (keyword == "KRITERION") {
			infile >> keyword;
			if (keyword == "ENERGY")
				hostParamters.tappingType = ENERGY;
			else {
				cout << "tapping criterion " << keyword << " not supported \n";
				exit(EXIT_FAILURE);
			}
			infile >> hostParamters.tapThreshold;
			infile >> hostParamters.tappingCheck;
		} else if (keyword == "TAPTYPE") {
			infile >> keyword;
			if (keyword == "RDELTA") {
				hostParamters.tappingType = RDELTA;
				infile >> khParamters.stampAcceleration;
				hostParamters.tappingAmplitude.resize(
						khParamters.number_of_systems);
				for (int i = 0; i < hostParamters.tappingAmplitude.size();
						++i) {
					infile >> hostParamters.tappingAmplitude[i].s[0];
					infile >> hostParamters.tappingAmplitude[i].s[1];
				}
			} else if (keyword == "HAMMER") {
				hostParamters.tappingType = RDELTA;
								infile >> khParamters.stampAcceleration;
				hostParamters.tappingAmplitude.resize(
						khParamters.number_of_systems);
				for (int i = 0; i < hostParamters.tappingAmplitude.size();
						++i) {
					infile >> hostParamters.tappingAmplitude[i].s[0];
					infile >> hostParamters.tappingAmplitude[i].s[1];
				}
			} else {
				cout << "tapping protocol " << keyword << "not supported \n";
				exit(EXIT_FAILURE);
			}
		} else if (keyword == "VISUALIZER") {
			infile >> hostParamters.visualization;
		} else if (keyword == "VBOXSIZE") {
			infile >> hostParamters.vboxX;
			infile >> hostParamters.vboxY;
		} else if (keyword == "VLINE") {
			infile >> hostParamters.vLineSize;
		} else if (keyword == "EGDES") {
			infile >> hostParamters.edges;
		} else if (keyword == "VINTERVAL") {
			infile >> hostParamters.visualizerIntervall;
		} else if (keyword == "BITMAPSAVE") {
			infile >> hostParamters.savesBitmaps;
		} else if (keyword == "BASENAME") {
			infile >> hostParamters.baseName;
		} else if (keyword == "SNAP") {
			infile >> keyword;
			if (keyword == "INF")
				hostParamters.snapshotIntervall = INT_MAX;
			else
				hostParamters.snapshotIntervall = atof(keyword.c_str());
		} else if (keyword == "INITIAL") {
			infile >> keyword;
			if (keyword == "DENSEST") {
				hostParamters.initialConfig = PDENSEST;
			} else if (keyword == "LOOSEST") {
				hostParamters.initialConfig = PLOOSEST;
			} else if (keyword == "FILE") {
				hostParamters.initialConfig = PFILE;
				infile >> hostParamters.inFileName;
			} else {
				cerr << "Particle initialisation type " << keyword
						<< " not supported \n";
				exit(0);
			}
		} else if(keyword == "NUMTAP") {
			infile >> hostParamters.numberOfTaps;
		} else if(keyword == "TIMEOFFSET") {
			infile >> hostParamters.startTimeStep;
			infile >> hostParamters.startSnapNumber;
		} else if(keyword== "OFFSETUPDATE") {
			infile >> hostParamters.offSetupdate;
		} else if(keyword == "SAVEDETAILS") {
			infile >> hostParamters.saveDatails;
		} else if(keyword == "MAXTIMESTEPS") {
			infile >> hostParamters.maxTimeSteps;
		} else {
			cerr << "Keyword " << keyword << " is unknown" << endl;
			exit(EXIT_FAILURE);
		}
	} catch (ifstream::failure &e) {
		cerr << "Error during read configuration file \n " << e.what() << endl;
		cerr << "Keyword: " << keyword << "\n";
		exit(0);
	}
	infile.exceptions(std::ifstream::badbit);
}
