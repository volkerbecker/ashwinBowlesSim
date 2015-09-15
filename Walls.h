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
			const cl::Context _context, ///< The open CL Context, must be defined elsewhere
			const cl::CommandQueue _queue, ///< the openl commandqueue, must be defined elsewhere
			double wallHorizontalUp, ///< the position of the upper wall
			double wallHorizontalDOwn, ///< the position of the bottom wall
			double wallVerticalLeft, ///< the position of the left wall
			double wallVerticalRight, ///< the position of the right wall
			double wallStampForce, ///< the wall stamp forces
			double wallMass ///< the walls' mass
			);
	~Walls();

	/// r/w acces to the stamp force
	float & stampForce() { return _stampForce;}

	/// ro acces to the stampforce
	const float & stampForce() const {return _stampForce;}

	/// r/w acces to the walls mass
	float & mass() { return _wallMass;}
	/// ro acces to the walls mass
	const float & mass() const { return _wallMass;}

	///< copy the wall data from device to host
	void getDataFromDevice();

	cl::Buffer & getOffsetBuffer() {return offsetForVerticalWallsBuffer;}
	cl::Buffer & getVerticalWallPositionBuffer() {return positionVerticalWallsBuffer;}
	cl::Buffer & getHorizontalWallPositionBuffer() {return positionHorizontalWallBuffer;}
	cl::Buffer & getWallVelocityBuffer() {return verticalWallVelocityBuffer;}
	cl::Buffer & getWallAccelarationBuffer() {return verticalWallAccelerationBuffer;}

	///< returns the position of the i'th vertical wall
	double getVerticalWallPosition(const int & i) {
		getDataFromDevice();
		return (double)offsetForVerticalWalls[i]+(double)infiniteVerticalWallsPositions[i];
	}

	///< returns the position of the i'th horizontal wall
	double getHorizontalWallPosition(const int & i) {
		getDataFromDevice();
		return (double)infiniteHorizontalWallPositions[i];
	}

	///< returns the velocity of the i'th vertical wall
	double getVerticalWallvelocity(const int & i) {
		getDataFromDevice();
		return (double)infiniteVerticalWallVelocity[i];
	}

	///< returns the acceleration of the i'th vertical wall
	double getVerticalWallAcceleration(const int & i) {
		getDataFromDevice();
		return (double)infinityVerticalWallAcceleration[i];
	}

protected:
	float infiniteHorizontalWallPositions[2];  ///< y position of the walls
	float infiniteVerticalWallsPositions[2]; ///< x position of the walls
	int offsetForVerticalWalls[2];

	float _wallMass; ///< mass of the walls
	float _stampForce; ///< stamp forces acting on the vertical walls

	//if the wall are moving
	float infiniteVerticalWallVelocity[2];
	float infinityVerticalWallAcceleration[2];



	//// OpenCL Buffers:
	const cl::Context &context; ///< opencl context, must be defined elsewhere
	const cl::CommandQueue &queue; ///< opencl command queue

	cl::Buffer offsetForVerticalWallsBuffer; ///< buffer connected with the positions offset
	cl::Buffer positionVerticalWallsBuffer;  ///< buffer connected with the positions of Verticla
	cl::Buffer positionHorizontalWallBuffer; ///< buffer connected with the positions of Horizontak Walls
	cl::Buffer verticalWallVelocityBuffer;	 ///< buffer connected with the velocities of vertical Walls
	cl::Buffer verticalWallAccelerationBuffer;	 ///< buffer connected with the acceleration of vertical Walls
};

#endif /* WALLS_H_ */
