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
#include <fstream>

int main(void) {
	ofstream energyStream("energy.dat");

	Parameters parameters;

	parameters.numberOfParticles=102400;
	parameters.mass=1;
	parameters.radius=0.5;
	parameters.diameter=parameters.radius*2;
	parameters.wallstampforce=1;
	parameters.timestep=0.0005;
	parameters.timestepSq=parameters.timestep*parameters.timestep;
	parameters.springConstant=10000;
	parameters.damping=0;
	parameters.inverseMass=1/parameters.mass;
	parameters.leftWall=0;
	parameters.rightWall=0;
	parameters.upperWall=0.75;
	parameters.lowerWall=-0.75;
	parameters.leftWallofset=0;
	parameters.rightWallOffset=parameters.numberOfParticles*(1+0.001)+1;
	parameters.stampAcceleration=0*parameters.inverseMass;
	parameters.jamming=false;
	parameters.viskosity=0.00;
	parameters.tappingAmplitude=0;

	puts("Hello World!!!");

	double wallDistance = parameters.upperWall - parameters.lowerWall;
	cl_float & radius = parameters.radius;
	double phi = acos(wallDistance / 2 / radius - 1);
	double dx = sin(phi) * 2 * radius;
	double length = parameters.numberOfParticles * 0.5 * dx
			+ (parameters.numberOfParticles * 0.5 - 1) * parameters.diameter
			+ 2 * radius;
	double leftwallPosition = (double) parameters.rightWallOffset
			+ (double) parameters.rightWall - length;
	parameters.leftWallofset = (int) leftwallPosition;
	cout << "left Wall:" << leftwallPosition << endl;
	parameters.leftWall = leftwallPosition - parameters.leftWallofset;


	Visualizer visualizer(800,800);
	AshwinBowlesSystem simulation(parameters,0.001,150,1.5);
	//simulation.upDateHostMemory();

	simulation.updateOffsetFreeData(150);
	cout << (*(simulation.getPrtToOffsetFreePositions())) << endl;;


	visualizer.initializeSystem(
			simulation.getPrtToOffsetFreePositions(),
			parameters.numberOfParticles,
			parameters.radius,
			150,150,0,-150+parameters.diameter,10);
	visualizer.updateimage();
	std:this_thread::sleep_for(std::chrono::seconds(5));

	double Ekin,Epot;
	for(int i=0;i<4000000;++i) {
		simulation.enqueueTimeStep();
		if(i%2000==0) {
			simulation.enqueOffestupdate();
			simulation.updateOffsetFreeData(150);
			visualizer.updateimage();
			simulation.getEnergy(Ekin,Epot);
			cout << "step: " << i << "Ekin " << Ekin << " Epot " << Epot << " Gesamt: " << Ekin+Epot << endl;
			energyStream << i << "\t" << Ekin << "\t" << Epot << "\t" << Ekin+Epot << "\n";
		//	std:this_thread::sleep_for(std::chrono::milliseconds(50));
		}
	}
	visualizer.close();


	return EXIT_SUCCESS;
}
