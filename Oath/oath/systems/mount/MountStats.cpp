#include "MountStats.hpp"
#include <nlohmann/json.hpp>

MountStats::MountStats()
    : stamina(100)
    , maxStamina(100)
    , staminaRegen(5)
    , speed(100)
    , carryCapacity(50)
    , loyalty(50)
    , training(0)
    , hunger(0)
    , fatigue(0)
    , health(100)
    , maxHealth(100)
    , canJump(false)
    , canSwim(false)
    , canClimb(false)
{
    // Initialize specialties from defaults - will be overridden by JSON
    specialTraining["combat"] = 0; // Combat training
    specialTraining["endurance"] = 0; // Long distance travel
    specialTraining["agility"] = 0; // Jumps and difficult terrain
    specialTraining["racing"] = 0; // Speed and bursts
}

void MountStats::loadFromJson(const nlohmann::json& j)
{
    stamina = j.value("stamina", 100);
    maxStamina = j.value("maxStamina", 100);
    staminaRegen = j.value("staminaRegen", 5);
    speed = j.value("speed", 100);
    carryCapacity = j.value("carryCapacity", 50);
    loyalty = j.value("loyalty", 50);
    training = j.value("training", 0);
    hunger = j.value("hunger", 0);
    fatigue = j.value("fatigue", 0);
    health = j.value("health", 100);
    maxHealth = j.value("maxHealth", 100);
    canJump = j.value("canJump", false);
    canSwim = j.value("canSwim", false);
    canClimb = j.value("canClimb", false);

    // Load any specialty training defined in JSON
    if (j.contains("specialTraining") && j["specialTraining"].is_object()) {
        for (auto& [key, value] : j["specialTraining"].items()) {
            specialTraining[key] = value;
        }
    }
}

bool MountStats::isExhausted() const
{
    return stamina <= 10 || fatigue >= 90;
}

bool MountStats::isStarving() const
{
    return hunger >= 90;
}

bool MountStats::isInjured() const
{
    return health <= maxHealth / 2;
}

int MountStats::getEffectiveSpeed() const
{
    float multiplier = 1.0f;

    // Reduce speed if tired or hungry
    if (fatigue > 50)
        multiplier -= (fatigue - 50) * 0.01f;
    if (hunger > 50)
        multiplier -= (hunger - 50) * 0.01f;
    if (health < maxHealth / 2)
        multiplier -= (1.0f - (float)health / maxHealth) * 0.5f;

    // Apply minimum multiplier
    if (multiplier < 0.3f)
        multiplier = 0.3f;

    return static_cast<int>(speed * multiplier);
}

bool MountStats::useStamina(int amount)
{
    if (stamina < amount)
        return false;

    stamina -= amount;
    fatigue += amount / 5; // Increase fatigue based on exertion
    if (fatigue > 100)
        fatigue = 100;

    return true;
}

void MountStats::rest(int minutes)
{
    // Recover stamina
    stamina += (staminaRegen * minutes) / 60;
    if (stamina > maxStamina)
        stamina = maxStamina;

    // Reduce fatigue
    fatigue -= minutes / 15; // 4 fatigue points per hour of rest
    if (fatigue < 0)
        fatigue = 0;

    // Increase hunger during rest
    hunger += minutes / 120; // 0.5 hunger per hour of rest
    if (hunger > 100)
        hunger = 100;
}

void MountStats::feed(int nutritionValue)
{
    hunger -= nutritionValue;
    if (hunger < 0)
        hunger = 0;

    // Slight health recovery from good feeding
    health += nutritionValue / 10;
    if (health > maxHealth)
        health = maxHealth;
}

void MountStats::train(const std::string& area, int amount)
{
    if (specialTraining.find(area) != specialTraining.end()) {
        specialTraining[area] += amount;
        if (specialTraining[area] > 100)
            specialTraining[area] = 100;
    }

    // Overall training level is average of all specialties
    int totalTraining = 0;
    for (const auto& [_, value] : specialTraining) {
        totalTraining += value;
    }
    training = totalTraining / specialTraining.size();
}