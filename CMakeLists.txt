include(${CMAKE_SOURCE_DIR}/cmake/${CMAKE_BUILD_TYPE}.cmake)

cmake_minimum_required(VERSION 3.0.0)
project(imgprocess_app VERSION 0.1.0 LANGUAGES C)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_subdirectory(lib)

list(APPEND inc lib/include)
list(APPEND src test/main.c)
list(APPEND lib imgprocess)
list(APPEND lib Threads::Threads)

add_executable(${PROJECT_NAME} ${src})
target_link_libraries(${PROJECT_NAME} PRIVATE ${lib})
target_include_directories(${PROJECT_NAME} PRIVATE ${inc})
