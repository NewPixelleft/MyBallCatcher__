#pragma once
#include <opencv2/opencv.hpp>
#include <rclcpp/rclcpp.hpp>
#include <chrono>

struct Ball {
    int r;
    cv::Point2f pos;
    cv::Point2f vel;
    cv::Scalar col;

    bool isTracking = false;
    std::chrono::steady_clock::time_point tracktime;
};
