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
    Mat map1, map2, newCamMat, cameraMatrix, distCoeffs, pano, mappedFrame1, mappedFrame2;
    
    //config file constants
    std::string filename = "out_camera_data.xml";
    std::string config_distortion_coeff = "distortion_coefficients";
    std::string config_camera_matrix= "camera_matrix";

    VideoCapture cap(0); // open camera 1
    if(!cap.isOpened()) {
    	cout <<  "Can't find camera 1" << endl;
        return -1;
    }
    //setup camera 1
    cap.set(CV_CAP_PROP_FRAME_WIDTH, 1920);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 1080);
	
    VideoCapture cap1(2); // open camera 2
    if(!cap1.isOpened()) {
        cout <<  "Can't find camera 2" << endl;
        return -1;
    }
    //setup camera 2
    cap1.set(CV_CAP_PROP_FRAME_WIDTH, 1920);
    cap1.set(CV_CAP_PROP_FRAME_HEIGHT, 1080);

    //declare video windows
    namedWindow("view1",1);

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
    cv::Rect myROI;
    myROI.x = 80;
    myROI.y = 170;
    myROI.width = imageSize.width - 2*80;
    myROI.height = imageSize.height - 2*170;
    vector<Mat> images(2);
    //initialize vector
    bool try_use_gpu = true;    // use GPU
    Stitcher stitcher = Stitcher::createDefault(try_use_gpu);
    std::vector<cv::detail::CameraParams> cams;
    Stitcher::Status status;
    bool has_transform = false;

    for(;;)
    {
        cap >> s1; 
        cap1 >> s2;
        remap(s1, mappedFrame1, map1, map2, INTER_LINEAR);
        remap(s2, mappedFrame2, map1, map2, INTER_LINEAR);
        images[0] = mappedFrame1(myROI);
        images[1] = mappedFrame2(myROI);

        // if( !has_transform ) {
        //     status = stitcher.estimateTransform(images);
        //     has_transform = true;
        //     cout << "estimated transform" << endl;
        //     cams = stitcher.cameras();
        // } 
        // stitcher.setCameras(cams);
        // status = stitcher.composePanorama(images, pano);   
        stitcher.stitch( images, pano );     
        imshow("view1", pano);
        if(waitKey(1) >= 0) break;
    }

    return 0;
}

