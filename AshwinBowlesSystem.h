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
			const float &initialHight); ///< hight of the box
	~AshwinBowlesSystem();

protected:
	std::vector<cl::Platform> plattforms;
	std::vector<cl::Device> devices;
	cl::Context context;
	cl::CommandQueue queue;

	ParticleSystem *particles;
	Walls *walls;
};



#endif /* ASHWINBOWLESSYSTEM_H_ */
