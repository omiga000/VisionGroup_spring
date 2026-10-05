#include<iostream>
#include<opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int h_min = 0, h_max = 179;
int s_min = 0, s_max = 255;
int v_min = 0, v_max = 255;
int main() {
    // Your code here

    Mat img = imread("D:/Scau_Work/Taurus/VisionGroup_spring/Image/01.png");

    if(img.empty()) {
        cout << "图片读取失败" << endl;
        return -1;
    }

    namedWindow("hsv", WINDOW_NORMAL);
    resizeWindow("hsv", 600, 400);


    createTrackbar("H_min", "hsv", &h_min, 179);
    createTrackbar("H_max", "hsv", &h_max, 179);
    createTrackbar("S_min", "hsv", &s_min, 255);
    createTrackbar("S_max", "hsv", &s_max, 255);
    createTrackbar("V_min", "hsv", &v_min, 255);
    createTrackbar("V_max", "hsv", &v_max, 255);

    Mat hsv_img, mask, result;
    cvtColor(img, hsv_img, COLOR_BGR2HSV);
    while(true) {
        
        inRange(hsv_img, Scalar(h_min, s_min, v_min), Scalar(h_max, s_max, v_max), mask);
        

        imshow("hsv", mask);

        char key = (char)waitKey(30);
        if(key=='s')
        {
            //保存
            FileStorage file("hsv_values.yml", FileStorage::WRITE);
            file << "h_min" << h_min;
            file << "h_max" << h_max;
            file << "s_min" << s_min;
            file << "s_max" << s_max;
            file << "v_min" << v_min;
            file << "v_max" << v_max;
            cout << "HSV values saved to hsv_values.yml" << endl;
            file.release();
        }

        if(key == 27) { // ESC key
            break;
        }
    }
    return 0;
}