#include <vector>
#include <memory> // for shared_ptr
#include <string>
#include <sstream>
#include <chrono> // for timer
#include <functional> // for bind
#include <cmath> // round
#include <iomanip> // std::setprecision
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
                       const std::chrono::microseconds _periodicity,
                       const rmw_qos_profile_t _qos_profile) :
           r_ptr(_r_ptr),
           subscription_name(_subscription_name),
           subscribers(),
           size(_size),
           periodicity(_periodicity),
           qos_profile(_qos_profile),

           count(0),
           publisher(_r_ptr->create_publisher<
                     cpp_testbed_runner::msg::TestbedMessage>(
                     _subscription_name, _qos_profile)),
           timer(_r_ptr->create_wall_timer(periodicity,
                     std::bind(&publisher_callback_t::publish_message, this))),
           node_logger(r_ptr->get_logger()) 
{
  // make the subscriber list for network metadata
  for (std::vector<subscribe_record_t>::const_iterator it =
                    r_ptr->ps_ptr->subscribers.begin();
                    it != r_ptr->ps_ptr->subscribers.end(); ++it) {
    if (it->subscription == subscription_name) {
      subscribers.push_back(it->robot_name);
    }
  }
}

// http://www.theconstructsim.com/wp-content/uploads/2019/03/ROS2-IN-5-DAYS-e-book.pdf
void publisher_callback_t::publish_message() {

  // compose msg
  std::shared_ptr<cpp_testbed_runner::msg::TestbedMessage> msg(
                std::make_shared<cpp_testbed_runner::msg::TestbedMessage>());
  msg->publisher_name = r_ptr->r;
  msg->tx_count = ++count;
  msg->message = std::string(size, subscription_name[0]);

  // compose network metadata log
  std::stringstream ss;
  long timestamp = _now_nanoseconds();
  for (std::vector<std::string>::const_iterator it =
                       subscribers.begin(); it != subscribers.end(); ++it) {
    // tx log for each subscriber: from, to, subscription, tx count, timestamp
    ss << r_ptr->r << ","             // from this robot name
       << *it << ","                  // to other robot name
       << subscription_name << ","    // subscription name
       << count << ","                // transmit count
       << timestamp << "\n";          // timestamp in nanoseconds
  }

  // publish the message
  publisher->publish(msg);

  // log the network metadata
  r_ptr->pipe_writer_ptr->log(ss.str());

  if (r_ptr->verbose) {
    RCLCPP_INFO(node_logger, "Publishing: %s transmit count %d size %d",
                             subscription_name.c_str(), count, size);
  }
}

// subscriber_callback
subscriber_callback_t::subscriber_callback_t(testbed_robot_t* _r_ptr,
                      const std::string _subscription_name,
                      const rmw_qos_profile_t _qos_profile) :
         r_ptr(_r_ptr),
         subscription_name(_subscription_name),

         subscription(0),

         qos_profile(_qos_profile),
         node_logger(r_ptr->get_logger()),
         count(0) {


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

  // rx log: from, to, subscription, tx count, rx count, msg size, timestamp
  std::stringstream ss;
  ss << msg->publisher_name << ","         // from other robot name
     << r_ptr->r << ","                    // to this robot name
     << subscription_name << ","           // subscription name 
     << msg->tx_count << ","               // transmit count
     << ++count << ","                     // received count
     << msg->message.size() << ","         // size of message field only
     << _now_nanoseconds() << "\n";        // timestamp in nanoseconds

  // network metadata log
  r_ptr->pipe_writer_ptr->log(ss.str());
  if(r_ptr->verbose) {
    RCLCPP_INFO(node_logger, "Receiving : %s received count %d size %d",
                   subscription_name.c_str(), ++count, msg->message.size());
  }
}

