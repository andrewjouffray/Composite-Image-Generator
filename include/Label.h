#ifndef LABEL_H
#define LABEL_H

#include <opencv2/opencv.hpp>
#include <iostream>
#include <omp.h>
#include <fstream>
#include <string>
#include <filesystem>
#include <chrono>
#include "./randomFunc.h"
#include "./Canvas.h"
#include <sys/stat.h>
#include "../vendors/tinyxml/tinyxml.h"
#include "../vendors/tinyxml/tinystr.h"
#include <thread>
#include<jsoncpp/json/writer.h>

namespace fs = std::filesystem;
using namespace std;

class Label{

public:

        int obj_affineProb;
        int obj_changeSatProb;
        int can_changeBrightProb;
        int can_blurrProb;
        int can_lowerRes;
        int canvas_per_frame;
        int max_objects;

        // paths and names of folders
        string labelName;
        string outputPath;
        string datasetName;
	vector<string>*	 backgrounds;
	vector<string>* inputs;
	string masks_png;
	string masks_json;
        string imgs;
        string xml;
        
        bool save_bnd_box;
        bool save_masks_png;
        bool save_masks_json;

	bool debug;

Label(string label, string dataset, string output, int affine, int saturation, int bright, int blurr, int lowRes, int canvasQt, int max_obj, vector<string>* input, vector<string>* background,bool save_bnd_box, bool save_masks_png, bool save_masks_json, bool debugArg);

void saveImg(cv::Mat img, string name);

void saveMask(cv::Mat mask, string name);

void saveXML(vector<vector<int>> rois, string name, cv::Mat img);

void saveJson(vector<vector<cv::Point>> contours, cv::Mat img, string name);

cv::Mat getRandomBackground();

uint64_t timeSinceEpochMillisec();


};

#endif
