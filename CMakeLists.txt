cmake_minimum_required(VERSION 3.10)

# set the project name
project(MosquittoServer)

# set C++ standard
set(CMAKE_CXX_STANDARD 14)

# add include directory
include_directories(/usr/include)

# add the executable
add_executable(MosquittoServer src/MosquittoServer.cpp)

# specify where the mosquitto library resides
link_directories(/usr/lib/x86_64-linux-gnu)

# link Mosquitto library to the executable
target_link_libraries(MosquittoServer mosquitto)

