cmake_minimum_required(VERSION 3.20)

project(Phoenix
    VERSION 0.1.0
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Phoenix
    core/src/main.cpp
    core/src/serial/WindowsSerialEnumerator.cpp
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
