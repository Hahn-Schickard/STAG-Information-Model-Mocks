cmake_minimum_required(VERSION 3.15)
project(PackageTest CXX)

find_package(Information_Model CONFIG REQUIRED)

add_executable(example example.cpp)
target_link_libraries(example Information_Model::Information_Model)
