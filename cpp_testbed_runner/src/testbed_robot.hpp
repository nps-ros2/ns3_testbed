#ifndef TESTBED_ROBOT_HPP
#define TESTBED_ROBOT_HPP

#include <vector>
#include <string>
#include "rclcpp/rclcpp.hpp"

#include "testbed_robot_callbacks.hpp"
#include "setup_reader.hpp"
#include "pipe_writer.hpp"

class testbed_robot_t : public rclcpp::Node {

  public:
  const std::string nns;
  const std::string r;
  const pipe_writer_t* pipe_writer_ptr;
  const bool verbose;
  const publishers_subscribers_t* ps_ptr;

  private:
  std::vector<publisher_callback_t*> publisher_callbacks;
  std::vector<subscriber_callback_t*> subscriber_callbacks;

  public:
  explicit testbed_robot_t(const std::string& _nns,
                           const std::string& _r,
                           const pipe_writer_t* const _pipe_writer_ptr,
                           const bool _verbose,
                           const publishers_subscribers_t* const _ps_ptr);
};

// entry function to start a testbed robot
void testbed_robot_run(std::string nns, std::string r,
                       const bool use_nns,
                       const pipe_writer_t* const pipe_writer_ptr,
                       const bool verbose,
                       publishers_subscribers_t* ps_ptr);

#endif

