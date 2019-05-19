/*
	Arthur and Shafat
	S19
    Detect Harris corners
	Based on OpenCV tutorials

	use the makefiles provided to run
	make harris
	../bin/harris
	
*/

#include "opencv2/opencv.hpp"
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d.hpp"

#include <cstdio>
#include <cstring>
#include <iostream>
#include <stdio.h>

using namespace cv;
using namespace std;

int main(int argc, char *argv[]) {
    VideoCapture *capdev;
	int quit = 0;
    int frameid = 0;
	char buffer[256];
	vector<int> pars;

	// open the video device
	capdev = new VideoCapture(0);
	if( !capdev->isOpened() ) {
		printf("Unable to open video device\n");
		return(-1);
	}

	namedWindow("Harris", 1); // identifies a window?

	Mat frame, frameGray, result, result_norm, result_norm_scaled;

    int thresh = 200;
    int max_thresh = 255;

	for(;!quit;) {
		
		*capdev >> frame; // get a new frame from the camera, treat as a stream
		// frame = imread("../data/checkerboard.png");

		if( frame.empty() ) {
		  printf("frame is empty\n");
		  break;
		}

        // convert to grayscale
        cvtColor( frame, frameGray, COLOR_BGR2GRAY );

        // parameters for the system
        int blockSize = 2;
        int apertureSize = 3;
        double k = 0.04;

        result = Mat::zeros( frame.size(), CV_32FC1 );

        // detect the harris corners
        cornerHarris( frameGray, result, blockSize, apertureSize, k );

        // normalize and scale the result
        normalize( result, result_norm, 0, 255, NORM_MINMAX, CV_32FC1, Mat() );
        convertScaleAbs( result_norm, result_norm_scaled );

        // draw circles on the original image around the corners
        for( int i = 0; i < result_norm.rows ; i++ )
        {
            for( int j = 0; j < result_norm.cols; j++ )
            {
                if( (int) result_norm.at<float>(i,j) > thresh )
                {
                    circle( frame, Point(j,i), 5,  Scalar(0), 2, 8, 0 );
                }
            }
        }

        // display the video frame in the window
        imshow("Harris corners", frame);

        // respond to keypresses
        int key = waitKey(10);
        switch(key) {
            // q quits the program
			case 'q':
			{
				quit = 1;
				break;
			}

            // capture a photo if the user hits p
			case 'p': 
			{
				sprintf(buffer, "../data/harris%03d.png", frameid++);
				imwrite(buffer, frame, pars);
				printf("Image written: %s\n", buffer);

				break;
			}

            default:
            break;
        }

	} // end for

	// terminate the video capture
	printf("Terminating\n");
	delete capdev;

	return(0);

}