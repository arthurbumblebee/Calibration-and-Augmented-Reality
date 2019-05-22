/*
	Arthur and Shafat
	S19
	Simple example of Calibration and Augmented Reality
	Based on OpenCV tutorials

	use the makefiles provided to run
	make aug
	../bin/aug test

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
	string camera_parameters_file = "camParameters.xml";
	string dist_coeffs_file = "distCoeffs.xml";

	Mat cameraMatrix = Mat::eye(3, 3, CV_64FC1);
	cameraMatrix.at<float>(0,2) = frame.cols/2;
	cameraMatrix.at<float>(1,2) = frame.rows/2;

	Mat distCoeffs = Mat::zeros(8, 1, CV_64F);
	vector<Mat> rotationVecs, translationVecs;
	Mat rotation_vector, translation_vector;

	for(;!quit;) {
		
		*capdev >> frame; // get a new frame from the camera, treat as a stream
		// frame = imread("../data/checkerboard.png");

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
		point_set = generatePointSet(boardSize);

		// printf("reading calibrations\n");
		FileStorage fsCam (camera_parameters_file, FileStorage::READ);
		FileStorage fsCoeff (dist_coeffs_file, FileStorage::READ);
		if (!fsCam.isOpened() | !fsCoeff.isOpened()){
			cerr << "failed to open parameters files" << endl;
			return 1;
		}
		fsCam["cameraMatrix"] >> cameraMatrix;
		fsCoeff["distCoeffs"] >> distCoeffs;

		if (targetFound){
			// improve the found corners' coordinate accuracy for chessboard
			cvtColor(frame, frameGray, COLOR_BGR2GRAY);
			cornerSubPix( frameGray, corner_set, Size(11,11),
				Size(-1,-1), TermCriteria( TermCriteria::EPS+TermCriteria::COUNT, 30, 0.1 ));
                
			// Draw the corners.
			// drawChessboardCorners( frame, boardSize, Mat(corner_set), targetFound );
			// printf("number of corners = %lu \n", corner_set.size());
			// printf("coordinates of first corner (x, y) : (%f, %f) \n", corner_set[0].x , corner_set[0].y);

			// calculate board's pose(rotation and translation)
			solvePnP(point_set, corner_set, cameraMatrix, distCoeffs, rotation_vector, translation_vector);
			// cout << "rotation vector : " << rotation_vector << endl;
			// cout << "translation vector : " << translation_vector << endl;
		
			// project 3d points
			vector<Point3d> corner_point3D;
			vector<Point2d> corner_point2D;
			corner_point3D.push_back(Point3d(0, 0, 0));
			corner_point3D.push_back(Point3d(1, 0, 0));
			corner_point3D.push_back(Point3d(0,-2, 0));
			corner_point3D.push_back(Point3d(0, 0, 3));
			projectPoints(corner_point3D, rotation_vector, translation_vector, cameraMatrix, distCoeffs, corner_point2D);

			// draw axes, x, y, z from origin
			line( frame, corner_point2D[0], corner_point2D[1], Scalar( 0, 0, 255 ), 2 ); // x axis
			line( frame, corner_point2D[0], corner_point2D[2], Scalar( 0, 255, 0 ), 2 ); // y axis
			line( frame, corner_point2D[0], corner_point2D[3], Scalar( 255, 0, 0 ), 2 ); // z axis

			circle(frame, corner_point2D[0], 5, Scalar(255,100,0), -1, LINE_AA, 0);
		
			// project a cube to the board
			vector<Point3d> cube3D;
			vector<Point2d> cube2D;
			cube3D.push_back(Point3d(3, 0, 0));
			cube3D.push_back(Point3d(3, -3, 0));
			cube3D.push_back(Point3d(6, 0, 0));
			cube3D.push_back(Point3d(6, -3, 0));
			cube3D.push_back(Point3d(3, 0, 3));
			cube3D.push_back(Point3d(6, 0, 3));
			cube3D.push_back(Point3d(6, -3, 3));
			cube3D.push_back(Point3d(3, -3, 3));

			projectPoints(cube3D, rotation_vector, translation_vector, cameraMatrix, distCoeffs, cube2D);

			line( frame, cube2D[0], cube2D[1], Scalar( 100, 100, 255 ), 2);
			line( frame, cube2D[0], cube2D[2], Scalar( 100, 100, 255 ), 2);
			line( frame, cube2D[1], cube2D[3], Scalar( 100, 100, 255 ), 2);
			line( frame, cube2D[2], cube2D[3], Scalar( 100, 100, 255 ), 2);
			line( frame, cube2D[2], cube2D[5], Scalar( 100, 100, 255 ), 2);
			line( frame, cube2D[4], cube2D[5], Scalar( 100, 100, 255 ), 2);
			line( frame, cube2D[7], cube2D[6], Scalar( 100, 100, 255 ), 2);
			line( frame, cube2D[1], cube2D[7], Scalar( 100, 100, 255 ), 2);
			line( frame, cube2D[0], cube2D[4], Scalar( 100, 100, 255 ), 2);
			line( frame, cube2D[5], cube2D[6], Scalar( 100, 100, 255 ), 2);
			line( frame, cube2D[3], cube2D[6], Scalar( 100, 100, 255 ), 2);
			line( frame, cube2D[4], cube2D[7], Scalar( 100, 100, 255 ), 2);

		}


		// display the video frame in the window
		imshow("Video", frame);

		// respond to keypresses
		int key = waitKey(10);
		switch(key) {
			case 'd':
			{
			// Draw the corners.
			drawChessboardCorners( frame, boardSize, Mat(corner_set), targetFound );
			
			break;
			}

			// s store vector corners found by findboardcorners into list
			case 's':
			{
				printf("saving calibration\n");
				corner_list.push_back(corner_set);
				point_list.push_back(point_set);
				// save current calibration image
				imwrite("../data/calib" +to_string(corner_list.size())+ ".png", frame);
		
				break;
			}

			// c calibrate If the user has selected enough calibration frames
				// --require at least 5--then let the user run a calibration
			case 'c':
			{
				if(corner_list.size() >= 5){
					printf("Calibrating...\n");
					// cout << corner_list.size() << endl;
					// cout << point_list.size() << endl;
					// cout << corner_list[0].size() << endl;
					// cout << point_list[0].size() << endl;

					int flag = CALIB_FIX_ASPECT_RATIO;
					double reProjectionError = calibrateCamera(point_list, corner_list, frame.size(), cameraMatrix, distCoeffs, rotationVecs, translationVecs, flag);

					cout << "reProjectionError : " << reProjectionError << endl;
					cout << "cameraMatrix \n" << cameraMatrix << endl;
					cout << "distCoeffs \n" << distCoeffs << endl;
					cout << "frame.cols/2 :" << frame.cols/2 << "\nframe.rows/2 :" << frame.rows/2 << endl;

					// writing to files
					FileStorage file1(camera_parameters_file, FileStorage::WRITE);
					file1 << "cameraMatrix" << cameraMatrix;

					FileStorage file2(dist_coeffs_file, FileStorage::WRITE);
					file2 << "distCoeffs" << distCoeffs;

				}
				break;
			}

			// q quits the program
			case 'q':
			{
				quit = 1;
				break;
			}

			// capture a photo if the user hits p
			case 'p': 
			{
				sprintf(buffer, "../data/board%03d.png", frameid++);
				imwrite(buffer, frame, pars);
				printf("Image written: %s\n", buffer);
				// imwrite("../data/board%03d.png" , frame);

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
