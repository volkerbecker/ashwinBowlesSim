/*
 * ParticleSystem.h
 *
 *  Created on: Sep 10, 2015
 *      Author: becker
 */

#ifndef PARTICLESYSTEM_H_
#define PARTICLESYSTEM_H_
#include <vector>
#include <CL/cl.hpp>
#include <iostream>
#include <climits>
#include "Paramters.h"

using namespace std;

/// Diese Klasse enthält das Partikelsystem und koppelt es mit einem
/// OpenCl buffer
class ParticleSystem {
public:
	ParticleSystem(
			const cl::Context & Context ,  ///< openCL context to connect particle data to gpu
			const cl::CommandQueue & queue, ///< openCL Command queue
			const Parameters &parameters, ///< system parameters relevant for kernel and host
			const HostParameters &HostParameters); ///< system parameters relevant for host only
	~ParticleSystem();

	/// get or set the system size
	unsigned int & size() {return _size;}

	/// get the system size
	const unsigned int &size() const {return _size;}

	/// acces to radius
	float & radius() {return _radius;}
	/// acces to radius
	const float & radius() const {return _radius;}
	/// acces to mass
	float & mass() {return _mass;}
	/// acces to mass
	const float & mass() const {return _mass;}



	/// \return reference to the Position offset Buffer
	cl::Buffer & getOffsetBuffer() {return offsetBuffer;}

	/// \return reference to the position Buffer
	cl::Buffer & getPositionBuffer() {return positionBuffer;}

	/// \return reference to the velocity Buffer
	cl::Buffer & getVelocityBuffer() {return velocityBuffer;}

	/// \return reference to the velocity Buffer
	cl::Buffer & getAccelerationBuffer() {return accelerationBuffer;}

	/// \return refrence to oldAccelaration buffer
	cl::Buffer & getOldAccelerationBuffer() {return oldAccelerationBuffer;}

	/// \return reference to the Position offsets
	vector<cl_int> & getOffset() {return offset;}
	const vector<cl_int> & getOffset() const {return offset;}

	/// \return reference to the positions
	vector<cl_float2> & getPositions() {return position;}
	const vector<cl_float2> & getPositions() const {return position;}

	/// \return reference to the velocities
	vector<cl_float2> & getVelocities() {return velocity;}
	const vector<cl_float2> & getVelocities() const {return velocity;}

	/// \return reference to the accelerations
	vector<cl_float2> & getAccelerations() {return acceleration;}
	const vector<cl_float2> & getAccelerations() const  {return acceleration;}

	///< get the position of the i'th particle
	cl_double2 getPosition(
			int number, ///< return the position of the i'th particle
			int columnlength=INT_MAX) {
		cl_double2 tmp;
		int tmpoffsetx=offset[number]%columnlength;
		double tmpoffsety=(offset[number]/columnlength)*4*radius();
		tmp.s[0]=tmpoffsetx+position[number].s[0];
		tmp.s[1]=(double)position[number].s[1]-tmpoffsety;
		return tmp;
	}


	///< copy particle data from gpu tu cpu memory
	void getParticleDataFromDevice();

	// update the offsetfree position buffer for visualization
	void updateOffsetfreePositions(
			const int &columnlengt=100); ///< if the offset%comulmlength=0 a new  row appears

	/// get a read only pointer to the offset free positions
	const float* getOffsetFreePositionPointer() {
		return (float*)offsetfreepositions.data();
	}

	/// sets the system to the densest possible state
	void createDensestState(
			const double & wallDistance, ///< the distance of the vertical walls
			const double & rightWall); ///< the right wall

	void createLoosetState(
				const double & wallDistance, ///< the distance of the vertical walls
				const double & rightWall); ///< the right wall

	void setRandomVelocity(const cl_float2 &Amplitude);

	void setVelocity(const cl_float2 &Amplitude);


	///ostream operator
	friend ostream& operator << (ostream& os, ParticleSystem & ps);



protected:
	unsigned int _size=0; ///< number of particles
	float _radius;
	float _mass;
	vector<cl_int> offset; ///< offset for the particle x-positions
	vector<cl_float2> position; ///< particles' x position relative to the offset
	vector<cl_float2> velocity; ///< particles velocities
	vector<cl_float2> acceleration; ///<particles accelerations

	vector<cl_float2> offsetfreepositions; //< positions for visualization;


	const cl::Context &context; ///< opencl context, must be defined elsewhere
	const cl::CommandQueue &queue; ///< opencl Command queue, must be defined elsewhere

	cl::Buffer offsetBuffer; ///< buffer connected with the positions offset
	cl::Buffer positionBuffer;       ///< buffer connected with the positions
	cl::Buffer velocityBuffer;	     ///< buffer connected with the velocities
	cl::Buffer accelerationBuffer;   ///< buffer connected with the accelerations
	cl::Buffer oldAccelerationBuffer;   ///< buffer connected with the oldaccelerations

	void createParticleString( ///< create a string of particles allog the y axis
			const float & initialDistance ///< distance between particles
			);
};

#endif /* PARTICLESYSTEM_H_ */
