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


	parseConfigurationFile("asgconf.icf",hostParameters,kernelHostParameters);
	exit(0);

	kernelHostParameters.numberOfParticles=10240;
	kernelHostParameters.mass=1;
	kernelHostParameters.radius=0.5;
	kernelHostParameters.diameter=kernelHostParameters.radius*2;
	kernelHostParameters.wallstampforce=1;
	kernelHostParameters.timestep=0.001;
	kernelHostParameters.timestepSq=kernelHostParameters.timestep*kernelHostParameters.timestep;
	kernelHostParameters.springConstant=4000;
	kernelHostParameters.damping=kernelHostParameters.springConstant*0.001;
	kernelHostParameters.inverseMass=1/kernelHostParameters.mass;
	kernelHostParameters.leftWall=0;
	kernelHostParameters.rightWall=0;
	kernelHostParameters.upperWall=0.75;
	kernelHostParameters.lowerWall=-0.75;
	kernelHostParameters.leftWallofset=0;
	kernelHostParameters.rightWallOffset=kernelHostParameters.numberOfParticles*(1+0.001)+1;
	kernelHostParameters.stampAcceleration=5*kernelHostParameters.inverseMass;
	kernelHostParameters.jamming=true;
	kernelHostParameters.viskosity=0.01;
	kernelHostParameters.tappingAmplitude=1;

	puts("Hello World!!!");

	double wallDistance = kernelHostParameters.upperWall - kernelHostParameters.lowerWall;
	cl_float & radius = kernelHostParameters.radius;
	double phi = acos(wallDistance / 2 / radius - 1);
	double dx = sin(phi) * 2 * radius;
	double length = kernelHostParameters.numberOfParticles * 0.5 * dx
			+ (kernelHostParameters.numberOfParticles * 0.5 - 1) * kernelHostParameters.diameter
			+ 2 * radius;
	double leftwallPosition = (double) kernelHostParameters.rightWallOffset
			+ (double) kernelHostParameters.rightWall - length;
	kernelHostParameters.leftWallofset = (int) leftwallPosition;
	cout << "left Wall:" << leftwallPosition << endl;
	kernelHostParameters.leftWall = leftwallPosition - kernelHostParameters.leftWallofset;


	Visualizer visualizer(800,800);
	AshwinBowlesSystem simulation(kernelHostParameters,0.001,150,1.5);
	//simulation.upDateHostMemory();

	simulation.updateOffsetFreeData(150);
	cout << (*(simulation.getPrtToOffsetFreePositions())) << endl;;


	visualizer.initializeSystem(
			simulation.getPrtToOffsetFreePositions(),
			kernelHostParameters.numberOfParticles,
			kernelHostParameters.radius,
			150,150,0,-150+kernelHostParameters.diameter,10);
	visualizer.updateimage();
	std:this_thread::sleep_for(std::chrono::seconds(5));

	double Ekin,Epot;
	for(int i=0;i<40000000;++i) {
		simulation.enqueueTimeStep();
		if(i%10000==0) {
			simulation.enqueOffestupdate();
			simulation.updateOffsetFreeData(150);
			visualizer.updateimage();
			simulation.getEnergy(Ekin,Epot);
			cout << "step: " << i << "Ekin " << Ekin << " Epot " << Epot << " Gesamt: " << Ekin+Epot << endl;
			energyStream << i << "\t" << Ekin << "\t" << Epot << "\t" << Ekin+Epot << "\n";
		//	std:this_thread::sleep_for(std::chrono::milliseconds(50));
			if(Ekin<1e-3) {
					simulation.velocityPulse((cl_float2){0,10});
						}
			}
		}
	visualizer.close();


	return EXIT_SUCCESS;
}
