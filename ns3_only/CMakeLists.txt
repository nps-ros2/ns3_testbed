cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

set(CMAKE_BUILD_TYPE Debug)

include_directories(src ~/repos/ns-allinone-3.30.1/ns-3.30.1/build)
link_directories(~/repos/ns-allinone-3.30.1/ns-3.30.1/build/lib)
list(APPEND CMAKE_CXX_FLAGS)

add_executable(wifi-simple-adhoc wifi-simple-adhoc.cc)
target_link_libraries(wifi-simple-adhoc
  ns3.30.1-core-debug
  ns3.30.1-network-debug
  ns3.30.1-wifi-debug
  ns3.30.1-mobility-debug
  ns3.30.1-internet-debug
)

add_executable(wifi-simple-infra wifi-simple-infra.cc)
target_link_libraries(wifi-simple-infra
  ns3.30.1-core-debug
  ns3.30.1-network-debug
  ns3.30.1-wifi-debug
  ns3.30.1-mobility-debug
  ns3.30.1-internet-debug
)

add_executable(wifi_80211n wifi_80211n.cc)
target_link_libraries(wifi_80211n
  ns3.30.1-core-debug
  ns3.30.1-network-debug
  ns3.30.1-wifi-debug
  ns3.30.1-mobility-debug
  ns3.30.1-internet-debug
)

