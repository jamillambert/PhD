#ifndef MOTION_H
#define MOTION_H
#include "spotPos.h"

class Motion {
	int variableDefsHere;
	double xVec;
	double yVec;
	double zVec;
	double motionPeriod;

public:
	Motion();
	Motion(double x, double y, double z, double period);
	spotPos moveSpot(spotPos SP, double deliveryTime) const;
	void setMotion(double x, double y, double z, double period);
	void addMotion(double x, double y, double z, double period);

};
#endif
