/*
 * AshwinBowlesSystem.h
 *
 *  Created on: Sep 15, 2015
 *      Author: becker
 */

#ifndef ASHWINBOWLESSYSTEM_H_
#define ASHWINBOWLESSYSTEM_H_

#include <CL/cl.hpp>
#include "ParticleSystem.h"
#include "Walls.h"
#include "Paramters.h"

class AshwinBowlesSystem {
public:
	AshwinBowlesSystem(
			const Parameters &parameters, ///< global System Parameters
			const float &initialdistance, ///< distance of particles at t=0
			const float &initialLength, ///< length of the box
			const float &initialHight); ///< height of the box
	~AshwinBowlesSystem();

protected:
	std::vector<cl::Platform> plattforms;
	std::vector<cl::Device> devices;
	cl::Context context;
	cl::CommandQueue queue;
	cl::Program clProgram;

	cl::Kernel verletStep1Kernel;  ///<Kernel that predicts the velocities and update the positions
	/// Kernel which will calculate the interactions between particle 12 34 56 etc
	cl::Kernel calculateAccelarationKernel1;
	/// Kernel which will calculate the interactions between particle 23 45 67 etc
	cl::Kernel calculateAccelarationKernel2;
	/// Kernel which will calculate the wall forces and then update the velocities
	cl::Kernel verletStep2Kernel;


	ParticleSystem *particles;
	Walls *walls;
};



#endif /* ASHWINBOWLESSYSTEM_H_ */
