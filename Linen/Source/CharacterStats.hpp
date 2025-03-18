#pragma once

#include <map>
#include <set>
#include <string>

namespace oath {

// Character stats structure
class CharacterStats {
public:
    // Base attributes
    int strength = 10;
    int dexterity = 10;
    int constitution = 10;
    int intelligence = 10;
    int wisdom = 10;
    int charisma = 10;

    // Derived stats
    int health = 100;
    int maxHealth = 100;
    int stamina = 100;
    int maxStamina = 100;
    int mana = 100;
    int maxMana = 100;

    // Skills and abilities
    std::map<std::string, int> skills;
    std::set<std::string> unlockedAbilities;

    // Faction reputation
    std::map<std::string, int> factionReputation;

    // Knowledge base
    std::set<std::string> knownFacts;

    // Temporary modifiers
    std::map<std::string, int> modifiers;

    CharacterStats()
    {
        // Initialize basic skills
        skills["combat"] = 0;
        skills["stealth"] = 0;
        skills["persuasion"] = 0;
        skills["survival"] = 0;
        skills["magic"] = 0;
    }

    // Skill methods
    bool hasSkill(const std::string& skill, int minLevel = 1) const
    {
        auto it = skills.find(skill);
        return it != skills.end() && it->second >= minLevel;
    }

    void improveSkill(const std::string& skill, int amount = 1)
    {
        skills[skill] += amount;
    }

    // Faction methods
    bool hasFactionRep(const std::string& faction, int minRep = 0) const
    {
        auto it = factionReputation.find(faction);
        return it != factionReputation.end() && it->second >= minRep;
    }

    void changeFactionRep(const std::string& faction, int amount)
    {
        factionReputation[faction] += amount;
    }

    // Knowledge methods
    bool hasKnowledge(const std::string& fact) const
    {
        return knownFacts.find(fact) != knownFacts.end();
    }

    void learnFact(const std::string& fact)
    {
        knownFacts.insert(fact);
    }

    // Ability methods
    bool hasAbility(const std::string& ability) const
    {
        return unlockedAbilities.find(ability) != unlockedAbilities.end();
    }

    void unlockAbility(const std::string& ability)
    {
        unlockedAbilities.insert(ability);
    }

    // Get effective stat value (with modifiers)
    int getEffectiveStat(const std::string& stat) const
    {
        int base = 0;

        // Get base value
        if (stat == "strength")
            base = strength;
        else if (stat == "dexterity")
            base = dexterity;
        else if (stat == "constitution")
            base = constitution;
        else if (stat == "intelligence")
            base = intelligence;
        else if (stat == "wisdom")
            base = wisdom;
        else if (stat == "charisma")
            base = charisma;
        else {
            // Check if it's a skill
            auto it = skills.find(stat);
            if (it != skills.end()) {
                base = it->second;
            }
        }

        // Apply modifier if it exists
        auto it = modifiers.find(stat);
        return it != modifiers.end() ? base + it->second : base;
    }
};

} // namespace oath