#include <iostream>
#include <chrono>
#include <opencv2/opencv.hpp>
#include<opencv2/geometry.hpp>
#include <opencv2/calib3d.hpp>


using namespace std;
using namespace cv;

// 视频路径
string imgPath = "D:/Scau_Work/Taurus/VisionGroup_spring/Image/armor_red.mp4";

// HSV 阈值（全局，方便后续加滑动条调试）
int h_min = 0,   h_max = 179;
int s_min = 0,   s_max = 255;
int v_min = 150, v_max = 255;

float W = 135.0f;
float H = 62.0f;

std::vector<cv::Point3f> objectPoints = {
    {-W / 2, -H / 2, 0},
    { W / 2, -H / 2, 0},
    { W / 2,  H / 2, 0},
    {-W / 2,  H / 2, 0}
};
Mat K,D;

int main()
{
    VideoCapture cap(imgPath);
    if (!cap.isOpened())
    {
        cout << "cap error!" << endl;
        return -1;                       // ★ 打不开直接退出
    }

    namedWindow("Armor", WINDOW_NORMAL);
    resizeWindow("Armor", 640, 480);

    Mat frame, hsv, mask;

    // FPS 统计
    double fps = 0;
    int    frame_count = 0;
    auto   fps_start = chrono::steady_clock::now();

    // 1. 打开 YAML 文件
    FileStorage fs("D:/Scau_Work/Taurus/VisionGroup_spring/build/Opencv/Application/camera_calibration.yml", FileStorage::READ);

    if (!fs.isOpened())
    {
        cerr << "无法打开 camera.yaml" << endl;
        return -1;
    }

    // 2. 读取相机内参和畸变系数
    fs["camera_matrix"] >> K;
    fs["distortion_coefficients"] >> D;

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
        inRange(hsv, Scalar(h_min, s_min, v_min),
                     Scalar(h_max, s_max, v_max), mask);

        // ---------- 2. 找轮廓 + 拟合灯条 ----------
        vector<vector<Point>> contours;
        findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        vector<RotatedRect> lightRects;
        for (size_t i = 0; i < contours.size(); i++)
        {
            if (contourArea(contours[i]) < 100) continue;

            RotatedRect rect = minAreaRect(contours[i]);
            lightRects.push_back(rect);

            // 画灯条矩形
            Point2f v[4];
            rect.points(v);
            for (int j = 0; j < 4; j++)
                line(frame, v[j], v[(j + 1) % 4], Scalar(0, 0, 255), 2);

            // 画灯条中心
            circle(frame, rect.center, 2, Scalar(0, 255, 0), 2);
        }

        // ---------- 3. 灯条配对 ----------
        if (lightRects.size() >= 2)          // ★ 少于 2 个直接跳过配对
        {
            for (size_t i = 0; i + 1 < lightRects.size(); i++)   // ★ 不要写 size()-1
            {
                RotatedRect rect = lightRects[i];
                for (size_t j = i + 1; j < lightRects.size(); j++)
                {
                    RotatedRect rect1 = lightRects[j];

                    // --- 角度过滤 ---
                    double diff = fabs(rect.angle - rect1.angle);
                    if (diff > 90) diff = 180 - diff;   // ★ 归一化到 [0,90]
                    if (diff > 10) continue;

                    // --- 画装甲板中心 + 连线 ---
                    Point2f centre((rect.center.x + rect1.center.x) / 2,
                                   (rect.center.y + rect1.center.y) / 2);
                    circle(frame, centre, 4, Scalar(0, 255, 0), 3);
                    line(frame, rect.center, rect1.center, Scalar(255, 0, 0), 2);

                    // --- 用两灯条 8 个顶点拟合装甲板外接矩形 ---
                    Point2f v[4], v1[4];
                    rect.points(v);
                    rect1.points(v1);

                    vector<Point2f> allPoints;
                    for (int k = 0; k < 4; k++)
                    {
                        allPoints.push_back(v[k]);
                        allPoints.push_back(v1[k]);
                    }

                    RotatedRect armorRect = minAreaRect(allPoints);
                    Point2f v3[4];
                    armorRect.points(v3);
                    for (int m = 0; m < 4; m++)
                        line(frame, v3[m], v3[(m + 1) % 4], Scalar(0, 0, 255), 2);
                    
                    vector<Point2f> imagePoints = {
                        v3[1], v3[2], v3[3], v3[0]
                    };

                    Mat rvec, tvec;

                    bool success = solvePnP(
                        objectPoints,
                        imagePoints,
                        K,
                        D,
                        rvec,
                        tvec
                    );

                    if (success)
                    {
                        // 直线距离，单位 mm
                        double distance = norm(tvec);

                        // 显示距离，转换为 m
                        putText(
                            frame,
                            cv::format("Distance: %.2f m",
                                   distance / 1000.0),
                            Point(10, 110),
                            FONT_HERSHEY_SIMPLEX,
                            0.8,
                            Scalar(0, 255, 0),
                            2
                        );

                        // 控制台输出
                        cout << "Distance: "
                             << distance / 1000.0
                             << " m" << endl;
                    }
                    else
                    {
                        cout << "solve fail!" << endl;
                    }

                }
            }
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
        imshow("Armor", frame);

        int key = waitKey(1);
        if (key == 27 || key == 'q') break;
    }

    cap.release();
    destroyAllWindows();
    return 0;
}