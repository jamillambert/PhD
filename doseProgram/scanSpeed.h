#ifndef SCANSPEED_
#define SCANSPEED_

struct scanSpeed {
	/*
	* Speed of scanning and no of paintings
	*/
	double layerTime;
	double energyChangeTime;
	std::vector<int> noPaintings;
};

#endif
