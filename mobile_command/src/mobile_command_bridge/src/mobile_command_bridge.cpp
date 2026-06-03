#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>

#include "drone_core/drone_controller.hpp"

#include <memory>
#include <string>
#include <sstream>
#include <algorithm>

class MobileCommandBridge : public rclcpp::Node
{
public:
    MobileCommandBridge()
    : Node("mobile_command_bridge")
    {
        controller_ = std::make_shared<DroneController>();

        controller_spin_thread_ = std::thread([this]() {
            rclcpp::spin(controller_);
        });

        controller_spin_thread_.detach();

        subscription_ =
            this->create_subscription<std_msgs::msg::String>(
                "/mobile_cmd",
                10,
                std::bind(
                    &MobileCommandBridge::command_callback,
                    this,
                    std::placeholders::_1));

        RCLCPP_INFO(
            this->get_logger(),
            "Mobile Command Bridge iniciado.");
    }

private:
    std::shared_ptr<DroneController> controller_;

    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;

    std::thread controller_spin_thread_;

    float extract_value(const std::string &line)
    {
        std::stringstream ss(line);
        std::string word;

        while (ss >> word) {
            try {
                return std::stof(word);
            } catch (...) {
            }
        }

        return 1.0f;
    }

    void command_callback(const std_msgs::msg::String::SharedPtr msg)
    {
        std::string line = msg->data;

        std::transform(
            line.begin(),
            line.end(),
            line.begin(),
            ::tolower);

        float value = extract_value(line);

        RCLCPP_INFO(
            this->get_logger(),
            "Comando mobile recebido: %s",
            line.c_str());

        if (line.find("arm") != std::string::npos)
        {
            controller_->arm();
        }

        else if (line.find("offboard") != std::string::npos)
        {
            controller_->engage_offboard_mode();
        }

        else if (line.find("takeoff") != std::string::npos)
        {
            controller_->takeoff(value);
        }

        else if (line.find("land") != std::string::npos)
        {
            controller_->land();
        }

        else if (line.find("rtl") != std::string::npos)
        {
            controller_->return_to_launch();
        }

        else if (line.find("right") != std::string::npos)
        {
            controller_->move_right(value);
        }

        else if (line.find("left") != std::string::npos)
        {
            controller_->move_left(value);
        }

        else if (line.find("forward") != std::string::npos)
        {
            controller_->move_forward(value);
        }

        else if (line.find("back") != std::string::npos)
        {
            controller_->move_back(value);
        }

        else if (line.find("up") != std::string::npos)
        {
            controller_->move_up(value);
        }

        else if (line.find("down") != std::string::npos)
        {
            controller_->move_down(value);
        }
    }
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);

    auto node = std::make_shared<MobileCommandBridge>();

    rclcpp::spin(node);

    rclcpp::shutdown();

    return 0;
}