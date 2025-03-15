#include <CharacterStats.hpp>

CharacterStats::CharacterStats()
{
    // Initialize basic skills
    skills["combat"] = 0;
    skills["stealth"] = 0;
    skills["persuasion"] = 0;
    skills["survival"] = 0;
    skills["alchemy"] = 0;
    skills["crafting"] = 0;
    skills["magic"] = 0;

    // Initialize some factions
    factionReputation["villagers"] = 0;
    factionReputation["merchants"] = 0;
    factionReputation["nobility"] = 0;
    factionReputation["bandits"] = -50;
}

bool CharacterStats::hasSkill(const std::string& skill, int minLevel) const
{
    auto it = skills.find(skill);
    return it != skills.end() && it->second >= minLevel;
}

bool CharacterStats::hasFactionReputation(const std::string& faction, int minRep) const
{
    auto it = factionReputation.find(faction);
    return it != factionReputation.end() && it->second >= minRep;
}

bool CharacterStats::hasKnowledge(const std::string& fact) const
{
    return knownFacts.find(fact) != knownFacts.end();
}

bool CharacterStats::hasAbility(const std::string& ability) const
{
    return unlockedAbilities.find(ability) != unlockedAbilities.end();
}

void CharacterStats::learnFact(const std::string& fact)
{
    knownFacts.insert(fact);
}

void CharacterStats::unlockAbility(const std::string& ability)
{
    unlockedAbilities.insert(ability);
}

void CharacterStats::improveSkill(const std::string& skill, int amount)
{
    skills[skill] += amount;
}

void CharacterStats::changeFactionRep(const std::string& faction, int amount)
{
    factionReputation[faction] += amount;
}
