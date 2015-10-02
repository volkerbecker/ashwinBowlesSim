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
#include <vector>
#include <cmath>
#include "vectormath.h"

#define OPENCL_PROGRAM_NAME "asbkernels.cl"

AshwinBowlesSystem::AshwinBowlesSystem() {
	asbTime=0;
	particles=nullptr;
}


AshwinBowlesSystem::AshwinBowlesSystem(const Parameters &parameters,
		const HostParameters & hostparameters) {


	setup(parameters, hostparameters);
	asbTime=0;
}

AshwinBowlesSystem::~AshwinBowlesSystem() {
	if(particles !=nullptr) delete particles;
	//delete walls;
}

void AshwinBowlesSystem::setup(const Parameters& parameters,
		const HostParameters & hostParameters) {
	this->parameter = parameters;

	//Create the openCl Stuff
	initializeOpenCL(hostParameters);
	//initialize particel System
	particles = new ParticleSystem(context, queue, parameters,hostParameters);
	particles->createDensestState(parameters.upperWall - parameters.lowerWall,
			parameters.rightWall + parameters.rightWallOffset);
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
	timeBuffer = cl::Buffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(asbTime), &asbTime);
	/// extract kernels;
	try {
		verletStep1Kernel = cl::Kernel(program, "verletStep1");
		updateOffsetsKernel = cl::Kernel(program, "updateOffset");
		calculateAccelarationKernel1 = cl::Kernel(program,
				"calculateAccelarationOnestep");

	} catch (cl::Error &error) {
		std::cerr << error.what() << "(" << error.err() << ")" << std::endl;
		exit(EXIT_FAILURE);
	}
	///set Kernel arguments
	try {
		verletStep1Kernel.setArg(0, particles->getPositionBuffer());
		verletStep1Kernel.setArg(1, particles->getVelocityBuffer());
		verletStep1Kernel.setArg(2, particles->getAccelerationBuffer());
		//		verletStep1Kernel.setArg(3,particles->getOldAccelerationBuffer());
		verletStep1Kernel.setArg(3, parameterBuffer);

		calculateAccelarationKernel1.setArg(0, particles->getOffsetBuffer());
		calculateAccelarationKernel1.setArg(1, particles->getPositionBuffer());
		calculateAccelarationKernel1.setArg(2, particles->getVelocityBuffer());
		calculateAccelarationKernel1.setArg(3,
				particles->getAccelerationBuffer());
		calculateAccelarationKernel1.setArg(4, timeBuffer);
		calculateAccelarationKernel1.setArg(5, parameterBuffer);
		calculateAccelarationKernel1.setArg(6, (cl_int) 0);

		updateOffsetsKernel.setArg(0, particles->getOffsetBuffer());
		updateOffsetsKernel.setArg(1, particles->getPositionBuffer());
	} catch (cl::Error &error) {
		std::cerr << error.what() << "(" << error.err() << ")" << std::endl;
		exit(EXIT_FAILURE);
	}
}


void AshwinBowlesSystem::initializeOpenCL(const HostParameters &hostParameters) {
	//Create the openCl Stuff
	try {
		//query plattforms
		cl::Platform::get(&plattforms);

		cout << "Available OpenCL Platforms: \n";
		for (int i = 0; i < plattforms.size(); ++i) {
			std::cout << i << ".)" << std::endl;
			std::cout << "\t" << plattforms[i].getInfo<CL_PLATFORM_VENDOR>()
					<< std::endl;
			std::cout << "\t" << plattforms[i].getInfo<CL_PLATFORM_NAME>()
					<< std::endl;
		}

		cout << "Platform "  << hostParameters.targetPlatform <<" was chosen" << std::endl;
		///todo platform selection via inputfile

		//query gpu-devices
		plattforms[hostParameters.targetPlatform].getDevices(CL_DEVICE_TYPE_ALL, &devices);
		/// todo decive sxelcetion via inpuit file
		std::cout << "Availible Devices:" << std::endl;
		for(int i=0;i<devices.size();++i) {
			std::cout << "device " << i << ": " << devices[i].getInfo<CL_DEVICE_NAME>() << std::endl;
		}
		cout << "\tDevice " << hostParameters.targetDevice << " was chosen." << endl;

		// create a context
		context = cl::Context(devices);

		// create a command queue
		queue = cl::CommandQueue(context, devices[hostParameters.targetDevice]);
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

bool AshwinBowlesSystem::isJammed(
			int &exitedBonds, ///< \return number of exited bonds
			std::vector<bool> &stateVector ///< \return the state vector
	) {
	stateVector.clear();
	stateVector.resize(particles->size());
	exitedBonds=0;
	bool isJammed=true;
	const vector<cl_float2> & pos = particles->getPositions();
	for(int i=1;i<pos.size();++i) {
		const double &y1=pos[i].s[1];
		const double &y2=pos[i-1].s[1];
		if( parameter.upperWall-y1 > parameter.radius*(1+0.01)
				&& y1-parameter.lowerWall > parameter.radius*(1+0.01)) {
			isJammed=false;
			break;
		}
		if(fabs(y2-y1)>0.05*parameter.radius) {
			stateVector[i-1]=false;
		} else {
			stateVector[i-1]=true;
			exitedBonds++;
			if(i>1) {
				if(stateVector[i-2]) {
					isJammed=false;
					break;
				}
			}
		}
	}
	return isJammed;
}


void AshwinBowlesSystem::saveState(const string &filename,const int &timeStep,const int &number) {
	ofstream saveFile(filename);
	saveFile << timeStep << "\t" << number << endl;
	saveFile.precision(20);
	saveFile << *particles;
	saveFile.close();
}


void AshwinBowlesSystem::getEnergy(double &Ekin, double &Epot) {
		//calculate kinetic Energy
		Ekin = 0;
		Epot = 0;
		for (int i = 0; i < particles->size(); ++i) {
			double velsq = (particles->getVelocities()[i].s[0])*(particles->getVelocities()[i].s[0])+
					(particles->getVelocities()[i].s[1])*(particles->getVelocities()[i].s[1]);
			Ekin += velsq * 0.5 * particles->mass();
			double posx1 = particles->getPosition(i).s[0];
			double posy1 = particles->getPosition(i).s[1];
			if (i < particles->size() - 1) {
				double posx2 = particles->getPosition(i + 1).s[0];
				double posy2 = particles->getPosition(i + 1).s[1];
				double overlapp = 2 * particles->radius()
						- sqrt(
								(posx1 - posx2) * (posx1 - posx2)
								+ (posy1 - posy2) * (posy1 - posy2));
				if (overlapp > 0) {
					Epot += parameter.springConstant * overlapp * overlapp*0.5;
				}
			}
			double wallOverlapp=particles->radius()-fabs(posy1-parameter.upperWall);
			Epot+= wallOverlapp >0 ? wallOverlapp*wallOverlapp*parameter.springConstant*0.5 : 0;
			wallOverlapp=particles->radius()-fabs(posy1-parameter.lowerWall);
			Epot+= wallOverlapp >0 ? wallOverlapp*wallOverlapp*parameter.springConstant*0.5 : 0;
			if(i==0) {
				wallOverlapp=particles->radius()-fabs(posx1-parameter.leftWall-parameter.leftWallofset);
				Epot+= wallOverlapp >0 ? wallOverlapp*wallOverlapp*parameter.springConstant*0.5 : 0;
			}
			if(i==particles->size()-1) {
				wallOverlapp=particles->radius()-fabs(posx1-parameter.rightWall-parameter.rightWallOffset);
				Epot+= wallOverlapp >0 ? wallOverlapp*wallOverlapp*parameter.springConstant*0.5 : 0;
			}

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

void AshwinBowlesSystem::cpuTimestep() {
	vector<cl_int> &posOffset = particles->getOffset();
	vector<cl_float2> &position = particles->getPositions();
	vector<cl_float2> &velocity = particles->getVelocities();
	vector<cl_float2> &acceleration = particles->getAccelerations();
	Parameters *paras = &(this->parameter);

	for (int id = 0; id < particles->size(); ++id) {
		velocity[id] += 0.5f * acceleration[id] * paras->timestep;
		position[id] += velocity[id] * paras->timestep;
	}



	for (int id = 0; id < particles->size(); ++id) {
		cl_float2 acc;
		acc.s[0]=0;
		acc.s[1]=0;
		//caclulate accelaration due to the nearest right neighbor
		//if ((id + 1) < paras->numberOfParticles) {
			acc += ((id + 1) < paras->numberOfParticles) ?
					calcAcceleration(&posOffset[id], &posOffset[id + 1],
					&position[id], &position[id + 1], &velocity[id],
					&velocity[id + 1], paras) : (cl_float2){0,0};
		//}
		//calculate acceleration due to the nearest left neighbor
		//if ((id) > 0) {
			acc += ((id) > 0) ?
					calcAcceleration(&posOffset[id], &posOffset[id - 1],
					&position[id], &position[id - 1], &velocity[id],
					&velocity[id - 1], paras) : (cl_float2){0,0};
		//}
	//
		//horizontalWalls
		//oben
		{
			float overlapp = paras->radius - (paras->upperWall - position[id].s[1]);
			float force = overlapp>0 ?
						fmax( paras->springConstant * overlapp
							+ paras->damping * velocity[id].s[1], 0) : 0;
			acc.s[1] -= force * paras->inverseMass;
		}
	//	//unten
		{
			float overlapp = paras->radius + (paras->lowerWall - position[id].s[1]);
				float force = overlapp >0 ? fmax(
					paras->springConstant * overlapp
					- paras->damping * velocity[id].s[1], 0) : 0;
				acc.s[1] += force * paras->inverseMass;
		}
		//calculate wall forces
		if (id == 0 || id == (paras->numberOfParticles - 1)) {
			int i = (id == 0 ? 0 : 1);
			int wallOffset = (
					id == 0 ? paras->leftWallofset : paras->rightWallOffset);
			float verticalWall = (id == 0 ? paras->leftWall : paras->rightWall);
			float overlapp = (float) (posOffset[id] - wallOffset)
					+ (position[id].s[0] - verticalWall);
			if (fabs(overlapp) < paras->radius) {
				overlapp += (i == 0 ? -paras->radius : paras->radius);
				overlapp *= -1;
				//printf("%i %i %f \n",id,i,overlapp);
				float force = (
						i == 0 ?
								fmax(
										paras->springConstant * overlapp
												- paras->damping * velocity[id].s[0],
										0) :
								fmin(
										paras->springConstant * overlapp
												- paras->damping * velocity[id].s[0],
										0));
				acc.s[0] += force * paras->inverseMass;
			}
		}
		if (id == 0) {
			acc.s[0] += paras->stampAcceleration;
		}
			acc-=paras->viskosity*velocity[id]*paras->inverseMass;
			velocity[id] += 0.5f * paras->timestep * acc;
			acceleration[id] = acc;
		if(id==0) asbTime +=paras->timestep;
	}
}

void AshwinBowlesSystem::updateOffsetCpu() {
	vector<cl_int> &posOffsets = particles->getOffset();
	vector<cl_float2> &positions = particles->getPositions();
	for (int id = 0; id < particles->size(); ++id) {
		int truncPos = (int) positions[id].s[0];
		posOffsets[id] += truncPos;
		positions[id].s[0] -= truncPos;
	}
}

