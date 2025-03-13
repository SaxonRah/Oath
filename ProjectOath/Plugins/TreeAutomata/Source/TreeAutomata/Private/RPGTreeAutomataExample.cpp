// RPGTreeAutomataExample.cpp
#include "RPGTreeAutomataExample.h"
#include "Engine/World.h"

ARPGTreeAutomataExample::ARPGTreeAutomataExample()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ARPGTreeAutomataExample::BeginPlay()
{
    Super::BeginPlay();
    
    // Spawn the automata manager
    AutomataManager = GetWorld()->SpawnActor<ATreeAutomataManager>(ATreeAutomataManager::StaticClass());
    
    // Create game state
    GameState = NewObject<URPGGameState>(this);
    
    // Create condition evaluator and action performer
    ConditionEvaluator = NewObject<URPGConditionEvaluator>(this);
    ActionPerformer = NewObject<URPGActionPerformer>(this);
    
    // Set the evaluator and performer on all automata
    AutomataManager->GetQuestAutomaton()->SetConditionEvaluator(ConditionEvaluator);
    AutomataManager->GetQuestAutomaton()->SetActionPerformer(ActionPerformer);
    
    AutomataManager->GetDialogueAutomaton()->SetConditionEvaluator(ConditionEvaluator);
    AutomataManager->GetDialogueAutomaton()->SetActionPerformer(ActionPerformer);
    
    AutomataManager->GetSkillTreeAutomaton()->SetConditionEvaluator(ConditionEvaluator);
    AutomataManager->GetSkillTreeAutomaton()->SetActionPerformer(ActionPerformer);
    
    AutomataManager->GetCraftingAutomaton()->SetConditionEvaluator(ConditionEvaluator);
    AutomataManager->GetCraftingAutomaton()->SetActionPerformer(ActionPerformer);
    
    // Set up example systems
    SetupQuestSystem();
    SetupDialogueSystem();
    SetupSkillTree();
    SetupCraftingSystem();
}

void ARPGTreeAutomataExample::SetupQuestSystem()
{
    UQuestAutomaton* QuestAutomaton = AutomataManager->GetQuestAutomaton();
    
    // Create a main quest
    FGuid DefendVillageQuestId = QuestAutomaton->CreateQuest("Defend the Village", 
        "The village is under threat and needs your help to prepare defenses.");
    QuestIDs.Add("DefendVillage", DefendVillageQuestId);
    
    // Add sub-quests
    FGuid RepairWallsQuestId = QuestAutomaton->CreateQuest("Repair the Walls", 
        "The village walls are damaged and need repair.");
    QuestIDs.Add("RepairWalls", RepairWallsQuestId);
    
    FGuid TrainMilitiaQuestId = QuestAutomaton->CreateQuest("Train Militia", 
        "The villagers need training to defend themselves.");
    QuestIDs.Add("TrainMilitia", TrainMilitiaQuestId);
    
    FGuid GatherSuppliesQuestId = QuestAutomaton->CreateQuest("Gather Supplies", 
        "Gather food and materials to sustain the village.");
    QuestIDs.Add("GatherSupplies", GatherSuppliesQuestId);
    
    // Add objectives to repair walls quest
    FGuid FindLumberObjectiveId = QuestAutomaton->AddObjective(RepairWallsQuestId, 
        "Find Lumber", "Collect 10 pieces of lumber from the forest.");
    QuestIDs.Add("FindLumber", FindLumberObjectiveId);
    
    FGuid ReinforcePalisadeObjectiveId = QuestAutomaton->AddObjective(RepairWallsQuestId, 
        "Reinforce Palisade", "Use the lumber to reinforce the village palisade.");
    QuestIDs.Add("ReinforcePalisade", ReinforcePalisadeObjectiveId);
    
    // Add transitions with conditions
    FStateTransition CompleteRepairTransition("Active", "Completed", "CompleteQuest");
    CompleteRepairTransition.Conditions.Add("HasFlag:RepairWallsComplete");
    CompleteRepairTransition.Actions.Add("GiveExperience:200");
    CompleteRepairTransition.Actions.Add("ModifyReputation:Village:10");
    QuestAutomaton->AddTransition(CompleteRepairTransition);
}

void ARPGTreeAutomataExample::SetupDialogueSystem()
{
    UDialogueAutomaton* DialogueAutomaton = AutomataManager->GetDialogueAutomaton();
    
    // Create a dialogue tree for a village elder
    FGuid ElderDialogueId = DialogueAutomaton->CreateDialogueTree("Village Elder");
    DialogueIDs.Add("ElderDialogue", ElderDialogueId);
    
    // Get the greeting node that was automatically created
    TArray<FAutomatonNode> ElderNodes = DialogueAutomaton->GetChildren(ElderDialogueId);
    FGuid GreetingId = ElderNodes[0].NodeId;
    DialogueIDs.Add("ElderGreeting", GreetingId);
    
    // Add dialogue options
    FGuid DefenseOptionId = DialogueAutomaton->AddDialogueNode(GreetingId, 
        "Our village is in danger. Bandits have been spotted nearby and we fear an attack. Will you help us?", 
        "Tell me about the danger");
    DialogueIDs.Add("ElderDefenseOption", DefenseOptionId);
    
    FGuid QuestOptionId = DialogueAutomaton->AddDialogueNode(DefenseOptionId, 
        "We need to repair our walls, train our people, and gather supplies. Can you help with any of these tasks?", 
        "I'll help with all of them");
    DialogueIDs.Add("ElderQuestOption", QuestOptionId);
    
    FGuid AcceptOptionId = DialogueAutomaton->AddDialogueNode(QuestOptionId, 
        "Thank you! That's a great relief. Please speak with our carpenter about the walls, the guard captain about training, and our storekeeper about supplies.", 
        "I'll get started right away");
    DialogueIDs.Add("ElderAcceptOption", AcceptOptionId);
    
    // Add special dialogue that only appears if player has high reputation
    FGuid SpecialRewardOptionId = DialogueAutomaton->AddDialogueNode(GreetingId, 
        "You've done so much for our village. Please take this ancient family heirloom as a token of our gratitude.", 
        "Thank you for this honor");
    DialogueIDs.Add("ElderRewardOption", SpecialRewardOptionId);
    
    // Add condition and actions to the special dialogue
    DialogueAutomaton->SetDialogueCondition(SpecialRewardOptionId, "HasReputation:Village:20");
    FStateTransition RewardTransition("Available", "Selected", "SelectDialogue");
    RewardTransition.Actions.Add("GiveItem:AncientAmulet");
    RewardTransition.Actions.Add("SetFlag:ElderRewardGiven:true");
    DialogueAutomaton->AddTransition(RewardTransition);
}

void ARPGTreeAutomataExample::SetupSkillTree()
{
    USkillTreeAutomaton* SkillTreeAutomaton = AutomataManager->GetSkillTreeAutomaton();
    
    // Create a skill tree for warrior class
    FGuid WarriorSkillTreeId = SkillTreeAutomaton->CreateSkillTree("Warrior");
    SkillIDs.Add("WarriorTree", WarriorSkillTreeId);
    
    // Add base combat skills
    FGuid BasicCombatId = SkillTreeAutomaton->AddSkill(WarriorSkillTreeId, "Basic Combat", 1, 
        "Fundamentals of combat techniques");
    SkillIDs.Add("BasicCombat", BasicCombatId);
    
    FGuid SwordMasteryId = SkillTreeAutomaton->AddSkill(BasicCombatId, "Sword Mastery", 2, 
        "Advanced sword techniques");
    SkillIDs.Add("SwordMastery", SwordMasteryId);
    
    FGuid ShieldMasteryId = SkillTreeAutomaton->AddSkill(BasicCombatId, "Shield Mastery", 2, 
        "Advanced shield techniques");
    SkillIDs.Add("ShieldMastery", ShieldMasteryId);
    
    // Add advanced skills
    FGuid WhirlwindId = SkillTreeAutomaton->AddSkill(SwordMasteryId, "Whirlwind", 3, 
        "Spin attack that hits all nearby enemies");
    SkillIDs.Add("Whirlwind", WhirlwindId);
    
    FGuid ShieldBashId = SkillTreeAutomaton->AddSkill(ShieldMasteryId, "Shield Bash", 3, 
        "Stun an enemy with your shield");
    SkillIDs.Add("ShieldBash", ShieldBashId);
    
    // Add special skill that requires both sword and shield mastery
    FGuid DefensiveStrikeId = SkillTreeAutomaton->AddSkill(WarriorSkillTreeId, "Defensive Strike", 5, 
        "A perfectly balanced attack and defense move");
    SkillIDs.Add("DefensiveStrike", DefensiveStrikeId);
    
    // Add custom transition with complex requirement
    FStateTransition UnlockDefensiveStrikeTransition("Locked", "Unlocked", "UnlockSkill");
    UnlockDefensiveStrikeTransition.Conditions.Add("HasFlag:SwordMasteryUnlocked");
    UnlockDefensiveStrikeTransition.Conditions.Add("HasFlag:ShieldMasteryUnlocked");
    UnlockDefensiveStrikeTransition.Actions.Add("ModifyStat:Strength:5");
    SkillTreeAutomaton->AddTransition(UnlockDefensiveStrikeTransition);
}

void ARPGTreeAutomataExample::SetupCraftingSystem()
{
    UCraftingAutomaton* CraftingAutomaton = AutomataManager->GetCraftingAutomaton();
    
    // Create a blacksmithing crafting system
    FGuid BlacksmithingId = CraftingAutomaton->CreateCraftingSystem("Blacksmithing");
    RecipeIDs.Add("Blacksmithing", BlacksmithingId);
    
    // Add basic recipes
    TArray<FString> IronSwordIngredients;
    IronSwordIngredients.Add("Iron Ingot");
    IronSwordIngredients.Add("Wooden Handle");
    FGuid IronSwordRecipeId = CraftingAutomaton->AddRecipe(BlacksmithingId, "Iron Sword", 
        IronSwordIngredients, "Iron Sword");
    RecipeIDs.Add("IronSword", IronSwordRecipeId);
    
    TArray<FString> IronShieldIngredients;
    IronShieldIngredients.Add("Iron Ingot");
    IronShieldIngredients.Add("Iron Ingot");
    IronShieldIngredients.Add("Wooden Frame");
    FGuid IronShieldRecipeId = CraftingAutomaton->AddRecipe(BlacksmithingId, "Iron Shield", 
        IronShieldIngredients, "Iron Shield");
    RecipeIDs.Add("IronShield", IronShieldRecipeId);
    
    // Add advanced recipes that require the basic ones
    TArray<FString> SteelSwordIngredients;
    SteelSwordIngredients.Add("Steel Ingot");
    SteelSwordIngredients.Add("Leather Grip");
    SteelSwordIngredients.Add("Iron Sword"); // Requires previous item
    FGuid SteelSwordRecipeId = CraftingAutomaton->AddRecipe(IronSwordRecipeId, "Steel Sword", 
        SteelSwordIngredients, "Steel Sword");
    RecipeIDs.Add("SteelSword", SteelSwordRecipeId);
    
    // Add custom transition for recipe discovery with condition
    FStateTransition DiscoverSteelSwordTransition("Undiscovered", "Discovered", "DiscoverRecipe");
    DiscoverSteelSwordTransition.Conditions.Add("HasStat:Blacksmithing:5");
    DiscoverSteelSwordTransition.Actions.Add("GiveExperience:100");
    CraftingAutomaton->AddTransition(DiscoverSteelSwordTransition);
}

void ARPGTreeAutomataExample::DemoQuestCompletion()
{
    UQuestAutomaton* QuestAutomaton = AutomataManager->GetQuestAutomaton();
    
    UE_LOG(LogTemp, Display, TEXT("=== QUEST DEMO ==="));
    
    // Accept the repair walls quest
    FGuid RepairWallsQuestId = QuestIDs["RepairWalls"];
    QuestAutomaton->TriggerEvent(RepairWallsQuestId, "AcceptQuest", GameState);
    UE_LOG(LogTemp, Display, TEXT("Accepted quest: %s"), *QuestAutomaton->GetNode(RepairWallsQuestId).Name);
    
    // Complete objectives
    FGuid FindLumberObjectiveId = QuestIDs["FindLumber"];
    QuestAutomaton->TriggerEvent(FindLumberObjectiveId, "CompleteObjective", GameState);
    UE_LOG(LogTemp, Display, TEXT("Completed objective: %s"), *QuestAutomaton->GetNode(FindLumberObjectiveId).Name);
    
    FGuid ReinforcePalisadeObjectiveId = QuestIDs["ReinforcePalisade"];
    QuestAutomaton->TriggerEvent(ReinforcePalisadeObjectiveId, "CompleteObjective", GameState);
    UE_LOG(LogTemp, Display, TEXT("Completed objective: %s"), *QuestAutomaton->GetNode(ReinforcePalisadeObjectiveId).Name);
    
    // Set flag to complete quest
    GameState->SetFlag("RepairWallsComplete", true);
    
    // Complete the quest
    QuestAutomaton->TriggerEvent(RepairWallsQuestId, "CompleteQuest", GameState);
    UE_LOG(LogTemp, Display, TEXT("Quest completed: %s"), *QuestAutomaton->GetNode(RepairWallsQuestId).Name);
    UE_LOG(LogTemp, Display, TEXT("Player reputation with Village: %d"), GameState->Reputation["Village"]);
}

void ARPGTreeAutomataExample::DemoDialogueProgression()
{
    UDialogueAutomaton* DialogueAutomaton = AutomataManager->GetDialogueAutomaton();
    
    UE_LOG(LogTemp, Display, TEXT("=== DIALOGUE DEMO ==="));
    
    // Start with greeting
    FGuid GreetingId = DialogueIDs["ElderGreeting"];
    FAutomatonNode GreetingNode = DialogueAutomaton->GetNode(GreetingId);
    UE_LOG(LogTemp, Display, TEXT("Elder: %s"), *GreetingNode.Metadata["Text"]);
    
    // Show available options
    TArray<FAutomatonNode> Options = DialogueAutomaton->GetAvailableDialogueOptions(GreetingId, GameState);
    UE_LOG(LogTemp, Display, TEXT("Available responses:"));
    for (const FAutomatonNode& Option : Options)
    {
        UE_LOG(LogTemp, Display, TEXT("- %s"), *Option.Metadata["PlayerResponse"]);
    }
    
    // Select the defense option
    FGuid DefenseOptionId = DialogueIDs["ElderDefenseOption"];
    DialogueAutomaton->TriggerEvent(DefenseOptionId, "SelectDialogue", GameState);
    FAutomatonNode DefenseNode = DialogueAutomaton->GetNode(DefenseOptionId);
    UE_LOG(LogTemp, Display, TEXT("You: %s"), *DefenseNode.Metadata["PlayerResponse"]);
    UE_LOG(LogTemp, Display, TEXT("Elder: %s"), *DefenseNode.Metadata["Text"]);
    
    // Continue conversation
    FGuid QuestOptionId = DialogueIDs["ElderQuestOption"];
    DialogueAutomaton->TriggerEvent(QuestOptionId, "SelectDialogue", GameState);
    FAutomatonNode QuestNode = DialogueAutomaton->GetNode(QuestOptionId);
    UE_LOG(LogTemp, Display, TEXT("You: %s"), *QuestNode.Metadata["PlayerResponse"]);
    UE_LOG(LogTemp, Display, TEXT("Elder: %s"), *QuestNode.Metadata["Text"]);
    
    // Increase reputation to unlock special dialogue
    GameState->ModifyReputation("Village", 25);
    UE_LOG(LogTemp, Display, TEXT("Village reputation increased to %d"), GameState->Reputation["Village"]);
    
    // Reset dialogue to greeting
    DialogueAutomaton->TriggerEvent(QuestOptionId, "ResetDialogue", GameState);
    
    // Check available options again - should include special option now
    Options = DialogueAutomaton->GetAvailableDialogueOptions(GreetingId, GameState);
    UE_LOG(LogTemp, Display, TEXT("Available responses after reputation increase:"));
    for (const FAutomatonNode& Option : Options)
    {
        UE_LOG(LogTemp, Display, TEXT("- %s"), *Option.Metadata["PlayerResponse"]);
    }
    
    // Select the special reward option
    FGuid SpecialRewardOptionId = DialogueIDs["ElderRewardOption"];
    DialogueAutomaton->TriggerEvent(SpecialRewardOptionId, "SelectDialogue", GameState);
    FAutomatonNode RewardNode = DialogueAutomaton->GetNode(SpecialRewardOptionId);
    UE_LOG(LogTemp, Display, TEXT("You: %s"), *RewardNode.Metadata["PlayerResponse"]);
    UE_LOG(LogTemp, Display, TEXT("Elder: %s"), *RewardNode.Metadata["Text"]);
    
    // Check inventory for new item
    UE_LOG(LogTemp, Display, TEXT("Inventory items:"));
    for (const FString& Item : GameState->Inventory)
    {
        UE_LOG(LogTemp, Display, TEXT("- %s"), *Item);
    }
}

void ARPGTreeAutomataExample::DemoSkillUnlocking()
{
    USkillTreeAutomaton* SkillTreeAutomaton = AutomataManager->GetSkillTreeAutomaton();
    
    UE_LOG(LogTemp, Display, TEXT("=== SKILL TREE DEMO ==="));
    
    // Give the player some skill points
    GameState->SkillPoints = 10;
    UE_LOG(LogTemp, Display, TEXT("Player has %d skill points"), GameState->SkillPoints);
    
    // Get available skills
    TArray<FAutomatonNode> AvailableSkills = SkillTreeAutomaton->GetAvailableSkills(GameState->SkillPoints);
    UE_LOG(LogTemp, Display, TEXT("Available skills to unlock:"));
    for (const FAutomatonNode& Skill : AvailableSkills)
    {
        int32 Cost = FCString::Atoi(*Skill.Metadata["PointsRequired"]);
        UE_LOG(LogTemp, Display, TEXT("- %s (Cost: %d)"), *Skill.Name, Cost);
    }
    
    // Unlock basic combat
    FGuid BasicCombatId = SkillIDs["BasicCombat"];
    int32 BasicCombatCost = FCString::Atoi(*SkillTreeAutomaton->GetNode(BasicCombatId).Metadata["PointsRequired"]);
    bool Success = SkillTreeAutomaton->UnlockSkill(BasicCombatId, GameState->SkillPoints);
    if (Success)
    {
        GameState->SkillPoints -= BasicCombatCost;
        UE_LOG(LogTemp, Display, TEXT("Unlocked %s! %d skill points remaining"), 
            *SkillTreeAutomaton->GetNode(BasicCombatId).Name, GameState->SkillPoints);
    }
    
    // Now unlock sword mastery
    FGuid SwordMasteryId = SkillIDs["SwordMastery"];
    int32 SwordMasteryCost = FCString::Atoi(*SkillTreeAutomaton->GetNode(SwordMasteryId).Metadata["PointsRequired"]);
    Success = SkillTreeAutomaton->UnlockSkill(SwordMasteryId, GameState->SkillPoints);
    if (Success)
    {
        GameState->SkillPoints -= SwordMasteryCost;
        GameState->SetFlag("SwordMasteryUnlocked", true);
        UE_LOG(LogTemp, Display, TEXT("Unlocked %s! %d skill points remaining"), 
            *SkillTreeAutomaton->GetNode(SwordMasteryId).Name, GameState->SkillPoints);
    }
    
    // Unlock shield mastery
    FGuid ShieldMasteryId = SkillIDs["ShieldMastery"];
    int32 ShieldMasteryCost = FCString::Atoi(*SkillTreeAutomaton->GetNode(ShieldMasteryId).Metadata["PointsRequired"]);
    Success = SkillTreeAutomaton->UnlockSkill(ShieldMasteryId, GameState->SkillPoints);
    if (Success)
    {
        GameState->SkillPoints -= ShieldMasteryCost;
        GameState->SetFlag("ShieldMasteryUnlocked", true);
        UE_LOG(LogTemp, Display, TEXT("Unlocked %s! %d skill points remaining"), 
            *SkillTreeAutomaton->GetNode(ShieldMasteryId).Name, GameState->SkillPoints);
    }
    
    // Try to unlock defensive strike (which requires both masteries)
    FGuid DefensiveStrikeId = SkillIDs["DefensiveStrike"];
    int32 DefensiveStrikeCost = FCString::Atoi(*SkillTreeAutomaton->GetNode(DefensiveStrikeId).Metadata["PointsRequired"]);
    Success = SkillTreeAutomaton->UnlockSkill(DefensiveStrikeId, GameState->SkillPoints);
    if (Success)
    {
        GameState->SkillPoints -= DefensiveStrikeCost;
        UE_LOG(LogTemp, Display, TEXT("Unlocked %s! %d skill points remaining"), 
            *SkillTreeAutomaton->GetNode(DefensiveStrikeId).Name, GameState->SkillPoints);
        UE_LOG(LogTemp, Display, TEXT("Strength increased to %d"), GameState->Stats["Strength"]);
    }
    
    // Get all unlocked skills
    TArray<FAutomatonNode> UnlockedSkills = SkillTreeAutomaton->GetUnlockedSkills();
    UE_LOG(LogTemp, Display, TEXT("All unlocked skills:"));
    for (const FAutomatonNode& Skill : UnlockedSkills)
    {
        UE_LOG(LogTemp, Display, TEXT("- %s"), *Skill.Name);
    }
}

void ARPGTreeAutomataExample::DemoCrafting()
{
    UCraftingAutomaton* CraftingAutomaton = AutomataManager->GetCraftingAutomaton();
    
    UE_LOG(LogTemp, Display, TEXT("=== CRAFTING DEMO ==="));
    
    // Add some basic materials to inventory
    GameState->AddItem("Iron Ingot");
    GameState->AddItem("Iron Ingot");
    GameState->AddItem("Iron Ingot");
    GameState->AddItem("Wooden Handle");
    GameState->AddItem("Wooden Frame");
    
    UE_LOG(LogTemp, Display, TEXT("Inventory items:"));
    for (const FString& Item : GameState->Inventory)
    {
        UE_LOG(LogTemp, Display, TEXT("- %s"), *Item);
    }
    
    // Discover iron sword recipe
    FGuid IronSwordRecipeId = RecipeIDs["IronSword"];
    CraftingAutomaton->DiscoverRecipe(IronSwordRecipeId);
    UE_LOG(LogTemp, Display, TEXT("Discovered recipe: %s"), *CraftingAutomaton->GetNode(IronSwordRecipeId).Name);
    
    // Check if we can craft it
    bool CanCraft = CraftingAutomaton->CanCraftRecipe(IronSwordRecipeId, GameState->Inventory);
    UE_LOG(LogTemp, Display, TEXT("Can craft Iron Sword: %s"), CanCraft ? TEXT("Yes") : TEXT("No"));
    
    // Try to discover steel sword (should fail due to insufficient skill)
    FGuid SteelSwordRecipeId = RecipeIDs["SteelSword"];
    bool Discovered = CraftingAutomaton->DiscoverRecipe(SteelSwordRecipeId);
    UE_LOG(LogTemp, Display, TEXT("Steel Sword recipe discovered: %s"), Discovered ? TEXT("Yes") : TEXT("No"));
    
    // Increase blacksmithing skill
    GameState->ModifyStat("Blacksmithing", 10);
    UE_LOG(LogTemp, Display, TEXT("Blacksmithing skill increased to %d"), GameState->Stats["Blacksmithing"]);
    
    // Try again
    Discovered = CraftingAutomaton->DiscoverRecipe(SteelSwordRecipeId);
    UE_LOG(LogTemp, Display, TEXT("Steel Sword recipe discovered: %s"), Discovered ? TEXT("Yes") : TEXT("No"));
    
    // Check if we can craft steel sword (should fail due to missing ingredients)
    CanCraft = CraftingAutomaton->CanCraftRecipe(SteelSwordRecipeId, GameState->Inventory);
    UE_LOG(LogTemp, Display, TEXT("Can craft Steel Sword: %s"), CanCraft ? TEXT("Yes") : TEXT("No"));
    
    // Craft iron sword first
    FAutomatonNode IronSwordRecipe = CraftingAutomaton->GetNode(IronSwordRecipeId);
    FString RequiredItemsStr = IronSwordRecipe.Metadata["RequiredItems"];
    TArray<FString> RequiredItems;
    RequiredItemsStr.ParseIntoArray(RequiredItems, TEXT(";"), true);
    
    for (const FString& Item : RequiredItems)
    {
        GameState->RemoveItem(Item);
    }
    GameState->AddItem(IronSwordRecipe.Metadata["Result"]);
    
    UE_LOG(LogTemp, Display, TEXT("Crafted Iron Sword"));
    UE_LOG(LogTemp, Display, TEXT("Updated inventory:"));
   for (const FString& Item : GameState->Inventory)
   {
       UE_LOG(LogTemp, Display, TEXT("- %s"), *Item);
   }
   
   // Add items needed for steel sword
   GameState->AddItem("Steel Ingot");
   GameState->AddItem("Leather Grip");
   
   // Check if we can craft steel sword now
   CanCraft = CraftingAutomaton->CanCraftRecipe(SteelSwordRecipeId, GameState->Inventory);
   UE_LOG(LogTemp, Display, TEXT("Can craft Steel Sword: %s"), CanCraft ? TEXT("Yes") : TEXT("No"));
   
   // Craft steel sword
   if (CanCraft)
   {
       FAutomatonNode SteelSwordRecipe = CraftingAutomaton->GetNode(SteelSwordRecipeId);
       FString SteelSwordRequiredItemsStr = SteelSwordRecipe.Metadata["RequiredItems"];
       TArray<FString> SteelSwordRequiredItems;
       SteelSwordRequiredItemsStr.ParseIntoArray(SteelSwordRequiredItems, TEXT(";"), true);

       for (const FString& Item : SteelSwordRequiredItems)  // Update this variable name too
       {
           GameState->RemoveItem(Item);
       }
       GameState->AddItem(SteelSwordRecipe.Metadata["Result"]);

       UE_LOG(LogTemp, Display, TEXT("Crafted Steel Sword"));
       UE_LOG(LogTemp, Display, TEXT("Final inventory:"));
       for (const FString& Item : GameState->Inventory)
       {
           UE_LOG(LogTemp, Display, TEXT("- %s"), *Item);
       }
   }
}