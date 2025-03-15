#include <JSONSerializer.hpp>

nlohmann::json serializeCharacterStats(const CharacterStats& stats)
{
    nlohmann::json statsData;

    // Basic stats
    statsData["strength"] = stats.strength;
    statsData["dexterity"] = stats.dexterity;
    statsData["constitution"] = stats.constitution;
    statsData["intelligence"] = stats.intelligence;
    statsData["wisdom"] = stats.wisdom;
    statsData["charisma"] = stats.charisma;

    // Skills
    statsData["skills"] = stats.skills;

    // Faction reputation
    statsData["factionReputation"] = stats.factionReputation;

    // Known facts
    statsData["knownFacts"] = nlohmann::json::array();
    for (const auto& fact : stats.knownFacts) {
        statsData["knownFacts"].push_back(fact);
    }

    // Unlocked abilities
    statsData["unlockedAbilities"] = nlohmann::json::array();
    for (const auto& ability : stats.unlockedAbilities) {
        statsData["unlockedAbilities"].push_back(ability);
    }

    return statsData;
}

void deserializeCharacterStats(const nlohmann::json& statsData, CharacterStats& stats)
{
    // Basic stats
    stats.strength = statsData["strength"];
    stats.dexterity = statsData["dexterity"];
    stats.constitution = statsData["constitution"];
    stats.intelligence = statsData["intelligence"];
    stats.wisdom = statsData["wisdom"];
    stats.charisma = statsData["charisma"];

    // Skills
    stats.skills = statsData["skills"].get<std::map<std::string, int>>();

    // Faction reputation
    stats.factionReputation = statsData["factionReputation"].get<std::map<std::string, int>>();

    // Known facts
    stats.knownFacts.clear();
    for (const auto& fact : statsData["knownFacts"]) {
        stats.knownFacts.insert(fact);
    }

    // Unlocked abilities
    stats.unlockedAbilities.clear();
    for (const auto& ability : statsData["unlockedAbilities"]) {
        stats.unlockedAbilities.insert(ability);
    }
}

nlohmann::json serializeWorldState(const WorldState& state)
{
    nlohmann::json worldData;

    worldData["locationStates"] = state.locationStates;
    worldData["factionStates"] = state.factionStates;
    worldData["worldFlags"] = state.worldFlags;
    worldData["daysPassed"] = state.daysPassed;
    worldData["currentSeason"] = state.currentSeason;

    return worldData;
}

void deserializeWorldState(const nlohmann::json& worldData, WorldState& state)
{
    state.locationStates = worldData["locationStates"].get<std::map<std::string, std::string>>();
    state.factionStates = worldData["factionStates"].get<std::map<std::string, std::string>>();
    state.worldFlags = worldData["worldFlags"].get<std::map<std::string, bool>>();
    state.daysPassed = worldData["daysPassed"];
    state.currentSeason = worldData["currentSeason"];
}

nlohmann::json serializeInventory(const Inventory& inventory)
{
    nlohmann::json inventoryData = nlohmann::json::array();

    for (const auto& item : inventory.items) {
        nlohmann::json itemData;
        itemData["id"] = item.id;
        itemData["name"] = item.name;
        itemData["type"] = item.type;
        itemData["value"] = item.value;
        itemData["quantity"] = item.quantity;

        nlohmann::json properties;
        for (const auto& [key, value] : item.properties) {
            if (std::holds_alternative<int>(value)) {
                properties[key] = std::get<int>(value);
            } else if (std::holds_alternative<float>(value)) {
                properties[key] = std::get<float>(value);
            } else if (std::holds_alternative<std::string>(value)) {
                properties[key] = std::get<std::string>(value);
            } else if (std::holds_alternative<bool>(value)) {
                properties[key] = std::get<bool>(value);
            }
        }
        itemData["properties"] = properties;

        inventoryData.push_back(itemData);
    }

    return inventoryData;
}

void deserializeInventory(const nlohmann::json& inventoryData, Inventory& inventory)
{
    inventory.items.clear();

    for (const auto& itemData : inventoryData) {
        Item item(
            itemData["id"],
            itemData["name"],
            itemData["type"],
            itemData["value"],
            itemData["quantity"]);

        // Load properties
        for (const auto& [key, value] : itemData["properties"].items()) {
            if (value.is_number_integer()) {
                item.properties[key] = value.get<int>();
            } else if (value.is_number_float()) {
                item.properties[key] = value.get<float>();
            } else if (value.is_string()) {
                item.properties[key] = value.get<std::string>();
            } else if (value.is_boolean()) {
                item.properties[key] = value.get<bool>();
            }
        }

        inventory.items.push_back(item);
    }
}