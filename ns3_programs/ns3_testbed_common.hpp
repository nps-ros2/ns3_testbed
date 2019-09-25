#ifndef NS3_TESTBED_COMMON_HPP
#define NS3_TESTBED_COMMON_HPP

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

void testbed_setup();
int get_testbed_options(int argc, char *argv[], int *count, int *length);
void set_mobility(ns3::NodeContainer& ns3_nodes, int count, int length);
void mobility_interval_function(const ns3::NodeContainer& ns3_nodes, int count);
void connect_tap_bridges(const ns3::NodeContainer& ns3_nodes,
                         const ns3::NetDeviceContainer& devices,
                         int count);
void start_testbed(std::string name, int count, int length);

#endif

