#pragma once

#include <map>
#include <string>

#include <nlohmann/json.hpp>

struct MountStats {
    int stamina;
    int maxStamina;
    int staminaRegen;
    int speed;
    int carryCapacity;
    int loyalty;
    int training;

    // Track the mount's condition
    int hunger;
    int fatigue;
    int health;
    int maxHealth;

    // Special ability flags
    bool canJump;
    bool canSwim;
    bool canClimb;

    // Training specialties
    std::map<std::string, int> specialTraining;

    MountStats();
    void loadFromJson(const nlohmann::json& j);
    bool isExhausted() const;
    bool isStarving() const;
    bool isInjured() const;
    int getEffectiveSpeed() const;
    bool useStamina(int amount);
    void rest(int minutes);
    void feed(int nutritionValue);
    void train(const std::string& area, int amount);
};