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



ParticleSystem::ParticleSystem(const cl::Context & clContext, ///< openCL context to connect particle data to gpu
		const cl::CommandQueue &_queue,
		const int & particleNumber, ///< Number of particles
		const float & radius, ///< radius of the particles
		const float & mass, ///< particles mass
		const float & initialdistance ///< distance of initial particles
		) : context(clContext),queue(_queue){
	this->_size=particleNumber;
	this->_radius=radius;
	this->_mass=mass;

	// initialize particles
	createParticleString(initialdistance);

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

void ParticleSystem::createParticleString(const float & initialDistance) {

	//resize the vectors
	position.resize(size());
	velocity.resize(size());
	acceleration.resize(size());
	offset.resize(size());

	int tmpOffset=0;
	float tmpPositionx=0;
	float deltaX=2*radius()+initialDistance;
	for(uint i=0;i<size();++i) {
		offset[i]=tmpOffset;
		position[i].s[0]=tmpPositionx;
		position[i].s[1]=0;  // y_i=0
		velocity[i].s[0]=velocity[i].s[1]=0; // v=0
		acceleration[i].s[0]=acceleration[i].s[1]=0; //a=0

		tmpPositionx+=deltaX;
		tmpOffset+=trunc(tmpPositionx);
		tmpPositionx-=trunc(tmpPositionx);
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

