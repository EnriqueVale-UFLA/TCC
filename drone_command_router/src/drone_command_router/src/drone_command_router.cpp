/**
 * Roteador central de comandos.
 *
 * Recebe comandos provenientes de:
 * - Voz
 * - Aplicativo/Web
 * - Telegram
 * - Visão Computacional
 *
 * Decide qual fonte possui controle do drone
 * em cada instante.
 */

#include <vector>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>

#include "drone_core/drone_controller.hpp"
#include <geometry_msgs/msg/twist.hpp>

#include <algorithm>
#include <memory>
#include <sstream>
#include <string>
#include <thread>

class DroneCommandRouter : public rclcpp::Node
{
public:
    DroneCommandRouter()
    : Node("drone_command_router")
    {
        controller_ = std::make_shared<DroneController>();

        controller_spin_thread_ = std::thread([this]() {
            rclcpp::spin(controller_);
        });

        controller_spin_thread_.detach();

        subscription_ =
            this->create_subscription<std_msgs::msg::String>(
                "/drone_command",
                10,
                std::bind(
                    &DroneCommandRouter::command_callback,
                    this,
                    std::placeholders::_1));
        
        aruco_subscription_ =
            this->create_subscription<geometry_msgs::msg::Twist>(
                "/drone/vision/cmd_vel",
                10,
                std::bind(
                    &DroneCommandRouter::aruco_callback,
                    this,
                    std::placeholders::_1));
        
        person_subscription_ =
            this->create_subscription<geometry_msgs::msg::Twist>(
                "/drone/vision/person_cmd_vel",
                10,
                std::bind(
                    &DroneCommandRouter::person_callback,
                    this,
                    std::placeholders::_1));

        
        RCLCPP_INFO(this->get_logger(), "Drone Command Router iniciado.");
        
    }

private:
    std::shared_ptr<DroneController> controller_;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
    std::thread controller_spin_thread_;
    bool aruco_auto_enabled_ = false;
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr aruco_subscription_;
    bool person_auto_enabled_ = false;
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr person_subscription_;

    std::vector<float> extract_values(const std::string &line)
    {
        std::stringstream ss(line);
        std::string word;
        std::vector<float> values;

        while (ss >> word) {
            try {
                values.push_back(std::stof(word));
            } catch (...) {
            }
        }

        return values;
    }

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

    void aruco_callback(const geometry_msgs::msg::Twist::SharedPtr msg)
    {
        if (!aruco_auto_enabled_) {
            return;
        }

        controller_->set_velocity(
            msg->linear.x,
            msg->linear.y,
            msg->linear.z,
            msg->angular.z);

        RCLCPP_INFO(
            this->get_logger(),
            "ArUco cmd -> vx: %.2f vy: %.2f",
            msg->linear.x,
            msg->linear.y);
    }

    void person_callback(const geometry_msgs::msg::Twist::SharedPtr msg)
    {
        if (!person_auto_enabled_) {
            return;
        }

        controller_->set_velocity(
            msg->linear.x,
            msg->linear.y,
            msg->linear.z,
            msg->angular.z);

        RCLCPP_INFO(
            this->get_logger(),
            "Person cmd -> vx: %.2f vy: %.2f yaw: %.2f",
            msg->linear.x,
            msg->linear.y,
            msg->angular.z);
    }

    void command_callback(const std_msgs::msg::String::SharedPtr msg)
    {
        std::string line = msg->data;

        std::transform(
            line.begin(),
            line.end(),
            line.begin(),
            [](unsigned char c) {
                return std::tolower(c);
            });

        float value = extract_value(line);

        RCLCPP_INFO(
            this->get_logger(),
            "Comando recebido em /drone_command: %s",
            line.c_str());

        if (line.find("arm") != std::string::npos ||
            line.find("ativar") != std::string::npos ||
            line.find("armar") != std::string::npos)
        {
            controller_->arm();
        }

        else if (line.find("disarm") != std::string::npos ||
                 line.find("desarmar") != std::string::npos)
        {
            controller_->disarm();
        }

        else if (line.find("offboard") != std::string::npos)
        {
            controller_->engage_offboard_mode();
        }

        else if (line.find("aruco on") != std::string::npos)
        {
            aruco_auto_enabled_ = true;

            RCLCPP_INFO(
                this->get_logger(),
                "Modo ArUco AUTOMATICO ativado.");
        }

        else if (line.find("aruco off") != std::string::npos)
        {
            aruco_auto_enabled_ = false;

            controller_->stop_velocity();

            RCLCPP_INFO(
                this->get_logger(),
                "Modo ArUco AUTOMATICO desativado.");
        }

        else if (line.find("person on") != std::string::npos)
        {
            person_auto_enabled_ = true;
            aruco_auto_enabled_ = false;

            RCLCPP_INFO(
                this->get_logger(),
                "Modo PERSON AUTOMATICO ativado.");
        }

        else if (line.find("person off") != std::string::npos)
        {
            person_auto_enabled_ = false;

            controller_->stop_velocity();

            RCLCPP_INFO(
                this->get_logger(),
                "Modo PERSON AUTOMATICO desativado.");
        }

        else if (line.find("velocity") != std::string::npos)
        {
            auto values = extract_values(line);

            float vx = values.size() > 0 ? values[0] : 0.0f;
            float vy = values.size() > 1 ? values[1] : 0.0f;
            float vz = values.size() > 2 ? values[2] : 0.0f;
            float yaw = values.size() > 3 ? values[3] : 0.0f;

            controller_->set_velocity(vx, vy, vz, yaw);
        }

        else if (line.find("stop") != std::string::npos)
        {
            controller_->stop_velocity();
        }

        else if (line.find("takeoff") != std::string::npos ||
                 line.find("decolar") != std::string::npos)
        {
            controller_->takeoff(value);
        }

        else if (line.find("land") != std::string::npos ||
                 line.find("pousar") != std::string::npos)
        {
            controller_->land();
        }

        else if (line.find("rtl") != std::string::npos ||
                 line.find("retornar") != std::string::npos ||
                 line.find("voltar") != std::string::npos)
        {
            controller_->return_to_launch();
        }

        else if (line.find("right") != std::string::npos ||
                 line.find("direita") != std::string::npos)
        {
            controller_->move_right(value);
        }

        else if (line.find("left") != std::string::npos ||
                 line.find("esquerda") != std::string::npos)
        {
            controller_->move_left(value);
        }

        else if (line.find("forward") != std::string::npos ||
                 line.find("frente") != std::string::npos ||
                 line.find("avancar") != std::string::npos ||
                 line.find("avançar") != std::string::npos)
        {
            controller_->move_forward(value);
        }

        else if (line.find("back") != std::string::npos ||
                 line.find("tras") != std::string::npos ||
                 line.find("trás") != std::string::npos ||
                 line.find("recuar") != std::string::npos)
        {
            controller_->move_back(value);
        }

        else if (line.find("up") != std::string::npos ||
                 line.find("subir") != std::string::npos)
        {
            controller_->move_up(value);
        }

        else if (line.find("down") != std::string::npos ||
                 line.find("descer") != std::string::npos)
        {
            controller_->move_down(value);
        }

        else if (line.find("altitude") != std::string::npos ||
                 line.find("altura") != std::string::npos)
        {
            controller_->set_altitude(value);
        }

        else if (line.find("status") != std::string::npos)
        {
            RCLCPP_INFO(
                this->get_logger(),
                "%s",
                controller_->get_status_text().c_str());
        }

        else
        {
            RCLCPP_WARN(
                this->get_logger(),
                "Comando desconhecido: %s",
                line.c_str());
        }
    }
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);

    auto node = std::make_shared<DroneCommandRouter>();

    rclcpp::spin(node);

    rclcpp::shutdown();

    return 0;
}