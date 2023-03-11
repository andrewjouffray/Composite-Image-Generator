#ifndef OOI_H
#define OOI_H


#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <chrono>
#include <random>
#include "randomFunc.h"

using namespace std;

class Ooi{

public: 
	int ooiHeight;
	int ooiWidth;
	int xAbsolutePos;
	int yAbsolutePos;
	int xOffset;
	int yOffset;
	int randomValue;
	float maxScale;
	float minScale;
	bool debug = false;
	cv::Mat image;
	vector<int> positions;

	Ooi(cv::Mat objectOfInterest, int colWidth, int colHeight, int xAbsolutePos, int affineProb, int saturationProb, bool debug);

	void rotate(int angle);

	void scaleImage(float scale);

	void affineTransform();

	void changeSaturation();	

	vector<int> getPosition();

	cv::Mat getObject();
		


};

#endif
