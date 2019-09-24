#include <getopt.h>
#include <cstdio>
#include <iostream> // std::cout
#include <iomanip> // std::setprecision

#include "ns3/core-module.h"
#include "ns3/node-container.h"
#include "ns3/network-module.h"
#include "ns3/tap-bridge-module.h"
#include "ns3/csma-module.h"

static int count = 5;

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

      // end
      {0,0,0,0}
    };

    int ch = getopt_long(argc, argv, "hHc:", long_options, &option_index);

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
      default:
//        std::cerr << "unexpected command character " << ch << "\n";
        exit(1);
    }
  }

//  // parse the remaining tokens that were not consumed by options
//  argc -= optind;
//  argv += optind;

}

// desired setup
void ns3_setup(ns3::NodeContainer& ns3_nodes) {

  // run ns3 real-time with checksums
  ns3::GlobalValue::Bind("SimulatorImplementationType",
                          ns3::StringValue("ns3::RealtimeSimulatorImpl"));
  ns3::GlobalValue::Bind("ChecksumEnabled", ns3::BooleanValue(true));

  // Use HardLimit mode, being sure ns-3 is modified to warn instead of halt.
  ns3::Config::SetDefault("ns3::RealtimeSimulatorImpl::SynchronizationMode",ns3::EnumValue(ns3::RealtimeSimulatorImpl::SYNC_HARD_LIMIT));

  // Create ns3_nodes
  ns3_nodes.Create(count);

  // install the CSMA devices onto our ns3_nodes
  ns3::CsmaHelper csma;
  ns3::NetDeviceContainer devices = csma.Install(ns3_nodes);

  // connect CSMA through TapBridge devices
  ns3::TapBridgeHelper tapBridge;
  tapBridge.SetAttribute("Mode", ns3::StringValue("UseLocal"));
  char buffer[10];
  for (int i=0; i<count; i++) {
    sprintf(buffer, "wifi_tap%d", i+1); // should have been named ns3_tap
    tapBridge.SetAttribute("DeviceName", ns3::StringValue(buffer));
    tapBridge.Install(ns3_nodes.Get(i), devices.Get(i));
  }
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

  std::cout << "Starting ns-3 CSMA simulator for " << count << " namespaces.\n";

  // run
  ns3::Simulator::Run();
  ns3::Simulator::Destroy();
  std::cout << "Ending ns-3 CSMA simulator.\n";
  return 0;
}

