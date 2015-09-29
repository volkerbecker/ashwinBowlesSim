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

int main(int argc, char *argv[]) {


	Parameters kernelHostParameters;
	HostParameters hostParameters;
	Visualizer *visualizer=nullptr;




	//read the paramter file
	parseConfigurationFile(argv[1],hostParameters,kernelHostParameters);
	//todo consitency check

	string tapStateFileName=hostParameters.baseName + ".tap.gstat";
	cout << hostParameters.baseName << endl;
	ofstream tappSave(tapStateFileName);
	ofstream stateSave(hostParameters.baseName + ".tap.state");

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

	double time;
	int tapNumber=hostParameters.startSnapNumber; // counter which counts the performed taps
	int i=hostParameters.startTimeStep; //number of the timestep
	vector<bool> state;
	int exitedBonds;

	while(tapNumber < hostParameters.numberOfTaps) {
		simulation.enqueueTimeStep();
		//check tapping criteroin

		if (i % hostParameters.tappingCheck == 0) {
			double Ekin, Epot; //kinetic an potential energy
			simulation.upDateHostMemory();
			simulation.getEnergy(Ekin, Epot);

			if (Ekin < hostParameters.tapThreshold) {
				if (simulation.isJammed(exitedBonds, state)) {
					cout << "state is jammed, exited bonds: " << exitedBonds;
				} else {
					cout << "state is not jammed";
				}
				cout << ", volume " << simulation.volume() << endl;
				//todo save tap data
				tappSave.precision(20);
				tappSave << tapNumber << "\t" << Epot << "\t" << Ekin << "\t"
						<< exitedBonds << "\t" << simulation.volume() << endl;
				++tapNumber;

				char fname[128];
				sprintf(fname, "%s.tap.%05d.pdat",
						(const char*) hostParameters.baseName.c_str(),
						tapNumber);
				if(hostParameters.saveDatails)
					simulation.saveState(fname, i, tapNumber);
				sprintf(fname, "%s.tap.%05d.state",
						(const char*) hostParameters.baseName.c_str(),
						tapNumber);
				for (int ii = 0; ii < state.size(); ++ii) {
					stateSave << state[ii];
				}
				//do the next excitation
				switch (hostParameters.tappingType) {
				case RDELTA:
					simulation.velocityPulse(
							(cl_float2 ) {
											hostParameters.tappingAmplitudeX,
											hostParameters.tappingAmplitudeY });
					break;

				case HAMMER:
					simulation.hammerPulse(
							(cl_float2 ) {
											hostParameters.tappingAmplitudeX,
											hostParameters.tappingAmplitudeY });
					hostParameters.tappingAmplitudeX *= -1;
					hostParameters.tappingAmplitudeY *= -1;
					break;
				default:
					break;
				}
				stateSave << endl;
			}
		}
		//do visualization
		if(i%hostParameters.visualizerIntervall==0 && hostParameters.visualization) {
			simulation.updateOffsetFreeData(hostParameters.vLineSize);
			visualizer->updateimage();
		}
		if(i%hostParameters.snapshotIntervall==0) {
			//todo save system state
			double Ekin,Epot; //kinetic an potential energy
			simulation.upDateHostMemory();
			simulation.getEnergy(Ekin,Epot);
			cout << "step: " << i << "Ekin " << Ekin << " Epot " << Epot << " Gesamt: " << Ekin+Epot << endl;
			char fname[25];
			sprintf(fname,"%s.snap.%05d.pdat",(const char*)hostParameters.baseName.c_str(),tapNumber);
			simulation.saveState(fname,i,tapNumber);
		}
		if(i%hostParameters.offSetupdate==0) {
			simulation.enqueOffestupdate();
		}
		++i;
	}
	if(hostParameters.visualization) visualizer->close();

	if(visualizer != nullptr) delete visualizer;
	tappSave.close();
	return EXIT_SUCCESS;
}
