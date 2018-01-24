#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include "config.hpp"
#include "utils.hpp"
#include "optimizer.hpp"

using namespace std;
using namespace ssvo;

int main(int argc, char const *argv[])
{
    google::InitGoogleLogging(argv[0]);
    if(argc != 2)
    {
        std::cout << " Usage: ./test_optimizer config_file" << std::endl;
        return -1;
    }

    Config::FileName = std::string(argv[1]);
    int width = Config::imageWidth();
    int height = Config::imageHeight();
    cv::Mat K = Config::cameraIntrinsic();
    cv::Mat DistCoef = Config::cameraDistCoef();

    Camera::Ptr cam = Camera::create(Config::imageWidth(), Config::imageHeight(), K, DistCoef);
    cv::Mat img = cv::Mat(width, height, CV_8UC1);

    KeyFrame::Ptr kf1 = KeyFrame::create(Frame::create(img, 0, cam));
    KeyFrame::Ptr kf2 = KeyFrame::create(Frame::create(img, 0, cam));
    KeyFrame::Ptr kf3 = KeyFrame::create(Frame::create(img, 0, cam));

    kf1->setPose(Eigen::Matrix3d::Identity(), Eigen::Vector3d(0.0,0.0,0.0));
    kf2->setPose(Eigen::Matrix3d::Identity(), Eigen::Vector3d(0.1,0.0,0.0));
    kf3->setPose(Eigen::Matrix3d::Identity(), Eigen::Vector3d(0.2,0.0,0.0));

    Eigen::Vector3d pose(0.1, 0.3, 3);
    MapPoint::Ptr mpt = MapPoint::create(pose);

    Eigen::Vector2d px1 = kf1->cam_->project(kf1->Tcw() * pose);
    Eigen::Vector2d px2 = kf1->cam_->project(kf2->Tcw() * pose);
    Eigen::Vector2d px3 = kf1->cam_->project(kf3->Tcw() * pose);
    std::cout << "px1: " << px1.transpose() << std::endl;
    std::cout << "px2: " << px2.transpose() << std::endl;
    std::cout << "px3: " << px3.transpose() << std::endl;

    Feature::Ptr ft1 = Feature::create(px1, kf1->cam_->lift(px1), 0, mpt);
    Feature::Ptr ft2 = Feature::create(px2, kf1->cam_->lift(px2), 0, mpt);
    Feature::Ptr ft3 = Feature::create(px3, kf1->cam_->lift(px3), 0, mpt);
    mpt->addObservation(kf1, ft1);
    mpt->addObservation(kf2, ft2);
    mpt->addObservation(kf3, ft3);
    mpt->setPose(pose[0]+0.001, pose[1]-0.001, pose[2]+0.05);

    double rpj_err_pre = 0;
    rpj_err_pre += utils::reprojectError(ft1->fn_.head<2>(), kf1->Tcw(), mpt->pose());
    rpj_err_pre += utils::reprojectError(ft2->fn_.head<2>(), kf2->Tcw(), mpt->pose());
    rpj_err_pre += utils::reprojectError(ft3->fn_.head<2>(), kf3->Tcw(), mpt->pose());

    Optimizer::refineMapPoint(mpt, 10, true, true);

    double rpj_err_aft = 0;
    rpj_err_aft += utils::reprojectError(ft1->fn_.head<2>(), kf1->Tcw(), mpt->pose());
    rpj_err_aft += utils::reprojectError(ft2->fn_.head<2>(), kf2->Tcw(), mpt->pose());
    rpj_err_aft += utils::reprojectError(ft3->fn_.head<2>(), kf3->Tcw(), mpt->pose());

    Eigen::Vector2d px11 = kf1->cam_->project(kf1->Tcw() * pose);
    Eigen::Vector2d px21 = kf1->cam_->project(kf2->Tcw() * pose);
    Eigen::Vector2d px31 = kf1->cam_->project(kf3->Tcw() * pose);
    std::cout << "px1: " << px11.transpose() << std::endl;
    std::cout << "px2: " << px21.transpose() << std::endl;
    std::cout << "px3: " << px31.transpose() << std::endl;

    std::cout << "Reproject Error changed from " << rpj_err_pre << " to " << rpj_err_aft << std::endl;

    return 0;
}
