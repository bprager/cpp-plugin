cmake_minimum_required(VERSION 3.10)

# set the project name
project(MosquittoServer)

# set C++ standard
set(CMAKE_CXX_STANDARD 14)

# add include directory
include_directories(/usr/include)

# specify the path to the mosquitto library
set(MOSQUITTO_PATH "/usr/lib/x86_64-linux-gnu")

# add the executable
add_executable(Server src/Server.cpp)

# specify where the mosquitto library resides
link_directories(/usr/lib/x86_64-linux-gnu)

# link Mosquitto library to the executable
target_link_libraries(Server mosquitto ${MOSQUITTO_PATH}/libmosquitto.so)

