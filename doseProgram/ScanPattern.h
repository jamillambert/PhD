#ifndef SCANPATTERN_H
#define SCANPATTERN_H
#include <map>
#include <vector>
#include "Motion.h"
#include "spotPos.h"
#include "scanSpeed.h"

class ScanPattern {
	Motion motion;
	scanSpeed speed;
	std::map<int, std::vector<spotPos> > spotPositions;
	int layers;
	int currentLayer;
	int currentSpotNo;
	spotPos lastSpot;

public:
	ScanPattern();
	ScanPattern(int xWidth, int yWidth, int zWidth, double spacing, std::map<int, double>& weights);
	ScanPattern(int xWidth, int yWidth, int zWidth, double spacing, std::map<int, double>& weights, Motion m, scanSpeed speed);
	void reset();
	int numberLayers();
	int layerSize(int layerNumber);
	spotPos getSpot(int layer, int spotNo);
	spotPos getSpot();
	spotPos getNextSpot();
	void setMotion(Motion motionInput);
	void setScanSpeed(scanSpeed speedInput);
	void defineScanPattern();
	void defineScanPattern(int xWidth, int yWidth, int zWidth, double spacing, std::map<int, double>& weights);

};

#endif
