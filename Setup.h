/*
 * Setup.h
 *
 *  Created on: 23 Sep 2015
 *      Author: becker
 */

#ifndef SETUP_H_
#define SETUP_H_

#include "Paramters.h"
#include <string>

using namespace std;

///< load the parameters from the configuration file
void parseConfigurationFile(const string &filename,HostParameters &hostParamters ///< comtains parameters only relevant for the host
		,Parameters khParameters); ///< contain parameters relevant for the host and the kernels

///< evaluates keyword
void evaluateKeyWord(ifstream &infile ///< stream with parameter file
		,const string keyword, ///< keyword
		HostParameters &hostParamters,
		Parameters khParamters); ///

#endif /* SETUP_H_ */
