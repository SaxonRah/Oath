cmake_minimum_required(VERSION 3.14)
project(Oath VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

include_directories(${CMAKE_SOURCE_DIR}/oath/include/nlohmann)
# include_directories(${CMAKE_SOURCE_DIR}/oath/systems/quest)
# include_directories(${CMAKE_SOURCE_DIR}/oath/systems/dialogue)
# include_directories(${CMAKE_SOURCE_DIR}/oath/systems/progression)
# include_directories(${CMAKE_SOURCE_DIR}/oath/systems/crafting)
# include_directories(${CMAKE_SOURCE_DIR}/oath/core)
# include_directories(${CMAKE_SOURCE_DIR}/oath/data)
# include_directories(${CMAKE_SOURCE_DIR}/oath/utils)
include_directories(${CMAKE_SOURCE_DIR}/oath/include)
include_directories(${CMAKE_SOURCE_DIR}/oath)
include_directories(${CMAKE_SOURCE_DIR})

# Set source files by directory
set(CORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/core/NodeID.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/core/TANode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/core/TAController.cpp
)

set(DATA_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/data/CharacterStats.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/data/WorldState.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/data/Inventory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/data/Item.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/data/Recipe.cpp
)

set(SYSTEM_QUEST_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/quest/QuestNode.cpp
)

set(SYSTEM_DIALOGUE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/dialogue/DialogueNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/dialogue/NPC.cpp
)

set(SYSTEM_PROGRESSION_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/progression/SkillNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/progression/ClassNode.cpp
)

set(SYSTEM_CRAFTING_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/crafting/CraftingNode.cpp
)

set(SYSTEM_WORLD_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/world/LocationNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/world/RegionNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/world/TimeNode.cpp
)

set(SYSTEM_WEATHER_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/weather/WeatherTypes.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/weather/WeatherCondition.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/weather/WeatherSystemNode.cpp
)

set(UTILS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/utils/JSONLoader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/utils/JSONSerializer.cpp
)

# Combine all source files
set(SOURCES
    ${CORE_SOURCES}
    ${DATA_SOURCES}
    ${SYSTEM_QUEST_SOURCES}
    ${SYSTEM_DIALOGUE_SOURCES}
    ${SYSTEM_PROGRESSION_SOURCES}
    ${SYSTEM_CRAFTING_SOURCES}
    ${SYSTEM_WORLD_SOURCES}
    ${SYSTEM_WEATHER_SOURCES}
    ${UTILS_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/main.cpp
)

# Create executable
add_executable(Oath ${SOURCES})

# Set include directories
target_include_directories(Oath PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Copy resource files to build directory
add_custom_command(TARGET Oath POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/oath/resources
    $<TARGET_FILE_DIR:Oath>/resources
)

# Add installation rules
install(TARGETS Oath
    RUNTIME DESTINATION bin
)

install(DIRECTORY oath/resources/
    DESTINATION bin/resources
)

# Set compiler warnings
if(MSVC)
    target_compile_options(Oath PRIVATE /W4)
else()
    target_compile_options(Oath PRIVATE -Wall -Wextra)
endif()
