cmake_minimum_required(VERSION 3.10)
project(TreeAutomataRPG VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/Source/Include)
include_directories(${CMAKE_SOURCE_DIR}/Source/Public)

# Define source files with main() functions that will each become executables
set(MAIN_SOURCES
    Source/Oath_JSON.cpp
    Source/System_AdvancedWeather_JSON.cpp
    Source/System_CrimeLaw_JSON.cpp
    Source/System_DiseaseHealth_JSON.cpp
    Source/System_EconomyMarket_JSON.cpp
    Source/System_FactionReputation_JSON.cpp
    Source/System_Mount_JSON.cpp
    Source/System_NPCRelationships_JSON.cpp
    Source/System_ReligionDeity_JSON.cpp
    Source/System_SpellCrafting_JSON.cpp
    # Old/Oath_RPGSystems.cpp
    # Old/System_AdvancedWeather.cpp
    # Old/System_CrimeLaw.cpp
    # Old/System_DiseaseHealth.cpp
    # Old/System_EconomyMarket.cpp
    # Old/System_FactionReputation.cpp
    # Old/System_Mount.cpp
    # Old/System_NPCRelationships.cpp
    # Old/System_ReligionDeity.cpp
    # Old/System_SpellCrafting.cpp
    # Old/Generators/World/Generator_World.cpp
)

# Define the primary executable from the main RPG systems file
add_executable(${PROJECT_NAME} Source/Oath_JSON.cpp)

# Create individual executables for each file with main()
foreach(SOURCE_FILE ${MAIN_SOURCES})
    # Extract filename without extension to use as executable name
    get_filename_component(EXECUTABLE_NAME ${SOURCE_FILE} NAME_WE)
    
    # Add executable
    add_executable(${EXECUTABLE_NAME} ${SOURCE_FILE})
    
    # Set output directories for each executable
    set_target_properties(${EXECUTABLE_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release
    )
    
    # Copy data directory for each executable
    add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:${EXECUTABLE_NAME}>/data
    )
endforeach()

# Add test files
add_executable(Test_Generator_NPC Source/Generators/NPC/Test_Generator_NPC.cpp)
set_target_properties(Test_Generator_NPC PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release
)

# Global settings for debug configuration
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

# Install targets
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
foreach(SOURCE_FILE ${MAIN_SOURCES})
    get_filename_component(EXECUTABLE_NAME ${SOURCE_FILE} NAME_WE)
    install(TARGETS ${EXECUTABLE_NAME} DESTINATION bin)
endforeach()
install(TARGETS Test_Generator_NPC DESTINATION bin/tests)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/data/ DESTINATION bin/data)