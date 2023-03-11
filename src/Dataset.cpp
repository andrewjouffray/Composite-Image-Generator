
#include "../include/Dataset.h"

Dataset::Dataset(string pathToDataset){ // load the config from yeet file

        // this sets the input path to a default value of "./"
	Dataset::inputPath = pathToDataset;

	// reads the config file and sets all the parameters
	vector<vector<string>> file = Dataset::parseFile(Dataset::inputPath);
	Dataset::setSettings(file);

	// gets the list of inputs and background files
	Dataset::backgrounds = Dataset::getBackgrounds();

	// gets all the Dataset::labels
	Dataset::labels = Dataset::getLabels();

	// creates the output directories to save everything in
	bool valid = Dataset::createOutputDirs();

        // creates the label map later used for training and testing
        create_label_map();

	for (string label_path : Dataset::labels){

	
		string labelName = Dataset::splitPath(label_path);

		cout << "============== creating Label object =========" << endl;
		cout << "Label " << labelName << endl;
		cout << "Dataset " << Dataset::datasetName << endl;
		cout << "Input path " << label_path << endl;
		cout << "Output path " << Dataset::outputPath << endl;
		cout << "Object affine " << Dataset::obj_affineProb << endl;
		cout << "obj_changeSatProb " << Dataset::obj_changeSatProb << endl;
		cout << "can_changeBrightProb " << Dataset::can_changeBrightProb << endl;
		cout << "can_blurrProb " << Dataset::can_blurrProb << endl;
		cout << "can_lowerRes " << Dataset::can_lowerRes << endl;
		cout << "canvas_per_frame " << Dataset::canvas_per_frame << endl;
		cout << "max_objects " << Dataset::max_objects << endl;
		cout << "===== video files to be augmented ======" << endl;


		vector<string> videoFiles = Dataset::getLabelFiles(label_path);
		cout << videoFiles.size() << endl;

		Label labelObject = Label(labelName, Dataset::datasetName, Dataset::outputPath, Dataset::obj_affineProb , Dataset::obj_changeSatProb, Dataset::can_changeBrightProb, Dataset::can_blurrProb, Dataset::can_lowerRes, Dataset::canvas_per_frame, Dataset::max_objects, &videoFiles, &backgrounds, Dataset::save_bnd_box, Dataset::save_masks_png, Dataset::save_masks_json, false);

	}

}

// gets only the .avi and .mp4 files in a path
vector<string> Dataset::getLabelFiles(string path){

	vector<string> ext;
	string avi = "avi";
	string mp4 = "mp4";
	ext.push_back(avi.c_str());
	ext.push_back(mp4.c_str());

	vector<string> videoFiles;
       
	videoFiles = Dataset::getFiles(path, ext);

	int size = videoFiles.size();

	if (size == 0){

		string message = "no valid input files at ";
		string error = message + path;
		throw error; 
	}else{
	
		cout << "Found " << size << " valid files at: " << path << endl;
	}

	return videoFiles;

}

// checks and creates all the output directories for a Label
bool Dataset::createOutputDirs(){

	cout << "Creating the output directories..." << endl;

        if (!dirExists(Dataset::outputPath.c_str())){

                cout << "> Label: " << Dataset::outputPath << " does not exist." << endl;

                if(fs::create_directory(Dataset::outputPath)){
                        cout << "> Label: created " << Dataset::outputPath << endl;
                }else{

                        cout << "> Label: could not create " << Dataset::outputPath;
			string message = "Error: Could not create output path.";
                        throw message;
                }

        }

        if(dirExists(Dataset::masks_png.c_str())){
                cout << "> Output for Dataset::masks_png/ already exists, using existing directory" << endl;
        }else{

                if(fs::create_directory(Dataset::masks_png)){
                        cout << "> Label: created " << Dataset::masks_png << endl;
                }else{

                        cout << "> Label: could not create " << Dataset::masks_png << endl;
			string message = "Error: Could not create masks_png path.";
                        throw message;
                }

        }

        if(dirExists(Dataset::masks_json.c_str())){
        cout << "> Output for Dataset::masks_json/ already exists, using existing directory" << endl;
        }else{

                if(fs::create_directory(Dataset::masks_json)){
                        cout << "> Label: created " << Dataset::masks_json << endl;
                }else{

                        cout << "> Label: could not create " << Dataset::masks_json << endl;
			string message = "Error: Could not create masks_json path.";
                        throw message;
                }

        }

        if(dirExists(Dataset::imgs.c_str())){
                cout << "> Output for Dataset::imgs/ already exists, using existing directory" << endl;
        }else{

                if(fs::create_directory(Dataset::imgs)){
                        cout << "> Label: created " << Dataset::imgs << endl;
                }else{

                        cout << "> Label: could not create " << Dataset::imgs << endl;
			string message = "Error: Could not create imgs path.";
                        throw message;

                }

        }

        if(dirExists(Dataset::xml.c_str())){
                cout << "> Output for Dataset::xml/ already exists, using existing directory" << endl;
        }else{

                if(fs::create_directory(Dataset::xml)){
                        cout << "> Label: created " << Dataset::xml << endl;
                }else{

                        cout << "> Label: could not create " << Dataset::xml << endl;
			string message = "Error: Could not create xml path.";
                        throw message;
                }

        }

        return true;



}

// checks if a directory exists
int Dataset::dirExists(const char* const path)
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

// saves all the names of the files in a given path that are of a specified extentions
vector<string> Dataset::getFiles(string path, vector<string> extentions){

        vector<string> files;

        for(const auto & entry : fs::directory_iterator(path)){
                string it = entry.path();

		for (string ext : extentions){
		
			string name = Dataset::splitPath(it);

			//cout << name.substr(name.length() - 3) << " and " << ext << endl;

			if(name.substr(name.length() - 3).compare(ext) == 0){	

                		files.push_back(it);
			}else if(ext.compare("none") == 0){
			
				files.push_back(it);

			}
		}

        }

        return files;


} 

// returns the list of all .avi or .mp4 files to be run my the augmentation algorithm.
vector<string> Dataset::getLabels(){

	cout << "Getting the Labels " << endl;

        vector<string> labels;

	vector<string> files;

	vector<string> ext;
	ext.push_back("none");

        if (dirExists(Dataset::inputPath.c_str())){
                files = getFiles(Dataset::inputPath, ext);
        }else{
                cout << "> Label: error could not find label path " << Dataset::inputPath << endl;
                string message = "> Error: Invalid input path.";
		throw message;
        }

	for (string path : files){
	
	
		if (path.length() > 5 && path.substr(path.length() - 4).compare("yeet") == 0){
		
			cout << "Dataset: Ignoring the yeet file" << endl;
		}else{
		
			labels.push_back(path);
			cout << "Dataset: Added label " << path << endl;
		
		}
	}

	int size = labels.size();

	if (size == 0){

		string message = "no valid Labels at ";
		string error = message + Dataset::inputPath;
		throw error; 
	}else{
	
		cout << "Found " << size << " Labels"  << endl;
	}

        return labels;
}

// returns a shuffled list of all the .jpg files to be used as a background.
vector<string> Dataset::getBackgrounds(){

        vector<string> backgrounds;
	vector<string> ext;
	ext.push_back("png");
	ext.push_back("jpg");

	cout << "Loading the background from " << Dataset::backgroundPath << endl;

        if (dirExists(Dataset::backgroundPath.c_str())){
                backgrounds = getFiles(Dataset::backgroundPath, ext);
        }else{
                cout << "> Label: error could not find background path " << Dataset::backgroundPath << endl;
                string message = "> Error: Invalid background image path.";
		throw message;
        }

	int size = backgrounds.size();

	if (size == 0){

		string message = "no valid input files at ";
		string error = message + Dataset::backgroundPath;
		throw error; 
	}else{
	
		cout << "Found " << size << " valid files at: " << Dataset::backgroundPath << endl;
	}

        // shuffle the background images around 
        auto rng = default_random_engine {};
        shuffle(begin(backgrounds), end(backgrounds), rng);

        return backgrounds;


}


//split a path and returns the last element
string Dataset::splitPath(string line){

	std::string delimiter = "/";

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

	string name = splitLine.back();
        return name;

}


// parses the .yeet file into a vector of string vectors
vector<vector<string>> Dataset::parseFile(string path)
{

	string fileName = "dataset_config.yeet";
	string pathToYeet = path + fileName;
	cout << "parsing file " << pathToYeet << endl;
        string line;
        ifstream myfile (pathToYeet);
        vector<vector<string>> parsedFile;
        if (myfile.is_open()){
                while ( getline (myfile,line) ){
                        //cout << line << '\n';

                        std::string delimiter = "=";

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
        else{ 
		string message =  "no dataset_config.yeet file found";
		throw message;

        }

        return parsedFile;
}

// goes through each line and reads the config
void Dataset::setSettings (vector<vector<string>> file){

        for(vector<string> line : file){

                // get the first word of the line
                string word = line.at(0);

                if(word.compare("dataset_name") == 0){

                        Dataset::datasetName = line.at(1);

                }
                else if(word.compare("output_path") == 0){

                        Dataset::outputPath = line.at(1);
                }
                else if(word.compare("background_path") == 0){

                        Dataset::backgroundPath = line.at(1);
                }
                else if(word.compare("max_objects_per_canvas") == 0){
                        Dataset::max_objects = stoi(line.at(1));
			if (Dataset::max_objects > 10){
			
				string message = "cannot add more than 10 object per canvas you noodle";
				throw message;
			}

                }
                else if(word.compare("canvases_per_frame") == 0){
                        Dataset::canvas_per_frame = stoi(line.at(1));
			if (Dataset::canvas_per_frame > 30){
			
				string message = "cannot create more than 30 canvases per frame";
				throw message;

			}

                }
                else if(word.compare("canvas_blurr") == 0){

                        Dataset::can_blurrProb = stoi(line.at(1));
			if (Dataset::can_blurrProb > 100){
			
				string message = "value above 100 in config";
				throw message;

			}

                }
                else if(word.compare("object_saturation") == 0){

                        Dataset::obj_changeSatProb = stoi(line.at(1));
			if (Dataset::obj_changeSatProb > 100){
			
				string message = "value above 100 in config";
				throw message;

			}

                }
                else if(word.compare("canvas_lower_resolution") == 0){

                        Dataset::can_lowerRes = stoi(line.at(1));
			if (Dataset::can_lowerRes > 100){
			
				string message = "value above 100 in config";
				throw message;
			}
                }
                else if(word.compare("canvas_change_brightness") == 0){

                        Dataset::can_changeBrightProb = stoi(line.at(1));
			if (Dataset::can_changeBrightProb > 100){
			
				string message = "value above 100 in config";
				throw message;
			}
                }
                else if(word.compare("object_affine_transform") == 0){

                        Dataset::obj_affineProb = stoi(line.at(1));
			if (Dataset::obj_affineProb > 100){
			
				string message = "value above 100 in config";
				throw message;
			}

                }
                else if (word.compare("save_masks_png") == 0){
                        string save_masks_png_str = line.at(1);
                        if (save_masks_png_str == "True" || save_masks_png_str == "true"){
                                Dataset::save_masks_png = true;
                        }else{
                                Dataset::save_masks_png = false;
                        }
                }
                else if (word.compare("save_masks_json") == 0){
                        string save_masks_json_str = line.at(1);
                        if (save_masks_json_str == "True" || save_masks_json_str == "true"){
                                Dataset::save_masks_json = true;
                        }else{
                                Dataset::save_masks_json = false;
                        }
                }
                else if (word.compare("save_bnd_box") == 0){
                        string save_bnd_box_str = line.at(1);
                        if (save_bnd_box_str == "True" || save_bnd_box_str == "true"){
                                Dataset::save_bnd_box = true;
                        }else{
                                Dataset::save_bnd_box = false;
                        }
                }
                else if (word.compare("input_path") == 0){

                        Dataset::inputPath = line.at(1);

                }
                else if(word.compare("//") == 0){

                        //do nothing (comments in the config file)
                }


        }
	
	if (Dataset::outputPath.compare("default") == 0){

		string add_path = "/input_data/";

		string current_path = fs::current_path();
	
		Dataset::outputPath = current_path + add_path + Dataset::datasetName + "/";
	}

	// three folders to be created
        Dataset::masks_json = Dataset::outputPath + "masks_json/";
        Dataset::masks_png = Dataset::outputPath + "masks_png/";
        Dataset::imgs = Dataset::outputPath + "images/";
        Dataset::xml = Dataset::outputPath + "bnd_box_xml/";

	// prints out all the settings allowing the user to check that everything is ok
        cout << "\n========================= Dataset Configuration ==================================" << endl;
        cout << "> readFile: path to background:                             " << Dataset::backgroundPath << endl;
        cout << "> readFile: output path:                                    " << Dataset::outputPath << endl;
        cout << "> readFile: images save path:                             " << Dataset::imgs << endl;
        cout << "> readFile: bnb box xml save path:                        " << Dataset::xml << endl;
        cout << "> readFile: masks json save path:                         " << Dataset::masks_json << endl;
        cout << "> readFile: masks png save path:                          " << Dataset::masks_png << endl;
        cout << "> readFile: dataset name:                                   " << Dataset::datasetName  << endl;
        cout << "> readFile: number of canvases created per video frame:     " << Dataset::canvas_per_frame << endl;
        cout << "> readFile: max number of objects to be put in each canvas: " << Dataset::max_objects << endl;
        cout << "\n========================= Canvas Modification ===================================" << endl;
        cout << "> readFile: chances of blurring canvas:                     " << Dataset::can_blurrProb << "%" << endl;
        cout << "> readFile: chances of lowering the canvas resolution:      " << Dataset::can_lowerRes << "%" << endl;
        cout << "> readFile: chances of changing the canvas brightness:      " << Dataset::can_changeBrightProb << "%" << endl;
        cout << "\n========================= Object Modification ===================================" << endl;
        cout << "> readFile: chances of changing object color saturation:    " << Dataset::obj_changeSatProb << "%" << endl;
        cout << "> readFile: chances of changing object affine transform:    " << Dataset::obj_affineProb << "%" << endl;

}

void Dataset::create_label_map(){

        int i = 1;

        vector<string> lines;

        // creates the label map
        for (string label_path : Dataset::labels){

	
	        string labelName = Dataset::splitPath(label_path);

                // create an item
                lines.push_back("item {");
                lines.push_back("       id: " + to_string(i));
                lines.push_back("       name: \"" + labelName + "\"");
                lines.push_back("}");
                
                i++;

        }

        // writes the label map
        try {
                //open file for writing
                ofstream fw(Dataset::outputPath + "label.pbtxt", std::ofstream::out);
                //check if file was successfully opened for writing
                if (fw.is_open())
                {
                        //store array contents to text file
                        for (int j = 0; j < lines.size(); j++) {
                                fw << lines.at(j) << "\n";
                        }
                        fw.close();
                }
                else cout << "Problem with opening file";
        }
        catch (const char* msg) {
                cerr << msg << endl;
        }

}

