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



ParticleSystem::ParticleSystem(const cl::Context & clContext, ///< openCL context to connect particle data to gpu
		const cl::CommandQueue &_queue,
		const Parameters &paramters,
		const HostParameters & hostParameters
	) : context(clContext),queue(_queue){
	this->_size=paramters.numberOfParticles;
	this->_radius=paramters.radius;
	this->_mass=paramters.mass;

	// initialize particles
	position=vector<cl_float2>(size(),(cl_float2){0,0});
	velocity=vector<cl_float2>(size(),(cl_float2){0,0});
	acceleration=vector<cl_float2>(size(),(cl_float2){0,0});
	offset=vector<cl_int>(size(),0);
	offsetfreepositions=vector<cl_float2>(size(),(cl_float2){0,0});;

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
	double tmpPosition=rightWall-radius();
	double yposition=wallDistance/2-radius();
	int sign=1;
	for(int i=size()-1;i>=0;i--)  {
		offset[i]=(int)tmpPosition;
		position[i].s[0]=(float)(tmpPosition-offset[i]);
		position[i].s[1]=(float)(sign*yposition);
		sign*=-1;
		tmpPosition-=dx;
	}
}

void ParticleSystem::createLoosetState(
		const double & wallDistance, ///< the distance of the vertical walls
		const double & rightWall) {///< the right wall

	double phi=acos(wallDistance/2/radius()-1);
	double dx=sin(phi)*2*radius();
	double tmpPosition=rightWall-radius();
	double yposition=wallDistance/2-radius();
	int sign=1;
	for(int i=size()-1;i>=1;i-=2)  {
		offset[i]=(int)tmpPosition;
		position[i].s[0]=(float)(tmpPosition-offset[i]);
		position[i].s[1]=(float)(sign*yposition);
		tmpPosition-=radius()*2;
		offset[i-1]=(int)tmpPosition;
		position[i-1].s[0]=(float)(tmpPosition-offset[i-1]);
		position[i-1].s[1]=(float)(sign*yposition);
		sign*=-1;
		tmpPosition-=dx;
	}
}


void ParticleSystem::createParticleString(const float & initialDistance) {

	//resize the vectors


	int tmpOffset=0;
	float tmpPositionx=0+radius();
	float deltaX=2*radius()+initialDistance;
	for(uint i=0;i<size();++i) {
		offset[i]=tmpOffset;
		position[i].s[0]=tmpPositionx;
		position[i].s[1]=0*(drand48()*2-2);  // y_i=0
		tmpPositionx+=deltaX;
		tmpOffset+=trunc(tmpPositionx);
		tmpPositionx-=trunc(tmpPositionx);
	}
	//velocity[0].s[0]=1;
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
	getParticleDataFromDevice();
	for(int i=0;i<size();++i) {
		cl_double2 tmp=getPosition(i,columnlength);
		offsetfreepositions[i].s[0]=(float)tmp.s[0];
		offsetfreepositions[i].s[1]=(float)tmp.s[1];
	}
}

void ParticleSystem::setRandomVelocity(const cl_float2 &Amplitude) {
	for (uint i = 0; i < size(); ++i) {
		velocity[i].s[0] = Amplitude.s[0] * (drand48() * 2 - 1);
		velocity[i].s[1] = Amplitude.s[1] * (drand48() * 2 - 1);
	}
	queue.enqueueWriteBuffer(velocityBuffer, CL_FALSE, 0,
			size() * sizeof(cl_float2), velocity.data());
	;
}

void ParticleSystem::setVelocity(const cl_float2 &Amplitude) {
	for (uint i = 0; i < size(); ++i) {
		velocity[i].s[0] = Amplitude.s[0];
		velocity[i].s[1] = Amplitude.s[1];
	}
	queue.enqueueWriteBuffer(velocityBuffer, CL_FALSE, 0,
			size() * sizeof(cl_float2), velocity.data());
	;
}



ostream& operator <<(ostream& os, ParticleSystem & ps) {
	ps.getParticleDataFromDevice();
	for (int i = 0; i < ps.size(); i++) {
		os << (double) ps.getPosition(i).s[0] << "\t"
				<< (double) ps.getPosition(i).s[1] << "\t"
				<< ps.velocity[i].s[0] << "\t" << ps.velocity[i].s[1] << "\t"
				<< ps.acceleration[i].s[0] << "\t" << ps.acceleration[i].s[1]
				<< "\n";
	}
	return os;
}




