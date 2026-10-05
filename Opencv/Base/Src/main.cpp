#include<iostream>
#include<opencv2/opencv.hpp>
using namespace std;

using namespace cv;
int h_min = 139, h_max = 179;
int s_min = 21, s_max = 255;
int v_min = 172, v_max = 255;
int main()
{
    Mat img = imread("D:/Scau_Work/Taurus/VisionGroup_spring/Image/01.png");
    if(img.empty())
    {
        cout<<"图片读取失败"<<endl;
        return -1;
    }
    namedWindow("image",WINDOW_NORMAL);
    imshow("image",img);

    Mat hsv_img;
    cvtColor(img,hsv_img,COLOR_BGR2HSV);
    namedWindow("hsv",WINDOW_AUTOSIZE);

    Mat mask;


   
    //掩膜


    inRange(hsv_img,Scalar(h_min,s_min,v_min),Scalar(h_max,s_max,v_max),mask);


    

    //形态学处理 注意核形状
    //闭运算
    Mat element = getStructuringElement(MORPH_ELLIPSE,Size(7,7));

    //morphologyEx(mask,mask,MORPH_OPEN,element);
        morphologyEx(mask,mask,MORPH_CLOSE,element);

    Mat element_open = getStructuringElement(MORPH_ELLIPSE,Size(5,5));
            morphologyEx(mask,mask,MORPH_OPEN,element_open);


    imshow("hsv",mask);

    waitKey(0);
    return 0;
}