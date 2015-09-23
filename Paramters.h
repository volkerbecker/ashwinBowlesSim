///Here a structure which contains all global parameters is defined
///It must be consistent with the corresponding Kernel structure

#ifndef PARAMETERS_H
#define PARAMETERS_H

#ifndef __OPENCL_VERSION__
#include <CL/cl.hpp>
#include <string>

typedef enum {
	PCHAIN,PDENSEST,PLOOSEST,PFILE, ///< particle initialisation types
	RANDOMDELTA, ///< protocol type
	ENERGY ///< taping kriterion
} asbEnum;

typedef struct HostParameters {
	bool takeSnapShots; ///< sollen regelmaeßige snapshots gemacht werden?
	std::string baseName; ///<basename für Ausgabedateien
	int snapshotIntervall; ///< number of timesteps between snapshots
	int timestepOffset; ///< wenn Simlation nicht bei t=0 anfaenft

	bool visualization; ///< soll visualisiert werden
	bool savesBitmaps; ///sollen visulisierungen als png Dateien gespeichert werden?
	int visualizerIntervall; ///< timesteps between visualization
} HostParameters;
#else
	typedef int cl_int;
	typedef float cl_float;
	typedef bool cl_bool;
#endif

///This structure contains all global, constant variables
typedef struct Parameters {
    cl_int numberOfParticles; ///< number of particles
    cl_float mass; ///<particles mass
    cl_float radius; ///<particles radius
    cl_float diameter; ///< particles dameter = 2 * radius
    cl_float timestep;
    cl_float timestepSq;
    cl_float springConstant;
    cl_float damping;
    cl_float inverseMass;
    cl_int 	leftWallofset;
    cl_float leftWall;
    cl_int rightWallOffset;
    cl_float rightWall;
    cl_float upperWall;
    cl_float lowerWall;
    cl_float stampAcceleration;
    cl_bool jamming;
    cl_float viskosity;
    cl_float tappingAmplitude;
} Parameters;

#endif
