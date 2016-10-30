#include <iostream>
#include <sstream>
#include <string>
#include <ctime>
#include <cstdio>
#include <cstdlib>

// For UDPSocket and SocketException
#include "PracticalSocket.h"
#include "config.h"

// Stitching
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/stitching/stitcher.hpp"

#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/stitching.hpp>

using namespace cv;
using namespace std;

int main(int argc, char * argv[]) {

    // if ((argc < 3) || (argc > 3)) { // Test for correct number of arguments
    //     cerr << "Usage: " << argv[0] << " <Server> <Server Port>\n";
    //     exit(1);
    // }

    string servAddress = "127.0.0.1"; // First arg: server address
    unsigned short servPort0 = Socket::resolveService("10000", "udp");
    unsigned short servPort1 = Socket::resolveService("10001", "udp");

    // Declare Mats
    Mat edges, edges1, view, map1, map2, frame, frame1, newCamMat, cameraMatrix, distCoeffs;

    // Config file
    std::string filename = "out_camera_data.xml";
    std::string config_distortion_coeff = "distortion_coefficients";
    std::string config_camera_matrix= "camera_matrix";

    // Camera 0
    VideoCapture cap(0); // open the default camera
    if(!cap.isOpened()) {
        cout <<  "Can't find camera 1" << endl;
        return -1;
    }
    cap.set(CV_CAP_PROP_FRAME_WIDTH, 1920);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, 1080);
    // declare video window
    namedWindow("Camera 0",1);

    // Camera 1
    VideoCapture cap1(2); // open the default camera
    if(!cap1.isOpened()) {
        cout <<  "Can't find camera 2" << endl;
        return -1;
    }
    cap1.set(CV_CAP_PROP_FRAME_WIDTH, 1920);
    cap1.set(CV_CAP_PROP_FRAME_HEIGHT, 1080);
    //declare video window
    namedWindow("Camera 1",2);


    // Get picture size
    Size imageSize;
    cap >> view;
    imageSize = view.size();
    cout << to_string(imageSize.width) << endl;
    cout << to_string(imageSize.height) << endl;

    // Read undistortion matrix and coefficients and calculate
    FileStorage fs(filename,FileStorage::READ);
    fs[config_camera_matrix] >> cameraMatrix;
    fs[config_distortion_coeff] >> distCoeffs;
    fs.release();

    fisheye::estimateNewCameraMatrixForUndistortRectify(cameraMatrix, distCoeffs, imageSize,
                                                               Matx33d::eye(), newCamMat, 1);
    fisheye::initUndistortRectifyMap(cameraMatrix, distCoeffs, Matx33d::eye(), newCamMat, imageSize,
                                             CV_16SC2, map1, map2);

    int x = 70;
    int y = 150;
    cv::Rect roi;
    roi.x = x;
    roi.y = y;

    Mat pano;
    Stitcher stitcher = Stitcher::createDefault(true); // create a Stitcher object

    try {
        UDPSocket sock;
        int jpegqual =  ENCODE_QUALITY; // Compression Parameter
        vector < uchar > encoded, encoded1;
        clock_t last_cycle, last_cycle1 = clock();
        for(;;) {
            // Capture pic and crop
            cap >> frame;
            cap1 >> frame1;
            remap(frame, edges, map1, map2, INTER_LINEAR);
            remap(frame1, edges1, map1, map2, INTER_LINEAR);
            roi.width = edges.size().width - (x*2);
            roi.height = edges.size().height - (y*2);
            cv::Mat edges_cropped = edges(roi);
            cv::Mat edges_cropped1 = edges1(roi);
            
            // /* Send camera 0 over socket */
            // vector < int > compression_params;
            // compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
            // compression_params.push_back(jpegqual);
            // imencode(".jpg", edges_cropped, encoded, compression_params);
            // int total_pack = 1 + (encoded.size() - 1) / PACK_SIZE;
            // int ibuf[1];
            // ibuf[0] = total_pack;
            // sock.sendTo(ibuf, sizeof(int), servAddress, servPort0);

            // for (int i = 0; i < total_pack; i++)
            //     sock.sendTo( & encoded[i * PACK_SIZE], PACK_SIZE, servAddress, servPort0);

            // waitKey(FRAME_INTERVAL);

            // clock_t next_cycle = clock();
            // double duration = (next_cycle - last_cycle) / (double) CLOCKS_PER_SEC;
            // cout << "\teffective FPS:" << (1 / duration) << " \tkbps:" << (PACK_SIZE * total_pack / duration / 1024 * 8) << endl;

            // cout << next_cycle - last_cycle;
            // last_cycle = next_cycle;

            // /* Send camera 1 over socket */
            // vector < int > compression_params1;
            // compression_params1.push_back(CV_IMWRITE_JPEG_QUALITY);
            // compression_params1.push_back(jpegqual);
            // imencode(".jpg", edges_cropped1, encoded1, compression_params1);
            // int total_pack1 = 1 + (encoded1.size() - 1) / PACK_SIZE;
            // int ibuf1[1];
            // ibuf1[0] = total_pack1;
            // sock.sendTo(ibuf1, sizeof(int), servAddress, servPort1);

            // for (int i = 0; i < total_pack1; i++)
            //     sock.sendTo( & encoded1[i * PACK_SIZE], PACK_SIZE, servAddress, servPort1);

            // waitKey(FRAME_INTERVAL);

            // cout << next_cycle - last_cycle;
            // last_cycle = next_cycle;

            vector<Mat> imgs;
            imgs.push_back(edges);
            imgs.push_back(edges1);
            Stitcher::Status status = stitcher.stitch(imgs, pano); // stitch the input images together
            imshow("Pano", pano);

            // imshow("Camera 0", edges_cropped);
            // imshow("Camera 1", edges_cropped1);

            if(waitKey(1) >= 0) break;
        }
    } catch (SocketException & e) {
        cerr << e.what() << endl;
        exit(1);
    }

    return 0;
}

