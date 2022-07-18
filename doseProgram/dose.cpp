#include <string>
#include <vector>
#include <map>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <cmath>

#include "spotPos.h"
#include "scanSpeed.h"
#include "Motion.h"
#include "ScanPattern.h"

/*
* This Program calculates the dose deliverd by a proton beam using
* formula from M. Lee et. al 1993. and proton energy data from
* J. Janni 1982.
* It calculates the dose delivered by a passively scatterd proton beam
* or a dynamically scanned proton beam, on a cubic water phantom.
* The target can be moved during treatment (intrafractional movement)
* or between planning and treatment (interfractional movement).
*
* Code written by Jamil Lambert 2004, improved functionality for PostDoc work 2008.
*
*/

typedef std::map<int, double>::const_iterator CI;
typedef std::map< std::string, std::vector<int> > sMap;
typedef std::map<int, std::map<int, double> > map2D;
typedef std::map<int, std::map<int, std::map<int, double> > > map3D;

/* If run from an input file (disp = false) and only errors are output.*/
bool disp = true;


void input(std::map<int, double>& inputMap, std::string fileName) {
	/*
	* Inputs (int, double) pairs from a file.
	* Used by calcPenumbra() and calcPeaks().
	*/
	std::ifstream inFile ( fileName.c_str() );
	while(inFile) {
		double energy;
		int range;
		inFile >> range;
		inFile >> energy;
		inputMap[range] = energy;
	}
}


bool inputAll(map2D& braggPeaks, int& maxRange) {
	/*
	* Depth dose for Bragg Peaks with peaks at each mm up to
	* maxRange are input.
	* Requires file "allPeaks"
	* Depth dose must have already been calculated from Janni Data.
	*/
	std::ifstream inFile ("allPeaks");
	if (!inFile) std::cout << "\nInput file Error";
	inFile >> maxRange;
	if (!inFile) std::cout << "\nInput file type Error";
	else {
		for ( int r = 0; r < maxRange; r++) {
			for (int Z = 0; Z < maxRange; Z++) {
				double energy;
				int depth;
				inFile >> depth;
				inFile >> energy;
				braggPeaks[r][Z] = energy;
			}
		}
	}
	return true;
}


double integrate(std::map<int, double> fx) {
	/*
	* Integrates area in cubic mm for a function with
	* values given at mm intervals.
	* Due to the way the data is stored this is the most
	* accurate method, the Simpson rule or other rule
	* would not work as well.
	*/
	double S = 0;
	for (int i = 0; i < fx.size(); i++) {
		S += fx[i];
	}
	return S;
}

bool outputAll(map2D& braggPeaks, int maxRange) {
	/*
	* Outputs The depth dose for all Bragg peaks with peaks at
	* depths at each mm up to maxRange.
	* Only needs to be run if the allPeaks file has a maxRange
	* too low, or does not exist.
	*/
	std::ofstream outFile ("allPeaks");
	if (!outFile){
		std::cout << "\n\nERROR with creating file, data not written to file";
		return true;
	}
	outFile << maxRange << "\n";
	for ( int r = 0; r < maxRange; r++) {
		/* Normalises Dose to 100% */
		double max = 0;
		for (int i = 0; i < braggPeaks[r].size(); i++) {
			if (braggPeaks[r][i] > max) {
				max = braggPeaks[r][i];
			}
		}
		for (int Z = 0; Z < maxRange; Z++) {
			double percentDose = (double)100 * braggPeaks[r][Z] / max;
			/* Normalised dose */
//			outFile << Z << "\t" << percentDose << "\n";
			/* not Normalised */
			outFile << Z << "\t" << braggPeaks[r][Z] << "\n";
		}
	}
	return true;
}


bool outputPeak(map2D& braggPeaks) {
	/*
	* Outputs a single Bragg peak or the Penumbra depending
	* on the function that called this.  The depth of the
	* peak or penumbra is entered by the user, and output
	* to a specified file.
	*/
	std::string fileName;
	int depth;
	double max = 0;
	int depthMax = 0;
	if (disp) std::cout << "\nEnter Output File Name: ";
	std::cin >> fileName;
	if (!std::cin) {
		std::cout << "Error with file name input";
		return true;
	}
	std::ofstream outFile ( fileName.c_str() );
	if (!outFile){
		std::cout << "\n\nERROR with creating file, data not written to file";
		return true;
	}
	if (disp) std::cout << "\nEnter Peak Depth to output: ";
	std::cin >> depth;
	if (!std::cin) {
		std::cout << "\n\nError with depth input\n";
		return true;
	}
	/* Commented code normalises peak to a maximum of 100% */
//	for (int i = 0; i < braggPeaks[depth].size(); i++) {
//		if (braggPeaks[depth][i] > max) {
//			max = braggPeaks[depth][i];
//			depthMax = i;
//		}
//	}
//	outFile << "\t\tPeak at: " << depthMax << "mm, mean proton range: ";
//  outFile << depth << "mm\n";
//	for (int Z = braggPeaks[depth].size()-1; Z > 0 ; Z--) {
//		double percentDose = (double)100 * braggPeaks[depth][Z] / max;
//		outFile << Z << "\t" << percentDose << "\n";
//		outFile << -Z << "\t" << braggPeaks[depth][Z] << "\n";
//	}
	for (CI p = braggPeaks[depth].begin(); p != braggPeaks[depth].end(); p++) {
		outFile << p->first << "\t" << p->second << "\n";
	}
	return true;
}


bool writeFile(map3D& doseData) {
	/*
	* Outputs the dose delivered in a single plane through z = 0;
	* in 1mm intervals.
	*/
	std::string fileName;
	if (disp) std::cout << "\nEnter Output File Name: ";
	std::cin >> fileName;
	if (!std::cin) {
		std::cout << "Error with file name input";
		return true;
	}
	int layerNumber;
	if (disp) std::cout << "\nEnter layer number: ";
	std::cin >> layerNumber;
	if (!std::cin) {
		std::cout << "Error with input outputting layer 0";
		layerNumber = 0;
	}
	std::ofstream outFile ( fileName.c_str() );
	if (!outFile){
		std::cout << "\n\nERROR with creating file, data not written to file";
		return true;
	}
	for (int y = 0; y < doseData[layerNumber].size(); y++) {
		for(int x = 0; x < doseData[layerNumber][y].size(); x++) {
			/* Writes the dose at 1mm intervals seperated by a tab charactor. One line for every mm in y*/
			outFile << doseData[layerNumber][x][y] << "\t";
		}
		outFile << "\n";
	}
	if (disp) std::cout << "\n\nData written to : " << fileName;
	return true;
}


bool writeHistogram(sMap& dose, int phantomSize, int targetSize, int beams, std::vector<double> maxMin) {
	/*
	* Outputs the dose volume histogram, to the specified file, DVH is
	* calculated first by doseVolume.
	*/
	std::vector<int> doseData;
	doseData = dose["target"];
	std::string fileName;
	if (disp) std::cout << "\n\nEnter Target Output File Name: ";
	std::cin >> fileName;
	if (!std::cin) {
		std::cout << "Error with file name input";
		return true;
	}
	std::ofstream targetFile ( fileName.c_str() );
	if (!targetFile){
		std::cout << "\n\nERROR with creating file, data not written to file";
		return true;
	}
	targetFile << "\t\tPhantom size: " << phantomSize << "\tTarget size: " << targetSize;
	targetFile << "\tMax Dose: " << maxMin[0] << "\tMin Dose: " << maxMin[1] << "\n";
	double percentVol;
	double volume = (double)targetSize*(double)targetSize;
	for(int i = 0; i < doseData.size(); i++) {
		/* Writes the percentage volume that recieved at least i% dose. */
		percentVol = (double)doseData[i]*(double)100/volume;
		targetFile << i << "\t" << percentVol << "\n";
	}
	if (disp) std::cout << "\n\nTarget data written to : " << fileName;
	/* Tissue DVH turned off, commented code works */
//	doseData = dose["tissue"];
//	if (disp) std::cout << "\n\nEnter Tissue Output File Name: ";
//	std::cin >> fileName;
//	if (!std::cin) {
//		std::cout << "Error with file name input";
//		return true;
//	}
//	std::ofstream tissueFile  ( fileName.c_str() );
//	if (!tissueFile){
//		std::cout << "\n\nERROR with creating file, data not written to file";
//		return true;
//	}
//	volume = (double)phantomSize*(double)phantomSize - volume;
//	for(int i = 0; i < doseData.size(); i++) {
//		/* Writes the percentage volume that recieved at least i% dose. */
//		percentVol = (double)doseData[i]*(double)100/volume;
//		tissueFile << i << "\t" << percentVol << "\n";
//	}
//	if (disp) std::cout << "\n\nData written to : " << fileName;
	return true;
}


void weight(std::map<int, double>& weight, map2D& doseData, int beams, int max, int min, int spacing, int phantomSize, double maxError) {
	/*
	* Returns nothing.  Writes weights to the file "weights" required by
	* calculateDose().  Itteratively calculates Bragg Peak weights until
	* the dose is within maxError% across the entire SOBP.
	*/
	if (disp) std::cout << "\n\nPlease Wait.\n";
	if(doseData.size() < max) {
		std::cout << "\n\nERROR input File first";
		return;
	}
	std::map<int, double> depthDose;
	std::map<int, double> noDose;
	std::map<int, double> doseInX;
	for (int i = 0; i < phantomSize; i++){
		weight[i] = 1;
	}
	double minDose, maxDose, error;
	if (max > phantomSize || min < 0 || max < min) {
		std::cout << "\n\nMax or Min value out of Range Error, max: " << max << " min: " << min;
		return;
	}
	for (int depth = max; depth >= min; depth = depth -5) {
		/* Sets initial weights, does not take into account the
		* dose delivered from Peaks of lower energy.
		*/
		weight[depth] = (100 - depthDose[depth]) / (double)100;
		if (weight[depth] < 0 || weight[depth] > 1) {
			std::cout << "\nWeight Error :" << weight[depth];
			break;
		}
		for (int x = 0; x < phantomSize; x++) {
			if (weight[depth] * doseData[depth][x] < 0)
				std::cout << "error";
			depthDose[x] = depthDose[x] + weight[depth] * doseData[depth][x];
		}
	}
	weight[max] -= 0.18; /* Weight of max peak is adjusted to reduce the required itterations. */
	for (int i = 0; i < 1000; i++) {
		/* Itterates until min Error is reached or 1000 itterations */
		std::map<int, double> depthDoseWorking;
		for (int i = 0; i < phantomSize; i++){
			depthDoseWorking[i] = 0;
		}
		for (int x = max; x >= min; x -= spacing) {
			for (int i = 0; i < (max+20); i++) {
				depthDoseWorking[i] += weight[x]*doseData[x+1][i];
			}
		}
		minDose = 200;
		maxDose = 0;
		for (int x = min; x < max; x++) {
			if (depthDoseWorking[x] < minDose)
				minDose = depthDoseWorking[x];
			if (depthDoseWorking[x] > maxDose)
				maxDose = depthDoseWorking[x];
		}
		error = maxDose - minDose;
		if (error < maxError) {
			if (disp) std::cout << "\nMin Error reached";
			depthDose = depthDoseWorking;
			break;
		}
		for (int x = max; x >= min; x -= spacing) {
			if (depthDoseWorking[x] < 100) {
				weight[x] = weight[x] + (100 - depthDoseWorking[x]) / (double)500;
			}
			else {
				weight[x] = weight[x] - (depthDoseWorking[x] - 100) / (double)500;
			}
			if (weight[x] < 0 || weight[x] > 2) {
				std::cout << "\nWeight Error :" << weight[x] << " x: " << x << " dose: " << depthDoseWorking[x];
				return;
			}
		}
		depthDose = depthDoseWorking;
	}
	std::string fileName = "weights";
	std::ofstream outFile( fileName.c_str() );
	outFile.precision(8);
	for (int i = 0; i < weight.size(); i++)
		outFile << weight[i] << "\n";
}


void normalise(map3D& dose) {
	/*
	* Normalises the dose to a maximum of 100%
	*/
	if (disp) std::cout << "\nNormalising dose distribution, Please Wait\n";
	double max = 0;
	std::map<int, std::map<int, std::map<int, double> > >::const_iterator 	xItterator;
	std::map<int, std::map<int, double> >::const_iterator 					yItterator;
	std::map<int, double>::const_iterator 									zItterator;
	for (xItterator = dose.begin(); xItterator != dose.end(); xItterator++) {
		int x = xItterator->first;
		for (yItterator = dose[x].begin(); yItterator != dose[x].end(); yItterator++) {
			int y = yItterator->first;
			for (zItterator = dose[x][y].begin(); zItterator != dose[x][y].end(); zItterator++) {
				if (zItterator->second > max)
					max = zItterator->second;
			}
		}
	}
	max = max / (double)100;
	for (xItterator = dose.begin(); xItterator != dose.end(); xItterator++) {
		int x = xItterator->first;
		for (yItterator = dose[x].begin(); yItterator != dose[x].end(); yItterator++) {
			int y = yItterator->first;
			for (zItterator = dose[x][y].begin(); zItterator != dose[x][y].end(); zItterator++) {
				int z = zItterator->first;
				dose[x][y][z] = dose[x][y][z]/max;
			}
		}
	}
	if (disp) std::cout << "\nDose normalised to 100% at the maximum, Max was: " << max << "\n";
}


void addMotion(ScanPattern::ScanPattern& SP, const scanSpeed& speed, const Motion& m) {
	//Move spot positions according to the defined motion
}


void addSpot(map3D& phantom, spotPos position, map2D& braggPeaks, map3D& penumbra) {
	/*
	* Add a spot to the given location, calculated up to 40mm either side of the beam and 40mm past the end of the peak
	*/
	int depth = position.z;
	for (int z = 0; z <= depth + 40; z++) {
		double peak = braggPeaks[depth][z] * position.weight;
		for (int x = 0; x < 40; x++) {
			for (int y = 0; y <= 40; y++) {
				double dose = peak * penumbra[z][x][y];
				//Symetrical beam so the dose delivered is the same for all four points around the spot
				phantom[position.x + x][position.y + y][z] += dose;
				phantom[position.x + x][position.y - y][z] += dose;
				phantom[position.x - x][position.y + y][z] += dose;
				phantom[position.x - x][position.y - y][z] += dose;
			}
		}
	}
//std::cout << "\n Spot Added \n";
}


void calculateDose(map3D& phantom, ScanPattern::ScanPattern SP, map2D& braggPeaks, map3D& penumbra) {
	/*
	* Adds single proton beam spots according to the scanning pattern
	* ScanPattern SP given as an argument.
	*/
	if (disp) std::cout << "\n\nPlease Wait.\n";
	spotPos spot;
	SP.reset();
	spot = SP.getSpot();
	while (spot.weight >= 0) {
		addSpot(phantom, spot, braggPeaks, penumbra);
		spot = SP.getNextSpot();
	}
	if (disp) std::cout << "Dose Calculated\n";
}


map3D calcPenumbra(map2D& braggPeaks, int maxRange) {
	/*
	* Returns the Beam penumbra for all depths up to maxRange
	* Requires file "protonEnergymm".
	* Penumbra is calculated from a maximum of 40 mm from the central axis
	* Penumbra is 0 for > 40 mm from the central axis.
	* Uses Formula from M. Lee et. al. 1993.
	*/

	if (disp) std::cout << "\nCalculating Penumbra, Please Wait\n";
	double M = 938.3;  						/* Proton rest mass (MeV) */
	double sqrtpi = sqrt(2 * 3.141592654);
	double L = 500;						/* radiation length of water in mm */
	double pv;								/* Momentum times velocity of the proton */
	std::map<int, double> re; 				/* Range Energy */
	double SDz;								/* Standard Deviation*/
	double normalisation;					/* To normalise the profile to equal 1 on axis */
	map3D Pmono;
	input(re, "protonEnergymm");			/* Input the range energy information from a file */
	for (int z = 1; z <= maxRange; z++) {
		pv = re[z] * (re[z] + (double)2 * M)/ (re[z] + M);
		double sqrtPart = (double)(z*z*z)/(double)3/L/(pv*pv);
		SDz = 14.1 * (1 + (double)1/(double)9 * log10(z/L)) * sqrt(sqrtPart);
		double TwoSdSquared = (double)2 * SDz * SDz;
		for (int x = 0; x <= 40; x++) {		/* Calculates the penumbra up to 40 mm in both x and y directions */
			for (int y = x; y <= 40; y++) {
				double distance = sqrt(x*x + y*y);
				double integral = 0;
				for (int i = -1000; i <= 0; i++) {
					double X = (double)i/(double)100;
					double temp =  (X * X + distance * distance - (double)2 * X * distance)/ TwoSdSquared;
					integral += exp(-temp)/100;    /* Integrating by summing in 100 elements per mm */
				}
				double value = integral / ( sqrtpi * SDz );
				if (x == 0 && y == 0)
					normalisation = value;
				Pmono[z][x][y] = value/normalisation;     /* Penumbra is normalised to 1 on the axis */
				Pmono[z][y][x] = value/normalisation;     /* penumbra at (x, y) == penumbra at (y, x) */
			}
		}
	}
	return Pmono;
}


map2D calcPeaks(int minRange, int maxRange, double sd) {
	/*
	* Returns the depth dose for Bragg peaks with maximum ranges
	* from minRange to maxRange.  Uses Formula from M. Lee et. al. 1993.
	* Calls integrate(std::map<int, double>) for integration.
	* Requires file "energylossmm".
	*/
	int maxCalc = maxRange +10;
	std::map<int, double> janniData;				/* janniData[R] is the energy loss per mm for a proton with range R. */
	map2D Dmono, Dele;						/* Dmono[R][Z] is Dmono(R,Z) in eqn 1 similarly Dele in eqn 3*/
	std::map<int, double> fx;					/* fx is the integrand in eqn 3 from Lee et. al. */
	input(janniData, "energylossmm");
	if (disp) std::cout << "\n\n\nPlease Wait.\n";
	for (int R = 0; R < maxCalc; R++) {
		double denomintor = (double)1 / (0.0012*R+1);
		for (int dist = 0; dist <= R; dist++){
			(Dmono[R])[R-dist] = janniData[dist] * (0.0012*dist+1) * denomintor;
		}
	}
	for (int Ro = minRange +3; Ro < maxRange +3; Ro++) {
		double dose, rrsd;
		for (int Z = 0; Z < maxCalc; Z++) {
			for (int R = 0; R < maxCalc;  R++) {
				rrsd = (R - Ro) * (R - Ro) / sd;
				fx[R] = exp(-rrsd) * Dmono[R][Z];
			}
			Dele[Ro -3][Z] = integrate(fx);
		}
	}
	if (disp) std::cout << "\nBragg Peaks Calculated from " << minRange << "mm to " << maxRange << "mm.";
	return Dele;
}


sMap calcDoseVol(map3D& dose, int beams, std::vector<int>& movement, int targetSize, int phantomSize, std::vector<double>& maxMin) {
	/*
	* Returns two vectors containing DVHs for the target and tissue.
	* DVH["target"][X] = percent Volume recieving at least X% Dose
	*/
	sMap DVH;
	if(dose.size() < phantomSize) {
		std::cout << "\n\nERROR calculate dose first";
		return DVH;
	}
	std::vector<int> targetDose(120);
	std::vector<int> tissueDose(120);
	maxMin[0] = 0;
	maxMin[1] = 120;
	/* tissueDose[i] is the number of cubic mm with percentage depth dose i */
	int xRange[2];
	int yRange[2];
	int zRange[2];
	/* Range in x and y that contains the target */
	xRange[0] = phantomSize/2 - targetSize/2;
	xRange[1] = phantomSize/2 + targetSize/2;
	yRange[0] = phantomSize/2 - targetSize/2;
	yRange[1] = phantomSize/2 + targetSize/2;
	zRange[0] = phantomSize/2 - targetSize/2;
	zRange[1] = phantomSize/2 + targetSize/2;
	for (int x = xRange[0]; x < xRange[1]; x++) {
		/* Loops throught all the points inside the target */
		for (int y = yRange[0]; y < yRange[1]; y++) {
			for (int z = zRange[0]; z < zRange[1]; z++) {
				if (dose[x][y][z] > maxMin[0])
					maxMin[0] = dose[x][y][z];
				if (dose[x][y][z] < maxMin[1])
					maxMin[1] = dose[x][y][z];
				int percentDose = (int)(dose[x][y][z] + 0.5);
				/* C++ always rounds down for int conversion, + 0.5 results
				*  in the 'normal' way to round a double to an int. */
				if (percentDose < 0 || percentDose >= 120)
					std::cout << "\n\nPercent dose out of Range Error 1: " << percentDose;
				else {
					for (int i = percentDose; i >= 0; i--)
						targetDose[i]++;
				}
			}
		}
	}
//	for (int y = 0; y < yRange[0]; y++) {
//		for (int x = 0; x < depthDose.size(); x++) {
//			/* Loops through all the points in the tissue with y < target position */
//			int percentDose = (int)(depthDose[y][x] + 0.5);
//			if (percentDose < 0 || percentDose >= 120)
//				std::cout << "\n\nPercent dose out of Range Error 2: " << percentDose;
//			else {
//				for (int i = percentDose; i > 0; i--)
//					tissueDose[i]++;
//			}
//		}
//	}
//	for (int y = yRange[0]; y < yRange[1]; y++) {
//		for (int x = 0; x < xRange[0]; x++) {
//			/* Loops through all the points in the tissue with y within the
//			* target position and x < target position. */
//			int percentDose = (int)(depthDose[y][x] + 0.5);
//			if (percentDose < 0 || percentDose >= 120)
//				std::cout << "\n\nPercent dose out of Range Error 3: " << percentDose;
//			else {
//				for (int i = percentDose; i > 0; i--)
//					tissueDose[i]++;
//			}
//		}
//		for (int x = xRange[1]; x < depthDose.size(); x++) {
//			/* Loops through all the points in the tissue with y within the
//			* target position and x > target position. */
//			int percentDose = (int)(depthDose[y][x] + 0.5);
//			if (percentDose < 0 || percentDose >= 120)
//				std::cout << "\n\nPercent dose out of Range Error 4: " << percentDose;
//			else {
//				for (int i = percentDose; i > 0; i--)
//					tissueDose[i]++;
//			}
//		}
//	}
//	for (int y = yRange[1]; y < depthDose.size(); y++) {
//		for (int x = 0; x < depthDose.size(); x++) {
//			/* Loops through all the points in the tissue with y > target position */
//			int percentDose = (int)(depthDose[y][x] + 0.5);
//			if (percentDose < 0 || percentDose >= 120)
//				std::cout << "\n\nPercent dose out of Range Error 5: " << percentDose;
//			else {
//				for (int i = percentDose; i > 0; i--)
//					tissueDose[i]++;
//			}
//		}
//	}
	DVH["tissue"] = tissueDose;
	DVH["target"] = targetDose;
	if (disp) std::cout << "\n\nDose Volume histogram calculated";
	return DVH;
}


bool setMovement(std::vector<int>& movement, std::vector<double>& intraMove) {
	/* User can change the amount of target movement */

	if (disp) std::cout << "\n\nMotion not working yet";
	return true;
}


bool variables(int& minRange, int& maxRange, double& sd, int& beams, int& size, int& phantomSize, double& error, int& margin){
	/*

	* Variables can be changed here, entering 'd' causes std::cin to fail

	* i.e. std::cin = false and all variables from that point on are set
	* to default values. std::cin is cleared in main().
	*/
	if (disp) std::cout << "\n\nEnter Phantom size (10-400mm) (Enter d for defaults): ";
	std::cin >> phantomSize;
	if(!std::cin || phantomSize < 10 || phantomSize > 400) {
		std::cout << "\nPhantom size set to default: 300mm";
		phantomSize = 300;
	}
	if (disp) std::cout << "\n\nEnter Target size (2-200mm): ";
	std::cin >> size;
	if(!std::cin || size < 2 || size > 200 || size > phantomSize -2) {
		std::cout << "\nSize set to default: 100mm";
		size = 100;
	}
	if (disp) std::cout << "\n\nEnter margin size (0-20mm): ";
	std::cin >> margin;
	if(!std::cin || margin < 0 || margin > 20) {
		std::cout << "\nMargin set to default: 10mm";
		margin = 10;
	}
	if (disp) if (disp) std::cout << "\n\nEnter the number of beams: ";
	std::cin >> beams;
	if(!std::cin || beams < 1 || beams > 3) {
		std::cout << "\n\nInvalid option, set to 2 beams";
		beams = 2;
	}
	if (disp) std::cout << "\n\nEnter minimum range (0-300mm): ";
	std::cin >> minRange;
	if(!std::cin || minRange < 0 || minRange > 300) {
		std::cout << "\nMin range set to default: 0mm";
		minRange = 0;
	}
	if (disp) std::cout << "\n\nEnter maximum range (10-400mm): ";
	std::cin >> maxRange;
	if(!std::cin || maxRange < 10 || maxRange > 400) {
		std::cout << "\nMax range set to default: 300mm";
		maxRange = 300;
	}
	if (disp) std::cout << "\n\nEnter SOBP standard deviation (1-100): ";
	std::cin >> sd;
	if(!std::cin || sd < 1 || sd > 100) {
		std::cout << "\nStandard deviation set to default: 10";
		sd = (double)10;
	}
	if (disp) std::cout << "\n\nEnter maximum error (1-10%): ";
	std::cin >> error;
	if(!std::cin || error < 1 || error > 20) {
		std::cout << "\nMaximum error set to default: 3%";
		error = (double)3;
	}
	return true;
}


int main(int argc, char* argv[]) {
	map2D braggPeaks;
	map3D penumbra;
	map3D phantom; /* The dose distribution in the phantom */
	ScanPattern::ScanPattern SP;
	Motion motion();
	std::map<int, double> weights;
	std::vector<int> movement(3);
	std::vector<double> intraMove(4);
	std::vector<double> maxMin(2);
	/* Default Intitial Conditions, can be changed during runtime. */
	int phantomSize = 30;
	int size = 100;
	int margin = 0;
	int beams = 1;
	int noPaintings = 1;
	double layerTime = 0.7073;
	double energyChangeTime = 2.0;
	double spotSeparation = 5.0;
	double spotSigma = 10.0;
	int minRange = 0;
	int maxRange = 310;
	int max = phantomSize/2 + size/2 + margin;
	int min = phantomSize/2 - size/2 - margin;
	double sd	= 10;
	double error = 2;
	if (argc == 1) {
		disp = true;
		inputAll(braggPeaks, maxRange);
		penumbra = calcPenumbra(braggPeaks, maxRange);
		//depthDose = calculateDose(phantom, SP);
	}
	else
		disp = false; /* Commands are run from a file no input/output during runtime */
	bool menu = true;
	while(menu) {
		/*
		* Displays the main user interface and reads in commands,
		* and executes the relevent functions, if EOF is
		* reached, the program exits.  If run from an input file
		* no output is desplayed to the terminal (disp = false), but
		* Errors are always output to the terminal
		*/
		std::string cmd, temp;
		if (std::cin.eof())
			break;
		else if (std::cin.fail()) {  /* If an invalid input has been entered the input buffer is cleared here. */
			std::cin.clear();
			std::cin >> cmd;
		}
		else {
			if (disp){
				std::cout << "\n\n\nMAIN MENU\n\nPlease Input your option\n ";
				std::cout << "\n  1. Calculate Proton Depth Dose from Janni data";
				std::cout << "\n  2. Change variables";
				std::cout << "\n  3. Output a single Bragg Peak";
				std::cout << "\n  4. Calculate Dose";
				std::cout << "\n  5. Output File";
				std::cout << "\n  6. Dose Volume Histogram";
				std::cout << "\n  7. Move the Target";
				std::cout << "\n  8. Create ScanPattern";
				std::cout << "\n  9. EXIT";
				std::cout << "\n\nCommand (1-9) ?";
			}
			else
				std::cout << "\n" << cmd;
			std::cin >> cmd;
			if (std::cin.eof())
				break;
			else if (std::cin.fail()) {
				std::cin.clear();
				std::cin >> cmd;
			}
			else if (cmd == "1" || cmd == "recalculatePeaks") {
				braggPeaks = calcPeaks(minRange, maxRange, sd);
				penumbra = calcPenumbra(braggPeaks, maxRange);
			}
			else if (cmd == "2" || cmd == "setVariables") {
				int oldMax = maxRange;
				menu = variables(minRange, maxRange, sd, beams, size, phantomSize, error, margin);
				max = phantomSize/2 + size/2 + margin;
				min = phantomSize/2 - size/2 - margin;
				if (maxRange > oldMax)
					if (disp) std::cout << "\nmaxRange > default.  Recalculate Depth Dose (Option 1).";
			}
			else if (cmd == "3" || cmd == "outputPeak")
				menu = outputPeak(braggPeaks);
			else if (cmd == "4") {
				if ( maxRange != braggPeaks.size() )
					std::cout << "\nmaxRange != braggPeaks.size(), recalculate peaks) " << maxRange << " " << braggPeaks.size();
				else {
					// if (weights.size() == 0) weight(weights, braggPeaks, beams, max, min, (int)spotSeparation, phantomSize, error);
					if (SP.numberLayers() == 0) SP.defineScanPattern();
std::cout << " \n layers " << SP.numberLayers();
					calculateDose(phantom, SP, braggPeaks, penumbra);
					normalise(phantom);
				}
			}
			else if (cmd == "5" || cmd == "writeFile")
				menu = writeFile(phantom);  //only writes one plane at the moment
			else if (cmd == "6" || cmd == "histogram") {
				sMap doseVol = calcDoseVol(phantom, beams, movement, size, phantomSize, maxMin);
				writeHistogram(doseVol, phantomSize, size, beams, maxMin);
			}
			else if (cmd == "7" || cmd == "setMovement")
				setMovement(movement, intraMove);
			else if (cmd == "8" || cmd == "setPattern") {
				SP.defineScanPattern();
			}
			else if (cmd == "9" || cmd == "exit")
				break;
			else if (cmd == "op" || cmd == "outputPenumbra")
				menu = outputPeak(penumbra[0]); //This prob doesnt work at the moment
			else if (cmd == "o" || cmd == "outputAll")
				menu = outputAll(braggPeaks, maxRange);
			else if (cmd == "c" || cmd == "calcDose")
				calculateDose(phantom, SP, braggPeaks, penumbra);
			else if (cmd == "p" || cmd == "calcPenumbra")
				penumbra = calcPenumbra(braggPeaks, maxRange);
			else if (cmd == "i" || cmd == "inputAll")
				inputAll(braggPeaks, maxRange);
			else if (cmd == "w" || cmd == "weight")
				weight(weights, braggPeaks, beams, max, min, (int)spotSeparation, phantomSize, error);
			else if (cmd == "n" || cmd == "normalise")
				normalise(phantom);
			else
				std::cout << "\n\nInvalid input. " << cmd;
		}
	}
}

/*
* Uses Formulas from:
* Lee M., Nahum A. E., Webb S.,  1993.
* An Empirical Method to Build up a Model of Proton Dose Distribution for a
* Radiotherapy Treatment-Planning Package.
* Phys. Med. Biol. 38  989-998
* and data from:
* Janni J. F. 1982.
* Proton Range-Energy Tables for 63 Compounds.
* Atomic Data and Nuclear Data Tables.  27 338-339
*/
