// /oath/systems/health/HealthNodes.hpp
#pragma once

#include "../../core/TAAction.hpp"
#include "../../core/TAInput.hpp"
#include "../../core/TANode.hpp"
#include "../../data/GameContext.hpp"

// Health system nodes
class HealthStateNode : public TANode {
public:
    HealthStateNode(const std::string& name);
    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;

private:
    HealthState* getHealthState(GameContext* context);
    DiseaseManager* getDiseaseManager(GameContext* context);
};

class DiseaseNode : public TANode {
public:
    std::string diseaseId;

    DiseaseNode(const std::string& name, const std::string& id);
    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;

private:
    HealthState* getHealthState(GameContext* context);
    DiseaseManager* getDiseaseManager(GameContext* context);
};

class TreatmentNode : public TANode {
public:
    std::string targetDiseaseId; // Optional, if treating a specific disease

    TreatmentNode(const std::string& name, const std::string& diseaseId = "");
    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;

private:
    GameContext* getGameContext();
    HealthState* getHealthState(GameContext* context);
    DiseaseManager* getDiseaseManager(GameContext* context);
};

class RestNode : public TANode {
public:
    RestNode(const std::string& name);
    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
    void applyRest(int hours, GameContext* context);

private:
    GameContext* getGameContext();
    HealthState* getHealthState(GameContext* context);
    DiseaseManager* getDiseaseManager(GameContext* context);
};

class EpidemicNode : public TANode {
public:
    std::string diseaseId;
    std::string regionName;
    float severityMultiplier;

    EpidemicNode(const std::string& name, const std::string& disease,
        const std::string& region, float severity = 1.0f);
    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;

private:
    GameContext* getGameContext();
    DiseaseManager* getDiseaseManager(GameContext* context);
};
