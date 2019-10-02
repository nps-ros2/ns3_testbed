#include <vector>
#include <string>
#include <iostream> // std::cout
#include <sstream> // strngstream
#include <fstream>
#include <locale> // tolower
#include <algorithm> // transform
#include <cassert>
#include <stdexcept>

#include "ns3_testbed_settings.hpp"

// diagnostic
static bool VERBOSE = false;

//https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
std::vector<std::string> _split(const std::string& str,
                                       const std::string& delim) {
  std::vector<std::string> tokens;
  size_t prev=0, pos=0;
  do {
    pos = str.find(delim, prev);
    if (pos == std::string::npos) pos = str.length();
    std::string token = str.substr(prev, pos-prev);
    if (!token.empty()) tokens.push_back(token);
    prev = pos + 1;
  }
  while (pos < str.length() && prev < str.length());
  return tokens;
}

std::vector<std::string> _range(std::string range) {

  range.erase(0,1); // remove "^r"

  std::vector<std::string> endpoints = _split(range, "-");

  unsigned int start = 0;
  unsigned int stop = 0;
  try {
    if(endpoints.size() == 1) {
      start = stoi(endpoints[0]);
      stop = start;
    } else if (endpoints.size() == 2) {
      start = stoi(endpoints[0]);
      stop = stoi(endpoints[1]);
    } else {
      std::cerr << "range error in '" << range << "'\n";
      assert(0);
    }
  } catch (const std::invalid_argument& ia) {
    std::cerr << "invalid range '" << range << "'\n";
    assert(0);
  }

  std::vector<std::string> nodes;
  for(unsigned int i=start; i<=stop; i++) {
    std::stringstream ss;
    ss << "R" << i;
    nodes.push_back(ss.str());
  }
  return nodes;
}

void _validate_header(std::string line, std::string mode) {
  if(mode == "mobility") {
    if(line != "node,x,y,walks") {
      std::cout << "invalid mobility line: '" << line << "'" << std::endl;
      assert(0);
    }
  }
}

mobility_record_t::mobility_record_t(const std::vector<std::string>& row) :
           robot_name(row[0]),
           x(stof(row[1])),
           y(stof(row[2])),
           walks(row[3]=="true"? true : false) {
}

mobility_record_t::mobility_record_t(const std::string& _robot_name,
                    float _x, float _y, bool _walks) :
           robot_name(_robot_name),
           x(_x), y(_y), walks(_walks) {
}


ns3_testbed_settings_t::ns3_testbed_settings_t(int _count, float _length,
                                               const std::string& filename) :
                   count(_count), length(_length), mobilities() {

  // open CSV file
  std::string line;
  std::vector<std::string> row;
  std::string mode = "start";
  std::ifstream f(filename);
  if(f.fail()) {
    std::cout << "CSV ns3_setup file open error on file '" << filename << "'\n";
    assert(0);
  }
  std::vector<mobility_record_t> labeled_mobilities;
  while (std::getline(f, line)) {

    // line to lower case
    std::transform(line.begin(), line.end(), line.begin(), 
                   [](unsigned char c){ return std::tolower(c); });

    std::vector<std::string> row = _split(line, ",");

    // verbose
    if(VERBOSE) {
      std::cout << "Row: '" << line << "'\n";
    }

    // no first column
    if(row.size() == 0) {
      continue;
    }

    // blank first column
    if(row[0] == "") {
      continue;
    }

    // mode mobility
    if (row[0] == "mobility") {
      mode = "mobility";
      continue;
    }

    // row [0][0] is not R so it must be a header
    std::string a=row[0];

    if(row[0][0]!='r') {
      // validate header
      _validate_header(line, mode);
      continue;
    }

    // consume range, either r<n> or r<n1-n2>
    std::vector<std::string> nodes = _range(row[0]);
    for (std::vector<std::string>::const_iterator it = nodes.begin();
            it != nodes.end(); ++it) {
      row[0] = *it;
      try {
        if (mode == "mobility") {
          labeled_mobilities.push_back(mobility_record_t(row));
        }
      } catch (const std::invalid_argument& ia) {
        std::cerr << "invalid argument in mode " << mode
                  << " on line '" << line << "'\n";
        assert(0);
      }
    }
  }

  // set mobilites for all robots, R1 maps to [0], etc.
  for (int i=0; i<count; i++) {
    std::stringstream ss;
    ss << "R" << i+1;
    const std::string name = ss.str();
    bool found = false;
    for (std::vector<mobility_record_t>::const_iterator it2 =
                     labeled_mobilities.begin();
                     it2 != labeled_mobilities.end(); ++it2) {
      if(it2->robot_name == name) {
        found = true;
        mobilities.push_back(*it2);
        break;
      }
    }
    if(!found) {
      // not in table so use random walk starting at 0,0,0
      mobilities.push_back(mobility_record_t(name, 0.0, 0.0, true));
    }
  }
}

