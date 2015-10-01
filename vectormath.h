/*
 * vectormath.h
 *
 *  Created on: 30 Sep 2015
 *      Author: becker
 */

#ifndef VECTORMATH_H_
#define VECTORMATH_H_
#include <CL/cl.hpp>
#include <cmath>

inline float length(const cl_float2 &vec) {
	return sqrt(vec.s[0]*vec.s[0]+vec.s[1]*vec.s[1]);
}

inline cl_float2 normalize(cl_float2 vec) {
	float vlength=length(vec);
	vec.s[0]/=vlength;
	vec.s[1]/=vlength;
	return vec;
}

inline float dot(const cl_float2 &v1, const cl_float2 &v2) {
	return v1.s[0]*v2.s[0]+v1.s[1]*v2.s[1];
}

inline cl_float2 calcAcceleration(
		const cl_int *offset1,
		const cl_int *offset2,
		const cl_float2 *pos1,
		const cl_float2 *pos2,
		const cl_float2 *vel1,
		const cl_float2 *vel2,
		const struct Parameters* paras)
{
	cl_float2 tmp;
	cl_float2 distance;
	distance.s[0]=(pos2->s[0])-(pos1->s[0]);
	distance.s[1]=(pos2->s[1])-(pos1->s[1]);
	distance.s[0]+=(*offset2-*offset1); ///evt lokalen speicher nutzen
	float overlap=-(length(distance)-paras->diameter); /// todo test whether precision is sufficient
	if(overlap>0) {
		float force;
		cl_float2 relVel;
		relVel.s[0]=(*vel2).s[0]-(*vel1).s[0];
		relVel.s[1]=(*vel2).s[1]-(*vel1).s[1];
		distance=normalize(distance); // calculate the normal vector
		force=-fmax(overlap*paras->springConstant-dot(relVel,distance)*paras->damping,0)*paras->inverseMass;
		tmp.s[0]=force*distance.s[0];
		tmp.s[1]=force*distance.s[1];
		return tmp;
	} else
		return (cl_float2){0,0};
}

inline cl_float2 operator +=(cl_float2 &v1,const cl_float2 &v2) {
	v1.s[0]+=v2.s[0];
	v1.s[1]+=v2.s[1];
	return v1;
}

inline cl_float2 operator +(const cl_float2 &v1,const cl_float2 &v2) {
	cl_float2 tmp=v1;
	tmp+=v2;
	return tmp;
}

inline cl_float2 operator -=(cl_float2 &v1,const cl_float2 &v2) {
	v1.s[0]-=v2.s[0];
	v1.s[1]-=v2.s[1];
	return v1;
}

inline cl_float2 operator -(const cl_float2 &v1,const cl_float2 &v2) {
	cl_float2 tmp=v1;
	tmp-=v2;
	return tmp;
}

inline cl_float2 operator *=(cl_float2 &v1,const cl_float2 &v2) {
	v1.s[0]*=v2.s[0];
	v1.s[1]*=v2.s[1];
	return v1;
}

inline cl_float2 operator *(const cl_float2 &v1,const cl_float2 &v2) {
	cl_float2 tmp=v1;
	tmp*=v2;
	return tmp;
}

inline cl_float2 operator /=(cl_float2 &v1,const cl_float2 &v2) {
	v1.s[0]/=v2.s[0];
	v1.s[1]/=v2.s[1];
	return v1;
}

inline cl_float2 operator /(const cl_float2 &v1,const cl_float2 &v2) {
	cl_float2 tmp=v1;
	tmp/=v2;
	return tmp;
}


inline cl_float2 operator *(const cl_float &v1,const cl_float2 &v2) {
	cl_float2 tmp;
	tmp.s[0]=v1*v2.s[0];
	tmp.s[1]=v1*v2.s[1];
	return tmp;
}

inline cl_float2 operator *(const cl_float2 &v1,const cl_float &v2) {
	cl_float2 tmp;
	return v2*v1;
}

inline cl_float2 operator /(const cl_float2 &v1,const cl_float &v2) {
	cl_float2 tmp;
	tmp.s[0]=v1.s[0]/v2;
	tmp.s[1]=v1.s[1]/v2;
	return tmp;
}



#endif /* VECTORMATH_H_ */
