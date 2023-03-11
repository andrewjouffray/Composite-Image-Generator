#ifndef DATASET_H
#define DATASET_H

#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <chrono>
#include <sys/stat.h>
#include "../vendors/tinyxml/tinyxml.h"
#include "../vendors/tinyxml/tinystr.h"
#include <algorithm>
#include <random>
#include "./Label.h"

namespace fs = std::filesystem;
using namespace std;

class Dataset{

public:

        // all the int values represent percentages
        int obj_affineProb = 30;
        int obj_changeSatProb = 10;

        int can_changeBrightProb = 10;
        int can_blurrProb = 10;
        int can_lowerRes = 10;

        int canvas_per_frame = 4;
        int max_objects = 5;

		// annotation save options
		bool save_masks_png = true;
		bool save_masks_json = true;
		bool save_bnd_box = true;

        // paths and names of folders
        string outputPath;
        string datasetName;
		string inputPath;
		string backgroundPath;
		vector<string> labels;
		vector<string> backgrounds;
        string masks_json;
		string masks_png;
        string imgs;
        string xml;


	Dataset(string pathToYeet);

	vector<string> getLabelFiles(string path);

	bool createOutputDirs();

	// checks if a directory exists
	int dirExists(const char* const path);

	// saves all the names of the files in a given path
	vector<string> getFiles(string path, vector<string> extentions);

	// returns the list of all .avi or .mp4 files to be run my the augmentation algorithm.
	vector<string> getLabels();

	// returns a shuffled list of all the .jpg files to be used as a background.
	vector<string> getBackgrounds();

	string splitPath(string line);

	vector<vector<string>> parseFile(string pathToYeet);

	void setSettings (vector<vector<string>> file);

	void create_label_map();

};

# endif
