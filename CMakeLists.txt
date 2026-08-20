cmake_minimum_required(VERSION 3.20)

project(binance_dataFetch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED)


add_executable(websocket src/main.cpp)

target_include_directories(websocket PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include 
)

target_link_libraries(websocket PRIVATE
    Boost::headers
    OpenSSL::SSL
    Threads::Threads
)

