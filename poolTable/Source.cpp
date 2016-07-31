/**
* @file openCVTest.cpp
* @brief Track Pool Balls on a Table
* @author Jake Thomas
*/

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <math.h>

using namespace cv;
using namespace std;

void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
	if (event == EVENT_LBUTTONDOWN)
	{
		cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
	}
	else if (event == EVENT_RBUTTONDOWN)
	{
		cout << "Right button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
	}
	else if (event == EVENT_MBUTTONDOWN)
	{
		cout << "Middle button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
	}
	/*else if (event == EVENT_MOUSEMOVE)
	{
	cout << "Mouse move over the window - position (" << x << ", " << y << ")" << endl;

	}*/
}

Mat hsvSliders(Mat img, Mat hsv, Mat hsvThresh) {
	namedWindow("Control", CV_WINDOW_AUTOSIZE); //create a window called "Control"
												//values that seem good:
												// 114-126, 100-255, 75-255
	int iLowH = 114;
	int iHighH = 126;

	int iLowS = 100;
	int iHighS = 255;

	int iLowV = 75;
	int iHighV = 255;

	//Create trackbars in "Control" window
	createTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
	createTrackbar("HighH", "Control", &iHighH, 179);

	createTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
	createTrackbar("HighS", "Control", &iHighS, 255);

	createTrackbar("LowV", "Control", &iLowV, 255);//Value (0 - 255)
	createTrackbar("HighV", "Control", &iHighV, 255);

	int iLastX = -1;
	int iLastY = -1;
	while (true) {
		//hsv is image with hsv conversion already, threshold it now
		inRange(hsv, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), hsvThresh); //Threshold the image based on sliders

																							  //morphological opening (removes small objects from the foreground)
		erode(hsvThresh, hsvThresh, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
		dilate(hsvThresh, hsvThresh, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

		//morphological closing (removes small holes from the foreground)
		dilate(hsvThresh, hsvThresh, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
		erode(hsvThresh, hsvThresh, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

		//Calculate the moments of the thresholded image
		Moments oMoments = moments(hsvThresh);

		double dM01 = oMoments.m01;
		double dM10 = oMoments.m10;
		double dArea = oMoments.m00;

		imshow("Thresholded Image", hsvThresh); //show the thresholded image

		imshow("Original", img); //show the original image
		if (waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
		{
			cout << "esc key is pressed by user" << endl;
			break;
		}
	}
	return hsvThresh;
}

int main()
{
	Mat img, gray, hsv, hsvThresh;
	img = imread("../data/shrunkCroppedPool1.png");
	cvtColor(img, gray, COLOR_BGR2GRAY);
	// smooth it, otherwise a lot of false circles may be detected
	int sizeB = 9;
	cvtColor(img, hsv, COLOR_BGR2HSV);
	/*
	//namedWindow("HSV", 1);
	//imshow("HSV", hsv);
	//threshold hsv/// Separate the image in 3 places ( H, S, V )
	//vector<Mat> hsv_planes;
	//split(img, hsv_planes);
	//threshold(hsv_planes[0], hsvThresh, 145, 230, THRESH_BINARY);
	//namedWindow("Threshold", 1);
	//imshow("Threshold", hsvThresh);
	//findHist(hsv);
	
	GaussianBlur(gray, gray, Size(sizeB, sizeB), 2, 2);
	namedWindow("Gray", 1);
	imshow("Gray", gray);
	namedWindow("src", 1);
	imshow("src", img);
	*/

	hsvThresh = hsvSliders(img, hsv, hsvThresh);
	GaussianBlur(hsvThresh, hsvThresh, Size(9, 9), 2, 2);
	/*
	// Table Range = x, y
	// Top Left = 95, 32
	// Top Right = 368, 33
	// Mid Left = 56, 262
	// Mid Right = 400, 263
	// Bot Left = 0, 637
	// Bot Right = 443, 639
	*/
	int tlX = 95; int tlY = 32;
	int trX = 368; int trY = 33;
	int mlX = 56; int mlY = 262;
	int mrX = 400; int mrY = 263;
	int blX = 0; int blY = 637;
	int brX = 443; int brY = 639;

	vector<Point2f> vert(6);
	vert[0] = Point(tlX, tlY);
	vert[1] = Point(trX, trY);
	vert[2] = Point(mrX, mrY);
	vert[3] = Point(brX, brY);
	vert[4] = Point(blX, blY);
	vert[5] = Point(mlX, mlY);
	Mat contourMapping = Mat::zeros(img.size(), CV_8UC1);
	/// Draw it in src
	for (int j = 0; j < 6; j++)
	{
		line(contourMapping, vert[j], vert[(j + 1) % 6], Scalar(255), 3, 8);
	}
	/// Get the contours
	vector<vector<Point> > contours; vector<Vec4i> hierarchy;
	findContours(contourMapping, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

	vector<Vec3f> circles;
	//			     image, circles,         method,dp, minDist, param1, param2, min radius, max radius
	HoughCircles(hsvThresh, circles, HOUGH_GRADIENT, 1, 20, 20, 10, 9, 15);
	for (size_t i = 0; i < circles.size(); i++)
	{
		Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
		//if(true){
		if (pointPolygonTest(contours[0], Point2f(center), false) == 1) {
			int radius = cvRound(circles[i][2]);
			// draw the circle center
			circle(img, center, 3, Scalar(0, 255, 0), -1, 8, 0);
			// draw the circle outline
			circle(img, center, radius, Scalar(0, 0, 255), 3, 8, 0);
		}
	}
	//namedWindow("hsvThresh", 1);
	//imshow("hsvThresh", hsvThresh);

	// Draw table outline
	for (int j = 0; j < 6; j++)
	{
		line(img, vert[j], vert[(j + 1) % 6], Scalar(0, 0, 255), 1, 8);
	}
	namedWindow("circles", 1);
	imshow("circles", img);
	//set the callback function for any mouse event
	setMouseCallback("circles", CallBackFunc, NULL);
	waitKey(0);
	return 0;
}
