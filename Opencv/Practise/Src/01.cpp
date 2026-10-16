#include<iostream>
#include<opencv2/opencv.hpp>
#include <opencv2/geometry.hpp> 
#include <opencv2/dnn.hpp>
using namespace std;
using namespace cv;
//图片路径
string imgPath = "D:/Scau_Work/Taurus/VisionGroup_spring/images/01.jpg";
string modelPath= "D:/Scau_Work/Taurus/VisionGroup_spring/tutorials/armor_number/model/mlp.onnx";
Mat img=imread(imgPath);
Mat img_clone=img.clone();
RotatedRect armorRect;
dnn::Net net;

// HSV 阈值（全局，方便后续加滑动条调试）
int h_min = 0,   h_max = 179;
int s_min = 0,   s_max = 255;
int v_min = 225, v_max = 255;


float W = 135.0f;
float H = 62.0f;

std::vector<cv::Point3f> objectPoints = {
    {-W / 2, -H / 2, 0},
    { W / 2, -H / 2, 0},
    { W / 2,  H / 2, 0},
    {-W / 2,  H / 2, 0}
};
Mat K,D;


auto longAxisAngle = [](const RotatedRect& r) {
    double angle = r.angle;

    if (r.size.height > r.size.width) {
        angle += 90.0;
    }

    return angle;
};

void detect_armor()
{
    try {
    net = dnn::readNetFromONNX(modelPath, dnn::ENGINE_CLASSIC);
    cout << "success" << endl;
}
catch (const cv::Exception& e) {
    cerr << "unsuccess" << e.what() << endl;
    return ;
}

    Mat hsv,mask,gray;


    cvtColor(img_clone,gray,COLOR_BGR2GRAY);
    threshold(gray,mask,230,255,THRESH_BINARY);
    imshow("mask",mask);

    //轮廓提取
    vector<vector<Point>>contours;
    findContours(mask,contours,RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    vector<RotatedRect>LightRects;
    for(int i=0;i<contours.size();i++)
    {
        double area=contourArea(contours[i]);
        if(area<500){continue;}
        if(area>750){continue;}

        RotatedRect rect=minAreaRect(contours[i]);
        double w=min(rect.size.height,rect.size.width);
        double h=max(rect.size.height,rect.size.width);
        double rat;
        if(w!=0)
        {
            rat=h/w;
        }
        if(rat<3){continue;}


        cout<<"detected Light"<<endl;
        LightRects.push_back(rect);
        drawContours(img_clone,contours[i],i,Scalar(0,255,0),2);
    }

    //装甲板识别
    for(int i=0;i<LightRects.size()-1;i++)
    {
        RotatedRect rect1=LightRects[i];
        for(int j=i+1;j<LightRects.size();j++)
        {
            RotatedRect rect2=LightRects[j];

            double diff=abs(longAxisAngle(rect1)-longAxisAngle(rect2));

            if(diff>90){diff=180-diff;}

            if(diff>10){continue;}

            cout<<"detected Armor"<<endl;


            Point2f Centre;
            Centre.x=(rect1.center.x+rect2.center.x)/2;
            Centre.y=(rect1.center.y+rect2.center.y)/2;
            circle(img_clone,(Point)Centre,4,Scalar(0,255,0),3);
            line(img_clone, rect1.center, rect2.center, Scalar(0, 255, 255), 2);

            //截取装甲板图像
            Point2f v1[4],v2[4];
            rect1.points(v1);
            rect2.points(v2);
            vector<Point2f>allPoints;
            for(int k=0;k<4;k++)
            {
                allPoints.push_back(v1[k]);
                allPoints.push_back(v2[k]);
            }
            armorRect=minAreaRect(allPoints);
            Point2f v3[4];
            armorRect.points(v3);
            for(int m=0;m<4;m++)
            {
                line(img_clone,v3[m],v3[(m+1)%4],Scalar(0,255,0),2);
            }
        }
    }    


}

void solvePnp()
{
    // 1. 打开 YAML 文件
    FileStorage fs("D:/Scau_Work/Taurus/VisionGroup_spring/build/Opencv/Application/camera_calibration.yml", FileStorage::READ);

    if (!fs.isOpened())
    {
        cerr << "无法打开 camera.yaml" << endl;
        return ;
    }

    // 2. 读取相机内参和畸变系数
    fs["camera_matrix"] >> K;
    fs["distortion_coefficients"] >> D;

    Point2f p[4];
    armorRect.points(p);

    // 按 x 排序：前两个在左侧，后两个在右侧
    std::sort(p, p + 4, [](const Point2f& a, const Point2f& b) {
    return a.x < b.x;
    });

    // 每一侧都让上面的点排在前面
    if (p[0].y > p[1].y) std::swap(p[0], p[1]);
    if (p[2].y > p[3].y) std::swap(p[2], p[3]);

    vector<Point2f> imagePoints = {
        p[0],  // 0：左上
        p[2],  // 1：右上
        p[3],  // 2：右下
        p[1]   // 3：左下
    }; 
    Mat rvec, tvec;
    
    bool success=solvePnP(objectPoints,imagePoints,K,D,rvec,tvec);

    if (success)
    {
        // 直线距离，单位 mm
        double distance = norm(tvec);

        // 显示距离，转换为 m
        putText(
            img_clone,
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

void detect_number()
{

    Mat input;
    Rect roiRect=armorRect.boundingRect();

    // 2. 向四周扩大 20 像素
    int padding = 25;

    roiRect.x -= padding;
    roiRect.y -= padding;
    roiRect.width += 2 * padding;
    roiRect.height += 2 * padding;
    roiRect &= Rect(0, 0, img.cols, img.rows);

    Mat roi=img(roiRect);

    // 将装甲板区域转换为灰度图
    Mat gray;
    cvtColor(roi, gray, COLOR_BGR2GRAY);

    int numberWidth = cvRound(gray.cols * 0.45);
    int numberX = (gray.cols - numberWidth) / 2;

    Rect numberRect(numberX, 0, numberWidth, gray.rows);
    Mat number=gray(numberRect);
    //imshow("number",number);


    Mat binary; 
    threshold(number, binary, 0, 255, THRESH_BINARY | THRESH_OTSU);


            // 显示灰度图
    string windowName = "Binary" ;
    namedWindow(windowName, WINDOW_NORMAL);
    imshow(windowName, binary);
    //匹配模型输入尺寸
    resize(binary,input,Size(20,28),0,0,INTER_NEAREST);

    Mat blob=dnn::blobFromImage(input,1.0/255.0);
    net.setInput(blob);

    Mat scores=net.forward();

    cout << "Output" << scores << endl;

    // 类别顺序与模型配套的 label.txt 一致
    vector<string> labels = {
    "1", "2", "3", "4", "5",
    "outpost", "guard", "base", "negative"
    };

    // 把模型输出整理成一行
    Mat scoresRow = scores.reshape(1, 1);

    // 找到最高分的位置
    Point maxPosition;
    minMaxLoc(scoresRow, nullptr, nullptr, nullptr, &maxPosition);

    // 将位置转换成类别名称
    string label = labels.at(maxPosition.x);

     putText(img_clone, string("Number: ") +label,
                Point(10, 70), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 2);

}
int main()
{
    if(img.empty())
    {
        cout<<"fail to open img"<<endl;
        return -1;
    }

    //命名图像窗口
    namedWindow("img",WINDOW_NORMAL);
    resizeWindow("img",640,480);

    detect_armor();
    solvePnp();
    detect_number();

    imshow("img_clone",img_clone);
    

    waitKey(0);
    return 0;
    
}