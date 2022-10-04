cmake_minimum_required(VERSION 3.13)

project(HHE_PPML_APP)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

find_package(SEAL 4.0.0 EXACT REQUIRED)

include_directories(/usr/local/include)
include_directories(asio-1.24.0/include)
link_directories(/usr/local/lib)

add_executable(server-test server.cpp)
add_executable(client-test client.cpp)

target_link_libraries(server-test SEAL::seal pthread)
target_link_libraries(client-test SEAL::seal pthread)
