//============================================================================
// Name        : ashwinBowles.cpp
// Author      : Volker Becker
// Version     :
// Copyright   : (c) 2015 by Volker Becker
// Description : Hello World in C, Ansi-style
//============================================================================
// ios::exceptions
#include "compilerSwitch.h"
#ifdef USE_VISUALIZER
#include "Visualizer.h"
#endif
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
#include <signal.h>
#include "vectormath.h"
#include <limits>

bool emergencystop=false;

void
termination_handler (int signum)
{
	cout << "sigint received " << endl;
	emergencystop=true;
}

int main(int argc, char *argv[]) {
	//For testing todo remove if no longer needed
	signal(SIGINT, termination_handler);
#ifdef TESTRUN
	ofstream trajectory("traj0.dat");
	ofstream trajectory1("traj1.dat");
#endif

	Parameters kernelHostParameters;
	HostParameters hostParameters;
#ifdef USE_VISUALIZER
	Visualizer *visualizer=nullptr;
#endif

	int frameNumber=0;

	//read the paramter file
	parseConfigurationFile(argv[1],hostParameters,kernelHostParameters);
	//todo consitency check

	string tapStateFileName=hostParameters.baseName + ".tap.gstat";
	cout << hostParameters.baseName << endl;
	ofstream tappSave(tapStateFileName);
	ofstream stateSave(hostParameters.baseName + ".tap.state");

#ifdef USE_VISUALIZER
	//create visualization objetc if needed
	if (hostParameters.visualization) {
		visualizer = new Visualizer(hostParameters.vboxX, hostParameters.vboxY);
	}
#endif

	AshwinBowlesSystem simulation(kernelHostParameters,hostParameters);
#ifdef USE_VISUALIZER
	if(hostParameters.visualization) {
		simulation.updateOffsetFreeData(hostParameters.vLineSize);
		visualizer->initializeSystem(
				simulation.getPrtToOffsetFreePositions(),
				kernelHostParameters.numberOfParticles*kernelHostParameters.number_of_systems,
				kernelHostParameters.radius,
				hostParameters.vLineSize,hostParameters.vLineSize,0,-hostParameters.vLineSize+kernelHostParameters.diameter,10);
				visualizer->updateimage();
	}
#endif

	// the main loop todo integrate it in simulation class if possible

	double time;
	int tapNumber=hostParameters.startSnapNumber; // counter which counts the performed taps
	int i=hostParameters.startTimeStep; //number of the timestep



	while(tapNumber < hostParameters.numberOfTaps && i < hostParameters.maxTimeSteps) {
		if(hostParameters.useOPENCL) simulation.enqueueTimeStep();
		else simulation.cpuTimestep();
		if(emergencystop) {
			if(hostParameters.useOPENCL) simulation.upDateHostMemory();
			simulation.saveState(hostParameters.baseName+".emerg.state",i,tapNumber);
			cout << "State file saved, program terminates now" << endl;
			exit(EXIT_SUCCESS);
		}

		if (i % hostParameters.tappingCheck == 0) {
			double Ekin, Epot; //kinetic an potential energy
			if(hostParameters.useOPENCL) simulation.upDateHostMemory();
			simulation.getEnergy(Ekin, Epot);
			if (Ekin < hostParameters.tapThreshold) {
				vector<bool> state;
				int exitedBonds[kernelHostParameters.number_of_systems];
				if (simulation.isJammed(exitedBonds, state)) {
					cout << "step: "<< i << " state is jammed (Ekin " << Ekin << "): ";

					tappSave << tapNumber << "\t";
					for (auto ebonds : exitedBonds) {
						tappSave.precision(20);
						tappSave << ebonds << "\t";
						cout << ebonds << " ";
					}
					tappSave << endl;
					cout << endl;
					++tapNumber;

					char fname[128]; //todo use c++ not c here
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
						simulation.velocityPulse(hostParameters.tappingAmplitude);
						break;

					case HAMMER:
						simulation.hammerPulse(hostParameters.tappingAmplitude);
						for(auto ta: hostParameters.tappingAmplitude)
							ta*=-1.0f;
						break;
					default:
						break;
					}
					stateSave << endl;
				} else {
					cout << "state is not jammed";
				}
			}
		}
#ifdef USE_VISUALIZER
		//do visualization
		if(i%hostParameters.visualizerIntervall==0 && hostParameters.visualization) {
			if(hostParameters.useOPENCL) {
				simulation.enqueOffestupdate();
				simulation.upDateHostMemory();
			} else {
				simulation.updateOffsetCpu();
			}
			simulation.updateOffsetFreeData(hostParameters.vLineSize);
			visualizer->updateimage();
			if(hostParameters.makeMovie) {
				char fname[125];
				sprintf(fname,"%s.frame%04d.png",(const char*)hostParameters.baseName.c_str(),frameNumber++);
				visualizer->snapshot(fname);
			}
		}
#endif
		if(i%hostParameters.snapshotIntervall==0) {
			//todo save system state
			double Ekin,Epot; //kinetic an potential energy
			if(hostParameters.useOPENCL)simulation.upDateHostMemory();
			simulation.getEnergy(Ekin,Epot);
			cout << "step: " << i << "Ekin " << Ekin << " Epot " << Epot << " Gesamt: " << Ekin+Epot << endl;
			char fname[125];
			sprintf(fname,"%s.snap.%05d.pdat",(const char*)hostParameters.baseName.c_str(),tapNumber);
			if(hostParameters.saveDatails)
				simulation.saveState(fname,i,tapNumber);
#ifdef TESTRUN
			float part[6];
			simulation.getParticle(0, part);
			trajectory << (i+1) * kernelHostParameters.timestep << " ";
			for (int i = 0; i < 6; ++i) {
				trajectory << part[i] << " ";
			}
			if (kernelHostParameters.numberOfParticles > 1) {
				trajectory << endl;
				simulation.getParticle(1, part);
				trajectory1 << (i+1) * kernelHostParameters.timestep << " ";
				for (int i = 0; i < 6; ++i) {
					trajectory1 << part[i] << " ";
				}
				trajectory1 << endl;
			}
#endif
		}
		if(i%hostParameters.offSetupdate==0) {
			if(hostParameters.useOPENCL) simulation.enqueOffestupdate();
			else simulation.updateOffsetCpu();
		}
		if( (i+1) == std::numeric_limits<int>::max() ) {
			cout << "Warning: time step counter exceeds numeric limits, reset to zero" << endl;
			i=0;
		} else {
			++i;
		}
}
#ifdef USE_VISUALIZER
	if(hostParameters.visualization) visualizer->close();
	if(visualizer != nullptr) delete visualizer;
#endif
	tappSave.close();
	return EXIT_SUCCESS;
}
