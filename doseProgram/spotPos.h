#ifndef SPOTPOS_
#define SPOTPOS_

struct spotPos {
	/*
	* Position and weight of a single Bragg peak
	*/
	int x;
	int y;
	int z;
	double weight;
};

#endif
