/*
 
author: Andrew Jouffray
date: jan 2021

A Canvas is the image that the neural network will end up seeing, millions of unique Canvas::canvases can be generated if needed. Each Canvas::canvas is a backround image, 
and between 2 and a user defined limit of object of interest copied on top of the Canvas::background image, each Canvas::canvas object has a Canvas::mask where a detailed outline of the Canvas::objects in interest in the image 
are available for the model to learn, Bounding boxes are also available to the model.

step1: define the size and aspect ratio of the Canvas::canvas 
step2: create the Ooi and copy them on top of the Canvas::canvas
step3: create a Canvas::mask of the Canvas::canvas
step4: get the Bounding box coordinates of each object in the Canvas::canvas
step5: replace the bacl pixels with the provided backgorund image
step6: randomely apply transforamtions: (resolution change, blurr, brightness change)

*/
#include "../include/Canvas.h"


// constructor, params need to be added I think.
Canvas::Canvas(cv::Mat ooiArg, cv::Mat backgroundArg, int maxOoi, int obj_affineProb, int obj_satProb, int brightness, int blurr, int lowRes , bool debug, vector<int>* labelColor){

	if(debug){
		cout << "constructing Canvas" << endl;
	}

	Canvas::debug = debug;
	Canvas::ooi = ooiArg;
	Canvas::background = backgroundArg;
	Canvas::maxObjects = maxOoi;
	Canvas::obj_affine = obj_affineProb;
	Canvas::obj_sat = obj_satProb;
	Canvas::brightnessProb = brightness;
	Canvas::blurrProb = blurr;
	Canvas::lowResProb = lowRes;

	// used to randomely apply transformations to the Canvas::canvas
	int canvasModProb = randomInt(1, 100);

	// define the size random between 500 and 860 pixels
	Canvas::height = randomInt(500, 860);

	// gets a aspect random ratio from the ratio list
	int randRatio = randomInt(0, 4);
	float ratio = Canvas::aspectRatios[randRatio];	
	float fWidth = Canvas::height * ratio;
	Canvas::width = (int)fWidth; // gets a width value as integer

	// create Canvas::canvas and set it to black Canvas::canvas to black
	cv::Mat blkImage(Canvas::height, Canvas::width, CV_8UC3, cv::Scalar(0, 0, 0));
	Canvas::canvas = blkImage;

	//define the number of Ooi to be created for this Canvas::canvas
	Canvas::numObjects = randomInt(2, Canvas::maxObjects);

	// one object per column to avoid overlapping of Canvas::objects on the image
	Canvas::columnWidth = Canvas::width / Canvas::numObjects;

	Canvas::createCanvas();

	// Creates a Canvas::mask of the silouette of the object of interest in a color unique 
	// to the label
	Canvas::createMasks(labelColor);


	// Calculates an accurate position of all the Canvas::objects of interest to draw bonding boxes arund them
	Canvas::rois = calculateRois();
				
				
	// replaces black with a Canvas::background image
	Canvas::addBackground();

	// transformations
	if (canvasModProb <= Canvas::brightnessProb){
		Canvas::changeBrightness();
	}

	if (canvasModProb <= Canvas::blurrProb){
		Canvas::blurr();
	}

	if (canvasModProb <= Canvas::lowResProb){
		Canvas::lowerRes();
	}
	


		
}

// generates the image
void Canvas::createCanvas(){

	for(int i = 0; i < Canvas::numObjects; i ++){

		int absolutePos = Canvas::columnWidth * i;

		// create an object to put on the Canvas::canvas
		Ooi objectOfInterest = Ooi(ooi, Canvas::columnWidth, Canvas::height, absolutePos, Canvas::obj_affine, Canvas::obj_sat, Canvas::debug );
		objects.push_back(objectOfInterest);

		// get the position of the object in the image
		vector<int> positions = objectOfInterest.getPosition();
		int x1 = positions.at(0);
		int y1 = positions.at(1);
		int x2 = positions.at(2);
		int y2 = positions.at(3);

		cv::Mat objectImage = objectOfInterest.getObject();
		// overlay the object onto the Canvas::canvas 
		try{	
			cv::Mat inset(Canvas::canvas, cv::Rect(x1, y1, x2, y2));
			objectImage.copyTo(inset);
				
		}catch(...){

			// maybe add a counter for failures?
			if(Canvas::debug ) {
					
				cout << "> (Canvas::canvas) could not insert object into canvas" << endl;
				cout << "> (Canvas::canvas) x1, y1, x2, y2: " << x1 << ", " <<y1<<", "<<x2<<", "<<y2 << endl;
				cout << "> (Canvas::canvas) canvas size, rows, cols: " <<Canvas::canvas.rows<<", "<<Canvas::canvas.cols << endl;
				cout << "> (Canvas::canvas) object height " << objectImage.rows << endl;
			}
			else{
						
				cout << "> (Canvas::canvas) could not insert object into Canvas::canvas" << endl;
			}
				
		}	

			
	}
		

		
		
}

void Canvas::lowerRes(){

	Canvas::log("Lowering the resolution");

	// reduction factor
	float factor = 0.0;

	//allows the resolution do be reduced more on larger images
	if (Canvas::height > 720){
		factor = randomFloat(1.0, 2.0);
	}else{
		factor = randomFloat(1.0, 1.5);
	}

	float newHeightf = Canvas::height/factor;
	float newWidthf = Canvas::width/factor;
	int newWidth = (int)newWidthf;
	int newHeight = (int)newHeightf;

	cv::Mat lowRes;
	cv::resize(Canvas::canvas, lowRes, cv::Size(newWidth, newHeight));
	cv::resize(lowRes, Canvas::canvas, cv::Size(Canvas::width, Canvas::height));

}

void Canvas::blurr(){


	Canvas::log(" > (Canvas) blurring the image");

	int kernelSize = randomInt(1, 5);

	if(kernelSize % 2 == 0){
		kernelSize = kernelSize + 1;
	}
		
	cv::GaussianBlur(Canvas::canvas, Canvas::canvas, cv::Size(kernelSize, kernelSize), 0);

}

// each label in a dataset has a different colored Canvas::mask
void Canvas::createMasks(vector<int>* mcolors){

	// FUTURE UPDATE: get the thresholded value once and not in each method

	Canvas::mask = Canvas::canvas.clone();
        cv::cvtColor(Canvas::mask, Canvas::mask, cv::COLOR_BGR2GRAY);
        double thresh = 4;
        double maxValue = 255;
        // Binary Threshold
        cv::threshold(Canvas::mask, Canvas::mask, thresh, maxValue, cv::THRESH_BINARY);
        cv::Mat grayMask;
        cv::cvtColor(Canvas::mask, Canvas::mask, cv::COLOR_GRAY2BGR);

	// assing the black Canvas::mask used in other methods
	Canvas::blackMask = mask.clone();

        inRange(Canvas::mask, cv::Scalar(255, 255, 255), cv::Scalar(255, 255, 255), grayMask);
        Canvas::mask.setTo(cv::Scalar(mcolors->at(0), mcolors->at(1), mcolors->at(2)), grayMask);

}

vector<cv::Rect> Canvas::calculateRois(){
			
	int thresh = 100;

	if(Canvas::debug){
		cv::imshow("blackMask before getting rois", Canvas::blackMask);
		int k = cv::waitKey(0);
	}

	// the input for find countout needs to be in gray so another conversions, need to see if there is a more effective way
	cv::Mat contourInput;
	cv::cvtColor(Canvas::blackMask, contourInput, cv::COLOR_BGR2GRAY);

        vector<vector<cv::Point>> contours;
        cv::findContours(contourInput, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

	vector<vector<cv::Point> > contours_poly( contours.size() ); // list of controur points? 
        vector<cv::Rect> boundRect( contours.size() );
        vector<cv::Rect> validRect( 0 );

        for( size_t i = 0; i < contours.size(); i++ )
        {
                int areax = cv::contourArea(contours[i]);
                // 2000 is a dummy value, best to calculate average
               	if (areax > 500){
			if (Canvas::debug){
				cout << "> (Canvas) roi area: " << areax << endl;
			}
                        cv::approxPolyDP( contours[i], contours_poly[i], 3, true );
                        validRect.push_back(cv::boundingRect( contours_poly[i] ));
               	}
	}
	
	if (Canvas::debug){

		cv::Mat copy = Canvas::blackMask.clone();
	
		for (int i = 0; i < validRect.size(); i ++){
		
			cv::Rect current = validRect[i];
			cv::Point pt1(current.x, current.y);
			cv::Point pt2(current.x + current.width, current.y + current.height);
			cv::rectangle(copy, pt1, pt2, cv::Scalar(0, 255, 0));

		}


		cv::imshow("blackMask after getting rois", copy);
	}
	// each rectangle is define like this [x,y,width,height]
	// Point point0 = Point(rectangle.x, rectangle.y);
	return validRect;


}

// this is an alternative to creating a png mask, to be used with the coco json format or the labelme format
vector<vector<cv::Point>> Canvas::calculateOutline(){

	int thresh = 100;

	if(Canvas::debug){
		cv::imshow("blackMask before getting rois", Canvas::blackMask);
		int k = cv::waitKey(0);
	}

	// the input for find countout needs to be in gray so another conversions, need to see if there is a more effective way
	cv::Mat contourInput;
	cv::cvtColor(Canvas::blackMask, contourInput, cv::COLOR_BGR2GRAY);

        vector<vector<cv::Point>> contours; // or this is the list of contour points
	vector<cv::Vec4i> hierarchy;


	if (Canvas::debug){
		cv::imshow("input to the contours:", contourInput);
	}

        cv::findContours(contourInput, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);



	vector<vector<cv::Point>> contours_poly( 0);

        for( size_t i = 0; i < contours.size(); i++ )
        {
		vector<cv::Point> contour = contours[i];

		if (hierarchy[i][3] == -1){
			vector<cv::Point> reduced;

			// the smllar the factor, more points will be removed
			// even number
			int reduction_factor = 2;

			// odd number
			int second_reduction_factor = 3;

			int areax = cv::contourArea(contour);
			// 1300 is a dummy value, best to calculate average, or not... I'm to lazy right now...
			if (areax > 1300){
				if (Canvas::debug){
					cout << "> (Canvas) roi area: " << areax << endl;
				}

				for (int j = 0; j < contour.size(); j++)
				{
				
					if(j % reduction_factor != 0 && j % second_reduction_factor != 0){

						reduced.push_back(contour[j]);
					}
				}

				if (Canvas::debug){

					cout << "before filtering " << contour.size() << endl;
					cout << "after filtering" <<  reduced.size() << endl;

				}

				contours_poly.push_back(reduced);
			}
		}
	}

	if (Canvas::debug){
		cout << "total contours: " << contours_poly.size() << endl;
	}

	return contours_poly;

}

// adds a Canvas::background image to the black Canvas::background of the Canvas::canvas
void Canvas::addBackground(){
	
        // resize the Canvas::background to fit the Canvas::canvas
        cv::resize(Canvas::background, Canvas::background, cv::Size(Canvas::blackMask.cols, Canvas::blackMask.rows));

	// all balck pixels become the bacjgorund image
        cv::Mat comb = Canvas::blackMask + Canvas::background;

	// all remaining white pixels become black
        comb.setTo(0, comb == 255); // comb means combined images

	// all black pixels are set to Canvas::canvas
        cv::Mat comb2 =  comb + Canvas::canvas;
        		
	Canvas::canvas = comb2;
	
}

void Canvas::changeBrightness(){

	Canvas::log("changing brightness");

	// HSV stands for Hue Saturation Value(Brightness)
       	cv::cvtColor(Canvas::canvas,Canvas::canvas,cv::COLOR_BGR2HSV);

       	// hsv values are on a scale from 0 to 255
        int defaultVal = 255;

        int randValue = randomInt(155, 355);

        // value is brightness, we modify this on the whole Canvas::canvas not on one object, we don't modify it here.
        int value = randValue - defaultVal;

        // iterate over the Ooi::image
        for(int y=0; y<Canvas::canvas.cols; y++)
        {
                for(int x=0; x<Canvas::canvas.rows; x++)
                {
                        // get the current values of a pixel
                        int cur3 = Canvas::canvas.at<cv::Vec3b>(cv::Point(y,x))[2];

                        // add the modified values 
                       	cur3 += value;

                        // wrap the value around if they exceed 255
                        if(cur3 < 0) cur3= 0; else if(cur3 > 255) cur3 = 255;

                        // set the new values 
			Canvas::canvas.at<cv::Vec3b>(cv::Point(y,x))[2] = cur3;
                }
        }

        cv::cvtColor(Canvas::canvas,Canvas::canvas,cv::COLOR_HSV2BGR);


}

cv::Mat Canvas::getCanvas(){
		
	return Canvas::canvas;
}

cv::Mat Canvas::getMask(){
		
	return Canvas::mask;
}

vector<vector<int>> Canvas::getRois(){

	vector<vector<int>> allRois;

	for (auto & rectangle : rois) {

		// possible error area make sure this is the propper order
		// make sure they are int
		// make sure width and height are the actuall references
		vector<int> currRect;
		currRect.push_back(rectangle.x);
		currRect.push_back(rectangle.y);
    		currRect.push_back(rectangle.x + rectangle.width);
		currRect.push_back(rectangle.y + rectangle.height);

		allRois.push_back(currRect);
	}		

	return allRois;
			
}


void Canvas::log(const char* message){

	if(Canvas::debug){
	
		cout << "> (Canvas) " <<  message << endl;
	
	
	}


}



