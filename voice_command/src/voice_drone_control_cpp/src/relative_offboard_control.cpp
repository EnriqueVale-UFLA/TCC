#include <rclcpp/rclcpp.hpp>
#include "std_msgs/msg/string.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <algorithm>

class RelativeOffboardControl : public rclcpp::Node
{
public:
    RelativeOffboardControl()
    : Node("relative_offboard_control")
    {
        voice_cmd_sub_ =
            this->create_subscription<std_msgs::msg::String>(
                "/voice_cmd",
                10,
                std::bind(
                    &RelativeOffboardControl::voice_cmd_callback,
                    this,
                    std::placeholders::_1));

        drone_command_pub_ =
            this->create_publisher<std_msgs::msg::String>(
                "/drone_command",
                10);

        input_thread_ = std::thread(
            std::bind(&RelativeOffboardControl::input_loop, this));

        input_thread_.detach();

        print_help();

        RCLCPP_INFO(this->get_logger(), "Interface de voz iniciada. Publicando em /drone_command.");
    }

private:
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr voice_cmd_sub_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr drone_command_pub_;
    std::thread input_thread_;

    float step_ = 0.5f;

    std::string normalize(std::string text)
    {
        std::transform(
            text.begin(),
            text.end(),
            text.begin(),
            [](unsigned char c) {
                return std::tolower(c);
            });

        return text;
    }

    float extract_value(const std::string &line)
    {
        std::stringstream ss(line);
        std::string word;
        float last_value = step_;

        while (ss >> word) {
            try {
                size_t idx;
                float value = std::stof(word, &idx);

                if (idx == word.size()) {
                    last_value = value;
                }
            } catch (...) {
            }

            if (word == "zero") last_value = 0.0f;
            else if (word == "meio" || word == "meia") last_value = 0.5f;
            else if (word == "um" || word == "uma") last_value = 1.0f;
            else if (word == "dois" || word == "duas") last_value = 2.0f;
            else if (word == "tres" || word == "três") last_value = 3.0f;
            else if (word == "quatro") last_value = 4.0f;
            else if (word == "cinco") last_value = 5.0f;
            else if (word == "seis") last_value = 6.0f;
            else if (word == "sete") last_value = 7.0f;
            else if (word == "oito") last_value = 8.0f;
            else if (word == "nove") last_value = 9.0f;
            else if (word == "dez") last_value = 10.0f;
        }

        return last_value;
    }

    void publish_drone_command(const std::string &command)
    {
        auto msg = std_msgs::msg::String();
        msg.data = command;

        drone_command_pub_->publish(msg);

        RCLCPP_INFO(
            this->get_logger(),
            "Publicado em /drone_command: %s",
            command.c_str());
    }

    void parse_command(const std::string &raw_line)
    {
        std::string line = normalize(raw_line);
        float value = extract_value(line);

        if (line.find("ajuda") != std::string::npos ||
            line.find("help") != std::string::npos)
        {
            print_help();
        }

        else if (line.find("pousar") != std::string::npos ||
                 line.find("usar") != std::string::npos ||
                 line.find("para usar") != std::string::npos)
        {
            publish_drone_command("land");
        }

        else if (line.find("retornar") != std::string::npos ||
                 line.find("voltar") != std::string::npos ||
                 line.find("rtl") != std::string::npos ||
                 line.find("return") != std::string::npos)
        {
            publish_drone_command("rtl");
        }

        else if (line.find("desarmar") != std::string::npos ||
                 line.find("desativar") != std::string::npos ||
                 line.find("desativa") != std::string::npos)
        {
            publish_drone_command("disarm");
        }

        else if (line.find("ativar") != std::string::npos ||
                 line.find("ativa") != std::string::npos ||
                 line.find("armar") != std::string::npos)
        {
            publish_drone_command("arm");
        }

        else if (line.find("offboard") != std::string::npos ||
                 line.find("modo voo") != std::string::npos ||
                 line.find("modo externo") != std::string::npos ||
                 line.find("controle externo") != std::string::npos ||
                 line.find("modo vou") != std::string::npos)
        {
            publish_drone_command("offboard");
        }

        else if (line.find("decolar") != std::string::npos ||
                 line.find("decola") != std::string::npos)
        {
            publish_drone_command("takeoff " + std::to_string(value));
        }

        else if (line.find("parar") != std::string::npos)
        {
            RCLCPP_INFO(
                this->get_logger(),
                "Parar recebido. Nenhum comando enviado.");
        }

        else if (line.find("altitude") != std::string::npos ||
                 line.find("altura") != std::string::npos)
        {
            publish_drone_command("altitude " + std::to_string(value));
        }

        else if (line.find("subir") != std::string::npos)
        {
            publish_drone_command("up " + std::to_string(value));
        }

        else if (line.find("descer") != std::string::npos)
        {
            publish_drone_command("down " + std::to_string(value));
        }

        else if (line.find("avancar") != std::string::npos ||
                 line.find("avançar") != std::string::npos ||
                 line.find("frente") != std::string::npos)
        {
            publish_drone_command("forward " + std::to_string(value));
        }

        else if (line.find("recuar") != std::string::npos ||
                 line.find("tras") != std::string::npos ||
                 line.find("trás") != std::string::npos)
        {
            publish_drone_command("back " + std::to_string(value));
        }

        else if (line.find("direita") != std::string::npos)
        {
            publish_drone_command("right " + std::to_string(value));
        }

        else if (line.find("esquerda") != std::string::npos)
        {
            publish_drone_command("left " + std::to_string(value));
        }

        else if (line.find("status") != std::string::npos)
        {
            publish_drone_command("status");
        }

        else {
            RCLCPP_WARN(
                this->get_logger(),
                "Comando não reconhecido: %s",
                line.c_str());
        }
    }

    void input_loop()
    {
        std::string line;

        while (rclcpp::ok()) {
            std::cout << "\nComando: ";
            std::getline(std::cin, line);

            if (!line.empty()) {
                parse_command(line);
            }
        }
    }

    void voice_cmd_callback(const std_msgs::msg::String::SharedPtr msg)
    {
        RCLCPP_INFO(
            this->get_logger(),
            "Comando recebido por voz: %s",
            msg->data.c_str());

        parse_command(msg->data);
    }

    void print_help()
    {
        std::cout << R"(
Comandos disponíveis:

  avancar 0.5
  frente 0.5
  recuar 0.5
  direita 0.5
  esquerda 0.5
  subir 0.5
  descer 0.5

  altitude 3
  status
  parar
  pousar
  ativar
  desarmar
  offboard
  rtl
  retornar

Observação:
  - Este nó NÃO controla mais o PX4 diretamente.
  - Ele publica comandos padronizados em /drone_command.
  - O drone_command_router é quem controla o drone.
)" << std::endl;
    }
};

int main(int argc, char *argv[])
{
    std::cout << "Starting voice command interface node..." << std::endl;

    rclcpp::init(argc, argv);

    auto node = std::make_shared<RelativeOffboardControl>();

    rclcpp::spin(node);

    rclcpp::shutdown();

    return 0;
}