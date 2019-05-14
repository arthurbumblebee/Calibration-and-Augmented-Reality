/*
	Arthur and Shafat
	S19
	Simple example of Calibration and Augmented Reality
	Based on OpenCV tutorials

	use the makefiles provided
	make aug
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

// a method that converts the corner_set coordinates from image-relative 
// coordinates to coordinates relative to the real world
vector<Point3f> generatePointSet(Size boardSize){
	vector<Point3f> pointSet;
	for (int i = 0; i < boardSize.height; i++){
		for (int j = 0; j < boardSize.width; j++){
			Point3f point(j, -i, 0);
			pointSet.push_back(point);
		}
	}
	return pointSet;
}

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
	Mat frame, frameGray;
	vector<Point2f> corner_set;
	vector<Point3f> point_set;
	vector<vector<Point2f>> corner_list;
	vector<vector<Point3f>> point_list;
	int columns, rows, chessBoardFlags;
	bool targetFound;

	Mat cameraMatrix = Mat::eye(3, 3, CV_64FC1);
	cameraMatrix.at<float>(0,2) = frame.cols/2;
    cameraMatrix.at<float>(1,2) = frame.rows/2;

	Mat distCoeffs = Mat::zeros(8, 1, CV_64F);

	for(;!quit;) {
		
		// *capdev >> frame; // get a new frame from the camera, treat as a stream
		frame = imread("../data/checkerboard.png");

		if( frame.empty() ) {
		  printf("frame is empty\n");
		  break;
		}

		// find the positions of internal corners of the chessboard
		columns = 9;
		rows = 6;
		Size boardSize(columns, rows);
		chessBoardFlags = CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE + CALIB_CB_FAST_CHECK;

		targetFound = findChessboardCorners( frame, boardSize, corner_set, chessBoardFlags);

		if (targetFound){
			// improve the found corners' coordinate accuracy for chessboard
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
			// s store vector corners found by findboardcorners into list
			case 's':
				printf("saving calibration\n");
				corner_list.push_back(corner_set);
				point_list.push_back(generatePointSet(boardSize));
				// save current calibration image
				imwrite("../data/calib" +to_string(corner_list.size())+ ".png", frame);
				
				// writing to files
				FileStorage fs1("camParameters.xml", FileStorage::WRITE);
				fs1 << "cameraMatrix" << cameraMatrix;

				FileStorage fs2("distCoeffs.xml", FileStorage::WRITE);
				fs2 << "distCoeffs" << distCoeffs;

				break;

			// c calibrate If the user has selected enough calibration frames
				// --require at least 5--then let the user run a calibration
			case 'c':
				if(corner_list.size() >= 1){
					printf("Calibrating...\n");
					// cout << corner_list.size() << endl;
					// cout << point_list.size() << endl;
					// cout << corner_list[0].size() << endl;
					// cout << point_list[0].size() << endl;

					vector<Mat> rotationVecs, translationVecs;
					int flag = CALIB_FIX_ASPECT_RATIO;
					double reProjectionError = calibrateCamera(point_list, corner_list, frame.size(), cameraMatrix, distCoeffs, rotationVecs, translationVecs, flag);

					cout << "reProjectionError : " << reProjectionError << endl;
					cout << "cameraMatrix \n" << cameraMatrix << endl;
					cout << "distCoeffs \n" << distCoeffs << endl;
					cout << "frame.cols/2 \n" << frame.cols/2 << "  frame.rows/2\n" << frame.rows/2 << endl;

				}
				break;

			// q quits the program
			case 'q':
				quit = 1;
				break;

			// capture a photo if the user hits c
			case 'p': 
				sprintf(buffer, "%s.%03d.png", label, frameid++);
				imwrite(buffer, frame, pars);
				printf("Image written: %s\n", buffer);
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
