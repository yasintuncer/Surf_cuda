// gpu_frame.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include<iostream>
#include <time.h>
#include "opencv2/opencv_modules.hpp"
#include "opencv2/core.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/cudafeatures2d.hpp"
#include "opencv2/xfeatures2d/cuda.hpp"

#define GOOD_MATCHES_COEFF 2

using namespace std;
using namespace cv;
using namespace cv::cuda;

	SURF_CUDA surf; //cuda xfeatures class
	GpuMat gpuFrame;
	GpuMat gpuImage;
	//Keypoint on gpu
	GpuMat kpImage;
	GpuMat kpFrame;
	//descriptors on gpu
	GpuMat dsImage;
	GpuMat dsFrame;
	// keypoint
	vector<KeyPoint> kp1;
	vector<KeyPoint> kp2;
	//descriptors
	vector<float> ds1;
	vector<float> ds2;

int main(int argc, const char* argv[])
{

	setDevice(0);


	Mat image;
	
	image = cv::imread("../data/deneme.jpg", IMREAD_GRAYSCALE);	//sample image 

	gpuImage.upload(image);				//load gpu memory

	if (gpuImage.empty())
	{
	cout << "Error!!: Upload is not succes" << endl;
	}
	
	
	surf(gpuImage, GpuMat(), kpImage, dsImage); //feature extraction sample image


	if (kp1.empty() || ds1.empty())
	{
		cout << "Keypoint and Descriptors not found" << endl;
		return -1;
	}


	VideoCapture cap(0);

	if (!cap.isOpened())
	{
		cout << "Capture not Started!!" << endl;
		return -1;
	}

	
	for (;;)
	{
		Mat frame;
		cap >> frame; //cam read
		cvtColor(frame, frame, CV_BGR2GRAY);

		gpuFrame.upload(frame);			//frame uploaded
		
		if (gpuFrame.empty())
		{
			cout << "Frame not loaded gpu " << endl;
			break;
		}
		
		surf(gpuFrame, GpuMat(), kpFrame, dsFrame); //feature extraction frame 
		
		cout << "FOUND " << kpFrame.cols << " keypoints on first image" << endl;
		cout << "FOUND " << kpImage.cols << " keypoints on second image" << endl;

		cout << ds2.size() << endl;
		vector<DMatch> matches;
		Ptr<cv::cuda::DescriptorMatcher> matcher = cv::cuda::DescriptorMatcher::createBFMatcher(NORM_L2);
		Mat img_matches;

		matcher->match(dsFrame, dsImage, matches);


		if (!matches.size() == 0)
		{
			double max_dist = 0;
			double min_dist = 100;
			for (int i = 0; i < (int)matches.size(); ++i)
			{
				double dist = matches[i].distance;
				if (dist < min_dist)
					min_dist = dist;
				if (dist > max_dist)
					max_dist = dist;
			}




			vector<DMatch> good_matches;
			for (int i = 0; i < (int)matches.size(); ++i)
			{
				if (matches[i].distance < GOOD_MATCHES_COEFF * min_dist)
					good_matches.push_back(matches[i]);
			}
			

			surf.downloadKeypoints(kpFrame, kp2);
			surf.downloadDescriptors(dsFrame, ds2);




			drawMatches(image, kp2, frame, kp1, good_matches, img_matches, Scalar::all(-1), Scalar::all(-1), vector<char>(),
				DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
		}
		else
		{
			img_matches = image;


		}
		
			

		cv::namedWindow("Surf image", CV_WINDOW_NORMAL);
		imshow("Surf image", img_matches);

		waitKey(20);




	}

	




	}

