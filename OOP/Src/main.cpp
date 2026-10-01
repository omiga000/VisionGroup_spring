#include<iostream>
#include<cmath>
using namespace std;
//结构体
typedef struct Point
{
    double x;
    double y;
};


typedef struct rect 
{
    int ID;
    int color;
    Point point;
    double h;  
    double w;
};
//类初始化函数public: armor(rect r):r(r) {};
    

class armor
{
    public:
    armor(rect r):r(r) {};

    Point getCentre() const { return Point{r.point.x + r.w/2, r.point.y + r.h/2}; }

    void amror_point() const { 
    cout<<r.point.x<<" "<<r.point.y<<endl;
    cout<<r.point.x + r.w<<" "<<r.point.y<<endl;
    cout<<r.point.x + r.w<<" "<<r.point.y + r.h<<endl;
    cout<<r.point.x<<" "<<r.point.y + r.h<<endl;  
    }

    double getDiagonal() const { return sqrt(r.w*r.w + r.h*r.h); }

    void getcolor() const { cout<<r.color<<endl; }


    private:
    rect r;

};








int main()
{
    rect r;
    cin>>r.ID>>r.color;
    cin>>r.point.x>>r.point.y;
    cin>>r.w>>r.h;
    //cout<<r.ID<<" "<<r.color<<" "<<r.point.x<<" "<<r.point.y<<" "<<r.w<<" "<<r.h<<endl;
    armor a(r);
    Point centre = a.getCentre();
    cout<<centre.x<<" "<<centre.y<<endl;
    double diagonal = a.getDiagonal();
    cout<<diagonal<<endl;
    a.amror_point();
    a.getcolor();   
    return 0;
}