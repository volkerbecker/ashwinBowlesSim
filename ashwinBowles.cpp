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

int main(void) {

	Parameters parameters;

	parameters.numberOfParticles=256*100;
	parameters.mass=1;
	parameters.radius=0.5;
	parameters.wallstampforce=1;

	puts("Hello World!!!");


	AshwinBowlesSystem simulation(parameters,0.1,256.2*100,5);

	for(int i=0;i<10000000;++i) {
		simulation.enqueueTimeStep();
	}


	return EXIT_SUCCESS;
}
