cmake_minimum_required(VERSION 3.20)

project(Phoenix
    VERSION 0.1.0
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Phoenix
    # Application
    core/src/main.cpp

    # Serial
    core/src/serial/WindowsSerialEnumerator.cpp
    core/src/serial/WindowsSerialTransport.cpp

    # Discovery
    core/src/discovery/LegacyXplproProbe.cpp
)    

target_include_directories(Phoenix
    PRIVATE
        core/include
)

if(WIN32)
    target_link_libraries(Phoenix
        PRIVATE
            Setupapi
    )
endif()
