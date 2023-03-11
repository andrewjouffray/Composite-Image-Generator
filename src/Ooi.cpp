/*

author: Andrew Jouffray
date: nov 2020

Ooi stands of Objects of interest, those are the objects that are added to the background Ooi::image (canvas)

The canvas creates between 1 - 10 ooi and needs to place them on the Ooi::image without overlapping between ooi. So
the algorithm defines a column on the canvas for each of those objects to be passed in. the left boundary of these
columns are the Ooi::xAbsolutePos. The vertical boundary is the top of the Ooi::image therefor Ooi::yAbsolutePos always starts at 0

step1: rotate
step2: scale
step3: position

note: everytime I mention 'object' in the comments, I'm referring to the object of interest (an Ooi::image) and not an object in the oop sence.

*/

#include "../include/Ooi.h"

// important note: I use namespace std in the header file, this might create namespace conflicts in some libraries.


// constructor
Ooi::Ooi(cv::Mat objectOfInterest, int colWidth, int colHeight, int xAbsolutePosArg, int affineProb, int saturationProb, bool debug){

	// prints a lot of info id set to true
	Ooi::debug = debug;
		
	Ooi::image = objectOfInterest;

	//define one random value that will be moded, to define probability of transformation
	Ooi::randomValue = randomInt(0, 1000); // this is a const?

	//Rotate the object to a random angle between 0 and 360
        int angle = Ooi::randomValue % 360;

        rotate(angle);

	// defines the probability that the Ooi::image will have an affine transform and/or a saturation change
	// 1 = 10%, 2 = 20%, 3 = 33% etc...
	int affineProbability = affineProb;
	int saturationProbability = saturationProb;

	int random = Ooi::randomValue % 100;
	if (random <= affineProbability){
		affineTransform();
	}
	if (random <= saturationProbability){
		changeSaturation();
	}
	// get the height and width after possible transformations
	Ooi::ooiWidth = Ooi::image.cols;
	Ooi::ooiHeight = Ooi::image.rows;


	// determine the maximum scale to shrink or expand the Ooi::image so that it still fits in the canvas
	float maxScaleHeight = ((float)colHeight / (float)Ooi::ooiHeight) - 0.01;

	float maxScaleWidth = ((float)colWidth / (float)Ooi::ooiWidth) - 0.01;
	if(maxScaleHeight > maxScaleWidth){
		Ooi::maxScale= maxScaleWidth;
	}else{
		Ooi::maxScale= maxScaleHeight;
	}
	// I use 1.3 to make sure that the minimum scale smaller than max but still proportional to max, so you
	// don't endup with huge object, and a tiny object the size of three pixel
	Ooi::minScale= Ooi::maxScale/ 1.3;

	float scale = randomFloat(minScale, maxScale);

	// print all scale info when Ooi::debugging at runtime this can be very helpful
	if (Ooi::debug){
		cout << "> (Ooi) ===== ooi width and height information ===== " << endl;
		cout << "> (Ooi) col height: " + to_string(colHeight) + "| col width: " + to_string(colWidth) << endl;
		cout << "> (Ooi) ooi height: " + to_string(Ooi::ooiHeight) + "| ooi width: " + to_string(Ooi::ooiWidth) << endl;
		cout << "> (Ooi) max scale height: " + to_string(maxScaleHeight) + "| max scale width: " + to_string(maxScaleWidth) << endl;
		cout << "> (Ooi) max scale: " + to_string(maxScale) + "| min scale: " + to_string(minScale) << endl;
	}

	scaleImage(scale);

	//defines the absolute position of the object in the canvas within given boundaries
	try{
		int maxXOffset = colWidth - Ooi::ooiWidth;
		int maxYOffset = colHeight - Ooi::ooiHeight;
		Ooi::xOffset = randomInt(0, maxXOffset);
            	Ooi::yOffset = randomInt(0, maxYOffset);
			
	}catch(...){
		cout << "> (Ooi) error while computing offsets" << endl;
		Ooi::xOffset = 1;
		Ooi::yOffset = 1;
	}

	//sets x1 y1
	Ooi::xAbsolutePos = xAbsolutePosArg + Ooi::xOffset;
	Ooi::yAbsolutePos = Ooi::yOffset;
		
}


// code from user Lars Schillingmann https://stackoverflow.com/questions/22041699/rotate-an-Ooi::image-without-cropping-in-opencv-in-c
void Ooi::rotate(int angle){

	angle = angle * -1;

	// get rotation matrix for rotating the Ooi::image around its center in pixel coordinates
    	cv::Point2f center((Ooi::image.cols-1)/2.0, (Ooi::image.rows-1)/2.0);

    	cv::Mat rot = cv::getRotationMatrix2D(center, (double)angle, 1.0);

    	// determine bounding rectangle, center not relevant
    	cv::Rect2f bbox = cv::RotatedRect(cv::Point2f(), Ooi::image.size(), (double)angle).boundingRect2f();

    	// adjust transformation matrix
    	rot.at<double>(0,2) += bbox.width/2.0 - Ooi::image.cols/2.0;
    	rot.at<double>(1,2) += bbox.height/2.0 - Ooi::image.rows/2.0;

    	cv::warpAffine(Ooi::image, Ooi::image, rot, bbox.size());

}

void Ooi::scaleImage(float scale){
		
	// updates the new height and width and scales the object.	
	Ooi::ooiWidth = Ooi::ooiWidth * scale;
	Ooi::ooiHeight = Ooi::ooiHeight * scale;
	cv::Size size(Ooi::ooiWidth, Ooi::ooiHeight);
	cv::resize(Ooi::image, Ooi::image, size);
}

void Ooi::affineTransform(){


        // rows = height and cols = width


        float randVal1 = randomFloat(0.20f, 0.45f);
        float randVal2 = randomFloat(0.70f, 0.90f);
        float randVal3 = randomFloat(0.10f, 0.35f);
        float inverted = randomFloat(0.0f, 1.0f);


        // print random value for troubleshooting
	if (Ooi::debug){

        	cout << "> (Ooi) Affine random Value1: " + to_string(randVal1) << endl;     
        	cout << "> (Ooi) Affine random Value2: " + to_string(randVal2) << endl;
        	cout << "> (Ooi) Affine random value3: " + to_string(randVal3) << endl;

        	//Ooi::image size for troubleshooting
        	cout << "> (Ooi) ooi col: " + to_string(Ooi::image.cols) << endl;
        	cout << "> (Ooi) ooi rows: " + to_string(Ooi::image.rows) << endl;
	}

        // points2f means a 2d set of floating point single precision vatiables
        // source
	cv::Point2f srcTri[3];

        if (inverted >= 0.5f){
                srcTri[0] = cv::Point2f( 0.f, 0.f ); // top left
               	srcTri[1] = cv::Point2f( Ooi::image.cols - 1.f, 0.f ); // bottom left
                srcTri[2] = cv::Point2f( 0.f, Ooi::image.rows - 1.f ); // top right
        }else{
                srcTri[0] = cv::Point2f( Ooi::image.cols - 1.f, Ooi::image.rows - 1.f ); // bottom right
                srcTri[1] = cv::Point2f( 0.f, Ooi::image.rows - 1.f ); // top right
                srcTri[2] = cv::Point2f( Ooi::image.cols - 1.f, 0.f ); // bottom left
        }

        //destination
	cv::Point2f dstTri[3];
        dstTri[0] = cv::Point2f( 0.f, Ooi::image.rows*randVal3 );
        dstTri[1] = cv::Point2f( Ooi::image.cols*0.85f, Ooi::image.rows*randVal1 );
        dstTri[2] = cv::Point2f( Ooi::image.cols*0.15f, Ooi::image.rows*randVal2 );

        // create a wrap matrix
	cv::Mat warp_mat = getAffineTransform( srcTri, dstTri );
	cv::warpAffine( Ooi::image, Ooi::image, warp_mat, Ooi::image.size() );

		
}
		
// converts the Ooi::image into HSV, and randomely modifies the hsv values
void Ooi::changeSaturation(){	


        // HSV stands for Hue Saturation Value(Brightness)
	cv::cvtColor(Ooi::image,Ooi::image, cv::COLOR_BGR2HSV);


	// hsv values are on a scale from 0 to 255
       	int defaultVal = 255;

        // less than 255 will reduce the current hue value
        // greater than 255 will increase the hue value

        int hueVal = randomInt(155, 355);
        int hue = hueVal  - defaultVal;


        int satVal = randomInt(155, 355);
       	int saturation = satVal  - defaultVal;

        // value is brightness, we modify this on the whole canvas not on one object, we don't modify it here.
        int value = 255 - defaultVal;

	// iterate over the Ooi::image
        for(int y=0; y<Ooi::image.cols; y++)
        {
                for(int x=0; x<Ooi::image.rows; x++)
                {
			// get the current values of a pixel
                	int cur1 = Ooi::image.at<cv::Vec3b>(cv::Point(y,x))[0];
                	int cur2 = Ooi::image.at<cv::Vec3b>(cv::Point(y,x))[1];
                	int cur3 = Ooi::image.at<cv::Vec3b>(cv::Point(y,x))[2];

			// add the modified values 
                	cur1 += hue;
                	cur2 += saturation;
                	cur3 += value;

			// wrap the value around if they exceed 255
                	if(cur1 < 0) cur1= 0; else if(cur1 > 255) cur1 = 255;
                	if(cur2 < 0) cur2= 0; else if(cur2 > 255) cur2 = 255;
                	if(cur3 < 0) cur3= 0; else if(cur3 > 255) cur3 = 255;

			// set the new values 
                	Ooi::image.at<cv::Vec3b>(cv::Point(y,x))[0] = cur1;
                	Ooi::image.at<cv::Vec3b>(cv::Point(y,x))[1] = cur2;
                	Ooi::image.at<cv::Vec3b>(cv::Point(y,x))[2] = cur3;
                }
        }

	// coverts the Ooi::image back into BGR
	cv::cvtColor(Ooi::image,Ooi::image,cv::COLOR_HSV2BGR);
		
}

vector<int> Ooi::getPosition(){
		
	Ooi::positions.push_back(Ooi::xAbsolutePos); 
	Ooi::positions.push_back(Ooi::yAbsolutePos); 
	Ooi::positions.push_back(Ooi::ooiWidth);
       	Ooi::positions.push_back(Ooi::ooiHeight);

	return Ooi::positions;
		
}

cv::Mat Ooi::getObject(){
		
	return Ooi::image;
		
}






