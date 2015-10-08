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
#include <signal.h>


class AshwinBowlesSystem {
public:
	AshwinBowlesSystem();
	///
	AshwinBowlesSystem(
			const Parameters &parameters, ///< global System Parameters relevant for host and kernels
			const HostParameters & hostparamters ///< Parameters relevant for the host#
			);
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
	}

	/// update the offset free postion data
	void updateOffsetFreeData(const int columnlength=100) {
		particles->updateOffsetfreePositions(columnlength);
	}

	///returns the kinetic and the potential Energy of the system
	void getEnergy(double &Ekin, double &Epot);


	/// test wther the particle system is jammed and returns
		/// the number of exited Bonds and the state vector
		/// 1 is an exited bond, 0 is an relaxed bond
		/// \returns true if the state fullfills the jamming condtion
	bool isJammed(
			int &exitedBonds, ///< \return number of exited bonds
			std::vector<bool> &stateVector ///< \return the state vector
	);

	//returns the physical volume fraction icluding overlap effects
	// make sure that the particle data is up to date
	double volume() {
		double length=particles->getPosition(particles->size()-1).s[0]-
				particles->getPosition(0).s[0]-particles->radius()*2;
		return length*particles->radius()*2;
	}


	void velocityPulse(const cl_float2 &amplitude) {
		particles->setRandomVelocity(amplitude);
	}

	void hammerPulse(const cl_float2 &amplitude) {
			particles->setVelocity(amplitude);
	}

	void cpuTimestep();
	void updateOffsetCpu();

	void saveState(const string &filename,const int &timeStep,const int &number);

	void getParticle(const int &i,float* pdat) {
		pdat[0]=particles->getPositions()[i].s[0]+particles->getOffset()[i];
		pdat[1]=particles->getPositions()[i].s[1];
		pdat[2]=particles->getVelocities()[i].s[0];
		pdat[3]=particles->getVelocities()[i].s[1];
		pdat[4]=particles->getAccelerations()[i].s[0];
		pdat[5]=particles->getAccelerations()[i].s[1];
	}




protected:
	std::vector<cl::Platform> plattforms;
	std::vector<cl::Device> devices;
	cl::Context context;
	cl::CommandQueue queue;
	cl::Program clProgram;
	float asbTime;

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


	Parameters parameter;

private:
	void initializeOpenCL(const HostParameters &hostParameters);
	cl::Program loadCLSource(const char *, const cl::Context &);
	void setup(const Parameters& parameters, const HostParameters &hostParameters);

	cl::NDRange globalp;
	cl::NDRange localp; //todo optmiale wgsize finden
	cl::NDRange globalAcc;

};



#endif /* ASHWINBOWLESSYSTEM_H_ */
