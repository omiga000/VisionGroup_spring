#include <iostream>
#include <chrono>
#include <opencv2/opencv.hpp>
#include<opencv2/geometry.hpp>

using namespace std;
using namespace cv;

// 视频路径
string imgPath = "D:/Scau_Work/Taurus/VisionGroup_spring/Image/emergy_red.mp4";

// HSV 阈值（全局，方便后续加滑动条调试）
int h_min = 0,   h_max = 30;
int h1_min=150,h1_max=179;
int s_min = 110,   s_max = 255;
int v_min = 100, v_max = 255;

int main()
{
    VideoCapture cap(imgPath);
    if (!cap.isOpened())
    {
        cout << "cap error!" << endl;
        return -1;                       // ★ 打不开直接退出
    }

    namedWindow("Emergy", WINDOW_NORMAL);
    resizeWindow("Emergy",1280, 960);

    namedWindow("mask", WINDOW_NORMAL);
    resizeWindow("mask", 640, 480);

    Mat frame, hsv, mask,mask1;

    // FPS 统计
    double fps = 0;
    int    frame_count = 0;
    auto   fps_start = chrono::steady_clock::now();
    
    while (true)
    {
        cap >> frame;
        if (frame.empty())
        {
            cout << "视频结束" << endl;
            break;
        }

        // ---------- 单帧计时开始 ----------
        auto frame_start = chrono::high_resolution_clock::now();

        // ---------- 1. 颜色阈值分割 ----------
        cvtColor(frame, hsv, COLOR_BGR2HSV);
        inRange(hsv, Scalar(h_min, s_min, v_min),Scalar(h_max, s_max, v_max), mask);
        inRange(hsv, Scalar(h1_min, s_min, v_min),Scalar(h1_max, s_max, v_max), mask1);
        bitwise_or(mask,mask1,mask);
        //闭合
        Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
        morphologyEx(mask, mask, MORPH_CLOSE, kernel);


         // ---------- 2. 找轮廓 + 拟合灯条 ----------
        vector<vector<Point>> contours;
        vector<Vec4i> hierarchy;
        findContours(mask, contours, hierarchy,RETR_TREE, CHAIN_APPROX_SIMPLE);
        for(int i=0;i<contours.size();i++)
        {
            if(hierarchy[i][3]!=-1){continue;}
            RotatedRect rect = minAreaRect(contours[i]);
            double h=max(rect.size.height,rect.size.width);
            double w=min(rect.size.height,rect.size.width);
            if(w<1){continue;}
            double rat=h/w;
            double area = contourArea(contours[i]);
        if(area>200&&area<500&&rat>0.8&&rat<1.2&&hierarchy[i][2]==-1){
            circle(frame,rect.center,10,Scalar(255,255,0),2);
        }
            // 排除小圆环、零碎灯光和中心 R 等小轮廓
        if (area < 2000) continue;

        // 保留形状比较细长的轮廓
        if (rat < 1.5 || rat > 2.5) continue;

        // 轮廓包围的面积，占外接矩形面积的比例
        double fill = area / (h * w);

        // 排除较宽、较饱满的轮廓
        if (fill > 0.65) continue;

            
        Point2f plateCenter;
        double bestArea=0;

        for(int j=hierarchy[i][2];j!=-1;j=hierarchy[j][0])
        {
            RotatedRect inner=minAreaRect(contours[j]);

            double innerH=max(inner.size.height,inner.size.width);
            double innerW=min(inner.size.height,inner.size.width);


            if(innerW<1){continue;}

            double innerArea=contourArea(contours[j]);

            if(innerArea<50||innerH/innerW>1.5){continue;}


            if(innerArea>bestArea)
            {
                plateCenter=inner.center;
                bestArea=innerArea;
            }
        }
        if(bestArea==0){continue;}

        circle(frame,plateCenter,5,Scalar(255,255,0),2);



             // 画灯条矩形
            Point2f v[4];
            rect.points(v);
            for (int j = 0; j < 4; j++)
                line(frame, v[j], v[(j + 1) % 4], Scalar(0, 255, 255), 2);

            
            
               
        }
       

        


        // ---------- 4. FPS 统计 ----------
        frame_count++;
        auto now = chrono::steady_clock::now();
        chrono::duration<double> fps_elapsed = now - fps_start;
        if (fps_elapsed.count() >= 1.0)
        {
            fps = frame_count / fps_elapsed.count();
            frame_count = 0;
            fps_start = now;
        }

        // ---------- 5. 单帧耗时 ----------
        auto frame_end = chrono::high_resolution_clock::now();
        chrono::duration<double, milli> frame_ms = frame_end - frame_start;

        // ---------- 6. 叠加文字 ----------
        putText(frame, string("FPS: ") + to_string((int)fps),
                Point(10, 30), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 2);
        putText(frame, string("Time: ") + to_string((int)frame_ms.count()) + " ms",
                Point(10, 70), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 2);

                // ---------- 7. 显示 ----------
        
        imshow("Emergy", frame);
        imshow("mask",mask);


        int key = waitKey(1);
        if (key == 27 || key == 'q') break;
    }
}