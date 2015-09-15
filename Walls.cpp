/*
 * Walls.cpp
 *
 *  Created on: Sep 13, 2015
 *      Author: becker
 */

#include "Walls.h"
#include <iostream>
#include <cmath>


Walls::Walls(
			const cl::Context _context,
			const cl::CommandQueue _queue,
			double wallHorizontalUp,
			double wallHorizontalDown,
			double wallVerticalLeft,
			double wallVerticalRight,
			double wallStampForce,
			double wallMass
			) : context(_context), queue(_queue) {
	stampForce()=(float)wallStampForce;
	mass()=(float)wallMass;
	infiniteHorizontalWallPositions[0]=wallHorizontalUp;
	infiniteHorizontalWallPositions[1]=wallHorizontalDown;

	int tmpoffset;
	float tmpposition;
	tmpoffset=(int)trunc(wallVerticalLeft);
	tmpposition=(float) (wallVerticalLeft-tmpoffset);
	offsetForVerticalWalls[0]=tmpoffset;
	infiniteVerticalWallsPositions[0]=tmpposition;

	tmpoffset = (int) trunc(wallVerticalRight);
	tmpposition = (float) (wallVerticalRight - tmpoffset);
	offsetForVerticalWalls[1] = tmpoffset;
	infiniteVerticalWallsPositions[1]=tmpposition;

	for(int i=0;i<2;i++)  {
		infiniteVerticalWallVelocity[i]=0;
		infinityVerticalWallAcceleration[i]=0;
	}


	infiniteVerticalWallsPositions[0]=tmpposition;
	try {
		offsetForVerticalWallsBuffer = cl::Buffer(context,
				CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
				sizeof(offsetForVerticalWalls), offsetForVerticalWalls);
		positionVerticalWallsBuffer = cl::Buffer(context,
				CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
				sizeof(infiniteVerticalWallsPositions),
				infiniteVerticalWallVelocity);
		positionHorizontalWallBuffer = cl::Buffer(context,
					CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
					sizeof(infiniteHorizontalWallPositions),
					infiniteHorizontalWallPositions);
		verticalWallVelocityBuffer = cl::Buffer(context,
				CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
				sizeof(infiniteVerticalWallVelocity),
				infiniteVerticalWallVelocity);
		verticalWallAccelerationBuffer = cl::Buffer(context,
				CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
				sizeof(infinityVerticalWallAcceleration),
				infinityVerticalWallAcceleration);
	} catch(cl::Error &error) {
		std::cerr << error.what() << "(" << error.err() << ")" << std::endl;
				exit(EXIT_FAILURE);
	}
}

Walls::~Walls() {
	// TODO Auto-generated destructor stub
}

void Walls::getDataFromDevice(){
	try {
		queue.enqueueReadBuffer(offsetForVerticalWallsBuffer, CL_TRUE, 0,
				sizeof(offsetForVerticalWalls), offsetForVerticalWalls);
		queue.enqueueReadBuffer(positionVerticalWallsBuffer, CL_TRUE, 0,
				sizeof(infiniteVerticalWallsPositions),
				infiniteVerticalWallsPositions);
		queue.enqueueReadBuffer(positionHorizontalWallBuffer, CL_TRUE, 0,
				sizeof(infiniteHorizontalWallPositions),
				infiniteHorizontalWallPositions);
		queue.enqueueReadBuffer(verticalWallVelocityBuffer, CL_TRUE, 0,
				sizeof(infiniteVerticalWallVelocity),
				infiniteVerticalWallVelocity);
		queue.enqueueReadBuffer(verticalWallAccelerationBuffer, CL_TRUE, 0,
				sizeof(infinityVerticalWallAcceleration),
				infinityVerticalWallAcceleration);
	} catch (cl::Error &error) {
		std::cerr << error.what() << "(" << error.err() << ")" << std::endl;
		exit(EXIT_FAILURE);
	}

}

