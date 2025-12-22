cmake_minimum_required(VERSION 3.10)
project(Ule4Jis)

# Set CMake policy for Boost
cmake_policy(SET CMP0167 OLD)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set Boost root
set(BOOST_ROOT "C:/local/boost_1_90_0")

# Find Boost with specific components
find_package(Boost REQUIRED)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

# Enable MFC
set(CMAKE_MFC_FLAG 2)  # Use MFC in a shared DLL

# Define Unicode
add_definitions(-DUNICODE -D_UNICODE)

# Source files (excluding stdafx.cpp for precompiled headers)
set(SOURCES
    Ule4Jis.cpp
    Ule4JisDlg.cpp
    KeyCondition.cpp
    KeyEmulator.cpp
    KeyHooker.cpp
    KeyHookEventArgs.cpp
    NormalKeyEmulation.cpp
    PressAndReleaseDecorator.cpp
    ShiftPressDecorator.cpp
    ShiftReleaseDecorator.cpp
    USonJISStrategy.cpp
    Ule4Jis.rc
)

# Create executable
add_executable(Ule4Jis WIN32 ${SOURCES})

# Precompiled headers
target_precompile_headers(Ule4Jis PRIVATE stdafx.h)

# Include directories
target_include_directories(Ule4Jis PRIVATE .)

# Link libraries
if(Boost_FOUND)
    target_link_libraries(Ule4Jis ${Boost_LIBRARIES})
endif()

# Set subsystem to WINDOWS
set_target_properties(Ule4Jis PROPERTIES
    LINK_FLAGS "/SUBSYSTEM:WINDOWS"
)