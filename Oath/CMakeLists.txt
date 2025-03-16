cmake_minimum_required(VERSION 3.14)
project(Oath VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

include_directories(${CMAKE_SOURCE_DIR}/oath/include/nlohmann)
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

set(UTILS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/utils/JSONLoader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/utils/JSONSerializer.cpp
)

set(SYSTEM_CRAFTING_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/crafting/CraftingNode.cpp
)

set(SYSTEM_CRIME_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/crime/BountyPaymentNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/crime/CrimeLawConfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/crime/CrimeLawContext.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/crime/CrimeLawSystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/crime/CrimeRecord.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/crime/CrimeType.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/crime/CrimeSystemNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/crime/CriminalRecord.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/crime/GuardEncounterNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/crime/GuardResponseType.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/crime/JailNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/crime/PickpocketNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/crime/TheftExecutionNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/crime/TheftNode.cpp
)

set(SYSTEM_DIALOGUE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/dialogue/DialogueNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/dialogue/NPC.cpp
)

set(SYSTEM_ECONOMY_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/economy/MarketTypes.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/economy/TradeCommodity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/economy/TradeRoute.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/economy/EconomicEvent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/economy/Market.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/economy/MarketNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/economy/BusinessInvestment.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/economy/InvestmentNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/economy/PropertyTypes.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/economy/Property.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/economy/PropertyNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/economy/EconomicSystemNode.cpp
)

set(SYSTEM_FACTION_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/faction/FactionRelationship.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/faction/Faction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/faction/FactionSystemNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/faction/FactionQuestNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/faction/FactionDialogueNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/faction/FactionLocationNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/faction/PoliticalEvent.cpp
)

set(SYSTEM_HEALTH_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/health/Symptom.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/health/Immunity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/health/Disease.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/health/HealingMethod.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/health/HealthState.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/health/DiseaseManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/health/HealthNodes.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/health/HealthSetup.cpp
)

set(SYSTEM_MOUNT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/mount/MountStats.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/mount/MountEquipment.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/mount/MountBreed.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/mount/MountSystemConfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/mount/Mount.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/mount/MountStable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/mount/MountTrainingSession.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/mount/MountStableNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/mount/MountInteractionNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/mount/MountTrainingNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/mount/MountEquipmentShopNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/mount/MountRacingNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/mount/MountBreedingNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/mount/MountSystemController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/mount/MountSystem.cpp
)


set(SYSTEM_PROGRESSION_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/progression/SkillNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/progression/ClassNode.cpp
)

set(SYSTEM_QUEST_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/quest/QuestNode.cpp
)

set(SYSTEM_RELATIONSHIP_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/relationship/RelationshipConfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/relationship/RelationshipNPC.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/relationship/NPCRelationshipManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/relationship/NPCInteractionNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/relationship/NPCInfoNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/relationship/RelationshipBrowserNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/relationship/RelationshipSystemController.cpp
)

set(SYSTEM_RELIGION_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/religion/BlessingNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/religion/DeityNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/religion/PrayerNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/religion/ReligionController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/religion/ReligiousGameContext.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/religion/ReligiousQuestNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/religion/ReligiousStats.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/religion/RitualNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/religion/TempleNode.cpp
)

set(SYSTEM_SPELLCRAFTING_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/spellcrafting/SpellUtils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/spellcrafting/SpellComponent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/spellcrafting/SpellModifier.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/spellcrafting/SpellDelivery.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/spellcrafting/SpellDesign.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/spellcrafting/SpellCraftingSystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/spellcrafting/SpellCraftingNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/spellcrafting/SpellExaminationNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/spellcrafting/SpellbookNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/spellcrafting/MagicTrainingNode.cpp
)

set(SYSTEM_WEATHER_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/weather/WeatherTypes.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/weather/WeatherCondition.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/weather/WeatherSystemNode.cpp
)

set(SYSTEM_WORLD_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/world/LocationNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/world/RegionNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/oath/systems/world/TimeNode.cpp
)



# Combine all source files
set(SOURCES
    ${CORE_SOURCES}
    ${DATA_SOURCES}
    ${UTILS_SOURCES}

    ${SYSTEM_CRAFTING_SOURCES}
    ${SYSTEM_CRIME_SOURCES}
    ${SYSTEM_DIALOGUE_SOURCES}
    ${SYSTEM_ECONOMY_SOURCES}
    ${SYSTEM_FACTION_SOURCES}
    ${SYSTEM_HEALTH_SOURCES}
    ${SYSTEM_MOUNT_SOURCES}
    ${SYSTEM_PROGRESSION_SOURCES}
    ${SYSTEM_QUEST_SOURCES}
    ${SYSTEM_RELATIONSHIP_SOURCES}
    ${SYSTEM_RELIGION_SOURCES}
    ${SYSTEM_SPELLCRAFTING_SOURCES}
    ${SYSTEM_WEATHER_SOURCES}
    ${SYSTEM_WORLD_SOURCES}

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
