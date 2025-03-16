#pragma once

#include <string>

class Mount;
struct MountSystemConfig;

// Mount training session for improving mount abilities
class MountTrainingSession {
public:
    Mount* mount;
    std::string trainingType;
    int duration;
    int difficulty;
    int successChance;
    int experienceGain;
    MountSystemConfig& config;

    MountTrainingSession(Mount* targetMount, const std::string& type, int sessionDuration, int sessionDifficulty, MountSystemConfig& cfg);
    bool conductTraining();
};