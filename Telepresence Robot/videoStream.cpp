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

using namespace cv;
using namespace std;

int main(int, char**)
{
    VideoCapture cap(0); // open the default camera
    if(!cap.isOpened()) {
    	cout <<  "Can't find camera" << endl;
        return -1;
    }

    cap.set(CV_CAP_PROP_FRAME_WIDTH, 1920);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 1080);
    Mat edges,view, map1, map2, newCamMat, cameraMatrix, distCoeffs;
    //config file
    std::string filename = "out_camera_data.xml";
    std::string config_distortion_coeff = "distortion_coefficients";
    std::string config_camera_matrix= "camera_matrix";

    //declare video window
    namedWindow("edges",1);


    //get size
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

	fisheye::estimateNewCameraMatrixForUndistortRectify(cameraMatrix, distCoeffs, imageSize,
                                                               Matx33d::eye(), newCamMat, 1);
    fisheye::initUndistortRectifyMap(cameraMatrix, distCoeffs, Matx33d::eye(), newCamMat, imageSize,
                                             CV_16SC2, map1, map2);


    for(;;)
    {
        Mat frame;
        cap >> frame; // get a new frame from camera
        remap(frame, edges, map1, map2, INTER_LINEAR);
        imshow("edges", edges);
        if(waitKey(1) >= 0) break;
    }
    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}

