//============================================================================
// Name        : ashwinBowles.cpp
// Author      : Volker Becker
// Version     :
// Copyright   : (c) 2015 by Volker Becker
// Description : Hello World in C, Ansi-style
//============================================================================

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

int main(void) {

	Parameters parameters;

	parameters.numberOfParticles=4000;
	parameters.mass=10;
	parameters.radius=0.5;
	parameters.diameter=parameters.radius*2;
	parameters.wallstampforce=1;
	parameters.timestep=0.001;
	parameters.timestepSq=parameters.timestep*parameters.timestep;
	parameters.springConstant=1000;
	parameters.damping=2;
	parameters.inverseMass=1/parameters.mass;
	parameters.leftWall=0;
	parameters.rightWall=0;
	parameters.upperWall=0.75;
	parameters.lowerWall=-0.75;
	parameters.leftWallofset=0;
	parameters.rightWallOffset=10000*0.001+10000+1;
	parameters.stampAcceleration=5*parameters.inverseMass;

	puts("Hello World!!!");


	Visualizer visualizer(800,800);
	AshwinBowlesSystem simulation(parameters,0.001,150,1.5);
	//simulation.upDateHostMemory();

	simulation.updateOffsetFreeData();


	visualizer.initializeSystem(
			simulation.getPrtToOffsetFreePositions(),
			parameters.numberOfParticles,
			parameters.radius,
			120,120,500,-60,10);
	visualizer.updateimage();


	for(int i=0;i<5000000;++i) {
		simulation.enqueueTimeStep();
		if(i%1000==0) {
			simulation.enqueOffestupdate();
			simulation.updateOffsetFreeData();
			visualizer.updateimage();
//			std:this_thread::sleep_for(std::chrono::milliseconds(5));
		}
	}
	visualizer.close();


	return EXIT_SUCCESS;
}
