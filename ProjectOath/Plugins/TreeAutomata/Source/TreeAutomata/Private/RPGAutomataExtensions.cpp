// RPGAutomataExtensions.cpp
#include "RPGAutomataExtensions.h"

// Quest Automaton Implementation
FGuid UQuestAutomaton::CreateQuest(const FString& QuestName, const FString& QuestDescription)
{
    // Add quest node under root
    FGuid QuestId = AddNode(QuestName, GetRootNodeId());
    
    if (QuestId.IsValid())
    {
        // Add metadata
        FAutomatonNode& QuestNode = GetNodeMutable(QuestId);
        QuestNode.Metadata.Add("Type", "Quest");
        QuestNode.Metadata.Add("Description", QuestDescription);
        QuestNode.Metadata.Add("Status", "Available");
        QuestNode.State = "Available";
        
        // Define quest state transitions
        FStateTransition AcceptTransition("Available", "Active", "AcceptQuest");
        FStateTransition CompleteTransition("Active", "Completed", "CompleteQuest");
        FStateTransition FailTransition("Active", "Failed", "FailQuest");
        
        AddTransition(AcceptTransition);
        AddTransition(CompleteTransition);
        AddTransition(FailTransition);
    }
    
    return QuestId;
}

FGuid UQuestAutomaton::AddObjective(const FGuid& QuestId, const FString& ObjectiveName, const FString& Description)
{
    // Add objective as child of quest
    FGuid ObjectiveId = AddNode(ObjectiveName, QuestId);
    
    if (ObjectiveId.IsValid())
    {
        // Add metadata
        FAutomatonNode& ObjectiveNode = GetNodeMutable(ObjectiveId);
        ObjectiveNode.Metadata.Add("Type", "Objective");
        ObjectiveNode.Metadata.Add("Description", Description);
        ObjectiveNode.State = "Incomplete";
        
        // Define objective state transitions
        FStateTransition CompleteTransition("Incomplete", "Complete", "CompleteObjective");
        AddTransition(CompleteTransition);
    }
    
    return ObjectiveId;
}

bool UQuestAutomaton::CompleteObjective(const FGuid& ObjectiveId)
{
    // Trigger completion event on the objective
    bool Success = TriggerEvent(ObjectiveId, "CompleteObjective");
    
    if (Success)
    {
        // Check if all objectives for the parent quest are complete
        FAutomatonNode ObjectiveNode = GetNode(ObjectiveId);
        FGuid QuestId = ObjectiveNode.ParentId;
        
        if (QuestId.IsValid())
        {
            // Get all objectives for this quest
            TArray<FAutomatonNode> Objectives = GetChildren(QuestId);
            bool AllComplete = true;
            
            for (const FAutomatonNode& Objective : Objectives)
            {
                if (Objective.Metadata.Contains("Type") && Objective.Metadata["Type"] == "Objective")
                {
                    if (Objective.State != "Complete")
                    {
                        AllComplete = false;
                        break;
                    }
                }
            }
            
            // Auto-complete quest if all objectives are done
            if (AllComplete)
            {
                TriggerEvent(QuestId, "CompleteQuest");
            }
        }
    }
    
    return Success;
}

bool UQuestAutomaton::IsQuestComplete(const FGuid& QuestId) const
{
    FAutomatonNode QuestNode = GetNode(QuestId);
    return QuestNode.State == "Completed";
}

TArray<FAutomatonNode> UQuestAutomaton::GetActiveQuests() const
{
    return FindNodesByState("Active");
}

TArray<FAutomatonNode> UQuestAutomaton::GetQuestObjectives(const FGuid& QuestId) const
{
    return GetChildren(QuestId);
}

// Dialogue Automaton Implementation
FGuid UDialogueAutomaton::CreateDialogueTree(const FString& NPCName)
{
    // Create dialogue tree under root
    FGuid DialogueTreeId = AddNode(NPCName + " Dialogue", GetRootNodeId());
    
    if (DialogueTreeId.IsValid())
    {
        // Add metadata
        FAutomatonNode& DialogueTreeNode = GetNodeMutable(DialogueTreeId);
        DialogueTreeNode.Metadata.Add("Type", "DialogueTree");
        DialogueTreeNode.Metadata.Add("NPC", NPCName);
        DialogueTreeNode.State = "Available";
        
        // Add initial greeting dialogue
        FGuid GreetingId = AddNode("Greeting", DialogueTreeId);
        if (GreetingId.IsValid())
        {
            FAutomatonNode& GreetingNode = GetNodeMutable(GreetingId);
            GreetingNode.Metadata.Add("Type", "DialogueNode");
            GreetingNode.Metadata.Add("Text", "Hello traveler!");
            GreetingNode.State = "Available";
        }
    }
    
    return DialogueTreeId;
}

FGuid UDialogueAutomaton::AddDialogueNode(const FGuid& ParentNodeId, const FString& DialogueText, 
                                        const FString& PlayerResponseText)
{
    // Add dialogue node
    FGuid DialogueNodeId = AddNode(PlayerResponseText, ParentNodeId);
    
    if (DialogueNodeId.IsValid())
    {
        // Add metadata
        FAutomatonNode& DialogueNode = GetNodeMutable(DialogueNodeId);
        DialogueNode.Metadata.Add("Type", "DialogueNode");
        DialogueNode.Metadata.Add("Text", DialogueText);
        DialogueNode.Metadata.Add("PlayerResponse", PlayerResponseText);
        DialogueNode.State = "Available";
        
        // Define standard dialogue transitions
        FStateTransition SelectTransition("Available", "Selected", "SelectDialogue");
        FStateTransition ResetTransition("Selected", "Available", "ResetDialogue");
        
        AddTransition(SelectTransition);
        AddTransition(ResetTransition);
    }
    
    return DialogueNodeId;
}

void UDialogueAutomaton::SetDialogueCondition(const FGuid& DialogueNodeId, const FString& Condition)
{
    if (GetNodes().Contains(DialogueNodeId))
    {
        FAutomatonNode& DialogueNode = GetNodeMutable(DialogueNodeId);
        
        // Add condition to metadata
        if (!DialogueNode.Metadata.Contains("Conditions"))
        {
            DialogueNode.Metadata.Add("Conditions", Condition);
        }
        else
        {
            DialogueNode.Metadata["Conditions"] += ";" + Condition;
        }
    }
}

TArray<FAutomatonNode> UDialogueAutomaton::GetAvailableDialogueOptions(const FGuid& CurrentNodeId, UObject* GameState)
{
    TArray<FAutomatonNode> AvailableOptions;
    
    // Get all children of the current dialogue node
    TArray<FAutomatonNode> AllOptions = GetChildren(CurrentNodeId);
    
    for (const FAutomatonNode& Option : AllOptions)
    {
        // Check if the option has conditions
        if (Option.Metadata.Contains("Conditions") && GetConditionEvaluator())
        {
            // Parse multiple conditions (separated by semicolons)
            TArray<FString> Conditions;
            Option.Metadata["Conditions"].ParseIntoArray(Conditions, TEXT(";"), true);
            
            bool AllConditionsMet = true;
            for (const FString& Condition : Conditions)
            {
                if (!GetConditionEvaluator()->EvaluateCondition(Condition, GameState))
                {
                    AllConditionsMet = false;
                    break;
                }
            }
            
            if (AllConditionsMet)
            {
                AvailableOptions.Add(Option);
            }
        }
        else
        {
            // No conditions, option is always available
            AvailableOptions.Add(Option);
        }
    }
    
    return AvailableOptions;
}

// Skill Tree Automaton Implementation
FGuid USkillTreeAutomaton::CreateSkillTree(const FString& ClassName)
{
    // Create skill tree under root
    FGuid SkillTreeId = AddNode(ClassName + " Skills", GetRootNodeId());
    
    if (SkillTreeId.IsValid())
    {
        // Add metadata
        FAutomatonNode& SkillTreeNode = GetNodeMutable(SkillTreeId);
        SkillTreeNode.Metadata.Add("Type", "SkillTree");
        SkillTreeNode.Metadata.Add("Class", ClassName);
        SkillTreeNode.State = "Available";
    }
    
    return SkillTreeId;
}

FGuid USkillTreeAutomaton::AddSkill(const FGuid& ParentSkillId, const FString& SkillName, 
                                 int32 PointsRequired, const FString& Description)
{
    // Add skill node
    FGuid SkillId = AddNode(SkillName, ParentSkillId);
    
    if (SkillId.IsValid())
    {
        // Add metadata
        FAutomatonNode& SkillNode = GetNodeMutable(SkillId);
        SkillNode.Metadata.Add("Type", "Skill");
        SkillNode.Metadata.Add("Description", Description);
        SkillNode.Metadata.Add("PointsRequired", FString::FromInt(PointsRequired));
        SkillNode.State = "Locked";
        
        // Define skill state transitions
        FStateTransition UnlockTransition("Locked", "Unlocked", "UnlockSkill");
        AddTransition(UnlockTransition);
    }
    
    return SkillId;
}

bool USkillTreeAutomaton::UnlockSkill(const FGuid& SkillId, int32 AvailablePoints)
{
    FAutomatonNode SkillNode = GetNode(SkillId);
    
    // Check if skill is already unlocked
    if (SkillNode.State == "Unlocked")
    {
        return true;
    }
    
    // Check if parent skills are unlocked
    FAutomatonNode ParentNode = GetParent(SkillId);
    
    // If parent is not the root and not unlocked, skill can't be unlocked
    if (ParentNode.NodeId != GetRootNodeId() && ParentNode.State != "Unlocked")
    {
        return false;
    }
    
    // Check if enough points are available
    int32 PointsRequired = FCString::Atoi(*SkillNode.Metadata["PointsRequired"]);
    if (AvailablePoints < PointsRequired)
    {
        return false;
    }
    
    // All requirements met, unlock the skill
    return TriggerEvent(SkillId, "UnlockSkill");
}

TArray<FAutomatonNode> USkillTreeAutomaton::GetAvailableSkills(int32 AvailablePoints) const
{
    TArray<FAutomatonNode> AvailableSkills;
    TArray<FAutomatonNode> LockedSkills = FindNodesByState("Locked");
    
    for (const FAutomatonNode& Skill : LockedSkills)
    {
        // Skip non-skill nodes
        if (!Skill.Metadata.Contains("Type") || Skill.Metadata["Type"] != "Skill")
        {
            continue;
        }
        
        // Check if parent is unlocked or is the root
        FAutomatonNode ParentNode = GetParent(Skill.NodeId);
        if (ParentNode.NodeId == GetRootNodeId() || ParentNode.State == "Unlocked")
        {
            // Check if enough points
            int32 PointsRequired = FCString::Atoi(*Skill.Metadata["PointsRequired"]);
            if (AvailablePoints >= PointsRequired)
            {
                AvailableSkills.Add(Skill);
            }
        }
    }
    
    return AvailableSkills;
}

TArray<FAutomatonNode> USkillTreeAutomaton::GetUnlockedSkills() const
{
    return FindNodesByState("Unlocked");
}

// Crafting Automaton Implementation
FGuid UCraftingAutomaton::CreateCraftingSystem(const FString& SystemName)
{
    // Create crafting system under root
    FGuid CraftingSystemId = AddNode(SystemName, GetRootNodeId());
    
    if (CraftingSystemId.IsValid())
    {
        // Add metadata
        FAutomatonNode& CraftingSystemNode = GetNodeMutable(CraftingSystemId);
        CraftingSystemNode.Metadata.Add("Type", "CraftingSystem");
        CraftingSystemNode.State = "Available";
    }
    
    return CraftingSystemId;
}

FGuid UCraftingAutomaton::AddRecipe(const FGuid& ParentRecipeId, const FString& RecipeName, 
                                 const TArray<FString>& RequiredItems, const FString& ResultItem)
{
    // Add recipe node
    FGuid RecipeId = AddNode(RecipeName, ParentRecipeId);
    
    if (RecipeId.IsValid())
    {
        // Add metadata
        FAutomatonNode& RecipeNode = GetNodeMutable(RecipeId);
        RecipeNode.Metadata.Add("Type", "Recipe");
        RecipeNode.Metadata.Add("Result", ResultItem);
        
        // Store required items
        FString RequiredItemsStr;
        for (int32 i = 0; i < RequiredItems.Num(); i++)
        {
            if (i > 0)
            {
                RequiredItemsStr += ";";
            }
            RequiredItemsStr += RequiredItems[i];
        }
        RecipeNode.Metadata.Add("RequiredItems", RequiredItemsStr);
        
        // Set initial state
        RecipeNode.State = "Undiscovered";
        
        // Define recipe state transitions
        FStateTransition DiscoverTransition("Undiscovered", "Discovered", "DiscoverRecipe");
        AddTransition(DiscoverTransition);
    }
    
    return RecipeId;
}

bool UCraftingAutomaton::DiscoverRecipe(const FGuid& RecipeId)
{
    return TriggerEvent(RecipeId, "DiscoverRecipe");
}

TArray<FAutomatonNode> UCraftingAutomaton::GetDiscoveredRecipes() const
{
    return FindNodesByState("Discovered");
}

bool UCraftingAutomaton::CanCraftRecipe(const FGuid& RecipeId, const TArray<FString>& AvailableItems) const
{
    FAutomatonNode RecipeNode = GetNode(RecipeId);
    
    // Check if recipe is discovered
    if (RecipeNode.State != "Discovered")
    {
        return false;
    }
    
    // Check if all required items are available
    if (RecipeNode.Metadata.Contains("RequiredItems"))
    {
        TArray<FString> RequiredItems;
        RecipeNode.Metadata["RequiredItems"].ParseIntoArray(RequiredItems, TEXT(";"), true);
        
        for (const FString& RequiredItem : RequiredItems)
        {
            if (!AvailableItems.Contains(RequiredItem))
            {
                return false;
            }
        }
    }
    
    return true;
}