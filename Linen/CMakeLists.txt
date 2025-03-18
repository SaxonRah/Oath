cmake_minimum_required(VERSION 3.14)
project(Linen VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

include_directories(${CMAKE_SOURCE_DIR}/Include/nlohmann)
include_directories(${CMAKE_SOURCE_DIR}/Source)
include_directories(${CMAKE_SOURCE_DIR})

# Set source files by directory
# set(CORE_SOURCES
#     ${CMAKE_CURRENT_SOURCE_DIR}/Source/GameContext.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/Source/GameSystemManager.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/Source/TAController.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/Source/TACore.cpp
# )


# Combine all source files
set(SOURCES
    # ${CORE_SOURCES}

    ${CMAKE_CURRENT_SOURCE_DIR}/CodeGolf_RPGSystems.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/CodeGolf_Linen.cpp
)

# Create executable
add_executable(Linen ${SOURCES})

# Set include directories
target_include_directories(Linen PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Copy resource files to build directory
add_custom_command(TARGET Linen POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/data
    $<TARGET_FILE_DIR:Linen>/data
)

# Add installation rules
install(TARGETS Linen
    RUNTIME DESTINATION bin
)

install(DIRECTORY Linen/data/
    DESTINATION bin/data
)

# Set compiler warnings
if(MSVC)
    target_compile_options(Linen PRIVATE /W4)
else()
    target_compile_options(Linen PRIVATE -Wall -Wextra)
endif()
