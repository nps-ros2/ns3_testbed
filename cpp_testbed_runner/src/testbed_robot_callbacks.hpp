#ifndef TESTBED_ROBOT_CALLBACKS_HPP
#define TESTBED_ROBOT_CALLBACKS_HPP

#include <vector>
#include <memory> // for shared_ptr
#include <string>
#include <chrono>
#include "rclcpp/rclcpp.hpp"
#include "cpp_testbed_runner/msg/testbed_message.hpp"

class testbed_robot_t;

// track Rx counts by Tx robot name
class rx_counts_t {
  private:
  std::map<std::string, int> counts;

  public:
  int inc(std::string tx_name); // increment and return value
  int val(std::string tx_name); // return value
};

class publisher_callback_t {
  private:
  testbed_robot_t* r_ptr;
  const std::string subscription_name;
  std::vector<std::string> subscribers;
  const unsigned int size;
  const std::chrono::microseconds periodicity;
  const rmw_qos_profile_t qos_profile;

  int count;
  rclcpp::Publisher<cpp_testbed_runner::msg::TestbedMessage>::SharedPtr
                                                               publisher;
  rclcpp::TimerBase::SharedPtr timer;
  rclcpp::Logger node_logger;

  public:
  publisher_callback_t(testbed_robot_t* _r_ptr,
                       const std::string& _subscription_name,
                       const unsigned int _size,
                       const std::chrono::microseconds _periodicity,
                       const rmw_qos_profile_t& _qos_profile);
  void publish_message();
};

class subscriber_callback_t {
  private:
  testbed_robot_t* r_ptr;
  const std::string subscription_name;
  rclcpp::Subscription<cpp_testbed_runner::msg::TestbedMessage>::SharedPtr
                                                               subscription;
  const rmw_qos_profile_t qos_profile;
  rclcpp::Logger node_logger;
  rx_counts_t rx_counts;

  public:
  subscriber_callback_t(testbed_robot_t* _r_ptr,
                        const std::string& _subscription_name,
                        const rmw_qos_profile_t& _qos_profile);

  void subscriber_callback(cpp_testbed_runner::msg::TestbedMessage::SharedPtr
                                                               msg);
};

#endif
