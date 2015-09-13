/*
 * Walls.cpp
 *
 *  Created on: Sep 13, 2015
 *      Author: becker
 */

#include "Walls.h"


Walls::Walls(
			const cl::Context _context,
			const cl::CommandQueue _queue,
			double wallHorizontalUp,
			double wallHorizontalDOwn,
			double wallVerticalLeft,
			double wallVerticalRight,
			double wallStampForce
			) : context(_context), queue(_queue) {};

Walls::~Walls() {
	// TODO Auto-generated destructor stub
}

