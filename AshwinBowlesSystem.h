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
#include <cmath>

class AshwinBowlesSystem {
public:
	///
	AshwinBowlesSystem(
			const Parameters &parameters, ///< global System Parameters
			const float &initialdistance, ///< distance of particles at t=0
			const float &initialLength, ///< length of the box
			const float &initialHight); ///< height of the box
	~AshwinBowlesSystem();

	/// add a timestep to the openCL  Command queue
	void enqueueTimeStep();

	void enqueOffestupdate() {
		queue.enqueueNDRangeKernel(updateOffsetsKernel,0,globalp,localp);
	}

	/// returns a pointer to the OffsetFreePostions
	/// necessary for visualization
	const float* getPrtToOffsetFreePositions() {
		return particles->getOffsetFreePositionPointer();
	}

	///uodate the host meory
	void upDateHostMemory() {
		particles->getParticleDataFromDevice();
		walls->getDataFromDevice();
	}

	/// update the offset free postion data
	void updateOffsetFreeData(const int columnlength=100) {
		queue.enqueueNDRangeKernel(updateOffsetsKernel,cl::NullRange,globalp,localp);
		particles->updateOffsetfreePositions(columnlength);
	}

	//returns the kinetic and the potential Energy of the system
	void getEnergy(double &Ekin, double &Epot) {
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

protected:
	std::vector<cl::Platform> plattforms;
	std::vector<cl::Device> devices;
	cl::Context context;
	cl::CommandQueue queue;
	cl::Program clProgram;
	float asbTime=0;

	cl::Kernel verletStep1Kernel;  ///<Kernel that predicts the velocities and update the positions
	/// Kernel which will calculate the interactions between particle 12 34 56 etc
	cl::Kernel calculateAccelarationKernel1;
	/// Kernel which will calculate the interactions between particle 23 45 67 etc
	cl::Kernel calculateAccelarationKernel2;
	/// Kernel which will calculate the wall forces and then update the velocities
	cl::Kernel verletStep2Kernel;

	/// Kernel which will update the position offsets
	cl::Kernel updateOffsetsKernel;

	cl::Buffer parameterBuffer;

	cl::Buffer timeBuffer;

	ParticleSystem *particles;
	Walls *walls;

	Parameters parameter;

private:
	void initializeOpenCL();
	cl::Program loadCLSource(const char *, const cl::Context &);

	cl::NDRange globalp;
	cl::NDRange localp; //todo optmiale wgsize finden
	cl::NDRange globalAcc;

};



#endif /* ASHWINBOWLESSYSTEM_H_ */
