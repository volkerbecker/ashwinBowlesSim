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

using namespace std;


void parseConfigurationFile(const string &filename,HostParameters &hostParamters ///< comtains parameters only relevant for the host
		,Parameters khParameters) {///< contain parameters relevant for the host and the kernels
	ifstream infile;
	infile.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
	try {
		infile.open(filename, std::ifstream::in);
		infile.exceptions ( std::ifstream::badbit );
		string keyword;
		while(!infile.eof()) {
			infile >> keyword;
			if(keyword[0]=='#') getline(infile,keyword); //ignore comment
			else
			{
				cout << keyword << endl;
				evaluateKeyWord(infile,keyword,hostParamters,khParameters);
			}

		}
		infile.close();
	} catch (ifstream::failure &e) {
		cerr << "Error during read configuration file \n" << e.what() << endl;
	}

}

void evaluateKeyWord(ifstream &infile ///< stream with parameter file
		,const string keyword, ///< keyword
		HostParameters &hostParamters,
		Parameters khParamters) {
	if(keyword == "PNUMBER"){
		infile >> khParamters.numberOfParticles;
	}
	else if(keyword == "PMASS") {
		infile >> khParamters.mass;
	}
	else if(keyword == "PRADIUS") {
		infile >> khParamters.radius;
		khParamters.diameter=2*khParamters.radius;
	}
	else if(keyword == "TIMESTEP") {
		infile >> khParamters.timestep;
		khParamters.timestepSq=khParamters.timestep*khParamters.timestep;
	}
	else if(keyword == "SPRINGCONST") {
		infile >> khParamters.springConstant;
	}
	else if(keyword == "NORMALDAMPING") {
		infile >> khParamters.damping;
	}
	else if(keyword == "SYSTEMSIZE") {
		double width,hight;
		infile >> width;
		infile >> hight;
		khParamters.leftWallofset = 0;
		khParamters.rightWallOffset = (int) width;
		khParamters.upperWall=hight/2;
		khParamters.lowerWall=-khParamters.upperWall;
	}
}
