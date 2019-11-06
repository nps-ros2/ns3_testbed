cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

set(CMAKE_BUILD_TYPE Debug)

include_directories(src ~/repos/ns-allinone-3.30.1/ns-3.30.1/build)
link_directories(~/repos/ns-allinone-3.30.1/ns-3.30.1/build/lib)
list(APPEND CMAKE_CXX_FLAGS)

add_library(ns3_testbed_settings ns3_testbed_settings.cpp)

add_library(ns3_testbed_common ns3_testbed_common.cpp)
target_link_libraries(ns3_testbed_common
  ns3_testbed_settings
  ns3.30.1-core-debug
  ns3.30.1-network-debug
  ns3.30.1-mobility-debug
  ns3.30.1-tap-bridge-debug
)

add_executable(ns3_mobility ns3_mobility.cpp)
target_link_libraries(ns3_mobility
  ns3_testbed_common
  ns3.30.1-core-debug
  ns3.30.1-network-debug
  ns3.30.1-wifi-debug
  ns3.30.1-mobility-debug
)

add_executable(ns3_csma ns3_csma.cpp)
target_link_libraries(ns3_csma
  ns3_testbed_common
  ns3.30.1-core-debug
  ns3.30.1-network-debug
  ns3.30.1-csma-debug
)

file(COPY ../csv_setup/ns3_defaults.csv DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ../csv_setup/ns3_1_4.csv DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ../csv_setup/ns3_25_7.csv DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ../csv_setup/ns3_1_gs.csv DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

