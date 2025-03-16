// /oath/systems/health/Immunity.cpp
#include "Immunity.hpp"

Immunity::Immunity(const std::string& disease, float immuneStrength, int duration, int currentDay)
    : diseaseId(disease)
    , strength(immuneStrength)
    , durationDays(duration)
    , dayAcquired(currentDay)
{
}

bool Immunity::isActive(int currentDay) const
{
    return durationDays == -1 || (currentDay - dayAcquired) < durationDays;
}

float Immunity::getEffectiveStrength(int currentDay) const
{
    if (!isActive(currentDay))
        return 0.0f;

    // Immunity can wane over time
    if (durationDays > 0) {
        float progress = static_cast<float>(currentDay - dayAcquired) / durationDays;
        return strength * (1.0f - progress * 0.5f); // Gradually reduces to 50% of original strength
    }

    return strength;
}