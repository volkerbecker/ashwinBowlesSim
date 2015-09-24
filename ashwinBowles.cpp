//============================================================================
// Name        : ashwinBowles.cpp
// Author      : Volker Becker
// Version     :
// Copyright   : (c) 2015 by Volker Becker
// Description : Hello World in C, Ansi-style
//============================================================================
// ios::exceptions
#include "Visualizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.hpp>
#include "ParticleSystem.h"
#include <iostream>
#include <unistd.h>
#include "AshwinBowlesSystem.h"
#include "Paramters.h"
#include <thread>
#include <fstream>
#include "Setup.h"

int main(void) {
	ofstream energyStream("energy.dat");



	Parameters kernelHostParameters;
	HostParameters hostParameters;
	Visualizer *visualizer=nullptr;

	//read the paramter file
	parseConfigurationFile("asgconf.icf",hostParameters,kernelHostParameters);
	//todo consitency check

	//create visualization objetc if needed
	if (hostParameters.visualization) {
		visualizer = new Visualizer(hostParameters.vboxX, hostParameters.vboxY);
	}

	AshwinBowlesSystem simulation(kernelHostParameters,hostParameters);

	if(hostParameters.visualization) {
		simulation.updateOffsetFreeData(hostParameters.vLineSize);
		visualizer->initializeSystem(
				simulation.getPrtToOffsetFreePositions(),
				kernelHostParameters.numberOfParticles,
				kernelHostParameters.radius,
				hostParameters.vLineSize,hostParameters.vLineSize,0,-hostParameters.vLineSize+kernelHostParameters.diameter,10);
				visualizer->updateimage();}

	// the main loop todo integrate it in simulation class if possible
	double Ekin,Epot; //kinetic an potential energy
	double time;
	int tapNumber=0; // counter which counts the performed taps
	int i=0; //number of the timestep
	while(tapNumber < hostParameters.numberOfTaps) {
		simulation.enqueueTimeStep();
		//check tapping criteroin
		if(i%hostParameters.tappingCheck==0) {
			simulation.upDateHostMemory();
			simulation.getEnergy(Ekin,Epot);
			if (Ekin < hostParameters.tapThreshold) {
				//todo save tap data
				++tapNumber;
				simulation.velocityPulse((cl_float2 ) {hostParameters.tappingAmplitudeX,hostParameters.tappingAmplitudeY});
			}
		}
		//do visualization
		if(i%hostParameters.visualizerIntervall==0 && hostParameters.visualization) {
			simulation.updateOffsetFreeData(hostParameters.vLineSize);
			visualizer->updateimage();
		}
		if(i%hostParameters.snapshotIntervall==0) {
			//todo save system state
			simulation.upDateHostMemory();
			simulation.getEnergy(Ekin,Epot);
			cout << "step: " << i << "Ekin " << Ekin << " Epot " << Epot << " Gesamt: " << Ekin+Epot << endl;
			energyStream << i << "\t" << Ekin << "\t" << Epot << "\t" << Ekin+Epot << "\n";
		}
		if(i%10000==0) {
			simulation.updateOffsetFreeData(hostParameters.vLineSize);
			visualizer->updateimage();
			simulation.getEnergy(Ekin,Epot);
			cout << "step: " << i << "Ekin " << Ekin << " Epot " << Epot << " Gesamt: " << Ekin+Epot << endl;
		}
		if(i%hostParameters.offSetupdate==0) {
			simulation.enqueOffestupdate();
		}
		++i;
	}
	visualizer->close();

	if(visualizer != nullptr) delete visualizer;
	return EXIT_SUCCESS;
}
