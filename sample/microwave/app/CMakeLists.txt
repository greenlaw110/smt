cmake_minimum_required(VERSION 2.8.12)
project(mw_app)
set(CMAKE_BUILD_TYPE Release)

#For the shared library:
set ( PROJECT_LINK_LIBS libsmt.so libmicrowave.so )
link_directories( ../statemachine/build ../../../build )

#Bring the headers, such as Student.h into the project
include_directories(../statemachine/include ../../../include)

#However, the file(GLOB...) allows for wildcard additions:
file(GLOB SOURCES "*.c")

add_executable(mw_app mw_machine_demo.c )
target_link_libraries( mw_app ${PROJECT_LINK_LIBS} )