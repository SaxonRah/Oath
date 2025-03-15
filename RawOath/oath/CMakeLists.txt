cmake_minimum_required(VERSION 3.14)
project(Oath VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(${CMAKE_SOURCE_DIR}/include)

# Find required packages
find_package(nlohmann_json 3.9.0 REQUIRED)

# Set source files by directory
set(CORE_SOURCES
    core/NodeID.cpp
    core/TANode.cpp
    core/TAController.cpp
)

set(DATA_SOURCES
    data/GameContext.cpp
    data/CharacterStats.cpp
    data/WorldState.cpp
    data/Inventory.cpp
    data/Item.cpp
    data/Recipe.cpp
)

set(SYSTEM_QUEST_SOURCES
    systems/quest/QuestNode.cpp
)

set(SYSTEM_DIALOGUE_SOURCES
    systems/dialogue/DialogueNode.cpp
    systems/dialogue/NPC.cpp
)

set(SYSTEM_PROGRESSION_SOURCES
    systems/progression/SkillNode.cpp
    systems/progression/ClassNode.cpp
)

set(SYSTEM_CRAFTING_SOURCES
    systems/crafting/CraftingNode.cpp
)

set(SYSTEM_WORLD_SOURCES
    systems/world/LocationNode.cpp
    systems/world/RegionNode.cpp
    systems/world/TimeNode.cpp
)

set(UTILS_SOURCES
    utils/JSONLoader.cpp
    utils/JSONSerializer.cpp
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
    ${UTILS_SOURCES}
    main.cpp
)

# Create executable
add_executable(Oath ${SOURCES})

# Set include directories
target_include_directories(Oath PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link libraries
target_link_libraries(Oath PRIVATE
    nlohmann_json::nlohmann_json
)

# Copy resource files to build directory
add_custom_command(TARGET Oath POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/resources
    $<TARGET_FILE_DIR:Oath>/resources
)

# Add installation rules
install(TARGETS Oath
    RUNTIME DESTINATION bin
)

install(DIRECTORY resources/
    DESTINATION bin/resources
)

# Set compiler warnings
if(MSVC)
    target_compile_options(Oath PRIVATE /W4)
else()
    target_compile_options(Oath PRIVATE -Wall -Wextra)
endif()

# Enable testing
# enable_testing()

# Create test executable (optional)
# set(TEST_SOURCES
    # tests/test_main.cpp
    # Add test source files here
# )

# Uncomment to add tests
# add_executable(OathTests ${TEST_SOURCES} ${CORE_SOURCES} ${DATA_SOURCES} ${SYSTEM_SOURCES} ${UTILS_SOURCES})
# target_link_libraries(OathTests PRIVATE nlohmann_json::nlohmann_json)
# add_test(NAME OathTests COMMAND OathTests)