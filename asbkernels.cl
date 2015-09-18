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

__kernel void verletStep1(__global float2 *position, __global float2 *velocity,
	__global float2 *acceleration,__global float2 *oldAcceleration,
	__constant struct Parameters* paras){
	int id=get_global_id(0);
	position[id]+=velocity[id]*paras->timestep+0.5f*acceleration[id]*paras->timestepSq;
	velocity[id]+=acceleration[id]*paras->timestep;
	oldAcceleration[id]=acceleration[id];
	acceleration[id]=0;
	//acceleration[id] = id==0 ? paras->stampAcceleration : 0; //todo test whether it is faster by doing it on host side via copybuffer and fillbuffer
}

__kernel void calculateAccelaration(__global int *posOffset,__global float2 *position, __global float2 *velocity,
		__global float2 *acceleration,__constant struct Parameters* paras,const int start) {
	int id=2*get_global_id(0)+start;
	int gid=get_global_id(0);
	if( (id+1) < paras->numberOfParticles) {
		__global int2 *posoff = (__global void*)(posOffset+start);
		__global float4 *pos = (__global void*)(position+start);
		float2 pos1= (float2)(pos[gid].x,pos[gid].y);
		float2 pos2= (float2)(pos[gid].z,pos[gid].w);
		float2 distance=pos2-pos1;
		distance.x+=(posoff[gid].y-posoff[gid].x); ///evt lokalen speicher nutzen
		float overlap=-(fast_length(distance)-paras->diameter); /// todo test whether precision is sufficient
		if(overlap>0) {
			__global float4 *vel = (__global void*)(velocity+start);
			float2 vel1=(float2)(vel[gid].x,vel[gid].y);
			float2 vel2=(float2)(vel[gid].z,vel[gid].w);
			distance=fast_normalize(distance); // calculate the normal vector
			float force=-fmax(overlap*paras->springConstant+dot(vel1-vel2,distance)*paras->damping,0);
			float2 acc = (force) * paras->inverseMass * distance;
//			if (id == 0)
//				printf(
//						"Force %f, Damping %f, Distance %f,%f overlapp: %f diameter %f\n",
//						force, paras->damping, distance.x, distance.y, overlap,paras->diameter);
			acceleration[id] += acc;
			acceleration[id+1]-=acc;
		}
	}
}

__kernel void verletStep2(
		__global int *posOffset,
		__global float2 *position,
		__global float2 *velocity,
		__global float2 *acceleration,
		__global float2 *oldAcceleration,
		__constant struct Parameters* paras) {
	int id=get_global_id(0);
	//calculate wall forces
	if(id==0 || id == (paras->numberOfParticles-1) ) {
		int i = (id==0 ? 0 : 1);
		int wallOffset = (id ==0 ? paras->leftWallofset : paras->rightWallOffset);
		float verticalWall = (id ==0 ? paras->leftWall : paras-> rightWall);
		float overlapp=(float)(posOffset[id]-wallOffset)+(position[id].x-verticalWall);
		if (fabs(overlapp) < paras->radius) {
			overlapp += (i == 0 ? -paras->radius : paras->radius);
			overlapp *=-1;
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
			acceleration[id].x += force * paras->inverseMass;
		}
	}
	//horizontalWalls
	//oben
	{
		float overlapp = paras->radius - (paras->upperWall - position[id].y);
		if (overlapp > 0) {
			float force = fmax(
					paras->springConstant * overlapp
							- paras->damping * velocity[id].x, 0);
			acceleration[id].y -= force * paras->inverseMass;
		}
	}

	//unten
	{
		float overlapp = paras->radius + (paras->lowerWall - position[id].y);
		if (overlapp > 0) {
			float force = fmax(
					paras->springConstant * overlapp
							- paras->damping * velocity[id].y, 0);
			acceleration[id].y += force * paras->inverseMass;
		}
	}


	velocity[id]+=0.5f*paras->timestep*(acceleration[id]-oldAcceleration[id]);
}

__kernel void updateOffset(__global int *posOffsets,__global float2 *positions) {
	int id=get_global_id(0);
	int truncPos=(int) positions[id].x;
	posOffsets[id]+=truncPos;
	positions[id].x-=truncPos;
}

/// This Kernel calulates the accelarations in one step, but to the price of doubling the
/// number of force calcluations
__kernel void calculateAccelarationOnestep(__global int *posOffset,__global float2 *position, __global float2 *velocity,
		__global float2 *acceleration,__constant struct Parameters* paras,const int start) {
	int id=get_global_id(0);
	if( (id+1) < paras->numberOfParticles) {
		float2 distance=position[id+1]-position[id];
		distance.x+=(posOffset[id+1]-posOffset[id]); ///evt lokalen speicher nutzen
		float overlap=-(fast_length(distance)-paras->diameter); /// todo test whether precision is sufficient
		if(overlap>0) {
			distance=fast_normalize(distance); // calculate the normal vector
			float force=-fmax(overlap*paras->springConstant+dot(velocity[id]-velocity[id+1],distance)*paras->damping,0);
			float2 acc = (force) * paras->inverseMass * distance;
//			if (id == 0)
//				printf(
//						"Force %f, Damping %f, Distance %f,%f overlapp: %f diameter %f\n",
//						force, paras->damping, distance.x, distance.y, overlap,paras->diameter);
			acceleration[id] += acc;
			//acceleration[id+1]-=acc;
		}
	}
	if( (id) >0 ) {
			float2 distance=position[id-1]-position[id];
			distance.x+=(posOffset[id-1]-posOffset[id]); ///evt lokalen speicher nutzen
			float overlap=-(fast_length(distance)-paras->diameter); /// todo test whether precision is sufficient
			if(overlap>0) {
				distance=fast_normalize(distance); // calculate the normal vector
				float force=-fmax(overlap*paras->springConstant+dot(velocity[id]-velocity[id+1],distance)*paras->damping,0);
				float2 acc = (force) * paras->inverseMass * distance;
	//			if (id == 0)
	//				printf(
	//						"Force %f, Damping %f, Distance %f,%f overlapp: %f diameter %f\n",
	//						force, paras->damping, distance.x, distance.y, overlap,paras->diameter);
				acceleration[id] += acc;
				//acceleration[id+1]-=acc;
			}
		}
}
