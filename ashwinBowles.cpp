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

int main(void) {
	puts("Hello World!!!");
	std::vector<cl::Platform> plattforms;
	cl::Platform::get(&plattforms);
	//query plattforms

	std::vector<cl::Device> devices;
	plattforms[0].getDevices(CL_DEVICE_TYPE_GPU, &devices);
	//query gpu-devices

	cl::Context context(devices);
	// create a context

	// create a command queue
	cl::CommandQueue queue = cl::CommandQueue(context, devices[0]);

	ParticleSystem particles(context,queue,20);

	Visualizer visualizer(500,500);
	visualizer.initializeSystem(particles.getOffsetFreePositionPointer(),20,
			0.5,25,25,0,-12.5,15);

	for(int i=0;i<particles.size();++i) {
		std::cout << particles.getPosition(i).s[0] << " " << particles.getPosition(i).s[1] << std::endl;
	}
	std::cout << particles << std::endl;
	particles.updateOffsetfreePositions();

	visualizer.updateimage();

	sleep(10);

	visualizer.close();



	return EXIT_SUCCESS;
}
