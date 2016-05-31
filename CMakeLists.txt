cmake_minimum_required(VERSION 2.8.12)
project(smt)
set(CMAKE_BUILD_TYPE Debug)

SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
#Bring the headers, such as Student.h into the project
include_directories(include)

#However, the file(GLOB...) allows for wildcard additions:
file(GLOB SOURCES "src/*.c")

#Generate the shared library from the sources
add_library(smt SHARED ${SOURCES})

#Set the location for library installation -- i.e., /usr/lib in this case
# not really necessary in this example. Use "sudo make install" to apply
install(TARGETS smt DESTINATION /usr/lib)
