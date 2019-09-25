#include <getopt.h>
#include <cstdio>
#include <iostream> // std::cout
#include <iomanip> // std::setprecision

#include "ns3/core-module.h"
#include "ns3/node-container.h"
#include "ns3/network-module.h"
//#include "ns3/tap-bridge-module.h"
#include "ns3/csma-module.h"

#include "ns3_testbed_common.hpp"

int main(int argc, char *argv[]) {
  int count;
  int length;
  get_testbed_options(argc, argv, &count, &length);

  // testbed setup
  testbed_setup();

  // ns3_nodes
  ns3::NodeContainer ns3_nodes;
  ns3_nodes.Create(count);

  // install the CSMA devices onto our ns3_nodes
  ns3::CsmaHelper csma;
  ns3::NetDeviceContainer devices = csma.Install(ns3_nodes);

  // connect CSMA through TapBridge devices
  connect_tap_bridges(ns3_nodes, devices, count);

  // start testbed
  start_testbed("ns3_mobility", count, length);
  return 0;
}

