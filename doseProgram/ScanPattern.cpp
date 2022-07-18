#include <map>
#include <vector>
#include "Motion.h"
#include "spotPos.h"
#include "scanSpeed.h"
#include "scanPattern.h"
#include <iostream>

ScanPattern::ScanPattern() {
	layers = 0;
	currentLayer = 0;
	currentSpotNo = 0;
	lastSpot.weight = -1;
}
ScanPattern::ScanPattern(int xWidth, int yWidth, int zWidth, double spacing, std::map<int, double>& weights) {
	lastSpot.weight = -1;
}
ScanPattern::ScanPattern(int xWidth, int yWidth, int zWidth, double spacing, std::map<int, double>& weights, Motion m, scanSpeed speed) {
	lastSpot.weight = -1;
}
void ScanPattern::reset() {
	currentLayer = 0;
	currentSpotNo = 0;
}
int ScanPattern::numberLayers() {
	return layers;
}
int ScanPattern::layerSize(int layerNumber) {
	return spotPositions[layerNumber].size();
}
spotPos ScanPattern::getSpot(int layer, int spotNo) {
	if (layer > layers || spotNo > spotPositions[layer].size())
		return lastSpot;
	else
		return spotPositions[layer][spotNo];
}
spotPos ScanPattern::getSpot() {
	return spotPositions[currentLayer][currentSpotNo];
//std::cout << "Layer Number : "<<currentLayer << " Spot No. " << currentSpotNo <<"\n";
}
spotPos ScanPattern::getNextSpot() {
	currentSpotNo++;
	if (currentSpotNo >= spotPositions[currentLayer].size()) {
		currentLayer++;
		currentSpotNo = 0;
//std::cout << "Layer Number : "<<currentLayer << " Spot No. " << currentSpotNo <<"\n";
		if (currentLayer > layers)
			return lastSpot;
		else
			return spotPositions[currentLayer][currentSpotNo];
	}
	else {
//std::cout << "Layer Number : "<<currentLayer << " Spot No. " << currentSpotNo<< " layer size" << spotPositions[currentLayer].size() <<"\n" ;
		return spotPositions[currentLayer][currentSpotNo];
	}
}
void ScanPattern::setMotion(Motion motionInput) {
	motion = motionInput;
}
void ScanPattern::setScanSpeed(scanSpeed speedInput) {
	speed = speedInput;
}
void ScanPattern::defineScanPattern() {
	spotPos newSpot;
	newSpot.weight = 1;
	speed.layerTime = 0.7073;
	speed.energyChangeTime = 2;
	speed.noPaintings.push_back(20);
	speed.noPaintings.push_back(3);
	speed.noPaintings.push_back(5);
	speed.noPaintings.push_back(3);
	speed.noPaintings.push_back(4);
	speed.noPaintings.push_back(3);
	speed.noPaintings.push_back(3);
	speed.noPaintings.push_back(2);
	speed.noPaintings.push_back(2);
	speed.noPaintings.push_back(2);
	speed.noPaintings.push_back(2);
	currentLayer = 0;
	for (int z = 280; z <= 300; z += 5) {
		newSpot.z = z;
		layers++;
		for (int y = -10; y <= 10; y += 5) {
			newSpot.y = y;
			for (int x = -10; x <= 10; x += 5) {
				newSpot.x = x;
				spotPositions[currentLayer].push_back(newSpot);
			}
		}
		currentLayer++;
	}
//std::cout << "\nNumber of spots: "<< spotPositions[255].size();
}
void ScanPattern::defineScanPattern(int xWidth, int yWidth, int zWidth, double spacing, std::map<int, double>& weights) {
	int temp;
}


