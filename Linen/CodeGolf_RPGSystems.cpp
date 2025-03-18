#include <ctime>
#include <functional>
#include <map>
#include <memory>
#include <random>
#include <string>
#include <vector>


namespace oath {

// Base Node ID class for all nodes in the system
class NodeID {
public:
    NodeID(const std::string& id)
        : id_(id)
    {
    }
    const std::string& getId() const { return id_; }

private:
    std::string id_;
};

// Tree Automata Node - the foundation of our entire system
class TANode {
public:
    TANode(const NodeID& id)
        : id_(id)
    {
    }
    virtual ~TANode() = default;

    // Add child nodes
    void addChild(std::shared_ptr<TANode> child)
    {
        children_.push_back(child);
    }

    // Process function to be implemented by specific nodes
    virtual void process(void* context)
    {
        for (auto& child : children_) {
            child->process(context);
        }
    }

    const NodeID& getId() const { return id_; }

protected:
    NodeID id_;
    std::vector<std::shared_ptr<TANode>> children_;
};

// Controller for the tree automata system
class TAController {
public:
    void registerNode(std::shared_ptr<TANode> node)
    {
        nodes_[node->getId().getId()] = node;
    }

    std::shared_ptr<TANode> getNode(const std::string& id)
    {
        return nodes_[id];
    }

    void execute(const std::string& startNodeId, void* context)
    {
        if (nodes_.find(startNodeId) != nodes_.end()) {
            nodes_[startNodeId]->process(context);
        }
    }

private:
    std::map<std::string, std::shared_ptr<TANode>> nodes_;
};

// WorldState holds data about the game world
struct WorldState {
    int time = 0;
    std::string currentRegion;
    std::map<std::string, int> playerStats;
    std::map<std::string, int> factionRelations;
    std::map<std::string, int> npcRelations;
    std::map<std::string, int> deityFavor;
    std::vector<std::string> diseases;
    int bounty = 0;
    std::map<std::string, double> economyFactors;
    std::string currentWeather;

    // Unified getter/setter for any stat
    int getStat(const std::string& category, const std::string& name) const
    {
        if (category == "player" && playerStats.find(name) != playerStats.end())
            return playerStats.at(name);
        if (category == "faction" && factionRelations.find(name) != factionRelations.end())
            return factionRelations.at(name);
        if (category == "npc" && npcRelations.find(name) != npcRelations.end())
            return npcRelations.at(name);
        if (category == "deity" && deityFavor.find(name) != deityFavor.end())
            return deityFavor.at(name);
        return 0;
    }

    void setStat(const std::string& category, const std::string& name, int value)
    {
        if (category == "player")
            playerStats[name] = value;
        else if (category == "faction")
            factionRelations[name] = value;
        else if (category == "npc")
            npcRelations[name] = value;
        else if (category == "deity")
            deityFavor[name] = value;
    }
};

// Base for all system nodes
class SystemNode : public TANode {
public:
    SystemNode(const NodeID& id)
        : TANode(id)
    {
    }
    virtual void process(void* context) override
    {
        WorldState* state = static_cast<WorldState*>(context);
        processSystem(state);
        TANode::process(context);
    }

protected:
    virtual void processSystem(WorldState* state) = 0;
};

// Utility functions for random number generation
namespace utils {
    std::mt19937 rng(std::time(nullptr));

    int randomInt(int min, int max)
    {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(rng);
    }

    bool randomChance(double probability)
    {
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        return dist(rng) < probability;
    }
}

// Core modules for each system
namespace systems {

    // Weather system
    class WeatherSystemNode : public SystemNode {
    public:
        WeatherSystemNode()
            : SystemNode(NodeID("weather_system"))
        {
        }

    protected:
        void processSystem(WorldState* state) override
        {
            // Determine weather based on region and time
            std::vector<std::string> weatherTypes = { "clear", "cloudy", "rainy", "stormy", "snowy" };
            double seasonFactor = (state->time % 4) / 4.0; // Simple season calculation
            int weatherIndex = utils::randomInt(0, 4);

            // Region-specific adjustments
            if (state->currentRegion == "mountains") {
                weatherIndex = std::min(4, weatherIndex + 1); // More snow in mountains
            } else if (state->currentRegion == "desert") {
                weatherIndex = std::max(0, weatherIndex - 2); // More clear in desert
            }

            state->currentWeather = weatherTypes[weatherIndex];

            // Weather effects on player stats
            if (state->currentWeather == "stormy") {
                state->playerStats["visibility"] = std::max(0, state->playerStats["visibility"] - 2);
            } else if (state->currentWeather == "snowy") {
                state->playerStats["mobility"] = std::max(0, state->playerStats["mobility"] - 2);
            }
        }
    };

    // Crime system
    class CrimeSystemNode : public SystemNode {
    public:
        CrimeSystemNode()
            : SystemNode(NodeID("crime_system"))
        {
        }

    protected:
        void processSystem(WorldState* state) override
        {
            // Process crime detection and bounty accumulation
            if (state->playerStats.find("lastCrime") != state->playerStats.end()) {
                int witnessChance = 30;

                // Adjust witness chance based on region
                if (state->currentRegion == "city")
                    witnessChance += 30;
                else if (state->currentRegion == "wilderness")
                    witnessChance -= 20;

                // Adjust based on time (night = lower chance)
                if (state->time % 24 >= 22 || state->time % 24 <= 5)
                    witnessChance -= 15;

                if (utils::randomChance(witnessChance / 100.0)) {
                    state->bounty += state->playerStats["lastCrime"];
                }
                state->playerStats.erase("lastCrime");
            }

            // Handle guard encounters based on bounty
            if (state->bounty > 0 && state->currentRegion != "wilderness") {
                double encounterChance = 0.05 * state->bounty;
                if (utils::randomChance(std::min(0.8, encounterChance))) {
                    // Guard encounter logic would go here
                    // Option to pay bounty, resist arrest, flee, etc.
                    if (state->playerStats.find("resist") != state->playerStats.end()) {
                        // Combat with guards
                        state->bounty += 10; // Additional bounty for resisting
                    } else if (state->playerStats.find("pay") != state->playerStats.end()) {
                        // Pay bounty
                        state->playerStats["gold"] -= state->bounty;
                        state->bounty = 0;
                    }
                }
            }
        }
    };

    // Health and disease system
    class HealthSystemNode : public SystemNode {
    public:
        HealthSystemNode()
            : SystemNode(NodeID("health_system"))
        {
        }

    protected:
        void processSystem(WorldState* state) override
        {
            // Process active diseases
            for (auto it = state->diseases.begin(); it != state->diseases.end();) {
                // Disease progression
                std::string disease = *it;
                int severity = state->getStat("disease", disease);

                // Disease progresses if untreated
                if (state->playerStats.find("treated_" + disease) == state->playerStats.end()) {
                    state->setStat("disease", disease, severity + 1);

                    // Apply disease effects
                    state->playerStats["health"] = std::max(0, state->playerStats["health"] - (severity / 10));

                    // Disease can be cured if treated or immunity develops
                    if (utils::randomChance(0.1) || severity > 100) {
                        // Develop immunity
                        state->playerStats["immune_" + disease] = 1;
                        it = state->diseases.erase(it);
                    } else {
                        ++it;
                    }
                } else {
                    // Treatment helps recovery
                    state->setStat("disease", disease, severity - 2);
                    if (severity <= 0) {
                        it = state->diseases.erase(it);
                    } else {
                        ++it;
                    }
                }
            }

            // Chance of contracting new diseases based on region
            double diseaseChance = 0.01;
            if (state->currentRegion == "swamp")
                diseaseChance *= 3;
            if (state->currentWeather == "rainy")
                diseaseChance *= 1.5;

            if (utils::randomChance(diseaseChance)) {
                std::vector<std::string> possibleDiseases = { "plague", "fever", "cough" };
                std::string newDisease = possibleDiseases[utils::randomInt(0, possibleDiseases.size() - 1)];

                // Check for immunity
                if (state->playerStats.find("immune_" + newDisease) == state->playerStats.end()) {
                    state->diseases.push_back(newDisease);
                    state->setStat("disease", newDisease, 1); // Initial severity
                }
            }
        }
    };

    // Economy system
    class EconomySystemNode : public SystemNode {
    public:
        EconomySystemNode()
            : SystemNode(NodeID("economy_system"))
        {
        }

    protected:
        void processSystem(WorldState* state) override
        {
            // Update market prices based on region and events
            std::vector<std::string> goods = { "food", "weapons", "cloth", "medicine" };

            for (const auto& good : goods) {
                // Base price adjustment
                double basePrice = 10.0;
                double priceMultiplier = 1.0;

                // Region-specific price adjustments
                if (state->currentRegion == "city") {
                    if (good == "food")
                        priceMultiplier *= 1.2;
                    if (good == "cloth")
                        priceMultiplier *= 0.8;
                } else if (state->currentRegion == "village") {
                    if (good == "food")
                        priceMultiplier *= 0.8;
                    if (good == "weapons")
                        priceMultiplier *= 1.3;
                }

                // Weather effects on economy
                if (state->currentWeather == "stormy" && good == "food") {
                    priceMultiplier *= 1.4; // Food more expensive during storms
                }

                // Random fluctuations
                priceMultiplier *= (0.9 + utils::randomInt(0, 20) / 100.0);

                // Store the price in the economy factors
                state->economyFactors[good + "_price"] = basePrice * priceMultiplier;
            }

            // Process property income if player owns property
            if (state->playerStats.find("owns_property") != state->playerStats.end()) {
                int propertyIncome = 5 * state->playerStats["property_level"];
                state->playerStats["gold"] += propertyIncome;
            }

            // Process investments
            for (auto it = state->playerStats.begin(); it != state->playerStats.end(); ++it) {
                if (it->first.find("invest_") == 0) {
                    std::string business = it->first.substr(7);
                    double returnRate = 0.03; // 3% return

                    // Better returns for certain businesses in certain regions
                    if (business == "mining" && state->currentRegion == "mountains") {
                        returnRate *= 1.5;
                    }

                    int investmentReturn = static_cast<int>(it->second * returnRate);
                    state->playerStats["gold"] += investmentReturn;
                }
            }
        }
    };

    // Faction system
    class FactionSystemNode : public SystemNode {
    public:
        FactionSystemNode()
            : SystemNode(NodeID("faction_system"))
        {
        }

    protected:
        void processSystem(WorldState* state) override
        {
            std::vector<std::string> factions = { "traders", "nobles", "thieves", "mages" };

            // Process faction relations over time
            for (const auto& faction : factions) {
                int relation = state->getStat("faction", faction);

                // Factions have relationships with each other
                for (const auto& otherFaction : factions) {
                    if (faction != otherFaction) {
                        // Rival factions
                        if ((faction == "nobles" && otherFaction == "thieves") || (faction == "thieves" && otherFaction == "nobles") || (faction == "traders" && otherFaction == "mages")) {

                            // If player helps one faction, the rival faction relation decreases
                            if (state->playerStats.find("helped_" + faction) != state->playerStats.end()) {
                                state->setStat("faction", otherFaction, relation - 1);
                                state->playerStats.erase("helped_" + faction);
                            }
                        }
                    }
                }

                // Factions control different areas - adjust prices and access
                if (state->currentRegion == "city" && faction == "nobles") {
                    // Nobles control cities
                    if (relation < 0) {
                        // Higher prices if disliked by nobles in the city
                        state->economyFactors["tax_rate"] = 1.2;
                    } else if (relation > 20) {
                        // Discounts if well-liked
                        state->economyFactors["tax_rate"] = 0.9;
                    }
                }

                // Political shifts happen over time
                if (state->time % 720 == 0) { // Monthly shift
                    // Random political changes
                    if (utils::randomChance(0.3)) {
                        int shift = utils::randomInt(-5, 5);
                        for (const auto& f : factions) {
                            state->setStat("faction", f, state->getStat("faction", f) + shift);
                        }
                    }
                }
            }
        }
    };

    // Mount system
    class MountSystemNode : public SystemNode {
    public:
        MountSystemNode()
            : SystemNode(NodeID("mount_system"))
        {
        }

    protected:
        void processSystem(WorldState* state) override
        {
            // Check if player has a mount
            if (state->playerStats.find("has_mount") != state->playerStats.end()) {
                // Mount stats
                int mountSpeed = state->playerStats["mount_speed"];
                int mountStamina = state->playerStats["mount_stamina"];
                int mountHealth = state->playerStats["mount_health"];

                // Mount gets tired during travel
                if (state->playerStats.find("traveling") != state->playerStats.end()) {
                    mountStamina = std::max(0, mountStamina - 1);
                    state->playerStats["mount_stamina"] = mountStamina;

                    // Speed bonus from mount, reduced when tired
                    double staminaFactor = std::max(0.5, mountStamina / 100.0);
                    state->playerStats["travel_speed"] += static_cast<int>(mountSpeed * staminaFactor);

                    // Special movement abilities
                    if (state->currentRegion == "mountains" && mountStamina > 50) {
                        state->playerStats["can_climb"] = 1;
                    }

                    if (mountStamina > 75) {
                        state->playerStats["can_jump_ravines"] = 1;
                    }
                }

                // Mount care - feeding
                if (state->playerStats.find("fed_mount") != state->playerStats.end()) {
                    mountStamina = std::min(100, mountStamina + 20);
                    state->playerStats["mount_stamina"] = mountStamina;
                    state->playerStats.erase("fed_mount");
                }

                // Mount training
                if (state->playerStats.find("training_mount") != state->playerStats.end()) {
                    int trainingType = state->playerStats["training_type"];
                    if (trainingType == 0) {
                        // Speed training
                        state->playerStats["mount_speed"] += 1;
                    } else if (trainingType == 1) {
                        // Stamina training
                        state->playerStats["mount_stamina_max"] += 2;
                    } else if (trainingType == 2) {
                        // Combat training
                        state->playerStats["mount_combat"] += 1;
                    }
                    state->playerStats.erase("training_mount");
                }

                // Weather effects on mount
                if (state->currentWeather == "stormy" || state->currentWeather == "snowy") {
                    // Mount moves slower in bad weather
                    state->playerStats["travel_speed"] = std::max(1, state->playerStats["travel_speed"] - 2);
                }
            }
        }
    };

    // Relationship system
    class RelationshipSystemNode : public SystemNode {
    public:
        RelationshipSystemNode()
            : SystemNode(NodeID("relationship_system"))
        {
        }

    protected:
        void processSystem(WorldState* state) override
        {
            std::vector<std::string> npcs = { "merchant", "guard", "innkeeper", "mage" };

            // Process NPC relationships
            for (const auto& npc : npcs) {
                int relation = state->getStat("npc", npc);

                // Gift effects
                if (state->playerStats.find("gift_to_" + npc) != state->playerStats.end()) {
                    int giftValue = state->playerStats["gift_to_" + npc];

                    // Different NPCs prefer different gifts
                    double giftMultiplier = 1.0;
                    if (npc == "merchant" && state->playerStats.find("gift_type") != state->playerStats.end()
                        && state->playerStats["gift_type"] == 0) {
                        // Merchant prefers valuable items
                        giftMultiplier = 1.5;
                    } else if (npc == "mage" && state->playerStats.find("gift_type") != state->playerStats.end()
                        && state->playerStats["gift_type"] == 1) {
                        // Mage prefers magical items
                        giftMultiplier = 1.5;
                    }

                    int relationshipIncrease = static_cast<int>(giftValue * giftMultiplier / 10);
                    state->setStat("npc", npc, relation + relationshipIncrease);

                    state->playerStats.erase("gift_to_" + npc);
                    if (state->playerStats.find("gift_type") != state->playerStats.end()) {
                        state->playerStats.erase("gift_type");
                    }
                }

                // Conversation effects
                if (state->playerStats.find("talked_to_" + npc) != state->playerStats.end()) {
                    // Success depends on charisma and previous relationship
                    double successChance = 0.5 + (state->playerStats["charisma"] / 100.0)
                        + (relation / 200.0);

                    if (utils::randomChance(successChance)) {
                        state->setStat("npc", npc, relation + 1);
                    }
                    state->playerStats.erase("talked_to_" + npc);
                }

                // Companion system
                if (state->playerStats.find("companion_" + npc) != state->playerStats.end()) {
                    // Loyalty based on relationship
                    int loyalty = relation / 10;

                    // Companions may leave if loyalty is too low
                    if (loyalty < 3 && utils::randomChance(0.1)) {
                        state->playerStats.erase("companion_" + npc);
                    }

                    // Companions provide bonuses
                    if (npc == "merchant")
                        state->economyFactors["shop_discount"] = 0.1;
                    else if (npc == "guard")
                        state->playerStats["combat"] += 2;
                    else if (npc == "mage")
                        state->playerStats["magic"] += 2;
                }

                // NPCs have schedules
                int hourOfDay = state->time % 24;
                if (npc == "merchant" && (hourOfDay < 8 || hourOfDay > 18)) {
                    state->playerStats["merchant_available"] = 0;
                } else if (npc == "merchant") {
                    state->playerStats["merchant_available"] = 1;
                }
            }
        }
    };

    // Religion system
    class ReligionSystemNode : public SystemNode {
    public:
        ReligionSystemNode()
            : SystemNode(NodeID("religion_system"))
        {
        }

    protected:
        void processSystem(WorldState* state) override
        {
            std::vector<std::string> deities = { "sun_god", "moon_goddess", "war_god", "harvest_goddess" };

            // Process deity favor
            for (const auto& deity : deities) {
                int favor = state->getStat("deity", deity);

                // Prayer effects
                if (state->playerStats.find("prayed_to_" + deity) != state->playerStats.end()) {
                    // Prayer increases favor
                    state->setStat("deity", deity, favor + 1);
                    state->playerStats.erase("prayed_to_" + deity);

                    // Prayer benefits
                    if (deity == "sun_god") {
                        // Sun god provides combat bonuses
                        state->playerStats["combat"] += favor / 10;
                    } else if (deity == "moon_goddess") {
                        // Moon goddess provides magic bonuses
                        state->playerStats["magic"] += favor / 10;
                    } else if (deity == "war_god") {
                        // War god provides health bonuses
                        state->playerStats["max_health"] += favor / 20;
                    } else if (deity == "harvest_goddess") {
                        // Harvest goddess provides food bonuses
                        state->playerStats["food"] += favor / 5;
                    }
                }

                // Temple quests
                if (state->playerStats.find("temple_quest_" + deity) != state->playerStats.end()) {
                    int questProgress = state->playerStats["temple_quest_" + deity];

                    if (questProgress >= 100) {
                        // Quest completed
                        state->setStat("deity", deity, favor + 10);
                        state->playerStats.erase("temple_quest_" + deity);

                        // Special blessings
                        if (deity == "sun_god") {
                            state->playerStats["blessed_weapon"] = 1;
                        } else if (deity == "moon_goddess") {
                            state->playerStats["blessed_magic"] = 1;
                        }
                    }
                }

                // Divine intervention
                if (favor > 50 && state->playerStats["health"] < 10 && utils::randomChance(favor / 200.0)) {
                    // Deity intervenes to save player
                    state->playerStats["health"] = state->playerStats["max_health"] / 2;
                    state->setStat("deity", deity, favor - 10); // Costs favor
                }

                // Religious conflicts
                for (const auto& otherDeity : deities) {
                    if (deity != otherDeity) {
                        // Rival deities
                        if ((deity == "sun_god" && otherDeity == "moon_goddess") || (deity == "moon_goddess" && otherDeity == "sun_god")) {

                            // Helping one deity displeases the other
                            if (state->playerStats.find("helped_" + deity) != state->playerStats.end()) {
                                int otherFavor = state->getStat("deity", otherDeity);
                                state->setStat("deity", otherDeity, otherFavor - 2);
                                state->playerStats.erase("helped_" + deity);
                            }
                        }
                    }
                }

                // Sacred days
                int dayOfYear = (state->time / 24) % 365;
                if (deity == "sun_god" && dayOfYear == 172) { // Summer solstice
                    // Sun god festival
                    state->setStat("deity", deity, favor + 5);
                    state->playerStats["sun_blessing"] = 1; // Temporary blessing
                }
            }
        }
    };

    // Spell crafting system
    class SpellCraftingSystemNode : public SystemNode {
    public:
        SpellCraftingSystemNode()
            : SystemNode(NodeID("spellcrafting_system"))
        {
        }

    protected:
        void processSystem(WorldState* state) override
        {
            // Process spell crafting if player is working on a spell
            if (state->playerStats.find("crafting_spell") != state->playerStats.end()) {
                int spellType = state->playerStats["spell_type"];
                int spellPower = state->playerStats["spell_power"];
                int spellComplexity = state->playerStats["spell_complexity"];
                int spellProgress = state->playerStats["spell_progress"];

                // Components increase spell progress
                if (state->playerStats.find("used_component") != state->playerStats.end()) {
                    int componentQuality = state->playerStats["component_quality"];
                    spellProgress += componentQuality;
                    state->playerStats["spell_progress"] = spellProgress;
                    state->playerStats.erase("used_component");
                }

                // Magic skill affects success rate
                double successChance = 0.4 + (state->playerStats["magic"] / 100.0) - (spellComplexity / 50.0);

                // Spell completion
                if (spellProgress >= 100) {
                    if (utils::randomChance(successChance)) {
                        // Spell successfully created
                        std::string spellName;
                        if (spellType == 0)
                            spellName = "fireball";
                        else if (spellType == 1)
                            spellName = "healing";
                        else if (spellType == 2)
                            spellName = "shield";

                        state->playerStats["spell_" + spellName] = spellPower;
                    } else {
                        // Spell failure
                        int backfireDamage = spellComplexity / 5;
                        state->playerStats["health"] = std::max(1, state->playerStats["health"] - backfireDamage);
                    }

                    // Reset spell crafting
                    state->playerStats.erase("crafting_spell");
                    state->playerStats.erase("spell_type");
                    state->playerStats.erase("spell_power");
                    state->playerStats.erase("spell_complexity");
                    state->playerStats.erase("spell_progress");
                }

                // Magical research
                if (state->playerStats.find("researching_magic") != state->playerStats.end()) {
                    int researchType = state->playerStats["research_type"];
                    int researchProgress = state->playerStats["research_progress"];

                    // Research progresses over time
                    researchProgress += 1 + (state->playerStats["intelligence"] / 20);
                    state->playerStats["research_progress"] = researchProgress;

                    if (researchProgress >= 100) {
                        // Research completed
                        if (researchType == 0) {
                            // Fire magic research
                            state->playerStats["fire_magic"] += 5;
                        } else if (researchType == 1) {
                            // Healing magic research
                            state->playerStats["healing_magic"] += 5;
                        } else if (researchType == 2) {
                            // Protection magic research
                            state->playerStats["protection_magic"] += 5;
                        }

                        // Reset research
                        state->playerStats.erase("researching_magic");
                        state->playerStats.erase("research_type");
                        state->playerStats.erase("research_progress");
                    }
                }
            }
        }
    };
}

// Create a unified system controller that incorporates all subsystems
class GameSystem {
public:
    GameSystem()
    {
        // Create and register all subsystems
        worldState_ = std::make_shared<WorldState>();
        controller_ = std::make_shared<TAController>();

        auto weather = std::make_shared<systems::WeatherSystemNode>();
        auto crime = std::make_shared<systems::CrimeSystemNode>();
        auto health = std::make_shared<systems::HealthSystemNode>();
        auto economy = std::make_shared<systems::EconomySystemNode>();
        auto faction = std::make_shared<systems::FactionSystemNode>();
        auto mount = std::make_shared<systems::MountSystemNode>();
        auto relationship = std::make_shared<systems::RelationshipSystemNode>();
        auto religion = std::make_shared<systems::ReligionSystemNode>();
        auto spellcrafting = std::make_shared<systems::SpellCraftingSystemNode>();

        // Register all systems
        controller_->registerNode(weather);
        controller_->registerNode(crime);
        controller_->registerNode(health);
        controller_->registerNode(economy);
        controller_->registerNode(faction);
        controller_->registerNode(mount);
        controller_->registerNode(relationship);
        controller_->registerNode(religion);
        controller_->registerNode(spellcrafting);

        // Link systems together to form the tree automata
        auto rootNode = std::make_shared<TANode>(NodeID("game_root"));
        rootNode->addChild(weather);
        rootNode->addChild(crime);
        rootNode->addChild(health);
        rootNode->addChild(economy);
        rootNode->addChild(faction);
        rootNode->addChild(mount);
        rootNode->addChild(relationship);
        rootNode->addChild(religion);
        rootNode->addChild(spellcrafting);

        controller_->registerNode(rootNode);

        // Initialize world state
        worldState_->currentRegion = "city";
        worldState_->playerStats["health"] = 100;
        worldState_->playerStats["max_health"] = 100;
        worldState_->playerStats["gold"] = 50;
        worldState_->playerStats["charisma"] = 10;
        worldState_->playerStats["intelligence"] = 10;
        worldState_->playerStats["magic"] = 5;
        worldState_->playerStats["combat"] = 5;

        // Initialize faction relations
        worldState_->factionRelations["traders"] = 0;
        worldState_->factionRelations["nobles"] = 0;
        worldState_->factionRelations["thieves"] = 0;
        worldState_->factionRelations["mages"] = 0;

        // Initialize deity favor
        worldState_->deityFavor["sun_god"] = 0;
        worldState_->deityFavor["moon_goddess"] = 0;
        worldState_->deityFavor["war_god"] = 0;
        worldState_->deityFavor["harvest_goddess"] = 0;
    }

    // Advance the game state by one time unit
    void update()
    {
        controller_->execute("game_root", worldState_.get());
        worldState_->time++;
    }

    // Process a player action
    void processAction(const std::string& action, const std::map<std::string, int>& params)
    {
        if (action == "commit_crime") {
            int crimeType = params.at("type");
            int severity = 0;

            if (crimeType == 0) { // Theft
                severity = 10;
                worldState_->playerStats["lastCrime"] = severity;

                // Add stolen goods to inventory
                worldState_->playerStats["gold"] += params.at("value");
            } else if (crimeType == 1) { // Assault
                severity = 25;
                worldState_->playerStats["lastCrime"] = severity;
            }
        } else if (action == "pray") {
            std::string deity = "sun_god";
            if (params.find("deity") != params.end()) {
                int deityId = params.at("deity");
                if (deityId == 0)
                    deity = "sun_god";
                else if (deityId == 1)
                    deity = "moon_goddess";
                else if (deityId == 2)
                    deity = "war_god";
                else if (deityId == 3)
                    deity = "harvest_goddess";
            }
            worldState_->playerStats["prayed_to_" + deity] = 1;
        } else if (action == "craft_spell") {
            worldState_->playerStats["crafting_spell"] = 1;
            worldState_->playerStats["spell_type"] = params.at("type");
            worldState_->playerStats["spell_power"] = params.at("power");
            worldState_->playerStats["spell_complexity"] = params.at("complexity");
            worldState_->playerStats["spell_progress"] = 0;
        } else if (action == "feed_mount") {
            worldState_->playerStats["fed_mount"] = 1;
        } else if (action == "give_gift") {
            std::string npc = "merchant";
            if (params.find("npc") != params.end()) {
                int npcId = params.at("npc");
                if (npcId == 0)
                    npc = "merchant";
                else if (npcId == 1)
                    npc = "guard";
                else if (npcId == 2)
                    npc = "innkeeper";
                else if (npcId == 3)
                    npc = "mage";
            }
            worldState_->playerStats["gift_to_" + npc] = params.at("value");
            if (params.find("gift_type") != params.end()) {
                worldState_->playerStats["gift_type"] = params.at("gift_type");
            }
        } else if (action == "buy_property") {
            if (worldState_->playerStats["gold"] >= params.at("cost")) {
                worldState_->playerStats["gold"] -= params.at("cost");
                worldState_->playerStats["owns_property"] = 1;
                worldState_->playerStats["property_level"] = params.at("level");
            }
        } else if (action == "invest") {
            std::string business = "shop";
            if (params.find("business") != params.end()) {
                int businessId = params.at("business");
                if (businessId == 0)
                    business = "shop";
                else if (businessId == 1)
                    business = "mine";
                else if (businessId == 2)
                    business = "farm";
            }
            if (worldState_->playerStats["gold"] >= params.at("amount")) {
                worldState_->playerStats["gold"] -= params.at("amount");
                worldState_->playerStats["invest_" + business] = params.at("amount");
            }
        } else if (action == "travel") {
            std::string region = "city";
            if (params.find("region") != params.end()) {
                int regionId = params.at("region");
                if (regionId == 0)
                    region = "city";
                else if (regionId == 1)
                    region = "village";
                else if (regionId == 2)
                    region = "mountains";
                else if (regionId == 3)
                    region = "forest";
                else if (regionId == 4)
                    region = "desert";
                else if (regionId == 5)
                    region = "swamp";
                else if (regionId == 6)
                    region = "wilderness";
            }
            worldState_->currentRegion = region;
            worldState_->playerStats["traveling"] = 1;
        }
    }

    // Get current game state information
    const WorldState& getState() const
    {
        return *worldState_;
    }

private:
    std::shared_ptr<WorldState> worldState_;
    std::shared_ptr<TAController> controller_;
};

// Example usage
int main()
{
    GameSystem game;

    // Process some player actions
    game.processAction("pray", { { "deity", 0 } });
    game.update();

    game.processAction("commit_crime", { { "type", 0 }, { "value", 20 } });
    game.update();

    game.processAction("buy_property", { { "cost", 100 }, { "level", 1 } });
    game.update();

    // Travel to different region
    game.processAction("travel", { { "region", 2 } });
    game.update();

    // Craft a spell
    game.processAction("craft_spell", { { "type", 0 }, { "power", 5 }, { "complexity", 3 } });
    game.update();

    return 0;
}
} // namespace oath