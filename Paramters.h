///Here a structure which contains all global parameters is defined
///It must be consistent with the corresponding Kernel structure

#ifndef PARAMETERS_H
#define PARAMETERS_H

#ifndef __OPENCL_VERSION__
#include <CL/cl.hpp>
#else
	typedef int cl_int;
	typedef float cl_float;
#endif

///This structure contains all global, constant variables
typedef struct Parameters {
    cl_int numberOfParticles; ///< number of particles
    cl_float mass; ///<particles mass
    cl_float radius; ///<particles radius
    cl_float wallstampforce; ///< the stamp force for jamming
} Parameters;

#endif
