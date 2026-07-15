cmake_minimum_required(VERSION 3.20)

project(Phoenix
    VERSION 0.1.0
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Phoenix
    # Application
    core/src/main.cpp
    core/src/dev/DevelopmentDeviceLoop.cpp

    # Logging
    core/src/logging/Log.cpp
    core/src/logging/SerialTraceLogger.cpp

    # Profile
    core/src/profile/JsonDeviceProfileStore.cpp

    # Protocol
    core/src/protocol/legacy/LegacyFrame.cpp
    core/src/protocol/legacy/LegacyFrameParser.cpp
    core/src/protocol/legacy/LegacyMessage.cpp

    # Runtime
    core/src/runtime/LegacyDeviceController.cpp
    core/src/runtime/LegacyDeviceSession.cpp
    core/src/runtime/LegacyUpdateScheduler.cpp

    # Serial
    core/src/serial/WindowsSerialEnumerator.cpp
    core/src/serial/WindowsSerialTransport.cpp
    core/src/serial/SerialDeviceClassifier.cpp

    # Discovery
    core/src/discovery/LegacyXplproProbe.cpp
)

target_include_directories(Phoenix
    PRIVATE
        core/include
)

target_compile_definitions(Phoenix
    PRIVATE
        PHOENIX_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

if(WIN32)
    target_link_libraries(Phoenix
        PRIVATE
            Setupapi
    )
endif()

enable_testing()

add_executable(PhoenixProtocolTests
    tests/protocol/LegacyFrameParserTests.cpp
    core/src/protocol/legacy/LegacyFrame.cpp
    core/src/protocol/legacy/LegacyFrameParser.cpp
    core/src/protocol/legacy/LegacyMessage.cpp
)

target_include_directories(PhoenixProtocolTests
    PRIVATE
        core/include
)

add_test(
    NAME PhoenixProtocolTests
    COMMAND PhoenixProtocolTests
)

add_executable(PhoenixRuntimeTests
    tests/runtime/LegacyDeviceSessionTests.cpp
    core/src/logging/SerialTraceLogger.cpp
    core/src/protocol/legacy/LegacyFrame.cpp
    core/src/protocol/legacy/LegacyFrameParser.cpp
    core/src/protocol/legacy/LegacyMessage.cpp
    core/src/runtime/LegacyDeviceSession.cpp
)

target_include_directories(PhoenixRuntimeTests
    PRIVATE
        core/include
)

add_test(
    NAME PhoenixRuntimeTests
    COMMAND PhoenixRuntimeTests
)

add_executable(PhoenixDeviceControllerTests
    tests/runtime/LegacyDeviceControllerTests.cpp
    core/src/logging/SerialTraceLogger.cpp
    core/src/protocol/legacy/LegacyFrame.cpp
    core/src/protocol/legacy/LegacyFrameParser.cpp
    core/src/protocol/legacy/LegacyMessage.cpp
    core/src/runtime/LegacyDeviceController.cpp
    core/src/runtime/LegacyDeviceSession.cpp
)

target_include_directories(PhoenixDeviceControllerTests
    PRIVATE
        core/include
)

add_test(
    NAME PhoenixDeviceControllerTests
    COMMAND PhoenixDeviceControllerTests
)

add_executable(PhoenixUpdateSchedulerTests
    tests/runtime/LegacyUpdateSchedulerTests.cpp
    core/src/logging/SerialTraceLogger.cpp
    core/src/protocol/legacy/LegacyFrame.cpp
    core/src/protocol/legacy/LegacyFrameParser.cpp
    core/src/protocol/legacy/LegacyMessage.cpp
    core/src/runtime/LegacyDeviceController.cpp
    core/src/runtime/LegacyDeviceSession.cpp
    core/src/runtime/LegacyUpdateScheduler.cpp
)

target_include_directories(PhoenixUpdateSchedulerTests
    PRIVATE
        core/include
)

add_test(
    NAME PhoenixUpdateSchedulerTests
    COMMAND PhoenixUpdateSchedulerTests
)

add_executable(PhoenixProfileTests
    tests/profile/JsonDeviceProfileStoreTests.cpp
    core/src/profile/JsonDeviceProfileStore.cpp
)

target_include_directories(PhoenixProfileTests
    PRIVATE
        core/include
)

add_test(
    NAME PhoenixProfileTests
    COMMAND PhoenixProfileTests
)
