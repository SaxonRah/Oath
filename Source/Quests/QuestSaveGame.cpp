// QuestSaveGame.cpp
#include "QuestSaveGame.h"

UQuestSaveGame::UQuestSaveGame()
{
}

FQuestObjectiveSaveData UQuestSaveGame::ConvertObjectiveToSaveData(const FQuestObjective& Objective)
{
    FQuestObjectiveSaveData SaveData;
    SaveData.Description = Objective.Description;
    SaveData.CurrentProgress = Objective.CurrentProgress;
    SaveData.RequiredProgress = Objective.RequiredProgress;
    SaveData.bIsCompleted = Objective.bIsCompleted;
    
    return SaveData;
}

FQuestObjective UQuestSaveGame::ConvertSaveDataToObjective(const FQuestObjectiveSaveData& SaveData)
{
    FQuestObjective Objective;
    Objective.Description = SaveData.Description;
    Objective.CurrentProgress = SaveData.CurrentProgress;
    Objective.RequiredProgress = SaveData.RequiredProgress;
    Objective.bIsCompleted = SaveData.bIsCompleted;
    
    return Objective;
}

FQuestSaveData UQuestSaveGame::ConvertQuestToSaveData(UQuest* Quest)
{
    FQuestSaveData SaveData;
    
    if (!Quest)
    {
        return SaveData;
    }
    
    SaveData.QuestName = Quest->QuestName;
    SaveData.Description = Quest->Description;
    SaveData.Type = Quest->Type;
    SaveData.Status = Quest->Status;
    SaveData.DifficultyLevel = Quest->DifficultyLevel;
    SaveData.QuestRenownReward = Quest->QuestRenownReward;
    SaveData.GoldReward = Quest->GoldReward;
    SaveData.QuestGiver = Quest->QuestGiver;
    SaveData.FactionName = Quest->FactionName;
    SaveData.FactionReputationReward = Quest->FactionReputationReward;
    SaveData.bCanBeAssignedToFollower = Quest->bCanBeAssignedToFollower;
    
    // Convert material rewards (needs string keys for serialization)
    for (const TPair<FResourceData, int32>& MaterialReward : Quest->MaterialRewards)
    {
        SaveData.MaterialRewards.Add(MaterialReward.Key.Name, MaterialReward.Value);
    }
    
    // Convert objectives
    for (const FQuestObjective& Objective : Quest->Objectives)
    {
        SaveData.Objectives.Add(ConvertObjectiveToSaveData(Objective));
    }
    
    return SaveData;
}

UQuest* UQuestSaveGame::ConvertSaveDataToQuest(const FQuestSaveData& SaveData)
{
    UQuest* Quest = NewObject<UQuest>();
    
    Quest->QuestName = SaveData.QuestName;
    Quest->Description = SaveData.Description;
    Quest->Type = SaveData.Type;
    Quest->Status = SaveData.Status;
    Quest->DifficultyLevel = SaveData.DifficultyLevel;
    Quest->QuestRenownReward = SaveData.QuestRenownReward;
    Quest->GoldReward = SaveData.GoldReward;
    Quest->QuestGiver = SaveData.QuestGiver;
    Quest->FactionName = SaveData.FactionName;
    Quest->FactionReputationReward = SaveData.FactionReputationReward;
    Quest->bCanBeAssignedToFollower = SaveData.bCanBeAssignedToFollower;
    
    // Material rewards would need to be properly reconstituted from a resource database
    // This is a placeholder implementation
    for (const TPair<FString, int32>& MaterialReward : SaveData.MaterialRewards)
    {
        FResourceData Resource;
        Resource.Name = MaterialReward.Key;
        // Other properties would need to be loaded from a database
        
        Quest->MaterialRewards.Add(Resource, MaterialReward.Value);
    }
    
    // Convert objectives
    for (const FQuestObjectiveSaveData& ObjectiveSaveData : SaveData.Objectives)
    {
        Quest->Objectives.Add(ConvertSaveDataToObjective(ObjectiveSaveData));
    }
    
    return Quest;
}