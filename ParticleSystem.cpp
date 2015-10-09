/*
 * ParticleSystem.cpp
 *
 *  Created on: Sep 10, 2015
 *      Author: becker
 */

#include "ParticleSystem.h"
#include <cmath>
#include <iostream>
#include <CL/cl.hpp>
#include <vector>
#include <fstream>



ParticleSystem::ParticleSystem(const cl::Context & clContext, ///< openCL context to connect particle data to gpu
		const cl::CommandQueue &_queue,
		const Parameters &paramters,
		const HostParameters & hostParameters
	) : context(clContext),queue(_queue){
	this->_size=paramters.numberOfParticles;
	this->_radius=paramters.radius;
	this->_mass=paramters.mass;
	this->rightborder=paramters.rightWall+paramters.rightWallOffset;
	number_of_systems()=paramters.number_of_systems;


	// initialize particles
	position=vector<cl_float2>(size()*number_of_systems(),(cl_float2){0,0});
	velocity=vector<cl_float2>(size()*number_of_systems(),(cl_float2){0,0});
	acceleration=vector<cl_float2>(size()*number_of_systems(),(cl_float2){0,0});
	offset=vector<cl_int>(size()*number_of_systems(),0);
	offsetfreepositions=vector<cl_float2>(size()*number_of_systems(),(cl_float2){0,0});

	switch (hostParameters.initialConfig) {
	case PCHAIN:
		createParticleString(0);
		break;
	case PDENSEST:
		createDensestState(paramters.upperWall - paramters.lowerWall,
					(double) paramters.rightWall
						+ (double) paramters.rightWallOffset);
		break;
	case PLOOSEST:
		createLoosetState(paramters.upperWall - paramters.lowerWall,
							(double) paramters.rightWall
								+ (double) paramters.rightWallOffset);
		break;
	case PFILE:
		loadParticleState(hostParameters);
		break;
	default:
		cerr << "No valid initial configuration was choosen \n";
		exit(EXIT_FAILURE);
	}

	// initialize offsert free buffer


	// create opencl Buffers
	offsetBuffer = cl::Buffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(cl_int) * offset.size(),offset.data());
	positionBuffer = cl::Buffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(cl_float2) * position.size(),position.data());
	velocityBuffer = cl::Buffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(cl_float2) * velocity.size(), velocity.data());
	accelerationBuffer = cl::Buffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(cl_float2) * acceleration.size(),acceleration.data());
}


ParticleSystem::~ParticleSystem() {
	// TODO Auto-generated destructor stub
}


void ParticleSystem::createDensestState(
		const double & wallDistance, ///< the distance of the vertical walls
		const double & rightWall) {///< the right wall

	double phi=acos(wallDistance/2/radius()-1);
	double dx=sin(phi)*2*radius();

	for (int sysnum = 0; sysnum < number_of_systems(); ++sysnum) {
		double tmpPosition=rightWall-radius();
		double yposition=wallDistance/2-radius();
		int sign=1;
		for (int i = size() - 1; i >= 0; i--) {
			int index=sysnum*size()+i;
			offset[index] = (int) tmpPosition;
			position[index].s[0] = (float) (tmpPosition - offset[index]);
			position[index].s[1] = (float) (sign * yposition);
			sign *= -1;
			tmpPosition -= dx;
		}
	}
}

void ParticleSystem::createLoosetState(
		const double & wallDistance, ///< the distance of the vertical walls
		const double & rightWall) {///< the right wall

	double phi=acos(wallDistance/2/radius()-1);
	double dx=sin(phi)*2*radius();

	for (int sysnum = 0; sysnum < number_of_systems(); ++sysnum) {
		double tmpPosition=rightWall-radius();
		double yposition=wallDistance/2-radius();
		int sign=1;
		for (int i = size() - 1; i >= 1; i -= 2) {
			int index=sysnum*size()+i;
			offset[index] = (int) tmpPosition;
			position[index].s[0] = (float) (tmpPosition - offset[index]);
			position[index].s[1] = (float) (sign * yposition);
			tmpPosition -= radius() * 2;
			offset[index - 1] = (int) tmpPosition;
			position[index - 1].s[0] = (float) (tmpPosition - offset[index - 1]);
			position[index - 1].s[1] = (float) (sign * yposition);
			sign *= -1;
			tmpPosition -= dx;
		}
	}
}


void ParticleSystem::createParticleString(const float & initialDistance) {

	//resize the vectors



	float deltaX = 2 * radius() + initialDistance;
	for (int sysnum = 0; sysnum < number_of_systems(); ++sysnum) {
		int tmpOffset = 0;
		float tmpPositionx = 0 + radius();
		for (uint i = 0; i < size(); ++i) {
			int index = sysnum * size() + i;
			offset[index] = tmpOffset;
			position[index].s[0] = tmpPositionx;
			position[index].s[1] = 0 * (drand48() * 2 - 2);  // y_i=0
			tmpPositionx += deltaX;
			tmpOffset += trunc(tmpPositionx);
			tmpPositionx -= trunc(tmpPositionx);
		}
	}
}

void ParticleSystem::getParticleDataFromDevice() {
	queue.enqueueReadBuffer(offsetBuffer, CL_TRUE, 0,
			sizeof(cl_int) * offset.size(), offset.data());
	queue.enqueueReadBuffer(positionBuffer, CL_TRUE, 0,
			sizeof(cl_float2) * position.size(), position.data());
	queue.enqueueReadBuffer(velocityBuffer, CL_TRUE, 0,
			sizeof(cl_float2) * velocity.size(), velocity.data());
	queue.enqueueReadBuffer(accelerationBuffer, CL_TRUE, 0,
			sizeof(cl_float2) * acceleration.size(), acceleration.data());
}

void ParticleSystem::updateOffsetfreePositions(const int &columnlength) {
	//getParticleDataFromDevice();
	for (int sysnum = 0; sysnum < number_of_systems(); sysnum++) {
		for (int i = 0; i < size(); ++i) {
			int index=sysnum*size()+i;
			cl_double2 tmp = getPosition(index, columnlength,sysnum);
			offsetfreepositions[index].s[0] = (float) tmp.s[0];
			offsetfreepositions[index].s[1] = (float) tmp.s[1];
		}
	}
}

void ParticleSystem::setRandomVelocity(const vector<cl_float2> &Amplitude) {
	for (int sysnum = 0; sysnum < number_of_systems(); sysnum++) {
		for (uint i = 0; i < size(); ++i) {
			int index=sysnum*size()+i;
			velocity[index].s[0] = Amplitude[sysnum].s[0] * (drand48() * 2 - 1);
			velocity[index].s[1] = Amplitude[sysnum].s[1] * (drand48() * 2 - 1);
		}
		queue.enqueueWriteBuffer(velocityBuffer, CL_FALSE, 0,
				velocity.size() * sizeof(cl_float2), velocity.data());
		;
	}
	//exit(0);
}

void ParticleSystem::setVelocity(const vector<cl_float2> &Amplitude) {
	for (int sysnum = 0; sysnum < number_of_systems(); sysnum++) {
		for (uint i = 0; i < size(); ++i) {
			int index = sysnum * size() + i;
			velocity[index].s[0] = Amplitude[sysnum].s[0];
			velocity[index].s[1] = Amplitude[sysnum].s[1];
		}
	}
	queue.enqueueWriteBuffer(velocityBuffer, CL_FALSE, 0,
			velocity.size() * sizeof(cl_float2), velocity.data());
	;
}

void ParticleSystem::loadParticleState(const HostParameters & hostparameters) {
	ifstream infile(hostparameters.inFileName);
	int stime,ssnap;
	infile >> stime;
	infile >> ssnap;
	if(stime !=  hostparameters.startTimeStep || ssnap != hostparameters.startSnapNumber) {
		cout << "warning: start time and/or snapshot number from particle file is/are not"
				"consistent with configuration file. Ignore the warning if this is intended."
				"\n";
	}
	for (int i=0;i<size()*number_of_systems();++i) {
		double posx,posy,velx,vely,accx,accy;
		infile >> posx;
		infile >> posy;
		infile >> velx;
		infile >> vely;
		infile >> accx;
		infile >> accy;
		if(infile.good()) {
			offset[i]=(int)posx;
			position[i].s[0]=(float)(posx-(int)posx);
			position[i].s[1]=(float)(posy);
			velocity[i].s[0]=velx;
			velocity[i].s[1]=vely;
			acceleration[i].s[0]=accx;
			acceleration[i].s[1]=accy;
		} else
		{
			cerr << "Error while reading particle Data \n";
			exit(EXIT_FAILURE);
		}
	}
	infile.close();
}



ostream& operator <<(ostream& os, ParticleSystem & ps) {
	for (int i = 0; i < ps.size()*ps.number_of_systems(); i++) {
		os << (double) ps.getPosition(i).s[0] << "\t"
				<< (double) ps.getPosition(i).s[1] << "\t"
				<< ps.velocity[i].s[0] << "\t" << ps.velocity[i].s[1] << "\t"
				<< ps.acceleration[i].s[0] << "\t" << ps.acceleration[i].s[1]
				<< "\n";
	}
	return os;
}




