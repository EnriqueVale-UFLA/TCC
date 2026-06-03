/**
 * Biblioteca central de controle do drone.
 *
 * Responsável por:
 * - Armar e desarmar
 * - Decolar e pousar
 * - Movimentos relativos
 * - Controle Offboard
 * - Publicação de setpoints PX4
 */

#ifndef DRONE_CORE__DRONE_CONTROLLER_HPP_
#define DRONE_CORE__DRONE_CONTROLLER_HPP_

#include <px4_msgs/msg/offboard_control_mode.hpp>
#include <px4_msgs/msg/trajectory_setpoint.hpp>
#include <px4_msgs/msg/vehicle_command.hpp>

#include <rclcpp/rclcpp.hpp>

#include <string>

class DroneController : public rclcpp::Node
{
public:
    DroneController();

    void arm();
    void disarm();
    void engage_offboard_mode();

    void takeoff(float altitude);
    void land();
    void return_to_launch();

    void set_velocity(float vx, float vy, float vz, float yaw_rate = 0.0f);
    void stop_velocity();

    void move(float dx, float dy, float dz);

    void move_forward(float meters);
    void move_back(float meters);
    void move_right(float meters);
    void move_left(float meters);
    void move_up(float meters);
    void move_down(float meters);

    void set_altitude(float altitude);
    std::string get_status_text() const;

private:
    rclcpp::Publisher<px4_msgs::msg::OffboardControlMode>::SharedPtr offboard_control_mode_pub_;
    rclcpp::Publisher<px4_msgs::msg::TrajectorySetpoint>::SharedPtr trajectory_setpoint_pub_;
    rclcpp::Publisher<px4_msgs::msg::VehicleCommand>::SharedPtr vehicle_command_pub_;

    rclcpp::TimerBase::SharedPtr timer_;

    uint64_t offboard_setpoint_counter_;

    float x_;
    float y_;
    float z_;

    uint64_t timestamp_now();

    enum class ControlMode
    {
        POSITION,
        VELOCITY
    };

    ControlMode control_mode_;

    float vx_;
    float vy_;
    float vz_;
    float yaw_rate_;

    void timer_callback();

    void publish_offboard_control_mode();
    void publish_trajectory_setpoint();

    void publish_vehicle_command(
        uint16_t command,
        float param1 = 0.0f,
        float param2 = 0.0f,
        float param3 = 0.0f,
        float param4 = 0.0f,
        float param5 = 0.0f,
        float param6 = 0.0f,
        float param7 = 0.0f);
};

#endif