#include <chrono>
#include <memory>
#include <cstdlib>
#include <string>
#include <functional>  
#include <stdexcept>   

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <cv_bridge/cv_bridge.h>

#include <opencv2/opencv.hpp>


using namespace std::chrono_literals;

// Creating Camera Node
// This should input from the webcam
class CameraNode : public rclcpp::Node {
public:
    CameraNode() : Node("camera_node") {
        // source 0 -> as far as I know it's a webcam
        this -> declare_parameter<std::string>  ("source", "0");
        // fps is a double cuz it could be a double <insert acikiwir emote here>
        this -> declare_parameter<double>       ("fps", 30.0);

        // constant width and height
        // 480p
        this -> declare_parameter<int>          ("width", 640);
        this -> declare_parameter<int>          ("height", 480);

        std::string sourceStr   = this->get_parameter("source").as_string();
        double fps              = this->get_parameter("fps").as_double();
        int width               = this->get_parameter("width").as_int();
        int height              = this->get_parameter("height").as_int();

        if (!valid_basic_parameter(fps, width, height))
            throw std::runtime_error("Negative basic parameters");
        
        try {
            // validating the sourceStr string
            // making sure it can be parsed
            size_t args_len = 0;
            int camera_idx= std::stoi(sourceStr, &args_len);
            if (args_len == sourceStr.size())
                cap.open(camera_idx);
            else cap.open(sourceStr);
        }
        catch (const std::invalid_argument&){
            cap.open(sourceStr);
        }
        catch (const std::out_of_range&){
            cap.open(sourceStr);
        }
        
        if (!cap.isOpened())
            throw std::runtime_error("Couldn't open the video source of: " + sourceStr);
        
        cap.set(cv::CAP_PROP_FRAME_WIDTH, width);
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, height);

        publisher = this->
                    create_publisher <sensor_msgs::msg::Image>(
                        "/camera/image_raw",
                        10
                    );
        auto frametime = std::chrono::duration<double>(1.0/fps);
        timer = this->create_wall_timer(
            std::chrono::duration_cast<std::chrono::nanoseconds>(frametime),
            std::bind(&CameraNode::publish_frame, this)
        );
        RCLCPP_INFO(
            this->get_logger(),
            "Camera node started\nSOURCE: %s\n%.1fFPS", sourceStr.c_str(), fps
        );
    }
private:
    cv::VideoCapture cap;
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr publisher;
    rclcpp::TimerBase::SharedPtr timer;

    void publish_frame(){
        cv::Mat frame;
        // we loop da video files
        if (!cap.read(frame) || frame.empty()){
            if (cap.get(cv::CAP_PROP_FRAME_COUNT) > 0) {
                cap.set(cv::CAP_PROP_POS_FRAMES, 0);
                return;
            }
            RCLCPP_WARN(
                this->get_logger(),
                "Failed to capture frame."
            );
            return;
        }

        auto msg = cv_bridge::CvImage(
            std_msgs::msg::Header(),
            "bgr8",
            frame
        ).toImageMsg();
        msg->header.stamp = this->get_clock()->now();
        msg->header.frame_id = "camera";
        publisher->publish(*msg);
    }



    bool valid_basic_parameter(double __fps, int __width, int __height){
        return __fps > 0.0 && __width > 0 && __height > 0;
    }
};

int main(int argc, char* argv[]){
    rclcpp::init(argc, argv);
    try{
        rclcpp::spin(std::make_shared<CameraNode>());
    }
    catch (const std::exception &e){
        RCLCPP_ERROR(rclcpp::get_logger("camera_node"), "%s", e.what());
    }
    rclcpp::shutdown();
    return 0;
}
