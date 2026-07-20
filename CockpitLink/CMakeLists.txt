cmake_minimum_required(VERSION 3.20)

project(CockpitLink
    VERSION 0.1.0
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(CockpitLinkHost
    host/src/protocol/Frame.cpp
    host/src/protocol/FrameParser.cpp
    host/src/serial/SerialDeviceClassifier.cpp
    host/src/serial/WindowsSerialEnumerator.cpp
    host/src/serial/WindowsSerialTransport.cpp
)

target_include_directories(CockpitLinkHost
    PUBLIC
        host/include
)

if(WIN32)
    target_link_libraries(CockpitLinkHost
        PUBLIC
            Setupapi
    )
endif()

add_executable(CockpitLinkProbe
    host/apps/CockpitLinkProbe.cpp
)

target_link_libraries(CockpitLinkProbe
    PRIVATE
        CockpitLinkHost
)

enable_testing()

add_executable(CockpitLinkProtocolTests
    host/tests/protocol/FrameParserTests.cpp
)

target_link_libraries(CockpitLinkProtocolTests
    PRIVATE
        CockpitLinkHost
)

add_test(
    NAME CockpitLinkProtocolTests
    COMMAND CockpitLinkProtocolTests
)
