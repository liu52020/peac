#include "AHCPlaneFitter.hpp"

struct OrganizedImage3D {
    const cv::Mat_<cv::Vec3f>& cloud;
    //note: ahc::PlaneFitter assumes mm as unit!!!
    OrganizedImage3D(const cv::Mat_<cv::Vec3f>& c): cloud(c) {}  // init cloud
    inline int width() const { return cloud.cols; }
    inline int height() const { return cloud.rows; }
    inline bool get(const int row, const int col, double& x, double& y, double& z) const {
        const cv::Vec3f& p = cloud.at<cv::Vec3f>(row,col);
        x = p[0];
        y = p[1];
        z = p[2];
        return z > 0 && isnan(z)==0; //return false if current depth is NaN
    }
};
typedef ahc::PlaneFitter< OrganizedImage3D > PlaneFitter;

int main()
{
    cv::Mat depth = cv::imread("/home/liuwei/data/stair/depth/123.png",cv::IMREAD_ANYDEPTH);
    const float f = 613;  // 焦距
    const float cx = 327;
    const float cy = 225;
    const float max_use_range = 10;

    cv::Mat_<cv::Vec3f> cloud(depth.rows, depth.cols);
    for(int r=0; r<depth.rows; r++)
    {
        const unsigned short* depth_ptr = depth.ptr<unsigned short>(r);
        cv::Vec3f* pt_ptr = cloud.ptr<cv::Vec3f>(r);
        for(int c=0; c<depth.cols; c++)
        {
            float z = (float)depth_ptr[c]/5000.0;
            if(z>max_use_range){z=0;}
            pt_ptr[c][0] = (c-cx)/f*z*1000.0;//m->mm
            pt_ptr[c][1] = (r-cy)/f*z*1000.0;//m->mm
            pt_ptr[c][2] = z*1000.0;//m->mm
        }
    }
    std::cout<<"图像数据的大小："<<cloud.size()<<std::endl;
    // 其实也可以进行一个滤波 平滑一下
    PlaneFitter pf;
    pf.minSupport = 3000;  // 最小支持的点数
    pf.windowWidth = 20;  // 将图像划分为40*40
    pf.windowHeight = 20;
    pf.doRefine = true;

    cv::Mat seg(depth.rows, depth.cols, CV_8UC3);
    OrganizedImage3D Ixyz(cloud);
    pf.run(&Ixyz, 0, &seg);


    cv::Mat depth_color;
    depth.convertTo(depth_color, CV_8UC1, 50.0/5000);
    applyColorMap(depth_color, depth_color, cv::COLORMAP_JET);
    cv::imshow("seg",seg);
    cv::imshow("depth",depth_color);
    cv::waitKey();
}
