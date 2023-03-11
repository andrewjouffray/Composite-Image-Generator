/*
author: Andrew Jouffray

The config class ise used by the dataset class to load the configuration of each of it's label

*/

#include <opencv2/opencv.hpp>
#include <iostream>
#include <omp.h>
#include <fstream>
#include <string>
#include <filesystem>
#include <chrono>
#include "./inculde/andomFunc.h"
#include "./include/Canvas.h"
#include <sys/stat.h>
#include "./vendors/tinyxml/tinyxml.h"
#include "./vendors/tinyxml/tinystr.h"


namespace fs = std::filesystem;
using namespace std;

class Config{

public:

        // all the int values represent percentages
        int obj_affineProb = 30;
        int obj_changeSatProb = 10;

        int can_changeBrightProb = 10;
        int can_blurrProb = 10;
        int can_lowerRes = 10;

        int canvas_per_frame = 4;
        int max_objects = 5;

        // paths and names of folders
        string labelName;
        string outputPath = fs::current_path() + "/data/output/"; // need to add the dataset name as the label
        string datasetName;
	String inputPath;
	String backgroundPath;
	vector<String> backgrounds;
	vector<String> inputs;
        string masks;
        string imgs;
        string xml;

Config(string pathToYeet){ // load the config from yeet file

	// reads the config file and sets all the parameters
	vector<vector<string>> file = parseFile(pathToYeet);
	setSettings(file);

	// gets the list of inputs and background files
	backgrounds = getBackgrounds();
	inputs = getInputs();

	// creates the output directories to save everything in
	bool valid = createOutputDirs();

	// for each input file
	for(string inFile : inputs){

		// make sue the input is a full path not just 
		cout << inFile << endl;


	}



}


// checks if a directory exists
int dirExists(const char* const path)
{
    struct stat info;

    int statRC = stat( path, &info );
    if( statRC != 0 )
    {
        if (errno == ENOENT)  { return 0; } // something along the path does not exist
        if (errno == ENOTDIR) { return 0; } // something in path prefix is not a dir
        return -1;
    }

    return ( info.st_mode & S_IFDIR ) ? 1 : 0;
}

// saves all the names of the files in a given path
vector<string> getFiles(string path){

        //cout << "adding " + path << endl;
        // this is it for now
        vector<string> files;

        for(const auto & entry : fs::directory_iterator(path)){
                string it = entry.path();

                //cout << it << endl;
                files.push_back(it);
        }

        return files;


}


// returns the list of all .avi or .mp4 files to be run my the augmentation algorithm.
vector<string> getInputs(){

        vector<string> inputs;

        if (dirExists(inputPath.c_str())){
                inputs = getFiles(inputPath);
        }else{
                cout << "> Label: error could not find path " << inputPath << endl;
                throw "> Error: Invalid input path.";
        }

        return inputs;
}

// returns a shuffled list of all the .jpg files to be used as a background.
vector<string> getBackgrounds(){

        vector<string> backgrounds;

        if (dirExists(backgroundPath.c_str())){
                backgrounds = getFiles(backgroundPath);
        }else{
                cout << "> Label: error could not find path " << backgroundPath << endl;
                throw "> Error: Invalid background image path.";
        }

        // shuffle the background images around 
        auto rng = default_random_engine {};
        shuffle(begin(backgrounds), end(backgrounds), rng);

        return backgrounds;


}

// parses the .yeet file into a vector of string vectors
vector<vector<string>> parseFile(string pathToYeet)
{

        string line;
        ifstream myfile (pathToYeet);
        vector<vector<string>> parsedFile;
        if (myfile.is_open()){
                while ( getline (myfile,line) ){
                        //cout << line << '\n';

                        std::string delimiter = " ";

                        vector<string> splitLine;
                        size_t pos = 0;
                        std::string token;
                        while ((pos = line.find(delimiter)) != std::string::npos) {
                                token = line.substr(0, pos);
                                // check if empty char
                                if (token.compare(" ") != 0 && token.compare("") != 0){
                                        splitLine.push_back(token);
                                }
                                line.erase(0, pos + delimiter.length());
                        }
                        if (token.compare(" ") != 0){
                                splitLine.push_back(line);
                        }
                        parsedFile.push_back(splitLine);
                }
         myfile.close();
 
        }
        else{ cout << "Unable to open file";

        }

        return parsedFile;
}

// goes through each line and reads the config
void setSettings (vector<vector<string>> file){

        for(vector<string> line : file){

                // get the first word of the line
                string word = line.at(0);

                if(word.compare("dataset_name") == 0){

                        datasetName = line.at(2);

                }else if(word.compare("label_name") == 0){

                        labelName = line.at(2);
                }
                else if(word.compare("output_path") == 0){

                        outputPath = line.at(2);
                }
                else if(word.compare("background_path") == 0){

                        backgroundPath = line.at(2);
                }
                else if(word.compare("max_objects_per_canvas") == 0){

                        objects = stoi(line.at(2));
                }
                else if(word.compare("canvases_per_frame") == 0){

                        mult = stoi(line.at(2));
                }
                else if(word.compare("canvas_blurr") == 0){

                        can_blurrProb = stoi(line.at(2));
                }
                else if(word.compare("object_saturation") == 0){

                        obj_changeSatProb = stoi(line.at(2));
                }
                else if(word.compare("canvas_lower_resolution") == 0){

                        can_lowerRes = stoi(line.at(2));
                }
                else if(word.compare("canvas_change_brightness") == 0){

                        can_changeBrightProb = stoi(line.at(2));
                }
                else if(word.compare("object_affine_transform") == 0){

                        obj_affineProb = stoi(line.at(2));
                }
                else if(word.compare("//") == 0){

                        //do nothing
                }


        }
	
	if (outputPath.compare("default")){
	
		outputPath = fs::current_path() + "/data/output/" + datasetName + "/";
	}

	// three folders to be created
        masks = outputPath + "masks/";
        imgs = outputPath + "imgs/";
        xml = outputPath + "xml/";

	// prints out all the settings allowing the user to check that everything is ok
        cout << "\n========================= Label Configuration ===================================" << endl;
        cout << "> readFile: path to background:                             " << pathToBackground << endl;
        cout << "> readFile: output path:                                    " << output << endl;
        cout << "> readFile: dataset name:                                   " << dataset << endl;
        cout << "> readFile: label name:                                     " << label << endl;
        cout << "> readFile: number of canvases created per video frame:     " << mult << endl;
        cout << "> readFile: max number of objects to be put in each canvas: " << objects << endl;
        cout << "\n========================= Canvas Modification ===================================" << endl;
        cout << "> readFile: chances of blurring canvas:                     " << can_blurrProb << "%" << endl;
        cout << "> readFile: chances of lowering the canvas resolution:      " << can_lowerRes << "%" << endl;
        cout << "> readFile: chances of changing the canvas brightness:      " << can_changeBrightProb << "%" << endl;
        cout << "\n========================= Object Modification ===================================" << endl;
        cout << "> readFile: chances of changing object color saturation:    " << obj_changeSatProb << "%" << endl;
        cout << "> readFile: chances of changing object affine transform:    " << obj_affineProb << "%" << endl;

}





}

// load config file specific to it
//
// load all backgrounds
//
// iterate over each video files
//
// for each frame, segment it 
//
// create a x canvases
//
// save them
