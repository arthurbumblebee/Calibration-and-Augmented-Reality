/*
	Arthur and Shafat
	S19
	Simple example of video capture and manipulation
	Based on OpenCV tutorials

	Compile command (macos)
	clang++ -o vid -I /opt/local/include vidDisplay.cpp -L /opt/local/lib -lopencv_core -lopencv_highgui -lopencv_video -lopencv_videoio

	use the makefiles provided
	make vid
*/

#include <cstdio>
#include <cstring>

#include "opencv2/opencv.hpp"
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d.hpp"


using namespace cv;
using namespace std;

int main(int argc, char *argv[]) {
	VideoCapture *capdev;
	char label[256];
	int quit = 0;
	int frameid = 0;
	char buffer[256];
	vector<int> pars;

	pars.push_back(5);

	// open the video device
	capdev = new VideoCapture(0);
	if( !capdev->isOpened() ) {
		printf("Unable to open video device\n");
		return(-1);
	}

	Size refS( (int) capdev->get(CAP_PROP_FRAME_WIDTH ),
		       (int) capdev->get(CAP_PROP_FRAME_HEIGHT));

	printf("Expected size: %d %d\n", refS.width, refS.height);

	namedWindow("Video", 1); // identifies a window?

	// matrices to hold multiple image outputs, etc
	Mat frame;

	for(;!quit;) {
		
		*capdev >> frame; // get a new frame from the camera, treat as a stream
		// frame = imread("../data/training/donut.001.png");

		if( frame.empty() ) {
		  printf("frame is empty\n");
		  break;
		}

		// find the positions of internal corners of the chessboard
		vector<Point2f> corner_set;
		int columns = 9;
		int rows = 6;
		bool targetFound;
		Size boardSize(columns, rows);
		int chessBoardFlags = CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE + CALIB_CB_FAST_CHECK;

		targetFound = findChessboardCorners( frame, boardSize, corner_set, chessBoardFlags);

		if (targetFound){
			// improve the found corners' coordinate accuracy for chessboard
			Mat frameGray;
			cvtColor(frame, frameGray, COLOR_BGR2GRAY);
			cornerSubPix( frameGray, corner_set, Size(11,11),
				Size(-1,-1), TermCriteria( TermCriteria::EPS+TermCriteria::COUNT, 30, 0.1 ));
                
			// Draw the corners.
			drawChessboardCorners( frame, boardSize, Mat(corner_set), targetFound );
			// printf("number of corners = %lu \n", corner_set.size());
			// printf("coordinates of first corner (x, y) : (%f, %f) \n", corner_set[0].x , corner_set[0].y);
		
		}

		// display the video frame in the window
		imshow("Video", frame);

		// respond to keypresses
		int key = waitKey(10);
		switch(key) {
			// q quits the program
			case 'q':
				quit = 1;
				break;

			// capture a photo if the user hits c
			case 'c': 
				sprintf(buffer, "%s.%03d.png", label, frameid++);
				imwrite(buffer, frame, pars);
				printf("Image written: %s\n", buffer);
				break;
			case 't':
				quit = 1;
				break;

			default:
				break;
		}

	} // end for

	// terminate the video capture
	printf("Terminating\n");
	delete capdev;

	return(0);
}
