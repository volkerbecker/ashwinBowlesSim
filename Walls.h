/*
 * Walls.h
 *
 *  Created on: Sep 13, 2015
 *      Author: becker
 */

#ifndef WALLS_H_
#define WALLS_H_

#include <CL/cl.hpp>


///contains the x an y position of the 4 walls and the correponding the cl buffers
class Walls {
public:
	Walls(
			const cl::Context _context,
			const cl::CommandQueue _queue,
			double wallHorizontalUp,
			double wallHorizontalDOwn,
			double wallVerticalLeft,
			double wallVerticalRight,
			double wallStampForce
			);
	~Walls();

protected:
	float infiniteHorizontalWallPositions[2];  ///< y position of the walls
	float infiniteVerticalWallsPositions[2]; ///< x position of the walls
	int offsetForVerticalWalls[2];

	float _wallMass; ///< mass of the walls
	float _infiniteVerticalWallStampForce; ///< stamp forces to the vertical walls

	//if the wall are moving
	float infiniteVerticalWallVelocity[2];


	//// OpenCL Buffers:
	const cl::Context &context; ///< opencl context, must be defined elsewhere
	const cl::CommandQueue &queue; ///< opencl command queue

	cl::Buffer offsetForVerticalWallsBuffer; ///< buffer connected with the positions offset
	cl::Buffer positionVerticalWallsBuffer;  ///< buffer connected with the positions of Verticla
	cl::Buffer positionHorizontalWallBuffer; ///< buffer connected with the positions of Horizontak Walls
	cl::Buffer infiniteVerticalWallVelocityBuffer;	 ///< buffer connected with the velocities of vertical Walls
	cl::Buffer infiniteVerticalWallStampForceBuffer; ///< buffer connected with the stamp force of the verticel

};

#endif /* WALLS_H_ */
