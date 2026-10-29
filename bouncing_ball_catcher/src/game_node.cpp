// Game node needs to do several things

#include <memory>
#include <functional>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <cstddef>
#include <algorithm>
#include <stdexcept>
#include <chrono>
#include <random>
#include <limits>

#include "colourParser.h"
#include "balls.h"
#include "game_configParser.h"

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <std_msgs/msg/int32.hpp>

#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

#define defaultQueueDepth 10

int ballspeed = 40;
int userscore = 0;

void updateBallSpeed(){
    ballspeed += 10;
    ballspeed = ballspeed * 11 / 10;
}

void updateUserScore(){
    userscore++;
}

class GameNode : public rclcpp::Node{
public:
    GameNode() : Node("game_node") {
        RCLCPP_INFO(
            this->get_logger(), "Game Node Initialized"
        );
        hsvBarrier cb;
        try {
            std::ifstream colfile("colourConfig.txt");
            if (parseColour(colfile, cb) != 0){
                std::cerr << "Failed to read colour file";
                return;
            }
        }
        catch(std::exception &e){
            std::cerr << "Failed in initiating colour file";
            return;
        }
        try{
            GAMECFG cfg;
            gameVarInit(cfg);

            balls = generateBalls(cfg.ballCount, cfg.ballRadius, ballspeed);
        }
        catch(std::exception &e){
            std::cerr << "Failed at initiating game config!";
            return;
        }
        
        this->declare_parameter <std::string>("target_color", cb.colname);


        this->declare_parameter<int>("huemin", cb.huemin);
        this->declare_parameter<int>("huemax", cb.huemax);

        this->declare_parameter<int>("satmin", cb.satmin);
        this->declare_parameter<int>("satmax", cb.satmax);

        this->declare_parameter<int>("valmin", cb.valmin);
        this->declare_parameter<int>("valmax", cb.valmax);

        this->declare_parameter<int>("minarea", 100);
        this->declare_parameter<double>("holdDuration", 2.0f);

        
        colname = this->get_parameter("target_color").as_string();

        huemin = this->get_parameter("huemin").as_int();
        huemax = this->get_parameter("huemax").as_int();

        satmin = this->get_parameter("satmin").as_int();
        satmax = this->get_parameter("satmax").as_int();

        valmin = this->get_parameter("valmin").as_int();
        valmax = this->get_parameter("valmax").as_int();

        minarea = this->get_parameter("minarea").as_int();
        holdDuration = this->get_parameter("holdDuration").as_double();

        scorePub = this->   
                    create_publisher<std_msgs::msg::Int32>("/game/score", 10);
        
        debugImagePub = this->create_publisher<sensor_msgs::msg::Image>(
                                "/game/debug_image", defaultQueueDepth
                                );
        
    }
private:
    std::vector <Ball> balls;
    int globalBallRadius;
    std::string colname;
    int huemin,
        huemax,
        satmin,
        satmax,
        valmin,
        valmax;
    std::chrono::steady_clock::time_point lastUpdate =
    std::chrono::steady_clock::now();
    rclcpp::Subscription
        <sensor_msgs::msg::Image>::SharedPtr
        imageSubscriber = (
            this -> create_subscription<sensor_msgs::msg::Image>
                    ("/camera/image_raw", defaultQueueDepth,
                        std::bind(
                            &GameNode::imageCallback,
                            this,
                            std::placeholders::_1
                        )
                    )
        );
    
    rclcpp::Publisher <std_msgs::msg::Int32>::SharedPtr scorePub;
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr debugImagePub;

    int minarea;
    double holdDuration;

    void imageCallback(const sensor_msgs::msg::Image::SharedPtr msg){
        cv::Mat frame;
        try{        
            frame = cv_bridge::toCvCopy(msg, "bgr8")->image;
            // RCLCPP_INFO(
            //     this->get_logger(),
            //     "Received Image: %d x %d", frame.cols, frame.rows
            // );
            // RCLCPP_INFO(
            //     this->get_logger(),
            //     "Colour: %s | H: %d-%d | S: %d-%d | V: %d-%d",
            //     colname.c_str(),
            //     huemin, huemax,
            //     satmin, satmax,
            //     valmin, valmax
            // );
        }
        // klo error
        catch (const cv_bridge::Exception &e){
            RCLCPP_ERROR(
                this->get_logger(),
                "Error when initiating CV! %s", e.what()
            );
            return;
        }
        cv::flip(frame, frame, 1);

        cv::Mat hsv;
        cv::Mat mask;

        cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);


        cv::inRange(
            hsv,
            cv::Scalar(huemin, satmin, valmin),
            cv::Scalar(huemax, satmax, valmax),
            mask
        );

        std::vector <
            std::vector <cv::Point>
        > contours;

        cv::findContours
            (   mask.clone(), 
                contours, 
                cv::RETR_EXTERNAL, 
                cv::CHAIN_APPROX_SIMPLE
            );
        
        double largestArea = 0.0;
        int largestIdx = -1;
        
        setLargestArea(largestArea, largestIdx, contours);

        int X = 0, Y = 0;
        bool detected = false;
        if (largestIdx >= 0 && largestArea > minarea){
            drawFrame(frame, contours[largestIdx], X, Y);
            detected = true;
        }
        
        auto now = std::chrono::steady_clock::now();

        float dt = std::chrono::duration<float>(
            now - lastUpdate
        ).count();

        lastUpdate = now;

        dt = std::min(dt, 0.05f);

        Ball *nearestBall = nullptr;
        float nearestDsq = std::numeric_limits <float>::max();
        if (detected){
            for (Ball &b: balls){
                float dx = static_cast<float>(X) - b.pos.x;
                float dy = static_cast<float>(Y) - b.pos.y;
                
                float dsq= dx * dx + dy * dy;
                if (dsq <= b.r *b.r && dsq < nearestDsq) {
                        
                        nearestDsq = dsq;
                        nearestBall = &b;
                    }
                if (&b != nearestBall){
                        b.isTracking = false;
                }
            }
        }
        for (Ball &b: balls){
            if (&b != nearestBall) b.isTracking = false;
        }

        if (nearestBall != nullptr){
            if (!nearestBall-> isTracking){
                nearestBall->isTracking = true;
                nearestBall->tracktime = now;
            } else {
                auto elapsed = std::chrono::duration<float>(
                    now - nearestBall->tracktime
                ).count();
                if (elapsed>= holdDuration){
                    updateUserScore();
                    nearestBall->isTracking = false;
                    // YOUR BALLS IS BEING TRACKED
                    // YOU WIN $100 IF THEY'RE TRACKED FOR 2 SECOND STRAIGHTS
                    // AFTER THAT, YOUR BALLS WILL RESPAWN SOMEWHERE ELSE

                    std_msgs::msg::Int32 scoreMsg;
                    scoreMsg.data = userscore;
                    scorePub->publish(scoreMsg);
                    // WORSE :  EVERYONE KNOWS THAT YOU EARN $100 
                    //          FROM EXPOSING YOUR BALLS TO THE
                    //          ROS2 STD_MSGS

                    RCLCPP_INFO(
                        this->get_logger(),
                        "Score: %d",
                        userscore
                    );

                    updateBallSpeed();
                    respawnBall(*nearestBall);
                }
            }
        }

        updateballs(dt, frame.cols, frame.rows);
        ballCollisions();
        drawBalls(frame);  
        showUserScore(frame);
        
        auto debugMsg = cv_bridge::CvImage(
            msg->header,
            "bgr8",
            frame
        ).toImageMsg();

        debugImagePub->publish(*debugMsg);
        
        cv::imshow("Blue Mask", mask);
        cv::imshow("Bouncing Ball Catcher", frame);
        cv::waitKey(1);
    }
    void showUserScore(cv::Mat &frame){
        std::string str = "SCORE: " + std::to_string(userscore);
        cv::putText(frame, str, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 124, 0), 2);
    }

    void setLargestArea
        (   double &largestArea, 
            int &idx, 
            const std::vector<std::vector<cv::Point>> &contGroup
        )
    {
        for (int i = 0; i < static_cast<int>(contGroup.size()); i++){
            double area = cv::contourArea(contGroup[i]);
            if (largestArea < area){
                largestArea = area;
                idx = i;
            }
        }
    }

    void drawFrame(cv::Mat &frame, const std::vector <cv::Point> &obj, int &X, int &Y){
        cv::Moments m = cv::moments(obj);
        cv::Rect box = cv::boundingRect(obj);
        cv::rectangle(frame, box, cv::Scalar(125, 255, 0), 2);
        if(m.m00 != 0.0){
            X = static_cast<int>(m.m10 /m.m00);
            Y = static_cast<int>(m.m01 /m.m00);

            cv::circle(
                frame,
                cv::Point (X, Y),
                5, 
                cv::Scalar(125, 255, 0),
                -1
            );
            std::string txt = "(" + std::to_string(X) + ", " +std::to_string(Y) + ")";
            cv::putText(
                frame, txt, cv::Point(X + 10, Y),
                cv::FONT_HERSHEY_SIMPLEX,
                0.5,
                cv::Scalar(125, 255, 0),
                2
            );
        }
    }

    void respawnBall(Ball &ball, int width = 640, int height = 480){
        static std::mt19937 rng(std::random_device{}());

        std::uniform_int_distribution <int> xDist(
            ball.r, width - ball.r
        );
        std::uniform_int_distribution <int> yDist(
            ball.r, height - ball.r
        );
        constexpr int maxAttempts = 1000;

        for (int attempt = 0; attempt < maxAttempts; ++attempt) {
            cv::Point2f position(
                static_cast<float>(xDist(rng)),
                static_cast<float>(yDist(rng))
            );

            bool overlaps = false;

            for (const Ball &other : balls) {
                if (&other == &ball) {
                    continue;
                }

                cv::Point2f delta = position - other.pos;
                float minDistance = static_cast<float>(
                    ball.r + other.r
                );

                if (delta.dot(delta) < minDistance * minDistance) {
                    overlaps = true;
                    break;
                }
            }

            if (!overlaps) {
                ball.pos = position;
                ball.vel = randomVelocity(ballspeed);
                ball.isTracking = false;
                return;
            }
        }

        RCLCPP_WARN(
            this->get_logger(),
            "Could not find a non-overlapping respawn position"
        );
    }

    void updateballs(
        float dt,
        int width = 640,
        int height = 480
    ) {
        for (Ball &b : balls) {
            b.pos.x += b.vel.x * dt;
            b.pos.y += b.vel.y * dt;
            if (b.pos.x - b.r < 0) {
                b.pos.x = static_cast<float>(b.r);
                b.vel.x = std::abs(b.vel.x);
            }
            else if (b.pos.x + b.r > width) {
                b.pos.x = static_cast<float>(width - b.r);
                b.vel.x = -std::abs(b.vel.x);
            }
            if (b.pos.y - b.r < 0) {
                b.pos.y = static_cast<float>(b.r);
                b.vel.y = std::abs(b.vel.y);
            }
            else if (b.pos.y + b.r > height) {
                b.pos.y = static_cast<float>(height - b.r);
                b.vel.y = -std::abs(b.vel.y);
            }
        }
    }
    std::vector<Ball> generateBalls(
        int count,
        int radius,
        float speed,
        int width = 640,
        int height = 480
    ) {
        if (count < 1 || radius <= 0 || speed <= 0.0f ||
            width < 2 * radius || height < 2 * radius) {
            throw std::invalid_argument(
                "YOUR BALLS ARE INVALID"
            );
        }

        static std::mt19937 rng(std::random_device{}());

        std::uniform_int_distribution<int> xDist(
            radius, width - radius
        );

        std::uniform_int_distribution<int> yDist(
            radius, height - radius
        );

        std::vector<Ball> retval;
        retval.reserve(count);

        const int maxAttempts = 1000;

        for (int i = 0; i < count; ++i) {
            bool placed = false;

            for (int attempt = 0; attempt < maxAttempts; ++attempt) {
                cv::Point2f position(
                    static_cast<float>(xDist(rng)),
                    static_cast<float>(yDist(rng))
                );

                bool overlaps = false;

                for (const Ball &other : retval) {
                    cv::Point2f delta = position - other.pos;

                    float minDistance =
                        static_cast<float>(radius + other.r);

                    if (delta.dot(delta) < minDistance * minDistance) {
                        overlaps = true;
                        break;
                    }
                }

                if (!overlaps) {
                    Ball newBall;

                    newBall.pos = position;
                    newBall.vel = randomVelocity(speed);
                    newBall.r = radius;
                    newBall.col = cv::Scalar(0, 0, 255);

                    retval.push_back(newBall);
                    placed = true;
                    break;
                }
            }

            if (!placed) {
                throw std::runtime_error(
                    "Could not place all balls without overlap"
                );
            }
        }

        return retval;
    }
    void ballCollisions() {
        for (std::size_t i = 0; i < balls.size(); ++i) {
            for (std::size_t j = i + 1; j < balls.size(); ++j) {
                Ball &a = balls[i];
                Ball &b = balls[j];

                cv::Point2f delta = b.pos - a.pos;
                float distanceSquared = delta.dot(delta);
                float minDistance =
                    static_cast<float>(a.r + b.r);
                if (distanceSquared >= minDistance * minDistance) {
                    continue;
                }
                if (distanceSquared < 1e-8f) {
                    delta = cv::Point2f(1.0f, 0.0f);
                    distanceSquared = 1.0f;
                }

                float distance = std::sqrt(distanceSquared);

                cv::Point2f normal = delta * (1.0f / distance);

                float overlap = minDistance - distance;

                a.pos -= normal * (overlap * 0.5f);
                b.pos += normal * (overlap * 0.5f);

                cv::Point2f relativeVelocity = b.vel - a.vel;

                float normalVelocity = relativeVelocity.dot(normal);
                if (normalVelocity >= 0.0f) {
                    continue;
                }
                a.vel += normal * normalVelocity;
                b.vel -= normal * normalVelocity;
            }
        }
    }
    void drawBalls(cv::Mat &frame){
        for (const Ball &b: balls){
            cv::Point center(
                static_cast<int>(b.pos.x),
                static_cast<int>(b.pos.y)
            );
            cv::circle(
                frame,
                center,
                b.r,
                b.col,
                2
            );
        }
    } 
    cv::Point2f randomVelocity(float speed) {
        static std::mt19937 rng(std::random_device{}());

        static std::uniform_real_distribution<float> angleDist(
            0.0f, 2.0f * CV_PI
        );

        float angle = angleDist(rng);

        return cv::Point2f(
            speed * std::cos(angle),
            speed * std::sin(angle)
        );
    }
};

int main(int argc, char *argv[]){
    rclcpp::init(argc, argv);
    rclcpp::spin(
        std::make_shared<GameNode>()
    );
    cv::destroyAllWindows();
    rclcpp::shutdown();
    return 0;
}