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
#include <fstream>

#define OPENCL_PROGRAM_NAME "asbkernels.cl"

AshwinBowlesSystem::AshwinBowlesSystem(const Parameters &parameters,
		const float &initialdistance,
		const float &initialLength,
		const float &initialHight) {
	//Create the openCl Stuff
	initializeOpenCL();

	//initialize particel System
	particles = new ParticleSystem(context,queue,parameters.numberOfParticles,
			parameters.radius,parameters.mass,initialdistance);
	//initialize walls
	walls = new Walls(context, queue, initialHight * 0.5f, -initialHight * 0.5f,
			0, initialLength, parameters.wallstampforce,parameters.mass);
	//compile the opencl program
	cl::Program program = loadCLSource(OPENCL_PROGRAM_NAME, context);

	try {
		program.build(devices);
	} catch (cl::Error &error) {
		std::cerr << error.what() << "(" << error.err() << ")" << std::endl;
		std::cerr << "Build Log:\t "
				<< program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0])
				<< std::endl;
		exit(-1);
	}
	std::cout << "Build Log:\t "
			<< program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0])
			<< std::endl;
}

AshwinBowlesSystem::~AshwinBowlesSystem() {
	delete particles;
	delete walls;
}

void AshwinBowlesSystem::initializeOpenCL() {
	//Create the openCl Stuff
	try {
		//query plattforms
		cl::Platform::get(&plattforms);

		cout << "Available OpenCL Platforms: \n";
		for (int i = 0; i < plattforms.size(); ++i) {
			char buffer[256];
			std::cout << i << ".)" << std::endl;
			std::cout << "\t" << plattforms[i].getInfo<CL_PLATFORM_VENDOR>()
					<< std::endl;
			std::cout << "\t" << plattforms[i].getInfo<CL_PLATFORM_NAME>()
					<< std::endl;
		}

		cout << "Platform " << 0 << " was chosen" << std::endl;
		///todo platform selection via inputfile

		//query gpu-devices
		plattforms[0].getDevices(CL_DEVICE_TYPE_GPU, &devices);
		/// todo decive sxelcetion via inpuit file

		// create a context
		context = cl::Context(devices);

		// create a command queue
		queue = cl::CommandQueue(context, devices[0]);
	} catch (cl::Error &error) {
		std::cerr << error.what() << "(" << error.err() << ")" << std::endl;
		exit(EXIT_FAILURE);
	}
}

cl::Program AshwinBowlesSystem::loadCLSource(const char *filename, const cl::Context &context) {
	try {
		std::ifstream sourceFile(filename); //open file
		std::string sourceCode((std::istreambuf_iterator<char>(sourceFile)),
				std::istreambuf_iterator<char>()); //copy file to c_string

		cout << *filename << " " << sourceCode ;
		cl::Program::Sources source(1,
				std::make_pair(sourceCode.c_str(), sourceCode.length() + 1));
		cl::Program program = cl::Program(context, source);
		return program;
	} catch (std::ifstream::failure &e) {
		std::cerr << "Exception opening/reading/closing file\n";
		exit(0);
	}
}
