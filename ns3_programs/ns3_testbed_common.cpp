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


// parse user input
int get_testbed_options(int argc, char *argv[], int *count, int *length) {

  // defaults
  *count = 5;
  *length = 30;

  // parse options
  int option_index; // not used
  while (1) {

    const struct option long_options[] = {
      // options
      {"help",                          no_argument, 0, 'h'},
      {"Help",                          no_argument, 0, 'H'},
      {"count",                   required_argument, 0, 'c'},
      {"length",                  required_argument, 0, 'l'},

      // end
      {0,0,0,0}
    };

    int ch = getopt_long(argc, argv, "hHc:l:", long_options, &option_index);

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
        std::cout << "Usage: -h|-H|-c <count>|-l <length>\n";
        exit(0);
      }
      case 'H': {	// Help
        std::cout << "Usage: -h|-H|-c <count>|-l <length>\n";
        exit(0);
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
void set_mobility(ns3::NodeContainer& ns3_nodes, int count, int length) {

  // all antenna locations start deterministically at 0, 0, 0
  ns3::Ptr<ns3::ListPositionAllocator>positionAlloc =
                         ns3::CreateObject<ns3::ListPositionAllocator>();
  for (int i=0; i<count; i++) {
    positionAlloc->Add(ns3::Vector(0.0, 0.0, 0.0));
  }

  // ground station
  ns3::MobilityHelper gs_mobility;
  gs_mobility.SetPositionAllocator(positionAlloc);
  gs_mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

  // robots
  ns3::MobilityHelper r_mobility;
  r_mobility.SetPositionAllocator(positionAlloc);
  float y=length; // box edge length
  r_mobility.SetMobilityModel(
          "ns3::RandomWalk2dMobilityModel", // model
          "Bounds", ns3::RectangleValue(ns3::Rectangle(-1.0,y,-1.0,y)),
          "Time", ns3::StringValue("2s"), // change after Time
          "Distance", ns3::StringValue("4.0"), // change after Distance
          "Mode", ns3::StringValue("Time"),   // use change after Time
          "Direction", ns3::StringValue(
                       "ns3::UniformRandomVariable[Min=0.0|Max=6.28318]"),
          "Speed", ns3::StringValue(
                       "ns3::UniformRandomVariable[Min=2.0|Max=10.0]")
  );

  // apply mobility to GS and robots
  gs_mobility.Install(ns3_nodes.Get(0));
  for (int i=1; i<count; i++) {
    r_mobility.Install(ns3_nodes.Get(i));
  }
}

void mobility_interval_function(const ns3::NodeContainer& ns3_nodes,
                                                            int count) {

  // schedule next interval
  ns3::Simulator::Schedule(ns3::Seconds(0.1), &mobility_interval_function,
                                                          ns3_nodes, count);

  // round to 1 decimal point
  std::cout << std::fixed << std::setprecision(1);

  // show GS x,y,z position
  ns3::Ptr<ns3::Node> node = ns3_nodes.Get(0);
  ns3::Ptr<ns3::ConstantPositionMobilityModel> mobility_model =
                   node->GetObject<ns3::ConstantPositionMobilityModel>();
  auto vector = mobility_model->GetPosition();
  std::cout << vector.x << "  " << vector.y << "  " << vector.z << "      ";

  // show robot x,y,z positions
  for (int i=1; i<count; i++) {
    ns3::Ptr<ns3::Node> node = ns3_nodes.Get(i);
    ns3::Ptr<ns3::RandomWalk2dMobilityModel> mobility_model =
                     node->GetObject<ns3::RandomWalk2dMobilityModel>();
    auto vector = mobility_model->GetPosition();
    std::cout << vector.x << "  " << vector.y << "  " << vector.z << "      ";
  }
  std::cout << std::endl;
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

void start_testbed(std::string name, int count, int length) {
  // set to run for a while
  ns3::Simulator::Stop(ns3::Seconds(60*60*24*365.)); // 1 year
  std::cout << "Starting " << name << ".\n"
            << "count: " << count << ", length: " << length << "\n";

  // run
  ns3::Simulator::Run();
  ns3::Simulator::Destroy();
  std::cout << "Ending" << name << ".\n"; // Ctrl-C does not reach this
}

