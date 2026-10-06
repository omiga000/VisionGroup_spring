#include <iostream>
#include <opencv2/opencv.hpp>
#include <chrono>
using namespace std;
using namespace cv;


//需求1：框选猫猫，保存图片
//需求2：拖动过程中展示框的线条，并且显示鼠标坐标已经像素值
//需求3：框选完成后，单独显示框选图片
//需求4：框选完成后，cout输出框选中心像素点坐标






String imgPath="D:/Scau_Work/Taurus/VisionGroup_spring/Image/cat.png";
Mat img = imread(imgPath);



Point startPoint(-1,-1), endPoint(-1,-1);
bool isDrawing = false;


void onMouse(int event, int x, int y, int flags, void* param)
{
    if(x < 0 || y < 0 || x >= img.cols || y >= img.rows) {
        return;
    }

    if(event == EVENT_LBUTTONDOWN) {
        //进行鼠标事件的判断
        isDrawing = true;
        startPoint.x = x;
        startPoint.y = y;
    } else if(event == EVENT_MOUSEMOVE && isDrawing) {
        //每次拖动都生成新的图像
        Mat imgCopy = img.clone();
        rectangle(imgCopy, Point(startPoint.x, startPoint.y), Point(x, y), Scalar(0, 255, 0), 2);
        putText(imgCopy, "Mouse: (" + to_string(x) + ", " + to_string(y) + ")", Point(x,y), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 0, 0), 2);
        putText(imgCopy, "Pixel Value: (" + to_string((int)img.at<Vec3b>(y, x)[0]) + ", " + to_string((int)img.at<Vec3b>(y, x)[1]) + ", " + to_string((int)img.at<Vec3b>(y, x)[2]) + ")", Point(x,y+20), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 0, 0), 2);
        imshow("Image", imgCopy);
    } else if(event == EVENT_LBUTTONUP && isDrawing) {
        isDrawing = false;
        endPoint.x = x;
        endPoint.y = y;

        // Ensure the rectangle is drawn from top-left to bottom-right
        //确保rect的合法性
        int x1 = min(startPoint.x, endPoint.x);
        int y1 = min(startPoint.y, endPoint.y);
        int x2 = max(startPoint.x, endPoint.x);
        int y2 = max(startPoint.y, endPoint.y);

        // Output the center pixel coordinates
        int centerX = (x1 + x2) / 2;
        int centerY = (y1 + y2) / 2;


        Rect roi(x1, y1, x2 - x1, y2 - y1);
        Mat roiImg = img(roi);
        
        
    
         putText(roiImg, "Mouse: (" + to_string(centerX) + ", " + to_string(centerY) + ")", Point(centerX, centerY), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 0, 0), 2);
        putText(roiImg, "Pixel Value: (" + to_string((int)img.at<Vec3b>(centerY, centerX)[0]) + ", " + to_string((int)img.at<Vec3b>(centerY, centerX)[1]) + ", " + to_string((int)img.at<Vec3b>(centerY, centerX)[2]) + ")", Point(centerX, centerY+20), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 0, 0), 2);
        imshow("ROI", roiImg);

        
        cout << "Center Pixel Coordinates: (" << centerX << ", " << centerY << ")" << endl;
        //输出像素
        cout<<"Center Pixel Value: (" << (int)img.at<Vec3b>(centerY, centerX)[0] << ", " << (int)img.at<Vec3b>(centerY, centerX)[1] << ", " << (int)img.at<Vec3b>(centerY, centerX)[2] << ")" << endl;

        // Save the ROI image
        imwrite("roi_image.png", roiImg);
    }
}
int main()
{
    
    if(img.empty()) {
        cout << "无法读取图片" << endl;
        return -1;
    }
    //目标图像框
    namedWindow("Image", WINDOW_AUTOSIZE);
    

    //绑定鼠标事件到目标框
    setMouseCallback("Image",onMouse);
    
    //放图进框
    imshow("Image", img);

    
    

    waitKey(0);
    return 0;
}