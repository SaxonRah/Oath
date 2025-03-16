#pragma once

#include <map>
#include <set>
#include <string>


// Character stats structure for progression and dialogue systems
struct CharacterStats {
    int strength = 10;
    int dexterity = 10;
    int constitution = 10;
    int intelligence = 10;
    int wisdom = 10;
    int charisma = 10;

    std::map<std::string, int> skills;
    std::map<std::string, int> factionReputation;
    std::set<std::string> knownFacts;
    std::set<std::string> unlockedAbilities;

    CharacterStats();
    bool hasSkill(const std::string& skill, int minLevel) const;
    bool hasFactionReputation(const std::string& faction, int minRep) const;
    bool hasKnowledge(const std::string& fact) const;
    bool hasAbility(const std::string& ability) const;
    void learnFact(const std::string& fact);
    void unlockAbility(const std::string& ability);
    void improveSkill(const std::string& skill, int amount);
    void changeFactionRep(const std::string& faction, int amount);
};