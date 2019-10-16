cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

set(CMAKE_BUILD_TYPE Debug)

include_directories(src ~/repos/ns-3-allinone/ns-3.29/build)
link_directories(~/repos/ns-3-allinone/ns-3.29/build/lib)
list(APPEND CMAKE_CXX_FLAGS)

add_library(ns3_testbed_settings ns3_testbed_settings.cpp)

add_library(ns3_testbed_common ns3_testbed_common.cpp)
target_link_libraries(ns3_testbed_common
  ns3_testbed_settings
  ns3.29-core-debug
  ns3.29-network-debug
  ns3.29-mobility-debug
  ns3.29-tap-bridge-debug
)

add_executable(ns3_mobility ns3_mobility.cpp)
target_link_libraries(ns3_mobility
  ns3_testbed_common
  ns3.29-core-debug
  ns3.29-network-debug
  ns3.29-wifi-debug
  ns3.29-mobility-debug
)

add_executable(ns3_csma ns3_csma.cpp)
target_link_libraries(ns3_csma
  ns3_testbed_common
  ns3.29-core-debug
  ns3.29-network-debug
  ns3.29-csma-debug
)

file(COPY ../csv_setup/ns3_defaults.csv DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ../csv_setup/ns3_1_4.csv DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ../csv_setup/ns3_25_7.csv DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

