#include <iostream>
#include <sstream>
#include <string>
#include <ctime>
#include <cstdio>

#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/stitching.hpp>

using namespace cv;
using namespace std;

int main(int, char**)
{
    //new remapping matrices and variables to store settings
    Mat map1, map2, newCamMat, cameraMatrix, distCoeffs;
    
    //config file constants
    std::string filename = "out_camera_data.xml";
    std::string config_distortion_coeff = "distortion_coefficients";
    std::string config_camera_matrix= "camera_matrix";

    VideoCapture cap(0); // open camera 1
    if(!cap.isOpened()) {
    	cout <<  "Can't find camera 1" << endl;
        return -1;
    }

    cap.set(CV_CAP_PROP_FRAME_WIDTH, 1920);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 1080);
    cap.set(CV_CAP_PROP_FPS, 30);
	
    VideoCapture cap1(1); // open camera 2
    if(!cap1.isOpened()) {
        cout <<  "Can't find camera 2" << endl;
        return -1;
    }

    cap1.set(CV_CAP_PROP_FRAME_WIDTH, 1920);
    cap1.set(CV_CAP_PROP_FRAME_HEIGHT, 1080);
    cap1.set(CV_CAP_PROP_FPS, 30);

    //declare video windows
    namedWindow("view1",1);
    resizeWindow("view1", 1280, 720);
    namedWindow("view2",1);
    resizeWindow("view2", 1280, 720);

    //get size, both cameras same dimensions
    Mat view;
    Size imageSize;
    cap >> view;
    imageSize = view.size();
    cout << to_string(imageSize.width) << endl;
    cout << to_string(imageSize.height) << endl;

    //read undistortion matrix and coefficients and calculate
    FileStorage fs(filename,FileStorage::READ);
	fs[config_camera_matrix] >> cameraMatrix;
	fs[config_distortion_coeff] >> distCoeffs;
	fs.release();
    //get remapping matrices for our camera
	fisheye::estimateNewCameraMatrixForUndistortRectify(cameraMatrix, distCoeffs, imageSize,
                                                               Matx33d::eye(), newCamMat, 1);
    fisheye::initUndistortRectifyMap(cameraMatrix, distCoeffs, Matx33d::eye(), newCamMat, imageSize,
                                             CV_16SC2, map1, map2);
    //initialize sticher
    Mat s1, s2, r1, r2;
    std::vector<Mat> images(2);
    for(int i = 0; i < 2;i++) {
        images.push_back(Mat());
    }
    bool try_use_gpu = true;    // use GPU
    cv::Stitcher stitcher = cv::Stitcher::createDefault(!try_use_gpu);
    cv::Stitcher::Status status;
    cap >> s1; 
    cap1 >> s2;
    remap(s1, r1, map1, map2, INTER_LINEAR);
    remap(s2, r1, map1, map2, INTER_LINEAR);
    images[0] = r1;
    images[1] = r2;
    status = stitcher.estimateTransform(images);
    if (status != cv::Stitcher::OK) {
        cout << "Can't stitcher.estimateTransform, error code = " << status << endl;  
        return -1;
    }

    for(;;)
    {
        Mat frame1, frame2, mappedFrame1, mappedFrame2, pano;
        cap >> frame1; 
        cap1 >> frame2;
        remap(frame1, mappedFrame1, map1, map2, INTER_LINEAR);
        remap(frame2, mappedFrame1, map1, map2, INTER_LINEAR);
        images[0] = mappedFrame1;
        images[1] = mappedFrame2;
        imshow("view1", mappedFrame1);
        imshow("view2", mappedFrame2);
        status = stitcher.composePanorama(images, pano);
        if(waitKey(1) >= 0) break;
    }
    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}

