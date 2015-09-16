#ifndef __OPENCL_VERSION__
#define __kernel
#define __global
#define __local
#define __constant
#define false 0
#define true 1
typedef unsigned int uint;
typedef struct float2 {
	float x, y;
} float2;
typedef struct int2 {
	int x, y;
} int2;
typedef struct float4 {
	float x, y,z,w;
} float4;
typedef struct int4 {
	int x, y,z,w;
} int4;
#endif  //um eclipse auszutriksen

#include "/home/becker/workspace/ashwinBowlesSim/Paramters.h"

__kernel void verletStep1(__global int *posOffset,__global float2 *position, __global float2 *velocity,
	__global float2 *acceleration,__constant struct Parameters* paras){
	int id=get_global_id(0);
}

__kernel void calculateAccelaration(__global int *posOffset,__global float2 *position, __global float2 *velocity,
		__global float2 *acceleration,__constant struct Parameters* paras,const int start) {
	int id=get_global_id(0)+start;
}

__kernel void verletStep2(__global int *posOffset,__global float2 *position, __global float2 *velocity,
		__global float2 *acceleration,__constant struct Parameters* paras,const int offset) {
	int id=get_global_id(0);
}

