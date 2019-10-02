#ifndef NS3_TESTBED_SETTINGS_HPP
#define NS3_TESTBED_SETTINGS_HPP

#include <vector>
#include <string>
#include <sstream> // strngstream
#include <fstream>
#include <locale> // tolower

class mobility_record_t {
  public:
  const std::string robot_name;
  const float x;
  const float y;
  const bool walks;

  mobility_record_t(const std::vector<std::string>& row);
  mobility_record_t(const std::string& _robot_name,
                    float _x, float _y, bool walks);
};

class ns3_testbed_settings_t {
public:
  const int count;
  const float length;
  std::vector<mobility_record_t> mobilities;

  ns3_testbed_settings_t(int _count, float _length,
                         const std::string& filename);
};

#endif

