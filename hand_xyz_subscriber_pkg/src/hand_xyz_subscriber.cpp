#include <memory>
#include <sstream>
#include <string>

#include "geometry_msgs/msg/point.hpp"
#include "rclcpp/rclcpp.hpp"

class HandXyzSubscriber : public rclcpp::Node
{
public:
  HandXyzSubscriber()
  : Node("hand_xyz_subscriber")
  {
    const std::string topic_name =
      this->declare_parameter<std::string>("topic_name", "/hand_xyz");

    subscription_ = this->create_subscription<geometry_msgs::msg::Point>(
      topic_name,
      rclcpp::QoS(10),
      [this, topic_name](const geometry_msgs::msg::Point::SharedPtr msg) {
        std::ostringstream json;
        json << "{\"x\":" << msg->x
             << ",\"y\":" << msg->y
             << ",\"z\":" << msg->z << "}";

        RCLCPP_INFO(
          this->get_logger(),
          "Received hand XYZ on %s: %s",
          topic_name.c_str(),
          json.str().c_str());
      });

    RCLCPP_INFO(this->get_logger(), "Listening for hand XYZ points on %s", topic_name.c_str());
  }

private:
  rclcpp::Subscription<geometry_msgs::msg::Point>::SharedPtr subscription_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<HandXyzSubscriber>());
  rclcpp::shutdown();
  return 0;
}
