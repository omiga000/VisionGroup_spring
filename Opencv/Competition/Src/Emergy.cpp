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
int s_min = 0,   s_max = 255;
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
    resizeWindow("Emergy", 640, 480);

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


         // ---------- 2. 找轮廓 + 拟合灯条 ----------
        vector<vector<Point>> contours;
        vector<Vec4i> hierarchy;
        findContours(mask, contours, hierarchy,RETR_TREE, CHAIN_APPROX_SIMPLE);
        for(int i=0;i<contours.size();i++)
        {
            RotatedRect rect = minAreaRect(contours[i]);
            double h=max(rect.size.height,rect.size.width);
            double w=min(rect.size.height,rect.size.width);
            double rat=h/w;

            if (contourArea(contours[i]) < 100) continue;
            
            if(contourArea(contours[i])<1000&&rat>1.5){continue;}
            

            
            if(contourArea(contours[i]) < 500){circle(frame,rect.center,15,Scalar(0,255,0),2);
                continue;}


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