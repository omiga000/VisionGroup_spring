#include <iostream>
#include <opencv2/opencv.hpp>
#include <chrono>
using namespace std;
using namespace cv;

int main() {
    VideoCapture cap(0);
    VideoWriter writer;
    bool ifrecording = false; // 是否保存视频


    if(!cap.isOpened()) {
        cout << "无法打开摄像头" << endl;
        return -1;
    }
    //曝光设置

    //关闭自动曝光
    cap.set(CAP_PROP_AUTO_EXPOSURE, 0.25); // 0.25 for manual mode


    int exposureValue_min=-13; // 曝光值，范围通常在 -13 到 -1 之间，具体取决于摄像头
    int exposureValue_max=-1;
    int exposureValue_value=50; // 曝光值，范围通常在 -13 到 -1 之间，具体取决于摄像头

    namedWindow("Video", WINDOW_NORMAL);



    createTrackbar("Exposure", "Video", &exposureValue_value, 100);
   

    Mat frame;




    double fps = 0;                 // 初始化
    int frame_count = 0;
    auto start = chrono::steady_clock::now();

    while(true) {
        cap >> frame;
        if(frame.empty()) {
            cout << "无法读取视频帧" << endl;
            break;
        }

        //计算真实曝光
        int real_exposure = exposureValue_min + (exposureValue_value * (exposureValue_max - exposureValue_min) / 100);
        cap.set(CAP_PROP_EXPOSURE, real_exposure); // 设置曝光值


        int w = frame.cols;         // 用实际帧尺寸
        int h = frame.rows;




        // 先统计 FPS
        frame_count++;
        auto now = chrono::steady_clock::now();
        chrono::duration<double> elapsed = now - start;
        if(elapsed.count() >= 1.0) {
            fps = frame_count / elapsed.count();
            frame_count = 0;
            start = now;
        }

        // 再画上去
        putText(frame, "Width: "  + to_string(w), Point(10, 30),
                FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 2);
        putText(frame, "Height: " + to_string(h), Point(10, 70),
                FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 2);
        putText(frame, "FPS: "    + to_string((int)fps), Point(10, 110),
                FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 2);


        //write

       

        imshow("Video", frame);
        if(ifrecording) {
            writer.write(frame);
        }
        char key = (char)waitKey(1);
        if(key == 27) { // ESC key
            break;
        }
        else if(key == 's') {
            if(!ifrecording) {
                // 开始保存视频
                string filename = "output.avi";
                int fourcc = VideoWriter::fourcc('M', 'J', 'P', 'G');
                writer.open(filename, fourcc, fps, Size(w, h));
                if(!writer.isOpened()) {
                    cout << "cannot open video writer" << endl;
                    return -1;
                }
                ifrecording = true;
                cout << "start recording: " << filename << endl;
            } else {
                // 停止保存视频
                writer.release();
                ifrecording = false;
                cout << "stop recording" << endl;
            }
        }
    }

    cap.release();
    writer.release();
    destroyAllWindows();
    return 0;
}