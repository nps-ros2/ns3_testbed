#include <getopt.h>
#include <cstdio>
#include <iostream> // std::cout
#include <iomanip> // std::setprecision

#include "ns3/core-module.h"
#include "ns3/node-container.h"
#include "ns3/network-module.h"
//#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
//#include "ns3/tap-bridge-module.h"

#include "ns3_testbed_common.hpp"
#include "ns3_testbed_settings.hpp"

int main(int argc, char *argv[]) {
  int count;
  int length;
  std::string setup_file;
  get_testbed_options(argc, argv, &count, &length, &setup_file);
  ns3_testbed_settings_t testbed_settings(count, length, setup_file);

  // testbed setup
  testbed_setup();

  // ns3_nodes
  ns3::NodeContainer ns3_nodes;
  ns3_nodes.Create(count);

  // Wifi settings
  ns3::WifiHelper wifi;
  wifi.SetStandard(ns3::WIFI_PHY_STANDARD_80211a);
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                          "DataMode", ns3::StringValue("OfdmRate54Mbps"));

  // physical layer
  ns3::YansWifiChannelHelper wifiChannel(ns3::YansWifiChannelHelper::Default());
  ns3::YansWifiPhyHelper wifiPhy(ns3::YansWifiPhyHelper::Default());
  float p=16.0206; // default
  wifiPhy.Set("TxPowerStart", ns3::DoubleValue(p));
  wifiPhy.Set("TxPowerEnd", ns3::DoubleValue(p));
  wifiPhy.SetChannel(wifiChannel.Create());

  // ad-hoc Wifi network
  ns3::WifiMacHelper wifiMac;
  wifiMac.SetType("ns3::AdhocWifiMac");

  // install the wireless devices onto our ghost ns3_nodes.
  ns3::NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, ns3_nodes);

  // install mobility model for our nodes
  set_mobility(ns3_nodes, testbed_settings);

  // connect Wifi through TapBridge devices
  connect_tap_bridges(ns3_nodes, devices, count);

  // start interval function
  mobility_interval_function(ns3_nodes, testbed_settings);

  // start testbed
  start_testbed("ns3_mobility", testbed_settings);
  return 0;
}

