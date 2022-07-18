#include <iostream>
#include "spotPos.h"
#include "Motion.h"

Motion::Motion() {
	// Default constructor
	xVec = 0;
	yVec = 0;
	zVec = 0;
	motionPeriod = 0;
	std::cout << "\nDefault motion constructor\nx = " << xVec << "\ny = " << yVec << "\nz = " << zVec << "\nperiod = " << motionPeriod;
}
Motion::Motion(double x, double y, double z, double period){
	// constructor that assigns all the variables
	xVec = x;
	yVec = y;
	zVec = z;
	motionPeriod = period;
	std::cout << "\nMotion constructor\nx = " << xVec << "\ny = " << yVec << "\nz = " << zVec << "\nperiod = " << motionPeriod;
}
spotPos Motion::moveSpot(spotPos SP, double deliveryTime) const {
	//move the spot according to the motion of the object and return the moved spot
}
void Motion::setMotion(double x, double y, double z, double period) {
	// Sets the motion
	xVec = x;
	yVec = y;
	zVec = z;
	motionPeriod = period;
	std::cout << "\nDefault motion constructor\nx = " << xVec << "\ny = " << yVec << "\nz = " << zVec << "\nperiod = " << motionPeriod;
}
void Motion::addMotion(double x, double y, double z, double period) {
	// Adds extra motion
	xVec = xVec + x;
	yVec = yVec + y;
	zVec = zVec + z;
	motionPeriod = period;
	std::cout << "\nDefault motion constructor\nx = " << xVec << "\ny = " << yVec << "\nz = " << zVec << "\nperiod = " << motionPeriod;
}

