#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>
#include "../include/Dataset.h"

namespace fs = std::filesystem;
using namespace std;

int dirExists(const char* const path)
{

    cout << "checking if " << path << " exists..." << endl;
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

int main(int argc, char** argv){

	if (argc == 1){
	
		cout << "Error: No argument entered" << endl;
		cout << "options: " << endl;
		cout << "	-init: initialize a new dataset and enter the augmentation options" << endl;
		cout << "	-run: runs the data augmentation algorithm" << endl;
		return 0;
	}

	for (int i = 1; i < argc; ++i){

        	string command  = argv[i];

		// check the command entered by the user
		if (command.compare("-run") == 0){

			string path;

			// if user specified a path
			if (argc > 2){
				path = argv[i+1];

				// add a / if there are none at the end.
				if (path.substr(path.length() -1).compare("/") != 0){
			
					path = path + "/";
				}
		
			}

			// default to local path
			else{
				path = "./";
			
			}

			// run the dataset
			try{
        				
				Dataset dataset = Dataset(path);

			}catch(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >& e){
			
				cout << e << endl;		
			}catch(std::out_of_range& e){

				cout << e.what() << endl;
				return -1;
			}
			return 0;
		
		}else if(command.compare("-init") == 0){
			
			i ++;	
			string path = argv[i];

			// create the main dir and write the yeet file

			string name;
			string output;
			string background;
			string input;
			int maxObjects;
			int canvases;
			int lowRes;
			int blur;
			int bright;
			int sat;
			int affine;

			
			if(dirExists(path.c_str())){

				char end = path.back();
				const char* comp = "/";
				if (end != *comp) {
			
					// add a '/' at the end 
					path = path + "/";
				}


				cout << "Enter a name for your dataset: ";
				cin >> name;

				
				path = path + name;
				output = path+"/output";
				input = path+"/input";


				cout << "> creating directory " << path << endl;
				if(fs::create_directory(path)){

					cout << "> created " << path << endl;

				}else{

					cout << "> could not create " << path << endl;
					exit(EXIT_FAILURE);
				}

				if(fs::create_directory(input)){

					cout << "> created " << input << endl;

				}else{

					cout << "> could not create " << input << endl;
					exit(EXIT_FAILURE);
				}

				if(fs::create_directory(output)){

					cout << "> created " << output << endl;

				}else{

					cout << "> could not create " << output << endl;
					exit(EXIT_FAILURE);
				}



			}else{
				cout << "> Please provide an existing directory for your dataset: "  << path << " not found" << endl;
				exit(EXIT_FAILURE);

			}

			// write the yeet file
			
			cout << "Enter the path to the background files:";
			cin >> background;

			cout << "Enter maximum objects per canvases:";
			cin >> maxObjects;

			cout << "Enter the number of canvases per frames:";
			cin >> canvases;
		
			cout << "% chance to lower the canvas resolution:";
			cin >> lowRes;

			cout << "% chance to blur the canvas:";
			cin >> blur;

			cout << "% chance to change the brightness:";
			cin >> bright;

			cout << "% chance to change the saturation of an object:";
			cin >> sat;

			cout << "% chance to affine transform an object:";
			cin >> affine;

			// create the yeet file
			string savePath = input + "/dataset_config.yeet";
			ofstream yeetFile(savePath);

			// Write to the file

			yeetFile << "dataset_name=" + name << "\n";
			yeetFile << "output_path=" + output << "\n";
			yeetFile << "background_path=" + background << "\n";
			yeetFile << "max_objects_per_canvas=" + to_string(maxObjects) << "\n";
			yeetFile << "canvases_per_frame=" + to_string(canvases) << "\n";
			yeetFile << "canvas_lower_resolution=" + to_string(lowRes) << "\n";
			yeetFile << "canvas_blurr=" + to_string(blur) << "\n";
			yeetFile << "canvas_change_brightness=" + to_string(bright) << "\n";
			yeetFile << "object_saturation=" + to_string(sat) << "\n";
			yeetFile << "object_affine_transform=" + to_string(affine) << "\n";

			// Close the file
			yeetFile.close();




		}else{
		
			cout << "Error: wrong command" << endl;
		}
	}

        return 0;
}



