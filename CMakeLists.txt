cmake_minimum_required(VERSION 3.20)

project(binance_dataFetch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED)
find_package(Java REQUIRED COMPONENTS Runtime)

set(SBE_TOOL_VERSION "1.35.1")
set(SBE_JAR "${CMAKE_SOURCE_DIR}/tools/sbe-all-${SBE_TOOL_VERSION}.jar")
set(SBE_SCHEMA "${CMAKE_SOURCE_DIR}/schema/spot_stream.xml")
set(SBE_GENERATED_DIR "${CMAKE_SOURCE_DIR}/include/generated")

if(NOT EXISTS "${SBE_JAR}")
    message(STATUS "Downloading sbe-all-${SBE_TOOL_VERSION}.jar")
    file(DOWNLOAD
        "https://repo1.maven.org/maven2/uk/co/real-logic/sbe-all/${SBE_TOOL_VERSION}/sbe-all-${SBE_TOOL_VERSION}.jar"
        "${SBE_JAR}"
        TLS_VERIFY ON)
endif()

set(SBE_GENERATED_HEADERS
    "${SBE_GENERATED_DIR}/spot_stream/MessageHeader.h"
    "${SBE_GENERATED_DIR}/spot_stream/TradesStreamEvent.h"
    "${SBE_GENERATED_DIR}/spot_stream/BestBidAskStreamEvent.h"
    "${SBE_GENERATED_DIR}/spot_stream/DepthSnapshotStreamEvent.h"
    "${SBE_GENERATED_DIR}/spot_stream/DepthDiffStreamEvent.h"
)

add_custom_command(
    OUTPUT ${SBE_GENERATED_HEADERS}
    COMMAND ${Java_JAVA_EXECUTABLE} --add-opens java.base/jdk.internal.misc=ALL-UNNAMED
            -Dsbe.generate.ir=true
            -Dsbe.target.language=Cpp
            -Dsbe.output.dir=${SBE_GENERATED_DIR}
            -Dsbe.errorLog=yes
            -jar ${SBE_JAR} ${SBE_SCHEMA}
    DEPENDS ${SBE_SCHEMA} ${SBE_JAR}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Generating SBE C++ codecs from schema/spot_stream.xml"
    VERBATIM
)

add_custom_target(generate_sbe DEPENDS ${SBE_GENERATED_HEADERS})

add_executable(websocket
    src/main.cpp
    src/net/websocket_client.cpp
    src/sbe/decoder.cpp
)
add_dependencies(websocket generate_sbe)

target_include_directories(websocket PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${SBE_GENERATED_DIR}
)

target_link_libraries(websocket PRIVATE
    Boost::headers
    OpenSSL::SSL
    Threads::Threads
)

