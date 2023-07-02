include(${CMAKE_SOURCE_DIR}/cmake/${CMAKE_BUILD_TYPE}.cmake)

cmake_minimum_required(VERSION 3.0.0)
project(imgprocess VERSION 0.1.0 LANGUAGES C)

list(APPEND inc include)
list(APPEND src source/common.c)
list(APPEND src source/resize.c)

add_library(${PROJECT_NAME} ${src})
target_include_directories(${PROJECT_NAME} PRIVATE ${inc})
