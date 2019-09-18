
#include <vector>
#include <memory> // for shared_ptr
#include <string>
#include <sstream>
#include <chrono> // for timer
#include <functional> // for bind
#include "rclcpp/rclcpp.hpp"
//#include "rclcpp/time.hpp"
// https://discourse.ros.org/t/ros2-how-to-use-custom-message-in-project-where-its-declared/2071
#include "cpp_testbed_runner/msg/testbed_message.hpp"

#include "testbed_robot.hpp"

// epoc time in nanoseconds using high_resolution_clock
long _now_nanoseconds() {
// https://stackoverflow.com/questions/31255486/c-how-do-i-convert-a-stdchronotime-point-to-long-and-back
  std::chrono::time_point<std::chrono::high_resolution_clock> now =
                                std::chrono::high_resolution_clock::now();
  auto now_ns = std::chrono::time_point_cast<std::chrono::nanoseconds>(now);
  auto epoch = now_ns.time_since_epoch();
  long t = std::chrono::duration_cast<std::chrono::nanoseconds>(epoch).count();
  return t;
}

// publisher_callback
publisher_callback_t::publisher_callback_t(testbed_robot_t* _r_ptr,
                       const std::string _subscription_name,
                       const unsigned int _size,
                       const std::chrono::microseconds _microseconds,
                       const rmw_qos_profile_t _qos_profile,
                       const bool _verbose) :
           r_ptr(_r_ptr),
           subscription_name(_subscription_name),
           size(_size),
           microseconds(_microseconds),
           qos_profile(_qos_profile),
           verbose(_verbose),

           count(0),
           publisher(_r_ptr->create_publisher<
                     cpp_testbed_runner::msg::TestbedMessage>(
                     _subscription_name, _qos_profile)),
           timer(_r_ptr->create_wall_timer(microseconds,
                     std::bind(&publisher_callback_t::publish_message, this))),
           node_logger(r_ptr->get_logger()) 
{
}

// http://www.theconstructsim.com/wp-content/uploads/2019/03/ROS2-IN-5-DAYS-e-book.pdf
void publisher_callback_t::publish_message() {

  std::shared_ptr<cpp_testbed_runner::msg::TestbedMessage> msg(
                std::make_shared<cpp_testbed_runner::msg::TestbedMessage>());

  msg->nanoseconds = _now_nanoseconds();
  msg->source = r_ptr->r;
  msg->message_name = subscription_name;
  msg->message_number = ++count;
  msg->message = std::string(size, subscription_name[0]);

  if (verbose) {
    RCLCPP_INFO(node_logger, "Publishing: %s count %d size %d",
                             subscription_name.c_str(), count, size);
  }

  publisher->publish(msg);
}

// subscriber_callback
subscriber_callback_t::subscriber_callback_t(testbed_robot_t* _r_ptr,
                      const std::string _subscription_name,
                      const rmw_qos_profile_t _qos_profile,
                      const bool _use_pipe,
                      const bool _verbose) :
         r_ptr(_r_ptr),
         subscription_name(_subscription_name),

         subscription(0),

         qos_profile(_qos_profile),
         use_pipe(_use_pipe),
         verbose(_verbose),
         node_logger(r_ptr->get_logger()) {


  auto callback = [this](
          const cpp_testbed_runner::msg::TestbedMessage::SharedPtr msg) -> void
          {
            this->subscriber_callback(msg);
          };
  subscription = _r_ptr->create_subscription<
                      cpp_testbed_runner::msg::TestbedMessage>(
                      subscription_name, callback);
}

void subscriber_callback_t::subscriber_callback(
              const cpp_testbed_runner::msg::TestbedMessage::SharedPtr msg) {

  // message summary
  std::stringstream ss;
  ss << msg->source << "-" << r_ptr->r << "," // src-dest
     << msg->message_name << ","              // subscription name
     << msg->message.size() << ","            // size of these records
     << msg->message_number << ","            // message number
     << msg->nanoseconds << ","               // time sent in nanoseconds
     << (_now_nanoseconds() - msg->nanoseconds) / 1000000.0; // delta ms
  if(verbose || !use_pipe) {
    RCLCPP_INFO(node_logger, ss.str().c_str());
  }

  if(use_pipe) {
    r_ptr->pipe_writer.log(ss.str());
  }
}

