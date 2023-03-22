/*
author: Andrew Jouffray
date: feb 2021

The Label is one of outputs of the neural network it's the name of an object that needs to be 
detected by the nework. Each Dataset has one or more labels each containing images of a particual object. Example: The Apple dataset might have labels such as "golden delicious" and "honey crisp" etc... each being a directory containing videos of the apples.

The Label class is instantiated by the Dataset class.

Step1: Find the label directory with the given path
Step2: Find and create the Output directories
Step3: Read the config.yaml and set all the configuration options
Step4: Generate the Canvases (using multithreading)
Step5: Save the Canvases.jpg Masks.jpg and Roi.Label::xml files


*/
#include "../include/Label.h"

Label::Label(string label, string dataset, string output, int affine, int saturation, int bright, int blurr, int lowRes, int canvasQt, int max_obj, vector<string>* input, vector<string>* background, bool output_boxes_pascalVoc, bool output_masks_png, bool output_masks_json, bool output_boxes_yolo, bool output_mask_yolo, bool debugArg, int label_index){

	// pointer to the background array;
	Label::backgrounds = background;

	// pointer to the array of videofiles
	Label::inputs = input;

	Label::labelName = label;
	Label::outputPath = output;
	Label::datasetName = dataset;
	Label::obj_affineProb = affine;
	Label::obj_changeSatProb = saturation;
	Label::can_changeBrightProb = bright;
	Label::can_blurrProb = blurr;
	Label::can_lowerRes = lowRes;
	Label::canvas_per_frame = canvasQt;
	Label::max_objects = max_obj;

	Label::output_boxes_pascalVoc = output_boxes_pascalVoc;
	Label::output_masks_json = output_masks_json;
	Label::output_masks_png = output_masks_png;
	Label::output_boxes_yolo = output_boxes_yolo;
	Label::output_mask_yolo = output_mask_yolo;


	// forgot why I am redefining them here
	Label::masks_png_path = Label::outputPath + "masks_png/";
	Label::masks_json_path = Label::outputPath + "masks_json/";
	Label::imgs_path = Label::outputPath + "images/";
	Label::bndBox_PASCALVOC_path = Label::outputPath + "bnd_box_PASCALVOC/";
	Label::masks_yolo_path = Label::outputPath + "masks_yolo/";
	Label::bndBox_yolo_path = Label::outputPath + "bnd_box_yolo/";
	
	Label::debug = debugArg;
	Label::label_index = label_index;
	
	// for each input file
	for(int i = 0; i < Label::inputs->size(); i ++){

		string inFile = Label::inputs->at(i);
		cout << "Label: Processing file " << inFile << endl;

		cv::VideoCapture cap(inFile);

        	// create a randomColor for the mask
       		vector<int> colors = {randomInt(10, 255), randomInt(10, 255), randomInt(10, 255)};

        	cv::Mat frame;


		int64_t  start = timeSinceEpochMillisec();

		#pragma omp parallel    // start the parallel region
		{

            #pragma omp single  // let the while loop execute by one thread and generate tasks
            while (1){

				if (!cap.read(frame)){

						cout << "> (Label) No more frames." << endl;
						break;

				}


				// it would be much smarter to divide the dataset into a video for each thread instead of what ever the fuck this is. 
				#pragma omp task
				{
					int id = omp_get_thread_num();
					for(int i = 0; i < Label::canvas_per_frame; i ++){

						if(Label::debug){ 
							cout << "giving task to thread "<< to_string(id) << endl;
						}

						cv::Mat background = Label::getRandomBackground();

						// creates and saves a canvas
						Canvas canvas(frame, background, Label::max_objects, Label::obj_affineProb, Label::obj_changeSatProb ,Label::can_changeBrightProb, Label::can_blurrProb, Label::can_lowerRes, Label::debug, &colors);
						int64_t  current = timeSinceEpochMillisec();

						string name = to_string(current) + "hpa" + to_string(id);

						cv::Mat img = canvas.getCanvas();
						saveImg(img, name);

						if (Label::output_masks_png){
							saveMask(canvas.getMask(), name);
						}


						if (Label::output_boxes_pascalVoc){
							saveXML(canvas.getRois(), name, img);
						}

						if (Label::output_boxes_yolo){
							saveYoloBox(canvas.getRois(), name, img);
						}

						if (Label::output_masks_json){
							// draw the point outline on the image to make sure it is accurate
							vector<vector<cv::Point>> contours = canvas.calculateOutline();
							saveJson(contours, img, name);

							// show the outline on top of the object for debugging
							if (Label::debug){
								for (int j = 0; j < contours.size(); j++){

									vector<cv::Point> outline = contours.at(j);

									cout << "Size of point outline (multiply by 2) " << outline.size() << endl;
								
									// draw the points
									cv::fillPoly(img, outline, cv::Scalar(255, 255, 255), 8, 0);	

								}

								// show the image for debugging
								cv::imshow("image with filled polys", img);
								int k = cv::waitKey(0);

							}
						}

						if (Label::output_mask_yolo){
							vector<vector<cv::Point>> contours = canvas.calculateOutline();
							Label::saveYoloMask(contours, img, name);
						}
				


					}

				}

			} // end of while loop and single region

		} // end of parallel region


        uint64_t  end = timeSinceEpochMillisec();

        uint64_t total = end - start;

        cout << end << endl;
        cout << total << endl;

	}



}

void Label::saveImg(cv::Mat img, string name){

	string path = Label::imgs_path + name + ".jpg";
	cv::imwrite(path, img);
}

void Label::saveMask(cv::Mat mask, string name){

	string path = Label::masks_png_path + name + ".png";
	cv::imwrite(path, mask);

}

void Label::saveXML(vector<vector<int>> rois, string name, cv::Mat img){

	// image dimentions 
	int iheight = img.rows;
	int iwidth = img.cols;
	int ichannels = img.channels();

	string sHeight = to_string(iheight);
    string sWidth = to_string(iwidth);
	string sChannel = to_string(ichannels);

	// save path of the image not the Label::xml
	string fullPath = Label::imgs_path + name + ".jpg";	

	// image being refered to path
	string img_filename = name + ".jpg";

	TiXmlDocument doc;
        TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "", "" );
        TiXmlElement * annotation = new TiXmlElement( "annotation" );
        TiXmlElement * folder = new TiXmlElement( "folder" );
        TiXmlElement * filename = new TiXmlElement( "filename" );
        TiXmlElement * path = new TiXmlElement( "path" );
        TiXmlElement * source = new TiXmlElement( "source" );
        TiXmlElement * database = new TiXmlElement( "database" );
        TiXmlElement * size = new TiXmlElement( "size" );
        TiXmlElement * width = new TiXmlElement( "width" );
        TiXmlElement * height = new TiXmlElement( "height" );
        TiXmlElement * depth = new TiXmlElement( "depth" );
        TiXmlElement * segmented = new TiXmlElement( "segmented" );


        TiXmlText * folderName = new TiXmlText( "xml" );
        TiXmlText * fileNameText = new TiXmlText( img_filename.c_str() );
        TiXmlText * databaseName = new TiXmlText( Label::datasetName.c_str() );
        TiXmlText * pathText = new TiXmlText( fullPath.c_str() );
        TiXmlText * widthVal = new TiXmlText( sWidth.c_str() );
        TiXmlText * heightVal = new TiXmlText( sHeight.c_str() );
        TiXmlText * depthVal = new TiXmlText( sChannel.c_str() );
        TiXmlText * segmentedVal = new TiXmlText( "0" );


        annotation->LinkEndChild( folder );
			folder->LinkEndChild(folderName);

        annotation->LinkEndChild( filename );
			filename->LinkEndChild(fileNameText);

        annotation->LinkEndChild( path );
			path->LinkEndChild(pathText);

        annotation->LinkEndChild( source );
			source->LinkEndChild( database );
					database->LinkEndChild( databaseName );

        annotation->LinkEndChild( size );
			size->LinkEndChild( width );
				width->LinkEndChild( widthVal );
			size->LinkEndChild( height );
				height->LinkEndChild( heightVal );
			size->LinkEndChild( depth );
				depth->LinkEndChild( depthVal );

        annotation->LinkEndChild( segmented );
			segmented->LinkEndChild( segmentedVal );

        // add all the objects here might need to create them in the loop
        for(vector<int> roi : rois){

			TiXmlElement * object = new TiXmlElement( "object" );
			TiXmlElement * name = new TiXmlElement( "name" );
			TiXmlElement * pose = new TiXmlElement( "pose" );
			TiXmlElement * truncated = new TiXmlElement( "truncated" );
			TiXmlElement * difficult = new TiXmlElement( "difficult" );
			TiXmlElement * bndbox = new TiXmlElement( "bndbox" );
			TiXmlElement * xmin = new TiXmlElement( "xmin" );
			TiXmlElement * ymin = new TiXmlElement( "ymin" );
			TiXmlElement * xmax = new TiXmlElement( "xmax" );
			TiXmlElement * ymax = new TiXmlElement( "ymax" );

			TiXmlText * objectName = new TiXmlText( Label::labelName.c_str() );
			TiXmlText * objectPose = new TiXmlText( "Unspecified" );
			TiXmlText * objectTruncated = new TiXmlText( "0" );
			TiXmlText * objectDifficult = new TiXmlText( "0" );

			string x1 = to_string(roi[0]);
			string y1 = to_string(roi[1]);
			string x2 = to_string(roi[2]);
			string y2 = to_string(roi[3]);

			TiXmlText * objectXmin = new TiXmlText( x1.c_str() );
			TiXmlText * objectYmin = new TiXmlText( y1.c_str() );
			TiXmlText * objectXmax = new TiXmlText( x2.c_str() );
			TiXmlText * objectYmax = new TiXmlText( y2.c_str() );



			annotation->LinkEndChild( object );
				object->LinkEndChild(name);
					name->LinkEndChild(objectName);
				object->LinkEndChild(pose);
					pose->LinkEndChild(objectPose);
				object->LinkEndChild(truncated);
					truncated->LinkEndChild(objectTruncated);
				object->LinkEndChild(difficult);
					difficult->LinkEndChild(objectDifficult);
				object->LinkEndChild(bndbox);
					bndbox->LinkEndChild(xmin);
						xmin->LinkEndChild(objectXmin);
					bndbox->LinkEndChild(ymin);
						ymin->LinkEndChild(objectYmin);
					bndbox->LinkEndChild(xmax);
						xmax->LinkEndChild(objectXmax);
					bndbox->LinkEndChild(ymax);
						ymax->LinkEndChild(objectYmax);


        }





        doc.LinkEndChild( decl );
        doc.LinkEndChild( annotation );
		string savePath = Label::bndBox_PASCALVOC_path + name + ".xml";
        doc.SaveFile( savePath.c_str() );
	// need to test this

}


// return the normalized x y coordinates of bounding boxes
void Label::saveYoloBox(vector<vector<int>> rois, string name, cv::Mat img){


	std::ofstream fs(Label::bndBox_yolo_path + name + ".txt"); 


    if(!fs)
    {
        std::cerr<<"Cannot open the output file."<<std::endl;
    }
		// for each contour

	
	// init the line with the label index


	// image dimentions 
	int iheight = img.rows;
	int iwidth = img.cols;

	//add all the damn contours
	for(vector<int> roi : rois){

		string line = to_string(Label::label_index);

		// normalize points
		float x1 = (static_cast<float>(roi[0]) / static_cast<float>(iwidth));
		float y1 = (static_cast<float>(roi[1]) / static_cast<float>(iheight));
		float x2 = (static_cast<float>(roi[2]) / static_cast<float>(iwidth));
		float y2 = (static_cast<float>(roi[3]) / static_cast<float>(iheight));

		// convert to centerx centery weidth height
		float centerX = x1 + ((x2 - x1) / 2);
		float centerY = y1 + ((y2 - y1) / 2);
		float box_width = x2 - x1;
		float box_height = y2 - y1;

		line.append(" " + to_string(centerX) + " " + to_string(centerY) + " " + to_string(box_width) + " " + to_string(box_height));

		fs<<line<<"\n";

	}



    fs.close();

}

// return the normalized x y coordinates
void Label::saveYoloMask(vector<vector<cv::Point>> contours, cv::Mat img, string name){


	std::ofstream fs(Label::masks_yolo_path + name + ".txt"); 


    if(!fs)
    {
        std::cerr<<"Cannot open the output file."<<std::endl;
    }
		// for each contour
	for(vector<cv::Point> contour : contours){
	
		Json::Value shape;
		Json::Value points(Json::arrayValue);

		// init the line with the label index
		string line = to_string(Label::label_index);

		// image dimentions 
		int iheight = img.rows;
		int iwidth = img.cols;

		//add all the damn contours
		for (cv:: Point point : contour){
			
			// normalize points
			float x = static_cast<float>(point.x) / static_cast<float>(iwidth);
			float y = static_cast<float>(point.y) / static_cast<float>(iheight);

			line.append(" " + std::to_string(x) + " " + std::to_string(y));

		}

		fs<<line<<"\n";

	}

    fs.close();

}

void Label::saveJson(vector<vector<cv::Point>> contours, cv::Mat img, string name){

	Json::Value annotation;
	Json::Value shapes(Json::arrayValue);

	// image dimentions 
	int iheight = img.rows;
	int iwidth = img.cols;

	string sHeight = to_string(iheight);
    string sWidth = to_string(iwidth);

	annotation["imageWidth"] = sWidth.c_str();
	annotation["imageHeight"] = sHeight.c_str();

	// for each contour
	for(vector<cv::Point> contour : contours){
	
		Json::Value shape;
		Json::Value points(Json::arrayValue);

		shape["label"] = Label::labelName;
		shape["shape_type"] = "polygon";

		//add all the damn contours
		for (cv:: Point point : contour){
		
			Json::Value point_json(Json::arrayValue);
			point_json.append(Json::Value(point.x));
			point_json.append(Json::Value(point.y));

			points.append(point_json);

		}

		shape["points"] = points;

		shapes.append(shape);

	}

	annotation["shapes"] = shapes;
	annotation["imagePath"] = name + ".jpg";

	Json::StreamWriterBuilder builder;
	builder["commentStyle"] = "None";
	builder["indentation"] = "   ";

	std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
	std::ofstream outputFileStream(Label::masks_json_path + name + ".json");
	writer -> write(annotation, &outputFileStream);
	
}

cv::Mat Label::getRandomBackground(){

	int rand = randomInt(0, Label::backgrounds->size() -1);

	string file = Label::backgrounds->at(rand);

	cv::Mat image = cv::imread(file);

	return image;
}

uint64_t Label::timeSinceEpochMillisec(){
        using namespace std::chrono;
        return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}



