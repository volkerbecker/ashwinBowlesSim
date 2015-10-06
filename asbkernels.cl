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

#define workgroupsize 256
#include "/home/becker/workspace/ashwinBowlesSim/Paramters.h"


///Function to calculate the acceleration
inline float2 calcAcceleration(
		const __global int *offset1,
		const __global int *offset2,
		const __global float2 *pos1,
		const __global float2 *pos2,
		const __global float2 *vel1,
		const __global float2 *vel2,
		__constant struct Parameters* paras)
{
	float2 distance=*pos2-*pos1;
	distance.x+=(*offset2-*offset1); ///evt lokalen speicher nutzen
	float overlap=-(length(distance)-paras->diameter); /// todo test whether precision is sufficient
	if(overlap>0) {
		float force;
		distance=normalize(distance); // calculate the normal vector
		force=-fmax(overlap*paras->springConstant-dot(*vel2-*vel1,distance)*paras->damping,0);
		return ( force * paras->inverseMass)*distance;
	} else
		return (float2)(0,0);
}

__kernel void verletStep1(__global float2 *position, __global float2 *velocity,
		__global float2 *acceleration, __constant struct Parameters* paras) {
	int id = get_global_id(0);
	velocity[id] += 0.5f * acceleration[id] * paras->timestep;
	position[id] += velocity[id] * paras->timestep;
	velocity[id] += 0.5f * acceleration[id] * paras->timestep;
}


__kernel void updateOffset(__global int *posOffsets,__global float2 *positions) {
	int id=get_global_id(0);
	int truncPos=(int) positions[id].x;
	posOffsets[id]+=truncPos;
	positions[id].x-=truncPos;
}

/// This Kernel calulates the accelarations in one step, but to the price of doubling the
/// number of force calcluations
__kernel void calculateAccelarationOnestep(__global int *posOffset,
		__global float2 *position, __global float2 *velocity,
		__global float2 *acceleration,__global float *time,__constant struct Parameters* paras,
		const int start) {
	int id = get_global_id(0);
	float2 acc = 0;

	//caclulate accelaration due to the nearest right neighbor
	//if ((id + 1) < paras->numberOfParticles) {
		acc += ((id + 1) < paras->numberOfParticles) ?
				calcAcceleration(&posOffset[id], &posOffset[id + 1],
				&position[id], &position[id + 1], &velocity[id],
				&velocity[id + 1], paras) : 0;
	//}
	//calculate acceleration due to the nearest left neighbor
	//if ((id) > 0) {
		acc += ((id) > 0) ?
				calcAcceleration(&posOffset[id], &posOffset[id - 1],
				&position[id], &position[id - 1], &velocity[id],
				&velocity[id - 1], paras) : 0;
	//}
//
	//horizontalWalls
	//oben
	{
		float overlapp = paras->radius - (paras->upperWall - position[id].y);
		float force = overlapp>0 ?
					fmax( paras->springConstant * overlapp
						+ paras->damping * velocity[id].y, 0) : 0;
		acc.y -= force * paras->inverseMass;
	}
//	//unten
	{
		float overlapp = paras->radius + (paras->lowerWall - position[id].y);
			float force = overlapp >0 ? fmax(
				paras->springConstant * overlapp
				- paras->damping * velocity[id].y, 0) : 0;
			acc.y += force * paras->inverseMass;
	}
	//calculate wall forces
	if (id == 0 || id == (paras->numberOfParticles - 1)) {
		int i = (id == 0 ? 0 : 1);
		int wallOffset = (
				id == 0 ? paras->leftWallofset : paras->rightWallOffset);
		float verticalWall = (id == 0 ? paras->leftWall : paras->rightWall);
		float overlapp = (float) (posOffset[id] - wallOffset)
				+ (position[id].x - verticalWall);
		if (fabs(overlapp) < paras->radius) {
			overlapp += (i == 0 ? -paras->radius : paras->radius);
			overlapp *= -1;
			//printf("%i %i %f \n",id,i,overlapp);
			float force = (
					i == 0 ?
							fmax(
									paras->springConstant * overlapp
											- paras->damping * velocity[id].x,
									0) :
							fmin(
									paras->springConstant * overlapp
											- paras->damping * velocity[id].x,
									0));
			acc.x += force * paras->inverseMass;
		}
	}
	if (id == 0) {
		acc.x += paras->stampAcceleration;
	}
		acc-=paras->viskosity*velocity[id]*paras->inverseMass;
		velocity[id] += 0.5f * paras->timestep * (acc-acceleration[id]);
		acceleration[id] = acc;
	if(id==0) *time +=paras->timestep;
}
