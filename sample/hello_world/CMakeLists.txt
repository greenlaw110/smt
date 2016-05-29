cmake_minimum_required(VERSION 2.8.12)
project(helloworld)
set(CMAKE_BUILD_TYPE Release)

#For the shared library:
set ( PROJECT_LINK_LIBS libsmt.so )
link_directories( ../../build )

#Bring the headers, such as Student.h into the project
include_directories(. ../../include)

#However, the file(GLOB...) allows for wildcard additions:
file(GLOB SOURCES "*.c")

add_executable(helloworld smt_user_defs.c helloworld.c )
target_link_libraries( helloworld ${PROJECT_LINK_LIBS} )