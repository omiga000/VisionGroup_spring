#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

// 标定图片所在文件夹（注意结尾要带 \\）
string imageDir = "D:\\Scau_Work\\Taurus\\VisionGroup_spring\\calib_images\\";

int main()
{
    // ========== 1. 棋盘格参数 ==========
    Size boardSize(7, 10);       // 内角点数量：横向 7，纵向 10
    float squareSize = 15.0f;    // 格子实际边长 (mm)

    // ========== 2. 找到所有标定图片 ==========
    vector<string> files;
    glob(imageDir + "*.jpg", files, false);

    if (files.empty())
    {
        cout << "No images found in: " << imageDir << endl;
        return -1;
    }
    cout << "Found " << files.size() << " images" << endl;

    // ========== 3. 预生成棋盘格 3D 点（所有图共用）==========
    vector<Point3f> corners3D;
    for (int i = 0; i < boardSize.height; i++)
        for (int j = 0; j < boardSize.width; j++)
            corners3D.push_back(Point3f(j * squareSize, i * squareSize, 0));

    // ========== 4. 准备容器（留空，循环里累积）==========
    vector<vector<Point2f>> imagePoints;    // 每张图的 2D 角点
    vector<vector<Point3f>> objectPoints;   // 每张图的 3D 点（相同）
    Size imageSize;                         // 图像尺寸（所有图必须一致）

    int successCount = 0;

    // ========== 5. 逐张处理 ==========
    for (size_t i = 0; i < files.size(); i++)
    {
        Mat img = imread(files[i]);
        if (img.empty())
        {
            cout << "[" << i << "] Failed to load: " << files[i] << endl;
            continue;
        }

        // 记录第一张成功读取的图像的尺寸
        if (imageSize.empty())
            imageSize = img.size();

        // 尺寸不一致则跳过
        if (img.size() != imageSize)
        {
            cout << "[" << i << "] Size mismatch, skip" << endl;
            continue;
        }

        // 转灰度
        Mat gray;
        cvtColor(img, gray, COLOR_BGR2GRAY);

        // 查找棋盘格角点
        vector<Point2f> corners;
        bool found = findChessboardCorners(gray, boardSize, corners,
            CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE);

        if (!found)
        {
            cout << "[" << i << "] Corners not found, skip" << endl;
            continue;
        }

        // 亚像素级角点优化
        cornerSubPix(gray, corners, Size(11, 11), Size(-1, -1),
            TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 30, 0.01));

        // 累积 3D-2D 点对
        objectPoints.push_back(corners3D);
        imagePoints.push_back(corners);
        successCount++;

        cout << "[" << i << "] OK, corners: " << corners.size() << endl;
    }

    cout << "Successfully processed " << successCount << " images" << endl;

    // 有效图片不足，直接退出
    if (successCount < 3)
    {
        cout << "Too few valid images (" << successCount << "), need >= 3" << endl;
        return -1;
    }

    // ========== 6. 标定 ==========
    Mat cameraMatrix = Mat::eye(3, 3, CV_64F);       // 内参矩阵
    Mat distCoeffs = Mat::zeros(1, 5, CV_64F);       // 畸变系数
    vector<Mat> rvecs, tvecs;                        // 每张图的外参

    double ret = calibrateCamera(objectPoints, imagePoints, imageSize,
                                  cameraMatrix, distCoeffs, rvecs, tvecs);

    cout << "Reprojection error: " << ret << " pixels" << endl;
    cout << "Camera matrix:\n" << cameraMatrix << endl;
    cout << "Distortion coefficients:\n" << distCoeffs << endl;

    // ========== 7. 保存参数 ==========
    FileStorage fs("camera_calibration.yml", FileStorage::WRITE);
    fs << "camera_matrix" << cameraMatrix;
    fs << "distortion_coefficients" << distCoeffs;
    fs << "image_width" << imageSize.width;
    fs << "image_height" << imageSize.height;
    fs << "reprojection_error" << ret;
    fs << "num_images_used" << successCount;
    fs.release();

    cout << "Parameters saved to camera_calibration.yml" << endl;
    return 0;
}