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

// https://www.nsnam.org/doxygen/80211n-mimo_8cc_source.html

int main(int argc, char *argv[]) {
  int count;
  int length;
  get_testbed_options(argc, argv, &count, &length);

  // testbed setup
  testbed_setup();

  // ns3_nodes
  ns3::NodeContainer ns3_nodes;
  ns3_nodes.Create(count);

  // Wifi settings
  // https://www.nsnam.org/docs/models/html/wifi-user.html
  ns3::WifiHelper wifiHelper;
//  wifiHelper.SetStandard(ns3::WIFI_PHY_STANDARD_80211n_2_4GHZ);
  wifiHelper.SetStandard(ns3::WIFI_PHY_STANDARD_80211n_5GHZ);
  wifiHelper.SetRemoteStationManager("ns3::ConstantRateWifiManager");
                                      "DataMode", ns3::StringValue("HtMcs7"),
                                      "ControlMode", StringValue("HtMcs0"));

  // use default ArfWifiManager instead of ConstantRateWifiManager
//  wifiHelper.SetRemoteStationManager("ns3::ConstantRateWifiManager");
//                                      "DataMode", ns3::StringValue("HtMcs7"),
//                                      "ControlMode", StringValue("HtMcs0"));

//  wifiHelper.SetStandard(ns3::WIFI_PHY_STANDARD_80211n_5GHZ);
//  wifiHelper.SetRemoteStationManager("ns3::ConstantRateWifiManager",
//                          "DataMode", ns3::StringValue("HtMcs1"),
//                          "ControlMode", ns3::StringValue("OfdmRate24Mbps"));

//  wifiHelper.SetRemoteStationManager("ns3::ConstantRateWifiManager",
//                          "DataMode", ns3::StringValue("OfdmRate54Mbps"));

//  wifiHelper.SetRemoteStationManager("ns3::ConstantRateWifiManager",
//                          "DataMode", ns3::StringValue("OfdmRate24Mbps"));

//  // physical layer
//  ns3::YansWifiChannelHelper wifiChannel(ns3::YansWifiChannelHelper::Default());

  ns3::YansWifiPhyHelper wifiPhyHelper(ns3::YansWifiPhyHelper::Default());

//  float p=16.0206; // default
//  wifiPhyHelper.Set("TxPowerStart", ns3::DoubleValue(p));
//  wifiPhyHelper.Set("TxPowerEnd", ns3::DoubleValue(p));
  wifiPhyHelper.SetChannel(wifiChannel.Create());

/*
  // ad-hoc Wifi network
  ns3::WifiMacHelper wifiMac;
  wifiMac.SetType("ns3::AdhocWifiMac");
*/
  // Infrastructure: StaWifiMac and ApWifiMac
  ns3::Ssid ssid = Ssid("ns3-ssid");

  // ApWifiMac
  ns3::WifiMacHelper wifiMacHelper;

  // StaWifiMac:
  ns3::WifiMacHelper wifiMacHelper;
  wifiMacHelper.SetType("ns3::StaWifiMac",
                        "Ssid", SsidValue(ssid),
                        "ActiveProbing", ns3::BooleanValue(false));

  ns3::NetDeviceContainer sta_device_container;
  sta_device_container = wifiHelper.Install(
//                        "BeaconGeneration", ns3::BooleanValue(true),
//                        "BeaconInterval", ns3::TimeValue(ns3::Seconds(2.5)));


  wifiMacHelper.SetType("ns3::AdhocWifiMac");

  // install the wireless devices onto our ghost ns3_nodes.
  ns3::NetDeviceContainer devices = wifi.Install(wifiPhyHelper, wifiMacHelper,
                                                 ns3_nodes);

  // install mobility model for our nodes
  set_mobility(ns3_nodes, count, length);

  // connect Wifi through TapBridge devices
  connect_tap_bridges(ns3_nodes, devices, count);

  // start interval function
  mobility_interval_function(ns3_nodes, count);

  // start testbed
  start_testbed("ns3_mobility", count, length);
  return 0;
}

