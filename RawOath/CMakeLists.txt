cmake_minimum_required(VERSION 3.10)
project(TreeAutomataRPG)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files
file(GLOB SOURCES "*.cpp")

# Create executable
add_executable(TreeAutomataRPG ${SOURCES})