#include<iostream>
#include<opencv2/opencv.hpp>
#include <opencv2/geometry.hpp> 
using namespace std;
using namespace cv;
//图片路径
string imgPath = "D:/Scau_Work/Taurus/VisionGroup_spring/Image/armor.png";
Mat img = imread(imgPath);
Mat mask;
void dectect_light()
{
     Mat hsv;
    cvtColor(img,hsv,COLOR_BGR2HSV);

    inRange(hsv,Scalar(0,0,120),Scalar(179,255,255),mask);

    vector<vector<Point>>contours;
    findContours(mask,contours,RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    //存储灯条数据
    vector<RotatedRect>lightRects;
    for(int i=0;i<contours.size();i++)
    {
        double area=contourArea(contours[i]);
        if(area<100){continue;}
        //旋转矩形
        RotatedRect rect=minAreaRect(contours[i]);

        //过滤条件
        if(rect.center.y<800){continue;}
        
        //保存
        lightRects.push_back(rect);
        
        //四个顶点
        Point2f v[4];
        rect.points(v);

        //画出灯条旋转矩形
        for(int j=0;j<4;j++)
        {
            line(img,v[j],v[(j+1)%4],Scalar(0,0,255),2);
        }
        //画出中心
        circle(img,(Point)rect.center,2,Scalar(0,255,0),2);
    }

    for(int i=0;i<lightRects.size()-1;i++)
    {
        RotatedRect rect=lightRects[i];

        for(int j=i+1;j<lightRects.size();j++)
        {
            RotatedRect rect1=lightRects[j];

            double diff=abs(rect.angle-rect1.angle);

            if(diff>90){diff=diff-180;}

            if(diff>10){continue;}

            Point2f Centre;
            Centre.x=(rect.center.x+rect1.center.x)/2;
            Centre.y=(rect.center.y+rect1.center.y)/2;
            circle(img,(Point)Centre,4,Scalar(0,255,0),3);
            line(img, rect.center, rect1.center, Scalar(0, 255, 255), 2);

            //截取装甲板图像
            Point2f v[4],v1[4];
            rect.points(v);
            rect1.points(v1);
            vector<Point2f>allPoints;
            for(int k=0;k<4;k++)
            {
                allPoints.push_back(v[k]);
                allPoints.push_back(v1[k]);
            }

            RotatedRect armorRect=minAreaRect(allPoints);
            Point2f v3[4];
            armorRect.points(v3);
            for(int m=0;m<4;m++)
            {
                line(img,v3[m],v3[(m+1)%4],Scalar(0,255,0),2);
            }

            // 1. 把 RotatedRect 转成外接正矩形 Rect
            Rect roi_rect = armorRect.boundingRect();
            roi_rect &= Rect(0, 0, img.cols, img.rows);  // 防止越界

            // 2. 裁剪 ROI
            Mat roi = img(roi_rect).clone();

            // 3. 窗口名用 to_string 拼接
            imshow("armor_roi_" + to_string(i) + "_" + to_string(j), roi);
        }
    }
}


int main()
{

    if(img.empty())
    {
        cout<<"img is empty!"<<endl;
        return -1;
    }

    namedWindow("Image",WINDOW_NORMAL);
    resizeWindow("Image",640,480);
    namedWindow("mask",WINDOW_NORMAL);
    resizeWindow("mask",640,480);


    
    dectect_light();

    imshow("mask",mask);
    imshow("Image",img);

    waitKey(0);

    return 0;
}