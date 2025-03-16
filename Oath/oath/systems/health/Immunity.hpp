// /oath/systems/health/Immunity.hpp
#pragma once

#include <string>

// Class to represent an immunity to a disease
class Immunity {
public:
    std::string diseaseId;
    float strength; // 0.0 to 1.0, where 1.0 is full immunity
    int durationDays; // How long the immunity lasts, -1 for permanent
    int dayAcquired; // Day the immunity was acquired

    Immunity(const std::string& disease, float immuneStrength, int duration, int currentDay);

    bool isActive(int currentDay) const;
    float getEffectiveStrength(int currentDay) const;
};