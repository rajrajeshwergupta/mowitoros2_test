#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "std_srvs/srv/set_bool.hpp"

#include "cv_bridge/cv_bridge.h"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

class ImageConversionNode : public rclcpp::Node
{
public:
  ImageConversionNode() : Node("image_conversion_node"), grayscale_mode_(false)
  {
    // Declare parameters for topic names
    input_topic_ = this->declare_parameter<std::string>("input_topic", "/image");
    output_topic_ = this->declare_parameter<std::string>("output_topic", "/converted_image");

    // Create subscriber and publisher
    image_sub_ = this->create_subscription<sensor_msgs::msg::Image>(
      input_topic_, 10,
      std::bind(&ImageConversionNode::imageCallback, this, std::placeholders::_1));

    image_pub_ = this->create_publisher<sensor_msgs::msg::Image>(output_topic_, 10);

    // Create service to toggle mode
    service_ = this->create_service<std_srvs::srv::SetBool>(
      "set_conversion_mode",
      std::bind(&ImageConversionNode::setModeCallback, this, std::placeholders::_1, std::placeholders::_2));

    RCLCPP_INFO(this->get_logger(), "Image conversion node started. Subscribing to [%s], publishing to [%s].",
                input_topic_.c_str(), output_topic_.c_str());
  }

private:
  void imageCallback(const sensor_msgs::msg::Image::SharedPtr msg)
  {
    try {
      // Convert ROS image to OpenCV image
      cv_bridge::CvImagePtr cv_ptr = cv_bridge::toCvCopy(msg, "bgr8");
      cv::Mat processed_image;

      if (grayscale_mode_) {
        cv::cvtColor(cv_ptr->image, processed_image, cv::COLOR_BGR2GRAY);
        cv_ptr->image = processed_image;
        cv_ptr->encoding = "mono8";
      } else {
        processed_image = cv_ptr->image;  // Leave color image as-is
      }

      image_pub_->publish(*cv_ptr->toImageMsg());

    } catch (cv_bridge::Exception & e) {
      RCLCPP_ERROR(this->get_logger(), "cv_bridge exception: %s", e.what());
    }
  }

  void setModeCallback(const std::shared_ptr<std_srvs::srv::SetBool::Request> request,
                       std::shared_ptr<std_srvs::srv::SetBool::Response> response)
  {
    grayscale_mode_ = request->data;
    response->success = true;
    response->message = grayscale_mode_ ? "Switched to Grayscale mode." : "Switched to Color mode.";

    RCLCPP_INFO(this->get_logger(), "%s", response->message.c_str());
  }

  // Internal state and members
  rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr image_sub_;
  rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr image_pub_;
  rclcpp::Service<std_srvs::srv::SetBool>::SharedPtr service_;

  std::string input_topic_;
  std::string output_topic_;
  bool grayscale_mode_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<ImageConversionNode>());
  rclcpp::shutdown();
  return 0;
}