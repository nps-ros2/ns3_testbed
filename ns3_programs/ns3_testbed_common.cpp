#include <getopt.h>
#include <cstdio>
#include <iostream> // std::cout
#include <iomanip> // std::setprecision

#include "ns3/core-module.h"
#include "ns3/node-container.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
//#include "ns3/wifi-module.h"
#include "ns3/tap-bridge-module.h"
//#include "ns3/random-variable-stream.h"

#include "ns3_testbed_common.hpp"

// realtime mode
void testbed_setup() {
  // Force flush of the stdout buffer.
  setvbuf(stdout, NULL, _IONBF, BUFSIZ);

  // run ns3 real-time with checksums
  ns3::GlobalValue::Bind("SimulatorImplementationType",
                          ns3::StringValue("ns3::RealtimeSimulatorImpl"));
  ns3::GlobalValue::Bind("ChecksumEnabled", ns3::BooleanValue(true));

  // Use HardLimit mode, being sure ns-3 is modified to warn instead of halt.
  ns3::Config::SetDefault("ns3::RealtimeSimulatorImpl::SynchronizationMode",
                ns3::EnumValue(ns3::RealtimeSimulatorImpl::SYNC_HARD_LIMIT));
}

void usage() {
  std::cout << "Usage: -h|-H|-s <setup file> -c <count>|-l <length>\n"
            << "-h : Print this help function.\n"
            << "-c <count>: Number of robots, starting at 1.\n"
            << "-l <length>: Mobility box edge length, in meters.\n"
            << "-s <setup file>: The CSV setup file.\n"
            ;
}

// parse user input
int get_testbed_options(int argc, char *argv[], int *count, int *length,
                        std::string *setup_file) {

  std::string _home(getenv("HOME"));
  const std::string default_setup_file(_home.append(
                               "/gits/ns3_testbed/csv_setup/ns3_defaults.csv"));

  // defaults
  *count = 5;
  *length = 30;
  *setup_file = default_setup_file;

  // parse options
  int option_index; // not used
  while (1) {

    const struct option long_options[] = {
      // options
      {"help",                          no_argument, 0, 'h'},
      {"Help",                          no_argument, 0, 'H'},
      {"count",                   required_argument, 0, 'c'},
      {"length",                  required_argument, 0, 'l'},
      {"setup_file",              required_argument, 0, 's'},

      // end
      {0,0,0,0}
    };

    int ch = getopt_long(argc, argv, "hHc:l:s:", long_options, &option_index);

    if (ch == -1) {
      // no more arguments
      break;
    }
    if (ch == 0) {
      // command options set flags and use ch==0
      continue;
    }
    switch (ch) {
      case 'h': {	// help
        usage();
        exit(0);
      }
      case 'H': {	// Help
        usage();
        exit(0);
      }
      case 's': {	// setup file
        *setup_file = optarg;
        break;
      }
      case 'c': {	// count
        *count = std::atoi(optarg);
        break;
      }
      case 'l': {	// length
        *length = std::atoi(optarg);
        break;
      }
      default:
//        std::cerr << "unexpected command character " << ch << "\n";
        exit(1);
    }
  }

//  // parse the remaining tokens that were not consumed by options
//  argc -= optind;
//  argv += optind;

}

// set mobility for ground station and robot nodes
//https://www.nsnam.org/doxygen/mobility-trace-example_8cc_source.html
//  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
// https://www.nsnam.org/docs/release/3.7/doxygen/classns3_1_1_random_walk2d_mobility_model.html
void set_mobility(ns3::NodeContainer& ns3_nodes,
                             const ns3_testbed_settings_t &testbed_settings) {

  // fill out our list position allocator
  ns3::Ptr<ns3::ListPositionAllocator>positionAlloc =
                         ns3::CreateObject<ns3::ListPositionAllocator>();
  for (std::vector<mobility_record_t>::const_iterator it =
                     testbed_settings.mobilities.begin();
                     it != testbed_settings.mobilities.end(); ++it) {
    positionAlloc->Add(ns3::Vector(it->x, it->y, 0.0));
  }

  // constant position mobility
  ns3::MobilityHelper constant_position_mobility;
  constant_position_mobility.SetPositionAllocator(positionAlloc);
  constant_position_mobility.SetMobilityModel(
                                      "ns3::ConstantPositionMobilityModel");

  // random walk2 mobility
  ns3::MobilityHelper random_walk2_mobility;
  random_walk2_mobility.SetPositionAllocator(positionAlloc);
  float l=testbed_settings.length; // box edge length
  random_walk2_mobility.SetMobilityModel(
          "ns3::RandomWalk2dMobilityModel", // model
          "Bounds", ns3::RectangleValue(ns3::Rectangle(-l,l,-l,l)),
          "Time", ns3::StringValue("2s"), // change after Time
          "Distance", ns3::StringValue("4.0"), // change after Distance
          "Mode", ns3::StringValue("Time"),   // use change after Time
          "Direction", ns3::StringValue(
                       "ns3::UniformRandomVariable[Min=0.0|Max=6.28318]"),
          "Speed", ns3::StringValue(
                       "ns3::UniformRandomVariable[Min=2.0|Max=10.0]")
  );

  for (int i=0; i<testbed_settings.count; i++) {
    if(testbed_settings.mobilities[i].walks) {
      random_walk2_mobility.Install(ns3_nodes.Get(i));
    } else {
      constant_position_mobility.Install(ns3_nodes.Get(i));
    }
  }
}

void mobility_interval_function(const ns3::NodeContainer& ns3_nodes,
                          const ns3_testbed_settings_t &testbed_settings) {

  // schedule next interval
  ns3::Simulator::Schedule(ns3::Seconds(0.1), &mobility_interval_function,
                                               ns3_nodes, testbed_settings);

  // round to 1 decimal point
  std::cout << std::fixed << std::setprecision(1);

  // show GS x,y,z position
  std::stringstream ss;
  ss << std::fixed << std::setprecision(1);
  for (int i=0; i<testbed_settings.count; i++) {
    ns3::Ptr<ns3::Node> node = ns3_nodes.Get(i);
    if(testbed_settings.mobilities[i].walks) {
      ns3::Ptr<ns3::RandomWalk2dMobilityModel> mobility_model =
                     node->GetObject<ns3::RandomWalk2dMobilityModel>();
      auto vector = mobility_model->GetPosition();
      ss << i << ":" << vector.x << "," << vector.y << "  ";
    } else {
      ns3::Ptr<ns3::ConstantPositionMobilityModel> mobility_model =
                   node->GetObject<ns3::ConstantPositionMobilityModel>();
      auto vector = mobility_model->GetPosition();
      ss << i << ":" << vector.x << "," << vector.y << "  ";
    }
  }
  ss << "\n";
  std::cout << ss.str();
}

void connect_tap_bridges(const ns3::NodeContainer& ns3_nodes,
                         const ns3::NetDeviceContainer& devices,
                         int count) {
  // connect TapBridge devices
  ns3::TapBridgeHelper tapBridge;
  tapBridge.SetAttribute("Mode", ns3::StringValue("UseLocal"));
  char buffer[10];
  for (int i=0; i<count; i++) {
    sprintf(buffer, "wifi_tap%d", i+1);
    tapBridge.SetAttribute("DeviceName", ns3::StringValue(buffer));
    tapBridge.Install(ns3_nodes.Get(i), devices.Get(i));
  }
}

void start_testbed(std::string name,
                   const ns3_testbed_settings_t& testbed_settings) {
  // set to run for a while
  ns3::Simulator::Stop(ns3::Seconds(60*60*24*365.)); // 1 year
  std::cout << "Starting " << name << ".\n"
            << "count: " << testbed_settings.count
            << ", length: " << testbed_settings.length << "\n";

  // run
  ns3::Simulator::Run();
  ns3::Simulator::Destroy();
  std::cout << "Ending" << name << ".\n"; // Ctrl-C does not reach this
}

