/*
 * AshwinBowlesSystem.cpp
 *
 *  Created on: Sep 15, 2015
 *      Author: becker
 */

#include "AshwinBowlesSystem.h"
#include <vector>
#include <iostream>
#include "Paramters.h"


AshwinBowlesSystem::AshwinBowlesSystem(const Parameters &parameters,
		const float &initialdistance,
		const float &initialLength,
		const float &initialHight) {
	//Create the openCl Stuff
	try{
		//query plattforms
		cl::Platform::get(&plattforms);

		//query gpu-devices
		plattforms[0].getDevices(CL_DEVICE_TYPE_GPU, &devices);

		// create a context
		context=cl::Context(devices);

		// create a command queue
		queue = cl::CommandQueue(context, devices[0]);
	} catch (cl::Error &error) {
		std::cerr << error.what() << "(" << error.err() << ")" << std::endl;
		exit(EXIT_FAILURE);
	}

	//initialize particel System
	particles = new ParticleSystem(context,queue,parameters.numberOfParticles,
			parameters.radius,parameters.mass,initialdistance);
	walls = new Walls(context, queue, initialHight * 0.5f, -initialHight * 0.5f,
			0, initialLength, parameters.wallstampforce,parameters.mass);
}

AshwinBowlesSystem::~AshwinBowlesSystem() {
	delete particles;
	delete walls;
}


