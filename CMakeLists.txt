cmake_minimum_required(VERSION 3.14)
project(Quirky)

set(CMAKE_CXX_STANDARD 20)

add_executable(Quirky src/main.cpp src/api/uci/uci_protocol.h src/api/xboard/xboard_protocol.h src/api/api.h src/api/api.cpp src/api/uci/uci_protocol.cpp src/util/io.h src/api/xboard/xboard_protocol.cpp)
