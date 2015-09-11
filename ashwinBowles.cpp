//============================================================================
// Name        : ashwinBowles.cpp
// Author      : Volker Becker
// Version     :
// Copyright   : (c) 2015 by Volker Becker
// Description : Hello World in C, Ansi-style
//============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.hpp>
#include "ParticleSystem.h"

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

	ParticleSystem particles(context,20);



	return EXIT_SUCCESS;
}
