// EmergentAI.h
#pragma once

#include <ctime>
#include <functional>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <random>
#include <string>
#include <vector>


using json = nlohmann::json;

// Forward declarations
class GameContext;
class QuestSystem;
class DialogueSystem;
class CharacterProgressionSystem;
class CraftingSystem;
class TimeSystem;
class WeatherSystem;
class CrimeSystem;
class HealthSystem;
class EconomySystem;
class FactionSystem;
class RelationshipSystem;
class ReligionSystem;
class SpellCraftingSystem;

/**
 * @brief Base class for NPC needs
 */
class Need {
public:
    enum class Priority {
        LOW,
        MEDIUM,
        HIGH,
        CRITICAL
    };

    Need(const std::string& id, float initialValue, float decayRate);
    virtual ~Need() = default;

    const std::string& getId() const { return m_id; }
    float getValue() const { return m_value; }
    Priority getPriority() const;

    virtual void update(float deltaTime);
    virtual void satisfy(float amount);

    virtual json toJson() const;
    virtual void fromJson(const json& data);

private:
    std::string m_id;
    float m_value; // 0.0 to 1.0
    float m_decayRate; // Units per game hour
};

/**
 * @brief Represents a single atomic action an NPC can perform
 */
class Action {
public:
    Action(const std::string& id, const std::map<std::string, float>& needEffects);
    virtual ~Action() = default;

    const std::string& getId() const { return m_id; }
    float getEffectOnNeed(const std::string& needId) const;
    bool canPerform(GameContext* context, const std::string& npcId) const;

    virtual void execute(GameContext* context, const std::string& npcId);
    virtual float getUtility(GameContext* context, const std::string& npcId) const;
    virtual float getDuration() const { return m_duration; }

    virtual json toJson() const;
    virtual void fromJson(const json& data);

protected:
    std::string m_id;
    std::map<std::string, float> m_needEffects;
    float m_duration; // in game hours

    virtual bool checkRequirements(GameContext* context, const std::string& npcId) const;
};

/**
 * @brief Represents an NPC in the game world
 */
class NPC {
public:
    NPC(const std::string& id, const std::string& name);
    ~NPC() = default;

    const std::string& getId() const { return m_id; }
    const std::string& getName() const { return m_name; }
    const std::vector<std::shared_ptr<Need>>& getNeeds() const { return m_needs; }
    std::shared_ptr<Need> getNeed(const std::string& needId) const;
    std::shared_ptr<Action> getCurrentAction() const { return m_currentAction; }

    void addNeed(std::shared_ptr<Need> need);
    void update(GameContext* context, float deltaTime);
    void performAction(GameContext* context, std::shared_ptr<Action> action);
    std::shared_ptr<Action> selectBestAction(GameContext* context, const std::vector<std::shared_ptr<Action>>& availableActions);

    json toJson() const;
    void fromJson(const json& data);

private:
    std::string m_id;
    std::string m_name;
    std::vector<std::shared_ptr<Need>> m_needs;
    std::shared_ptr<Action> m_currentAction;
    float m_actionProgress; // 0.0 to 1.0

    // NPC state data
    std::string m_currentLocation;
    std::map<std::string, int> m_inventory;
    std::map<std::string, int> m_skills;
    std::map<std::string, float> m_factionStanding;
    std::map<std::string, float> m_relationships;

    void completeCurrentAction(GameContext* context);
};

/**
 * @brief Context for game systems to interact
 */
class GameContext {
public:
    GameContext();
    ~GameContext() = default;

    // System getters
    QuestSystem* getQuestSystem() { return m_questSystem.get(); }
    DialogueSystem* getDialogueSystem() { return m_dialogueSystem.get(); }
    CharacterProgressionSystem* getCharacterProgressionSystem() { return m_progressionSystem.get(); }
    CraftingSystem* getCraftingSystem() { return m_craftingSystem.get(); }
    TimeSystem* getTimeSystem() { return m_timeSystem.get(); }
    WeatherSystem* getWeatherSystem() { return m_weatherSystem.get(); }
    CrimeSystem* getCrimeSystem() { return m_crimeSystem.get(); }
    HealthSystem* getHealthSystem() { return m_healthSystem.get(); }
    EconomySystem* getEconomySystem() { return m_economySystem.get(); }
    FactionSystem* getFactionSystem() { return m_factionSystem.get(); }
    RelationshipSystem* getRelationshipSystem() { return m_relationshipSystem.get(); }
    ReligionSystem* getReligionSystem() { return m_religionSystem.get(); }
    SpellCraftingSystem* getSpellCraftingSystem() { return m_spellCraftingSystem.get(); }

    // NPC management
    void addNPC(std::shared_ptr<NPC> npc);
    std::shared_ptr<NPC> getNPC(const std::string& npcId) const;
    std::vector<std::shared_ptr<NPC>> getAllNPCs() const;

    // Action registration
    void registerAction(std::shared_ptr<Action> action);
    std::shared_ptr<Action> getAction(const std::string& actionId) const;
    std::vector<std::shared_ptr<Action>> getAllActions() const;

    // Game state
    void update(float deltaTime);

    // Serialization
    json toJson() const;
    void fromJson(const json& data);

    // Load from file
    bool loadFromFile(const std::string& filename);
    bool saveToFile(const std::string& filename) const;

private:
    // Game systems
    std::unique_ptr<QuestSystem> m_questSystem;
    std::unique_ptr<DialogueSystem> m_dialogueSystem;
    std::unique_ptr<CharacterProgressionSystem> m_progressionSystem;
    std::unique_ptr<CraftingSystem> m_craftingSystem;
    std::unique_ptr<TimeSystem> m_timeSystem;
    std::unique_ptr<WeatherSystem> m_weatherSystem;
    std::unique_ptr<CrimeSystem> m_crimeSystem;
    std::unique_ptr<HealthSystem> m_healthSystem;
    std::unique_ptr<EconomySystem> m_economySystem;
    std::unique_ptr<FactionSystem> m_factionSystem;
    std::unique_ptr<RelationshipSystem> m_relationshipSystem;
    std::unique_ptr<ReligionSystem> m_religionSystem;
    std::unique_ptr<SpellCraftingSystem> m_spellCraftingSystem;

    // NPC and action storage
    std::map<std::string, std::shared_ptr<NPC>> m_npcs;
    std::map<std::string, std::shared_ptr<Action>> m_actions;

    void initializeSystems();
};

// Base class for all game systems
class GameSystem {
public:
    GameSystem(GameContext* context)
        : m_context(context)
    {
    }
    virtual ~GameSystem() = default;

    virtual void update(float deltaTime) = 0;
    virtual json toJson() const = 0;
    virtual void fromJson(const json& data) = 0;

protected:
    GameContext* m_context;
};

// System header declarations (simplified)
class QuestSystem : public GameSystem {
public:
    QuestSystem(GameContext* context);
    void update(float deltaTime) override;
    json toJson() const override;
    void fromJson(const json& data) override;

    // Quest-specific methods here
};

class DialogueSystem : public GameSystem {
public:
    DialogueSystem(GameContext* context);
    void update(float deltaTime) override;
    json toJson() const override;
    void fromJson(const json& data) override;

    // Dialogue-specific methods here
};

class CharacterProgressionSystem : public GameSystem {
public:
    CharacterProgressionSystem(GameContext* context);
    void update(float deltaTime) override;
    json toJson() const override;
    void fromJson(const json& data) override;

    // Character progression-specific methods here
};

class CraftingSystem : public GameSystem {
public:
    CraftingSystem(GameContext* context);
    void update(float deltaTime) override;
    json toJson() const override;
    void fromJson(const json& data) override;

    // Crafting-specific methods here
};

class TimeSystem : public GameSystem {
public:
    TimeSystem(GameContext* context);
    void update(float deltaTime) override;
    json toJson() const override;
    void fromJson(const json& data) override;

    // Time-specific methods here
    int getHour() const { return m_hour; }
    int getDay() const { return m_day; }
    int getMonth() const { return m_month; }
    int getYear() const { return m_year; }

private:
    int m_hour;
    int m_day;
    int m_month;
    int m_year;
};

class WeatherSystem : public GameSystem {
public:
    enum class WeatherType {
        CLEAR,
        CLOUDY,
        RAINY,
        STORMY,
        SNOWY
    };

    WeatherSystem(GameContext* context);
    void update(float deltaTime) override;
    json toJson() const override;
    void fromJson(const json& data) override;

    // Weather-specific methods here
    WeatherType getCurrentWeather() const { return m_currentWeather; }

private:
    WeatherType m_currentWeather;
};

class CrimeSystem : public GameSystem {
public:
    CrimeSystem(GameContext* context);
    void update(float deltaTime) override;
    json toJson() const override;
    void fromJson(const json& data) override;

    // Crime-specific methods here
};

class HealthSystem : public GameSystem {
public:
    HealthSystem(GameContext* context);
    void update(float deltaTime) override;
    json toJson() const override;
    void fromJson(const json& data) override;

    // Health-specific methods here
};

class EconomySystem : public GameSystem {
public:
    EconomySystem(GameContext* context);
    void update(float deltaTime) override;
    json toJson() const override;
    void fromJson(const json& data) override;

    // Economy-specific methods here
};

class FactionSystem : public GameSystem {
public:
    FactionSystem(GameContext* context);
    void update(float deltaTime) override;
    json toJson() const override;
    void fromJson(const json& data) override;

    // Faction-specific methods here
};

class RelationshipSystem : public GameSystem {
public:
    RelationshipSystem(GameContext* context);
    void update(float deltaTime) override;
    json toJson() const override;
    void fromJson(const json& data) override;

    // Relationship-specific methods here
};

class ReligionSystem : public GameSystem {
public:
    ReligionSystem(GameContext* context);
    void update(float deltaTime) override;
    json toJson() const override;
    void fromJson(const json& data) override;

    // Religion-specific methods here
};

class SpellCraftingSystem : public GameSystem {
public:
    SpellCraftingSystem(GameContext* context);
    void update(float deltaTime) override;
    json toJson() const override;
    void fromJson(const json& data) override;

    // Spell crafting-specific methods here
};