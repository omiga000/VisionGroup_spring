#include<iostream>
#include<opencv2/opencv.hpp>
#include <opencv2/geometry.hpp> 
using namespace std;
using namespace cv;
//图片路径
string imgPath = "D:/Scau_Work/Taurus/VisionGroup_spring/Image/apple.png";
Mat img = imread(imgPath);


//HSV阈值范围
int h_min = 0, h_max = 26;
int h1_min = 162, h1_max = 179;
int s_min = 124, s_max = 255;
int v_min = 85, v_max = 255;



int main()
{
    if(img.empty())
    {
        cout << "图片读取失败" << endl;
        return -1;
    }
    Mat img_clone = img.clone();


    //转换hsv
    Mat hsv_img, mask,mask1,mask2;
    cvtColor(img, hsv_img, COLOR_BGR2HSV);


    namedWindow("hsv", WINDOW_AUTOSIZE);
    //掩膜，红色h有有两个范围，所以bitwise_or两个掩膜
    inRange(hsv_img, Scalar(h_min, s_min, v_min), Scalar(h_max, s_max, v_max), mask1);
    inRange(hsv_img, Scalar(h1_min, s_min, v_min), Scalar(h1_max, s_max, v_max), mask2);
    bitwise_or(mask1, mask2, mask);

    //形态学处理
    Mat result;
    Mat kernel = getStructuringElement(MORPH_RECT, Size(21,21));
    morphologyEx(mask, result, MORPH_OPEN, kernel);

    Mat kernel2 = getStructuringElement(MORPH_RECT, Size(13, 13));
    morphologyEx(result, result, MORPH_CLOSE, kernel2);


    vector<vector<Point>> contours;
    findContours(result, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    int maxArea = 0;
    int maxContourIndex = -1;

    for(int i = 0; i < contours.size(); i++)
    {
        double area = cv::contourArea(contours[i]);
        if(area > maxArea)
        {
            maxArea = area;
            maxContourIndex = i;
        }
    }
    
    if(maxContourIndex != -1)
    {
        vector<Point>hull;
        convexHull(contours[maxContourIndex], hull);//凸包算法，解决树叶遮挡问题

        drawContours(img_clone, vector<vector<Point>>{hull}, -1, Scalar(0, 0, 255), 2);

        Rect rect=boundingRect(hull);

        
        rectangle(img_clone,rect,Scalar(255,0,0),2);


        //drawContours(img_clone, contours, maxContourIndex, Scalar(0, 255, 0), 2);
    }
    imshow("result", result);
    imshow("Image", img_clone);

    waitKey(0);
    return 0;
}