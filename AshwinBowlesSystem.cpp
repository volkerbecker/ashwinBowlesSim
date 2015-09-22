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
		const float &initialdistance, const float &initialLength,
		const float &initialHight) {


	this->parameter = parameters;
	//Create the openCl Stuff
	initializeOpenCL();

	//initialize particel System
	particles = new ParticleSystem(context, queue, parameters.numberOfParticles,
			parameters.radius, parameters.mass, initialdistance);

	particles->createDensestState(parameters.upperWall-
			parameters.lowerWall,parameters.rightWall+parameters.rightWallOffset);

	//compile the opencl program
	cl::Program program = loadCLSource(OPENCL_PROGRAM_NAME, context);

	globalp = cl::NDRange(parameter.numberOfParticles);
	localp = cl::NullRange; //todo optmiale wgsize finden
	//globalAcc=cl::NDRange((int) parameter.numberOfParticles / 2);

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
	/// initalize paramter Buffer


	parameterBuffer = cl::Buffer(context,
	CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(parameter),
			&this->parameter);

	timeBuffer = cl::Buffer(context,CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,sizeof(asbTime),&asbTime);

	/// extract kernels;
	try {
		verletStep1Kernel = cl::Kernel(program, "verletStep1");
//		calculateAccelarationKernel1 = cl::Kernel(program,
//				"calculateAccelaration");
//		calculateAccelarationKernel2 = cl::Kernel(program,
//				"calculateAccelaration");
//		verletStep2Kernel = cl::Kernel(program, "verletStep2");
		updateOffsetsKernel = cl::Kernel(program, "updateOffset");
		calculateAccelarationKernel1= cl::Kernel(program,"calculateAccelarationOnestep");

	} catch (cl::Error &error) {
		std::cerr << error.what() << "(" << error.err() << ")" << std::endl;
		exit(EXIT_FAILURE);
	}

	///set Kernel arguments

	try {
		verletStep1Kernel.setArg(0,particles->getPositionBuffer());
		verletStep1Kernel.setArg(1,particles->getVelocityBuffer());
		verletStep1Kernel.setArg(2,particles->getAccelerationBuffer());
//		verletStep1Kernel.setArg(3,particles->getOldAccelerationBuffer());
		verletStep1Kernel.setArg(3, parameterBuffer);

		calculateAccelarationKernel1.setArg(0,particles->getOffsetBuffer());
		calculateAccelarationKernel1.setArg(1,particles->getPositionBuffer());
		calculateAccelarationKernel1.setArg(2,particles->getVelocityBuffer());
		calculateAccelarationKernel1.setArg(3,particles->getAccelerationBuffer());
		calculateAccelarationKernel1.setArg(4,timeBuffer);
		calculateAccelarationKernel1.setArg(5,parameterBuffer);
		calculateAccelarationKernel1.setArg(6, (cl_int) 0);

//		calculateAccelarationKernel2.setArg(0,particles->getOffsetBuffer());
//		calculateAccelarationKernel2.setArg(1,particles->getPositionBuffer());
//		calculateAccelarationKernel2.setArg(2,particles->getVelocityBuffer());
//		calculateAccelarationKernel2.setArg(3,particles->getAccelerationBuffer());
//		calculateAccelarationKernel2.setArg(4,parameterBuffer);
//		calculateAccelarationKernel2.setArg(5, (cl_int) 1);

//		verletStep2Kernel.setArg(0,particles->getOffsetBuffer());
//		verletStep2Kernel.setArg(1,particles->getPositionBuffer());
//		verletStep2Kernel.setArg(2,particles->getVelocityBuffer());
//		verletStep2Kernel.setArg(3,particles->getAccelerationBuffer());
//		verletStep2Kernel.setArg(4,particles->getOldAccelerationBuffer());
//		verletStep2Kernel.setArg(5,parameterBuffer);

		updateOffsetsKernel.setArg(0,particles->getOffsetBuffer());
		updateOffsetsKernel.setArg(1,particles->getPositionBuffer());
	} catch (cl::Error &error) {
		std::cerr << error.what() << "(" << error.err() << ")" << std::endl;
		exit(EXIT_FAILURE);
	}
}

AshwinBowlesSystem::~AshwinBowlesSystem() {
	delete particles;
	//delete walls;
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

void AshwinBowlesSystem::enqueueTimeStep() {
	try{
		cl::NDRange globaldouble(particles->size()*2);
		queue.enqueueNDRangeKernel(verletStep1Kernel,cl::NullRange,globalp,localp);
		queue.enqueueNDRangeKernel(calculateAccelarationKernel1,cl::NullRange,globalp,localp);
//		queue.enqueueNDRangeKernel(verletStep2Kernel,cl::NullRange,globalp,localp);
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
		cl::Program::Sources source(1,
				std::make_pair(sourceCode.c_str(), sourceCode.length()));
		cl::Program program = cl::Program(context, source);
		return program;
	} catch (std::ifstream::failure &e) {
		std::cerr << "Exception opening/reading/closing file\n";
		exit(EXIT_FAILURE);
	}
}
