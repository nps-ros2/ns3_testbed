#include <getopt.h>
#include <cstdio>
#include <iostream> // std::cout
#include <iomanip> // std::setprecision

#include "ns3/core-module.h"
#include "ns3/node-container.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/tap-bridge-module.h"
#include "ns3/random-variable-stream.h"

static int count = 5;
static int length = 30;

// parse user input
int get_options(int argc, char *argv[]) {

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
        count = std::atoi(optarg);
        break;
      }
      case 'l': {	// length
        length = std::atoi(optarg);
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
void set_mobility(ns3::NodeContainer& ns3_nodes) {

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

// desired approach
void ns3_setup(ns3::NodeContainer& ns3_nodes) {

  // run ns3 real-time with checksums
  ns3::GlobalValue::Bind("SimulatorImplementationType",
                          ns3::StringValue("ns3::RealtimeSimulatorImpl"));
  ns3::GlobalValue::Bind("ChecksumEnabled", ns3::BooleanValue(true));

//  ns3::GlobalValue::Bind("SynchronizationMode",
//                          ns3::StringValue("ns3::RealtimeSimulatorImpl::HardLimit"));
  ns3::Config::SetDefault("ns3::RealtimeSimulatorImpl::SynchronizationMode",ns3::EnumValue(ns3::RealtimeSimulatorImpl::SYNC_HARD_LIMIT));

  // Create ns3_nodes
  ns3_nodes.Create(count);

  // Wifi settings
  ns3::WifiHelper wifi;
  wifi.SetStandard(ns3::WIFI_PHY_STANDARD_80211a);
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                          "DataMode", ns3::StringValue("OfdmRate54Mbps"));

  // ad-hoc Wifi network
  ns3::WifiMacHelper wifiMac;
  wifiMac.SetType("ns3::AdhocWifiMac");

  // physical layer
  ns3::YansWifiChannelHelper wifiChannel(ns3::YansWifiChannelHelper::Default());
  ns3::YansWifiPhyHelper wifiPhy(ns3::YansWifiPhyHelper::Default());
  float p=16.0206; // default
  wifiPhy.Set("TxPowerStart", ns3::DoubleValue(p));
  wifiPhy.Set("TxPowerEnd", ns3::DoubleValue(p));
  wifiPhy.SetChannel(wifiChannel.Create());

  // install the wireless devices onto our ghost ns3_nodes.
  ns3::NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, ns3_nodes);

  // install mobility
  set_mobility(ns3_nodes);

  // connect Wifi through TapBridge devices
  ns3::TapBridgeHelper tapBridge;
  tapBridge.SetAttribute("Mode", ns3::StringValue("UseLocal"));
  char buffer[10];
  for (int i=0; i<count; i++) {
    sprintf(buffer, "wifi_tap%d", i+1);
    tapBridge.SetAttribute("DeviceName", ns3::StringValue(buffer));
    tapBridge.Install(ns3_nodes.Get(i), devices.Get(i));
  }
}

void interval_function(const ns3::NodeContainer& ns3_nodes) {

  // schedule next interval
  ns3::Simulator::Schedule(ns3::Seconds(0.1), &interval_function, ns3_nodes);

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

int main(int argc, char *argv[]) {
  get_options(argc, argv);

  // Force flush of the stdout buffer.
  setvbuf(stdout, NULL, _IONBF, BUFSIZ);

//  ns3::CommandLine cmd;
//  cmd.Parse(argc, argv);

  // set up ns-3
  ns3::NodeContainer ns3_nodes;
  ns3_setup(ns3_nodes);

  // set to run for a while
  ns3::Simulator::Stop(ns3::Seconds(60*60*24*365.)); // 1 year
//  ns3::Simulator::Stop(ns3::Seconds(6.0)); // 6 seconds
//  ns3::Simulator::Stop(ns3::Seconds(60*3)); // 3 minutes
//  ns3::Simulator::Stop(ns3::Seconds(3));

  std::cout << "Starting ns-3 Wifi simulator for " << count << " namespaces.\n";

  // start interval function
  interval_function(ns3_nodes);

  // run
  ns3::Simulator::Run();
  ns3::Simulator::Destroy();
  std::cout << "Ending ns-3 Wifi simulator.\n";
  return 0;
}

