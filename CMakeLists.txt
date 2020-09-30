cmake_minimum_required(VERSION 3.10)

# set the project name
set(ProjectName SOEMGui) 
set (CMAKE_CXX_STANDARD 17)

project(${ProjectName})
include(cmake/StandardProjectSettings.cmake)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

set(${ProjectName}_BIN_DIR "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${${ProjectName}_BIN_DIR})

#SET(CMAKE_AUTOUIC ON)

add_subdirectory(src)


