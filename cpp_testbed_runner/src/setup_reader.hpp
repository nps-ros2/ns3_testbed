#ifndef SETUP_READER_HPP
#define SETUP_READER_HPP

#include <vector>
#include <string>
#include <sstream> // strngstream
#include <fstream>
#include <locale> // tolower
#include <chrono>

#include "rclcpp/rclcpp.hpp"

class publish_record_t {
  public:
  const std::string robot_name;
  const std::string subscription;
  const std::chrono::microseconds microseconds; // period
  const unsigned int size;
  const rmw_qos_profile_t qos_profile;

  publish_record_t(const std::vector<std::string>& row);
};

class subscribe_record_t {
public:

  const std::string robot_name;
  const std::string subscription;
  const rmw_qos_profile_t qos_profile;

  subscribe_record_t(const std::vector<std::string>& row);
};

class publishers_subscribers_t {
public:
  std::vector<publish_record_t> publishers;
  std::vector<subscribe_record_t> subscribers;

  publishers_subscribers_t(std::string filename, bool verbose);
};

#endif
