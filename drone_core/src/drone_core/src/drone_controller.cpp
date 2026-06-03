#include "drone_core/drone_controller.hpp"

#include <cmath>
#include <chrono>
#include <sstream>
#include <limits>

using namespace std::chrono_literals;

DroneController::DroneController()
: Node("drone_controller"),
  offboard_setpoint_counter_(0),
  x_(0.0f),
  y_(0.0f),
  z_(0.0f),
  control_mode_(ControlMode::POSITION),
  vx_(0.0f),
  vy_(0.0f),
  vz_(0.0f),
  yaw_rate_(0.0f)
{
    offboard_control_mode_pub_ =
        this->create_publisher<px4_msgs::msg::OffboardControlMode>(
            "/fmu/in/offboard_control_mode", 10);

    trajectory_setpoint_pub_ =
        this->create_publisher<px4_msgs::msg::TrajectorySetpoint>(
            "/fmu/in/trajectory_setpoint", 10);

    vehicle_command_pub_ =
        this->create_publisher<px4_msgs::msg::VehicleCommand>(
            "/fmu/in/vehicle_command", 10);

    timer_ = this->create_wall_timer(
        100ms,
        std::bind(&DroneController::timer_callback, this));

    RCLCPP_INFO(this->get_logger(), "DroneController iniciado.");
}

uint64_t DroneController::timestamp_now()
{
    return this->get_clock()->now().nanoseconds() / 1000;
}

void DroneController::timer_callback()
{
    publish_offboard_control_mode();
    publish_trajectory_setpoint();

    if (offboard_setpoint_counter_ < 1000) {
        offboard_setpoint_counter_++;
    }
}

void DroneController::publish_offboard_control_mode()
{
    px4_msgs::msg::OffboardControlMode msg{};

    msg.timestamp = timestamp_now();

    msg.position = (control_mode_ == ControlMode::POSITION);
    msg.velocity = (control_mode_ == ControlMode::VELOCITY);
    msg.acceleration = false;
    msg.attitude = false;
    msg.body_rate = false;

    offboard_control_mode_pub_->publish(msg);
}

void DroneController::publish_trajectory_setpoint()
{
    px4_msgs::msg::TrajectorySetpoint msg{};

    msg.timestamp = timestamp_now();

    if (control_mode_ == ControlMode::POSITION) {
        msg.position = {x_, y_, z_};
        msg.velocity = {
            std::numeric_limits<float>::quiet_NaN(),
            std::numeric_limits<float>::quiet_NaN(),
            std::numeric_limits<float>::quiet_NaN()
        };
        msg.yaw = 0.0f;
        msg.yawspeed = 0.0f;
    } else {
        msg.position = {
            std::numeric_limits<float>::quiet_NaN(),
            std::numeric_limits<float>::quiet_NaN(),
            std::numeric_limits<float>::quiet_NaN()
        };
        msg.velocity = {vx_, vy_, vz_};
        msg.yaw = 0.0f;
        msg.yawspeed = yaw_rate_;
    }

    trajectory_setpoint_pub_->publish(msg);
}

void DroneController::publish_vehicle_command(
    uint16_t command,
    float param1,
    float param2,
    float param3,
    float param4,
    float param5,
    float param6,
    float param7)
{
    px4_msgs::msg::VehicleCommand msg{};

    msg.timestamp = timestamp_now();
    msg.command = command;

    msg.param1 = param1;
    msg.param2 = param2;
    msg.param3 = param3;
    msg.param4 = param4;
    msg.param5 = param5;
    msg.param6 = param6;
    msg.param7 = param7;

    msg.target_system = 1;
    msg.target_component = 1;
    msg.source_system = 1;
    msg.source_component = 1;
    msg.from_external = true;

    vehicle_command_pub_->publish(msg);
}

void DroneController::arm()
{
    publish_vehicle_command(
        px4_msgs::msg::VehicleCommand::VEHICLE_CMD_COMPONENT_ARM_DISARM,
        1.0f);

    RCLCPP_INFO(this->get_logger(), "ARM enviado");
}

void DroneController::disarm()
{
    publish_vehicle_command(
        px4_msgs::msg::VehicleCommand::VEHICLE_CMD_COMPONENT_ARM_DISARM,
        0.0f);

    RCLCPP_INFO(this->get_logger(), "DISARM enviado");
}

void DroneController::engage_offboard_mode()
{
    publish_vehicle_command(
        px4_msgs::msg::VehicleCommand::VEHICLE_CMD_DO_SET_MODE,
        1.0f,
        6.0f);

    RCLCPP_INFO(this->get_logger(), "OFFBOARD enviado");
}

void DroneController::takeoff(float altitude)
{
    control_mode_ = ControlMode::POSITION;

    if (altitude <= 0.0f) {
        altitude = 1.0f;
    }

    z_ = -std::abs(altitude);

    RCLCPP_INFO(
        this->get_logger(),
        "Decolagem/altitude alvo definida: %.2f m",
        std::abs(z_));
}

void DroneController::land()
{
    publish_vehicle_command(
        px4_msgs::msg::VehicleCommand::VEHICLE_CMD_NAV_LAND);

    RCLCPP_INFO(this->get_logger(), "LAND enviado");
}

void DroneController::return_to_launch()
{
    publish_vehicle_command(
        px4_msgs::msg::VehicleCommand::VEHICLE_CMD_NAV_RETURN_TO_LAUNCH);

    RCLCPP_INFO(this->get_logger(), "RETURN TO LAUNCH enviado");
}

void DroneController::move(float dx, float dy, float dz)
{
    control_mode_ = ControlMode::POSITION;

    x_ += dx;
    y_ += dy;
    z_ += dz;

    RCLCPP_INFO(
        this->get_logger(),
        "Novo alvo: x=%.2f, y=%.2f, z=%.2f",
        x_, y_, z_);
}

void DroneController::move_forward(float meters)
{
    move(std::abs(meters), 0.0f, 0.0f);
}

void DroneController::move_back(float meters)
{
    move(-std::abs(meters), 0.0f, 0.0f);
}

void DroneController::move_right(float meters)
{
    move(0.0f, std::abs(meters), 0.0f);
}

void DroneController::move_left(float meters)
{
    move(0.0f, -std::abs(meters), 0.0f);
}

void DroneController::move_up(float meters)
{
    move(0.0f, 0.0f, -std::abs(meters));
}

void DroneController::move_down(float meters)
{
    move(0.0f, 0.0f, std::abs(meters));
}

void DroneController::set_altitude(float altitude)
{
    control_mode_ = ControlMode::POSITION;
    
    z_ = -std::abs(altitude);

    RCLCPP_INFO(
        this->get_logger(),
        "Altitude alvo definida: %.2f m",
        std::abs(z_));
}

std::string DroneController::get_status_text() const
{
    std::ostringstream oss;

    oss << "Alvo atual: "
        << "x=" << x_
        << ", y=" << y_
        << ", z=" << z_
        << " | altitude=" << std::abs(z_) << " m";

    return oss.str();
}

void DroneController::set_velocity(float vx, float vy, float vz, float yaw_rate)
{
    control_mode_ = ControlMode::VELOCITY;

    vx_ = vx;
    vy_ = vy;
    vz_ = vz;
    yaw_rate_ = yaw_rate;

    RCLCPP_INFO(
        this->get_logger(),
        "Velocidade alvo: vx=%.2f, vy=%.2f, vz=%.2f, yaw_rate=%.2f",
        vx_, vy_, vz_, yaw_rate_);
}

void DroneController::stop_velocity()
{
    control_mode_ = ControlMode::VELOCITY;

    vx_ = 0.0f;
    vy_ = 0.0f;
    vz_ = 0.0f;
    yaw_rate_ = 0.0f;

    RCLCPP_INFO(this->get_logger(), "Velocidade zerada");
}