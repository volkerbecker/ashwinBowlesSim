///Here a structure which contains all global parameters is defined
///It must be consistent with the corresponding Kernel structure

#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <CL/cl.hpp>

///This structure contains all global, constant variables
typedef struct Parameters {
    cl_int numberOfParticles; ///< number of particles
    cl_float mass; ///<particles mass
    cl_float radius; ///<particles radius
    cl_float wallstampforce; ///< the stamp force for jamming
} Parameters;

#endif
