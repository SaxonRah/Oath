# Usage Examples for All Classes

Here are examples of how to use each of the implemented RPG system classes in Unreal Engine 5 with the StateTree plugin.

## 1. Quest System Usage

```cpp
// In a GameMode or GameInstance class
void AMyRPGGameMode::SetupQuestSystem()
{
    // Get the player character
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    APawn* Pawn = PC->GetPawn();
    
    // Make sure the components exist
    UQuestDataComponent* QuestData = Pawn->FindComponentByClass<UQuestDataComponent>();
    UPlayerSkillComponent* SkillComp = Pawn->FindComponentByClass<UPlayerSkillComponent>();
    
    if (!QuestData || !SkillComp)
    {
        UE_LOG(LogRPG, Error, TEXT("Missing required components for quest system"));
        return;
    }
    
    // Create a StateTree component if it doesn't exist
    UStateTreeComponent* StateTreeComp = Pawn->FindComponentByClass<UStateTreeComponent>();
    if (!StateTreeComp)
    {
        StateTreeComp = NewObject<UStateTreeComponent>(Pawn);
        StateTreeComp->RegisterComponent();
    }
    
    // Setup a main quest with sub-quests
    FQuestTreeNode* MainQuest = NewObject<FQuestTreeNode>();
    MainQuest->QuestID = FName("Quest_HeroesJourney");
    MainQuest->QuestTitle = FText::FromString("The Hero's Journey");
    MainQuest->QuestDescription = FText::FromString("Embark on an epic adventure to save the realm.");
    
    // Create sub-quests
    FQuestTreeNode* FirstSubQuest = NewObject<FQuestTreeNode>();
    FirstSubQuest->QuestID = FName("Quest_FindMentor");
    FirstSubQuest->QuestTitle = FText::FromString("Find a Mentor");
    FirstSubQuest->QuestDescription = FText::FromString("Seek out the wise sage in the mountains.");
    
    // Add quest rewards
    FirstSubQuest->RewardIDs.Add("Item_WisdomScroll");
    FirstSubQuest->RewardIDs.Add("Gold_100");
    
    // Add prerequisite to a sub-quest
    FQuestTreeNode* SecondSubQuest = NewObject<FQuestTreeNode>();
    SecondSubQuest->QuestID = FName("Quest_ObtainRelic");
    SecondSubQuest->QuestTitle = FText::FromString("Obtain the Ancient Relic");
    SecondSubQuest->QuestDescription = FText::FromString("Retrieve the relic from the forgotten temple.");
    SecondSubQuest->Requirements.RequiredQuestsCompleted.Add("Quest_FindMentor");
    SecondSubQuest->Requirements.RequiredSkill = "Arcana";
    SecondSubQuest->Requirements.RequiredLevel = 3;
    
    // Link quests
    MainQuest->SubQuestIDs.Add(FirstSubQuest->QuestID);
    MainQuest->SubQuestIDs.Add(SecondSubQuest->QuestID);
    
    // Add to state tree
    StateTreeComp->AddStateTreeInstance(MainQuest);
    StateTreeComp->AddStateTreeInstance(FirstSubQuest);
    StateTreeComp->AddStateTreeInstance(SecondSubQuest);
    
    // Register external data
    StateTreeComp->RegisterExternalData(QuestData);
    StateTreeComp->RegisterExternalData(SkillComp);
    
    // Make the first quest available to start the chain
    QuestData->SetQuestStatus(FirstSubQuest->QuestID, EQuestStatus::Available);
    
    // Start the state tree
    StateTreeComp->StartStateTree();
}

// Elsewhere, to check quest status and progress
void APlayerCharacter::CheckQuestProgress()
{
    UQuestDataComponent* QuestData = FindComponentByClass<UQuestDataComponent>();
    if (QuestData)
    {
        EQuestStatus Status = QuestData->GetQuestStatus(FName("Quest_FindMentor"));
        if (Status == EQuestStatus::InProgress)
        {
            // Update quest progress when player reaches the sage location
            if (IsPlayerAtSageLocation())
            {
                QuestData->SetQuestStatus(FName("Quest_FindMentor"), EQuestStatus::Completed);
                // This will trigger StateTree to evaluate and potentially make the next quest available
            }
        }
    }
}
```

## 2. Dialogue System Usage

```cpp
// In an NPC class or NPC controller
void ANPCCharacter::SetupDialogueTree()
{
    // Create or find the StateTree component
    UStateTreeComponent* StateTreeComp = FindComponentByClass<UStateTreeComponent>();
    if (!StateTreeComp)
    {
        StateTreeComp = NewObject<UStateTreeComponent>(this);
        StateTreeComp->RegisterComponent();
    }
    
    // Create dialogue manager component
    UDialogueManagerComponent* DialogueManager = FindComponentByClass<UDialogueManagerComponent>();
    if (!DialogueManager)
    {
        DialogueManager = NewObject<UDialogueManagerComponent>(this);
        DialogueManager->RegisterComponent();
    }
    
    // Create a greeting dialogue node
    FDialogueTreeNode* GreetingNode = NewObject<FDialogueTreeNode>();
    GreetingNode->DialogueNodeID = FName("Dialogue_Greeting");
    GreetingNode->NPCText = FText::FromString("Greetings, traveler! What brings you to our humble village?");
    
    // Create dialogue options
    FDialogueOption Option1;
    Option1.OptionText = FText::FromString("I'm looking for information about the ancient temple.");
    Option1.NextNodeID = FName("Dialogue_TempleInfo");
    
    FDialogueOption Option2;
    Option2.OptionText = FText::FromString("I need supplies for my journey.");
    Option2.NextNodeID = FName("Dialogue_Supplies");
    
    FDialogueOption Option3;
    Option3.OptionText = FText::FromString("I've heard there are bandits in the area. [Requires Intimidation 2]");
    Option3.NextNodeID = FName("Dialogue_Bandits");
    Option3.RequiredSkill = FName("Intimidation");
    Option3.RequiredSkillLevel = 2;
    
    GreetingNode->DialogueOptions.Add(Option1);
    GreetingNode->DialogueOptions.Add(Option2);
    GreetingNode->DialogueOptions.Add(Option3);
    
    // Create response nodes
    FDialogueTreeNode* TempleInfoNode = NewObject<FDialogueTreeNode>();
    TempleInfoNode->DialogueNodeID = FName("Dialogue_TempleInfo");
    TempleInfoNode->NPCText = FText::FromString("The temple? It's a dangerous place. Only the elder knows the way there.");
    TempleInfoNode->KnowledgeGained.Add(FName("Knowledge_TempleElderConnection"));
    
    FDialogueOption TempleOption1;
    TempleOption1.OptionText = FText::FromString("Where can I find the elder?");
    TempleOption1.NextNodeID = FName("Dialogue_Elder");
    
    FDialogueOption TempleOption2;
    TempleOption2.OptionText = FText::FromString("I'll take my chances without help.");
    TempleOption2.NextNodeID = FName("Dialogue_Farewell");
    
    TempleInfoNode->DialogueOptions.Add(TempleOption1);
    TempleInfoNode->DialogueOptions.Add(TempleOption2);
    
    // Add nodes to state tree
    StateTreeComp->AddStateTreeInstance(GreetingNode);
    StateTreeComp->AddStateTreeInstance(TempleInfoNode);
    
    // Register external data
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    APawn* PlayerPawn = PC->GetPawn();
    
    UPlayerSkillComponent* PlayerSkills = PlayerPawn->FindComponentByClass<UPlayerSkillComponent>();
    UPlayerKnowledgeComponent* PlayerKnowledge = PlayerPawn->FindComponentByClass<UPlayerKnowledgeComponent>();
    UFactionComponent* FactionComp = PlayerPawn->FindComponentByClass<UFactionComponent>();
    
    StateTreeComp->RegisterExternalData(DialogueManager);
    StateTreeComp->RegisterExternalData(PlayerSkills);
    StateTreeComp->RegisterExternalData(PlayerKnowledge);
    StateTreeComp->RegisterExternalData(FactionComp);
}

// To start a conversation with this NPC
void ANPCCharacter::StartConversation(APlayerCharacter* Player)
{
    UStateTreeComponent* StateTreeComp = FindComponentByClass<UStateTreeComponent>();
    UDialogueManagerComponent* DialogueManager = FindComponentByClass<UDialogueManagerComponent>();
    
    if (StateTreeComp && DialogueManager)
    {
        // Set the current player reference
        DialogueManager->SetCurrentPlayer(Player);
        
        // Start with the greeting node
        DialogueManager->TransitionToDialogueNode(FName("Dialogue_Greeting"));
        
        // Start the state tree if not already running
        if (!StateTreeComp->IsStateTreeRunning())
        {
            StateTreeComp->StartStateTree();
        }
        
        // Enable player UI for dialogue
        Player->EnableDialogueUI(DialogueManager);
    }
}

// In the player's UI handler, to select a dialogue option
void UDialogueWidget::OnOptionSelected(int32 OptionIndex)
{
    UDialogueManagerComponent* DialogueManager = GetDialogueManager();
    if (DialogueManager)
    {
        DialogueManager->SelectOption(OptionIndex);
    }
}
```

## 3. Skill System Usage

```cpp
// Setting up the player's skill tree
void APlayerCharacter::InitializeSkillTree()
{
    // Find or create components
    UPlayerSkillComponent* SkillComp = FindComponentByClass<UPlayerSkillComponent>();
    UPlayerStatsComponent* StatsComp = FindComponentByClass<UPlayerStatsComponent>();
    UPlayerAbilityComponent* AbilityComp = FindComponentByClass<UPlayerAbilityComponent>();
    
    if (!SkillComp || !StatsComp)
    {
        UE_LOG(LogRPG, Error, TEXT("Missing required components for skill system"));
        return;
    }
    
    UStateTreeComponent* StateTreeComp = FindComponentByClass<UStateTreeComponent>();
    if (!StateTreeComp)
    {
        StateTreeComp = NewObject<UStateTreeComponent>(this);
        StateTreeComp->RegisterComponent();
    }
    
    // Create base combat skills
    FSkillTreeNode* CombatNode = NewObject<FSkillTreeNode>();
    CombatNode->SkillID = FName("Skill_Combat");
    CombatNode->SkillName = FText::FromString("Combat");
    CombatNode->SkillDescription = FText::FromString("Basic combat training that improves your ability to fight.");
    CombatNode->MaxLevel = 5;
    CombatNode->CharacterLevelRequired = 1;
    
    // Create effect for the skill
    FSkillEffect DamageEffect;
    DamageEffect.StatModified = FName("PhysicalDamage");
    DamageEffect.ValuePerLevel = 2.0f;
    CombatNode->SkillEffects.Add(DamageEffect);
    
    // Create specialized combat skills
    FSkillTreeNode* SwordNode = NewObject<FSkillTreeNode>();
    SwordNode->SkillID = FName("Skill_Swordsmanship");
    SwordNode->SkillName = FText::FromString("Swordsmanship");
    SwordNode->SkillDescription = FText::FromString("Advanced techniques for fighting with swords.");
    SwordNode->MaxLevel = 5;
    SwordNode->CharacterLevelRequired = 3;
    
    // Add prerequisite
    FSkillPrerequisite CombatPrereq;
    CombatPrereq.PrerequisiteSkill = FName("Skill_Combat");
    CombatPrereq.RequiredLevel = 2;
    SwordNode->Prerequisites.Add(CombatPrereq);
    
    // Unlock ability when reaching level 1
    SwordNode->UnlockedAbilities.Add(FName("Ability_SwordSlash"));
    
    // Add sword-specific effect
    FSkillEffect SwordEffect;
    SwordEffect.StatModified = FName("SwordDamage");
    SwordEffect.ValuePerLevel = 3.0f;
    SwordNode->SkillEffects.Add(SwordEffect);
    
    // Link skills
    CombatNode->ChildSkills.Add(SwordNode->SkillID);
    
    // Add to state tree
    StateTreeComp->AddStateTreeInstance(CombatNode);
    StateTreeComp->AddStateTreeInstance(SwordNode);
    
    // Register external data
    StateTreeComp->RegisterExternalData(SkillComp);
    StateTreeComp->RegisterExternalData(StatsComp);
    StateTreeComp->RegisterExternalData(AbilityComp);
    
    // Unlock initial skills
    SkillComp->UnlockSkill(FName("Skill_Combat"));
    
    // Add starting skill points
    SkillComp->AddSkillPoints(3);
    
    // Start the state tree
    StateTreeComp->StartStateTree();
}

// In a skill UI handler
void USkillTreeWidget::OnSkillSelected(FName SkillID)
{
    APlayerCharacter* Player = Cast<APlayerCharacter>(GetOwningPlayerPawn());
    if (Player)
    {
        UPlayerSkillComponent* SkillComp = Player->FindComponentByClass<UPlayerSkillComponent>();
        if (SkillComp)
        {
            // Tell the skill system we're trying to improve this skill
            SkillComp->SetAttemptingToImprove(SkillID, true);
            
            // The StateTree will handle checking requirements and processing the improvement
            // in the next tick of the SkillTreeNode
        }
    }
}

// Applying skill effects in gameplay
float APlayerCharacter::CalculateDamage(FName WeaponType)
{
    float BaseDamage = 10.0f;
    
    UPlayerStatsComponent* StatsComp = FindComponentByClass<UPlayerStatsComponent>();
    if (StatsComp)
    {
        // Get general damage bonus from stats (affected by Combat skill)
        BaseDamage += StatsComp->GetStatValue(FName("PhysicalDamage"));
        
        // Get weapon-specific damage bonus
        if (WeaponType == FName("Sword"))
        {
            BaseDamage += StatsComp->GetStatValue(FName("SwordDamage"));
        }
    }
    
    return BaseDamage;
}
```

## 4. Crafting System Usage

```cpp
// Setting up a crafting station
void ACraftingStation::InitializeCraftingRecipes()
{
    // Create state tree component
    UStateTreeComponent* StateTreeComp = FindComponentByClass<UStateTreeComponent>();
    if (!StateTreeComp)
    {
        StateTreeComp = NewObject<UStateTreeComponent>(this);
        StateTreeComp->RegisterComponent();
    }
    
    // Create crafting component
    UCraftingComponent* CraftingComp = FindComponentByClass<UCraftingComponent>();
    if (!CraftingComp)
    {
        CraftingComp = NewObject<UCraftingComponent>(this);
        CraftingComp->RegisterComponent();
    }
    
    // Create a basic potion recipe
    FCraftingTreeNode* HealthPotionNode = NewObject<FCraftingTreeNode>();
    HealthPotionNode->RecipeID = FName("Recipe_HealthPotion");
    HealthPotionNode->RecipeName = FText::FromString("Health Potion");
    HealthPotionNode->CraftingSkillRequired = FName("Alchemy");
    HealthPotionNode->SkillLevelRequired = 1;
    
    // Add ingredients
    FCraftingIngredient Herb;
    Herb.ItemID = FName("Item_Herb");
    Herb.Quantity = 2;
    
    FCraftingIngredient Water;
    Water.ItemID = FName("Item_Water");
    Water.Quantity = 1;
    
    FCraftingIngredient Vial;
    Vial.ItemID = FName("Item_Vial");
    Vial.Quantity = 1;
    Vial.bConsumed = false; // Tool that isn't consumed
    
    HealthPotionNode->Ingredients.Add(Herb);
    HealthPotionNode->Ingredients.Add(Water);
    HealthPotionNode->Ingredients.Add(Vial);
    
    // Set result
    HealthPotionNode->PrimaryResult.ItemID = FName("Item_HealthPotion");
    HealthPotionNode->PrimaryResult.Quantity = 1;
    HealthPotionNode->PrimaryResult.BaseSuccessChance = 0.9f;
    HealthPotionNode->PrimaryResult.QualityDeterminant = FName("Alchemy");
    
    // Add bonus result (rare chance)
    FCraftingResult BonusResult;
    BonusResult.ItemID = FName("Item_PotentHealthPotion");
    BonusResult.Quantity = 1;
    BonusResult.BaseSuccessChance = 0.1f; // 10% chance
    HealthPotionNode->BonusResults.Add(BonusResult);
    
    // Create advanced potion that's unlocked by crafting the basic one
    FCraftingTreeNode* GreaterHealthPotionNode = NewObject<FCraftingTreeNode>();
    GreaterHealthPotionNode->RecipeID = FName("Recipe_GreaterHealthPotion");
    GreaterHealthPotionNode->RecipeName = FText::FromString("Greater Health Potion");
    GreaterHealthPotionNode->CraftingSkillRequired = FName("Alchemy");
    GreaterHealthPotionNode->SkillLevelRequired = 3;
    
    // Add unlocked recipe to the basic potion
    HealthPotionNode->UnlockedRecipes.Add(GreaterHealthPotionNode->RecipeID);
    
    // Add recipe nodes to state tree
    StateTreeComp->AddStateTreeInstance(HealthPotionNode);
    StateTreeComp->AddStateTreeInstance(GreaterHealthPotionNode);
    
    // Mark base recipes as known
    CraftingComp->UnlockRecipe(HealthPotionNode->RecipeID);
    
    // Experimental recipes that can be discovered
    FCraftingTreeNode* ExperimentalNode = NewObject<FCraftingTreeNode>();
    ExperimentalNode->RecipeID = FName("Recipe_Experiment");
    ExperimentalNode->RecipeName = FText::FromString("Alchemical Experiment");
    ExperimentalNode->CraftingSkillRequired = FName("Alchemy");
    ExperimentalNode->SkillLevelRequired = 2;
    ExperimentalNode->bExperimental = true;
    
    // Add potential discoveries
    ExperimentalNode->PotentialDiscoveries.Add(FName("Recipe_FireResistPotion"));
    ExperimentalNode->PotentialDiscoveries.Add(FName("Recipe_FrostResistPotion"));
    
    StateTreeComp->AddStateTreeInstance(ExperimentalNode);
    CraftingComp->UnlockRecipe(ExperimentalNode->RecipeID);
}

// When a player interacts with the crafting station
void ACraftingStation::OnPlayerInteract(APlayerCharacter* Player)
{
    // Get components
    UStateTreeComponent* StateTreeComp = FindComponentByClass<UStateTreeComponent>();
    UCraftingComponent* CraftingComp = FindComponentByClass<UCraftingComponent>();
    
    if (StateTreeComp && CraftingComp)
    {
        // Register player components as external data
        UPlayerSkillComponent* PlayerSkills = Player->FindComponentByClass<UPlayerSkillComponent>();
        UInventoryComponent* PlayerInventory = Player->FindComponentByClass<UInventoryComponent>();
        
        StateTreeComp->RegisterExternalData(CraftingComp);
        StateTreeComp->RegisterExternalData(PlayerSkills);
        StateTreeComp->RegisterExternalData(PlayerInventory);
        
        // Open crafting UI for player
        Player->OpenCraftingUI(CraftingComp);
        
        // Start state tree if not already running
        if (!StateTreeComp->IsStateTreeRunning())
        {
            StateTreeComp->StartStateTree();
        }
    }
}

// In the crafting UI, when player attempts to craft an item
void UCraftingWidget::OnCraftButtonClicked()
{
    UCraftingComponent* CraftingComp = GetCraftingComponent();
    if (CraftingComp && SelectedRecipeID != NAME_None)
    {
        // Set active recipe and the state tree will handle the crafting process
        CraftingComp->SetActiveRecipe(SelectedRecipeID);
    }
}
```

## 5. World Progression System Usage

```cpp
// In a game instance or world manager class
void ARPGWorldManager::SetupWorldProgressionSystem()
{
    // Create components
    UWorldStateComponent* WorldStateComp = FindComponentByClass<UWorldStateComponent>();
    if (!WorldStateComp)
    {
        WorldStateComp = NewObject<UWorldStateComponent>(this);
        WorldStateComp->RegisterComponent();
    }
    
    UStateTreeComponent* StateTreeComp = FindComponentByClass<UStateTreeComponent>();
    if (!StateTreeComp)
    {
        StateTreeComp = NewObject<UStateTreeComponent>(this);
        StateTreeComp->RegisterComponent();
    }
    
    // Initialize world variables
    WorldStateComp->SetWorldVariable(FName("Economy_Prosperity"), 50.0f);
    WorldStateComp->SetWorldVariable(FName("Environment_Stability"), 75.0f);
    WorldStateComp->SetWorldVariable(FName("Town_Defense"), 30.0f);
    WorldStateComp->SetWorldVariable(FName("Faction_BanditActivity"), 60.0f);
    
    // Setup seasonal cycle
    FWorldStateTreeNode* WinterNode = NewObject<FWorldStateTreeNode>();
    WinterNode->WorldEventID = FName("Season_Winter");
    WinterNode->EventName = FText::FromString("Winter");
    WinterNode->EventDescription = FText::FromString("Cold temperatures make survival harder but reduce bandit activity.");
    WinterNode->EventType = EWorldEventType::SeasonChange;
    WinterNode->TimeBetweenChecks = 10.0f; // For debugging, would be longer in real game
    WinterNode->EventDuration = 600.0f; // 10 minutes for testing
    
    // Winter effects
    FWorldEventEffect WinterEconomy;
    WinterEconomy.VariableName = FName("Economy_Prosperity");
    WinterEconomy.ChangeValue = -10.0f;
    
    FWorldEventEffect WinterBandits;
    WinterBandits.VariableName = FName("Faction_BanditActivity");
    WinterBandits.ChangeValue = -20.0f;
    
    WinterNode->EventEffects.Add(WinterEconomy);
    WinterNode->EventEffects.Add(WinterBandits);
    
    // Cycle to spring after winter
    WinterNode->NextPossibleEvents.Add(FName("Season_Spring"));
    WinterNode->bCyclical = true;
    
    // Create town development event triggered by prosperity
    FWorldStateTreeNode* TownGrowthNode = NewObject<FWorldStateTreeNode>();
    TownGrowthNode->WorldEventID = FName("Town_Growth");
    TownGrowthNode->EventName = FText::FromString("Town Growth");
    TownGrowthNode->EventDescription = FText::FromString("The town is expanding with new buildings and opportunities.");
    TownGrowthNode->EventType = EWorldEventType::TownDevelopment;
    TownGrowthNode->TimeBetweenChecks = 30.0f;
    TownGrowthNode->EventDuration = 0.0f; // Permanent until changed
    
    // Required conditions
    FWorldEventCondition ProsperityCondition;
    ProsperityCondition.VariableName = FName("Economy_Prosperity");
    ProsperityCondition.MinValue = 75.0f;
    TownGrowthNode->RequiredConditions.Add(ProsperityCondition);
    
    // Growth effects
    FWorldEventEffect GrowthDefense;
    GrowthDefense.VariableName = FName("Town_Defense");
    GrowthDefense.ChangeValue = 15.0f;
    TownGrowthNode->EventEffects.Add(GrowthDefense);
    
    // Trigger new quests when town grows
    TownGrowthNode->QuestsTriggered.Add(FName("Quest_NewMarketplace"));
    
    // Add nodes to state tree
    StateTreeComp->AddStateTreeInstance(WinterNode);
    StateTreeComp->AddStateTreeInstance(TownGrowthNode);
    
    // Register external data
    UQuestDataComponent* QuestDataComp = GetWorld()->GetGameInstance()->FindComponentByClass<UQuestDataComponent>();
    StateTreeComp->RegisterExternalData(WorldStateComp);
    StateTreeComp->RegisterExternalData(QuestDataComp);
    
    // Start state tree
    StateTreeComp->StartStateTree();
    
    // Subscribe to events
    WorldStateComp->OnWorldEventStarted.AddDynamic(this, &ARPGWorldManager::OnWorldEventStarted);
    WorldStateComp->OnWorldEventEnded.AddDynamic(this, &ARPGWorldManager::OnWorldEventEnded);
}

// Event handlers
void ARPGWorldManager::OnWorldEventStarted(FName EventID, FText EventName, FText EventDescription, EWorldEventType EventType)
{
    // Notify players about the event
    for (APlayerController* PC : PlayerControllers)
    {
        // Show UI notification
        AMyPlayerController* MyPC = Cast<AMyPlayerController>(PC);
        if (MyPC)
        {
            MyPC->ShowWorldEventNotification(EventName, EventDescription);
        }
    }
    
    // Spawn visual effects based on event type
    if (EventType == EWorldEventType::SeasonChange)
    {
        if (EventID == FName("Season_Winter"))
        {
            // Start snow particles
            ActivateSnowEffects();
        }
    }
    else if (EventType == EWorldEventType::TownDevelopment)
    {
        // Spawn new buildings or NPCs
        SpawnTownDevelopmentActors(EventID);
    }
}

void ARPGWorldManager::OnWorldEventEnded(FName EventID)
{
    if (EventID == FName("Season_Winter"))
    {
        // Stop snow effects
        DeactivateSnowEffects();
    }
}

// Player interaction with world state
void APlayerCharacter::InfluenceWorldState(FName VariableName, float Value)
{
    // Find the world manager
    ARPGWorldManager* WorldManager = Cast<ARPGWorldManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ARPGWorldManager::StaticClass()));
    if (WorldManager)
    {
        UWorldStateComponent* WorldStateComp = WorldManager->FindComponentByClass<UWorldStateComponent>();
        if (WorldStateComp)
        {
            // Player's actions affect the world state
            WorldStateComp->ChangeWorldVariable(VariableName, Value);
            
            // Log for the player
            UE_LOG(LogRPG, Log, TEXT("Your actions have influenced the world (%s: %f)"), *VariableName.ToString(), Value);
        }
    }
}
```

## 6. Skill Component Usage

```cpp
// Initializing player skills
void APlayerCharacter::InitializeSkills()
{
    // Create component
    UPlayerSkillComponent* SkillComp = FindComponentByClass<UPlayerSkillComponent>();
    if (!SkillComp)
    {
        SkillComp = NewObject<UPlayerSkillComponent>(this);
        SkillComp->RegisterComponent();
    }
    
    // Unlock basic skills
    SkillComp->UnlockSkill(FName("Combat"));
    SkillComp->UnlockSkill(FName("Survival"));
    SkillComp->UnlockSkill(FName("Arcana"));
    
    // Give starting skill points
    SkillComp->AddSkillPoints(5);
    
    // Subscribe to skill events
    SkillComp->OnSkillLevelChanged.AddDynamic(this, &APlayerCharacter::OnSkillLevelUp);
    SkillComp->OnSkillUnlocked.AddDynamic(this, &APlayerCharacter::OnNewSkillUnlocked);
}

// Using skills during gameplay
void APlayerCharacter::AttemptLockpick(UInteractableComponent* LockComponent)
{
    UPlayerSkillComponent* SkillComp = FindComponentByClass<UPlayerSkillComponent>();
    if (SkillComp)
    {
        int32 LockpickSkill = SkillComp->GetSkillLevel(FName("Lockpicking"));
        int32 LockDifficulty = LockComponent->GetLockDifficulty();
        
        // Calculate success chance based on skill
        float BaseChance = 0.2f; // 20% base chance
        float SkillBonus = 0.1f * LockpickSkill; // +10% per skill level
        float SuccessChance = FMath::Clamp(BaseChance + SkillBonus - (0.1f * LockDifficulty), 0.05f, 0.95f);
        
        // Roll for success
        if (FMath::FRand() <= SuccessChance)
        {
            // Success
            LockComponent->Unlock();
            
            // Grant experience
            float ExpGain = 10.0f * LockDifficulty;
            SkillComp->AddExperience(FName("Lockpicking"), ExpGain);
            
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Lock picked successfully!"));
        }
        else
        {
            // Failure
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Failed to pick lock!"));
            
            // Still gain some experience from failure
            float ExpGain = 3.0f * LockDifficulty;
            SkillComp->AddExperience(FName("Lockpicking"), ExpGain);
        }
    }
}
```

```cpp
// Event handlers
void APlayerCharacter::OnSkillLevelUp(FName SkillID, int32 NewLevel)
{
    // Show notification to player
    FString Message = FString::Printf(TEXT("%s skill increased to level %d!"), *SkillID.ToString(), NewLevel);
    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, Message);
    
    // Play level up effect
    UNiagaraComponent* FXComp = UNiagaraSystem::SpawnSystemAttached(
        SkillLevelUpFX,
        GetMesh(),
        NAME_None,
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        EAttachLocation::SnapToTarget,
        true
    );
    
    // Handle special skill thresholds
    if (SkillID == FName("Arcana") && NewLevel == 5)
    {
        // Unlock special ability at level 5
        UPlayerAbilityComponent* AbilityComp = FindComponentByClass<UPlayerAbilityComponent>();
        if (AbilityComp)
        {
            AbilityComp->UnlockAbility(FName("Ability_Teleport"));
        }
    }
}

void APlayerCharacter::OnNewSkillUnlocked(FName SkillID)
{
    // Show more prominent notification
    FString Message = FString::Printf(TEXT("New skill unlocked: %s!"), *SkillID.ToString());
    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, Message);
    
    // Update skill UI
    USkillTreeWidget* SkillWidget = Cast<USkillTreeWidget>(SkillTreeWidgetInstance);
    if (SkillWidget)
    {
        SkillWidget->RefreshSkillTree();
    }
}

// Using skill component to check skill requirements
bool AQuestGiver::CanAcceptQuest(APlayerCharacter* Player, FName QuestID)
{
    UPlayerSkillComponent* SkillComp = Player->FindComponentByClass<UPlayerSkillComponent>();
    if (!SkillComp)
    {
        return false;
    }
    
    // Check if player meets skill requirements for this quest
    FQuestInfo QuestInfo = QuestDatabase->FindQuest(QuestID);
    
    bool bMeetsRequirements = true;
    
    // Check all required skills
    for (const auto& RequiredSkill : QuestInfo.SkillRequirements)
    {
        int32 PlayerSkillLevel = SkillComp->GetSkillLevel(RequiredSkill.SkillName);
        if (PlayerSkillLevel < RequiredSkill.RequiredLevel)
        {
            bMeetsRequirements = false;
            break;
        }
    }
    
    return bMeetsRequirements;
}
```

## 7. World State Component Usage

```cpp
// Initializing world state with default values
void AWorldManager::InitializeWorldState()
{
    UWorldStateComponent* WorldStateComp = FindComponentByClass<UWorldStateComponent>();
    if (!WorldStateComp)
    {
        WorldStateComp = NewObject<UWorldStateComponent>(this);
        WorldStateComp->RegisterComponent();
    }
    
    // Set initial values for world variables
    WorldStateComp->SetWorldVariable(FName("Climate_Temperature"), 70.0f);
    WorldStateComp->SetWorldVariable(FName("Climate_Rainfall"), 50.0f);
    WorldStateComp->SetWorldVariable(FName("Village_Population"), 100.0f);
    WorldStateComp->SetWorldVariable(FName("Village_Food"), 75.0f);
    WorldStateComp->SetWorldVariable(FName("Faction_BanditThreat"), 40.0f);
    WorldStateComp->SetWorldVariable(FName("Faction_NobleInfluence"), 60.0f);
    
    // Set multipliers for some variables
    WorldStateComp->SetWorldVariableMultiplier(FName("Village_FoodProduction"), 1.0f);
    
    // Subscribe to variable changes
    WorldStateComp->OnWorldVariableChanged.AddDynamic(this, &AWorldManager::OnWorldVariableChanged);
    WorldStateComp->OnWorldEventStarted.AddDynamic(this, &AWorldManager::OnWorldEventStarted);
}

// Responding to world state changes
void AWorldManager::OnWorldVariableChanged(FName VariableName, float NewValue)
{
    // Log change
    UE_LOG(LogWorld, Log, TEXT("World variable changed: %s = %f"), *VariableName.ToString(), NewValue);
    
    // Check for critical thresholds
    if (VariableName == FName("Village_Food") && NewValue < 20.0f)
    {
        // Food shortage crisis
        UE_LOG(LogWorld, Warning, TEXT("CRISIS: Village food shortage!"));
        
        // Spawn emergency quest
        SpawnEmergencyQuest(FName("Quest_FoodShortage"));
        
        // Update NPCs to talk about food shortage
        UpdateNPCDialogueContext(FName("Context_FoodShortage"));
    }
    else if (VariableName == FName("Faction_BanditThreat") && NewValue > 80.0f)
    {
        // Bandit attack imminent
        UE_LOG(LogWorld, Warning, TEXT("CRISIS: Bandit attack imminent!"));
        
        // Decrease village population as people flee
        UWorldStateComponent* WorldStateComp = FindComponentByClass<UWorldStateComponent>();
        if (WorldStateComp)
        {
            WorldStateComp->ChangeWorldVariable(FName("Village_Population"), -10.0f);
        }
        
        // Spawn raid event
        ScheduleWorldEvent(FName("Event_BanditRaid"), 300.0f); // Raid in 5 minutes
    }
}

// Handling world events
void AWorldManager::OnWorldEventStarted(FName EventID, FText EventName, FText EventDescription, EWorldEventType EventType)
{
    // Broadcast to all players
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (PC)
        {
            // Show notification
            AMyHUD* HUD = Cast<AMyHUD>(PC->GetHUD());
            if (HUD)
            {
                HUD->ShowWorldEventNotification(EventName, EventDescription);
            }
        }
    }
    
    // Handle specific event types
    if (EventType == EWorldEventType::NaturalDisaster)
    {
        // Play global sound
        UGameplayStatics::PlaySound2D(this, DisasterSoundCue);
        
        // Spawn VFX based on disaster type
        if (EventID == FName("Event_Earthquake"))
        {
            // Shake all player cameras
            for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
            {
                APlayerController* PC = It->Get();
                if (PC)
                {
                    PC->ClientStartCameraShake(EarthquakeCameraShake);
                }
            }
            
            // Damage buildings
            UWorldStateComponent* WorldStateComp = FindComponentByClass<UWorldStateComponent>();
            if (WorldStateComp)
            {
                WorldStateComp->ChangeWorldVariable(FName("Village_BuildingIntegrity"), -30.0f);
            }
        }
    }
    else if (EventType == EWorldEventType::EconomicEvent)
    {
        // Update merchant prices
        for (TActorIterator<AMerchantNPC> It(GetWorld()); It; ++It)
        {
            AMerchantNPC* Merchant = *It;
            Merchant->UpdatePricesBasedOnEvent(EventID);
        }
    }
}

// Player action affecting world state
void APlayerCharacter::CompleteTownDefenseQuest()
{
    // Find world manager
    AWorldManager* WorldManager = GetWorldManager();
    if (WorldManager)
    {
        UWorldStateComponent* WorldStateComp = WorldManager->FindComponentByClass<UWorldStateComponent>();
        if (WorldStateComp)
        {
            // Decrease bandit threat
            WorldStateComp->ChangeWorldVariable(FName("Faction_BanditThreat"), -20.0f);
            
            // Increase town defense
            WorldStateComp->ChangeWorldVariable(FName("Village_Defense"), 15.0f);
            
            // Increase player's reputation with town
            UPlayerReputationComponent* RepComp = FindComponentByClass<UPlayerReputationComponent>();
            if (RepComp)
            {
                RepComp->AdjustReputation(FName("Faction_Village"), 20);
            }
        }
    }
}

// Using the world state in environment actors
void AWeatherController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Get current world state variables
    AWorldManager* WorldManager = GetWorldManager();
    if (WorldManager)
    {
        UWorldStateComponent* WorldStateComp = WorldManager->FindComponentByClass<UWorldStateComponent>();
        if (WorldStateComp)
        {
            float Temperature = WorldStateComp->GetWorldVariable(FName("Climate_Temperature"));
            float Rainfall = WorldStateComp->GetWorldVariable(FName("Climate_Rainfall"));
            
            // Update weather effects based on world variables
            UpdateSnowAmount(Temperature);
            UpdateRainIntensity(Rainfall);
            
            // Update foliage based on season
            for (TActorIterator<AFoliageActor> It(GetWorld()); It; ++It)
            {
                AFoliageActor* Foliage = *It;
                Foliage->UpdateSeasonalAppearance(Temperature);
            }
        }
    }
}
```

## 8. Complete Integration Example

```cpp
// In a game mode class, showing how all systems work together
void ARPGGameMode::InitializeGameSystems()
{
    // Create world manager if it doesn't exist
    AWorldManager* WorldManager = Cast<AWorldManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AWorldManager::StaticClass()));
    if (!WorldManager)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        WorldManager = GetWorld()->SpawnActor<AWorldManager>(AWorldManager::StaticClass(), SpawnParams);
    }
    
    // Initialize world state
    WorldManager->InitializeWorldState();
    
    // Setup world progression events
    UStateTreeComponent* WorldStateTreeComp = WorldManager->FindComponentByClass<UStateTreeComponent>();
    UWorldStateComponent* WorldStateComp = WorldManager->FindComponentByClass<UWorldStateComponent>();
    
    if (WorldStateTreeComp && WorldStateComp)
    {
        // Create seasonal cycle events
        SetupSeasonalEvents(WorldStateTreeComp);
        
        // Create economy events
        SetupEconomyEvents(WorldStateTreeComp);
        
        // Create faction events
        SetupFactionEvents(WorldStateTreeComp);
        
        // Start the world state tree
        WorldStateTreeComp->RegisterExternalData(WorldStateComp);
        WorldStateTreeComp->StartStateTree();
    }
    
    // Create quest manager if it doesn't exist
    AQuestManager* QuestManager = Cast<AQuestManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AQuestManager::StaticClass()));
    if (!QuestManager)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        QuestManager = GetWorld()->SpawnActor<AQuestManager>(AQuestManager::StaticClass(), SpawnParams);
    }
    
    // Setup initial quests
    QuestManager->InitializeQuests();
    
    // Connect quest manager to world state
    UQuestDataComponent* QuestDataComp = QuestManager->FindComponentByClass<UQuestDataComponent>();
    if (QuestDataComp && WorldStateTreeComp)
    {
        WorldStateTreeComp->RegisterExternalData(QuestDataComp);
    }
}

// When a player joins the game
void ARPGGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    
    APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(NewPlayer->GetPawn());
    if (PlayerCharacter)
    {
        // Initialize player skill system
        PlayerCharacter->InitializeSkills();
        
        // Initialize player stats
        PlayerCharacter->InitializeStats();
        
        // Initialize player inventory
        PlayerCharacter->InitializeInventory();
        
        // Setup player state tree
        UStateTreeComponent* PlayerStateTreeComp = PlayerCharacter->FindComponentByClass<UStateTreeComponent>();
        if (!PlayerStateTreeComp)
        {
            PlayerStateTreeComp = NewObject<UStateTreeComponent>(PlayerCharacter);
            PlayerStateTreeComp->RegisterComponent();
        }
        
        // Setup skill progression nodes
        UPlayerSkillComponent* SkillComp = PlayerCharacter->FindComponentByClass<UPlayerSkillComponent>();
        UPlayerStatsComponent* StatsComp = PlayerCharacter->FindComponentByClass<UPlayerStatsComponent>();
        UPlayerAbilityComponent* AbilityComp = PlayerCharacter->FindComponentByClass<UPlayerAbilityComponent>();
        
        if (PlayerStateTreeComp && SkillComp && StatsComp)
        {
            // Register player components with state tree
            PlayerStateTreeComp->RegisterExternalData(SkillComp);
            PlayerStateTreeComp->RegisterExternalData(StatsComp);
            if (AbilityComp)
            {
                PlayerStateTreeComp->RegisterExternalData(AbilityComp);
            }
            
            // Create skill tree nodes
            SetupPlayerSkillTree(PlayerStateTreeComp);
            
            // Start player state tree
            PlayerStateTreeComp->StartStateTree();
        }
        
        // Connect player to world event notifications
        AWorldManager* WorldManager = Cast<AWorldManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AWorldManager::StaticClass()));
        if (WorldManager)
        {
            UWorldStateComponent* WorldStateComp = WorldManager->FindComponentByClass<UWorldStateComponent>();
            if (WorldStateComp)
            {
                // Create UI handler for world events
                AMyPlayerController* MyPC = Cast<AMyPlayerController>(NewPlayer);
                if (MyPC)
                {
                    MyPC->CreateWorldEventNotificationHandler();
                }
                
                // Subscribe player to world events
                WorldStateComp->OnWorldEventStarted.AddDynamic(MyPC, &AMyPlayerController::OnWorldEventStarted);
                WorldStateComp->OnWorldEventEnded.AddDynamic(MyPC, &AMyPlayerController::OnWorldEventEnded);
            }
        }
    }
}

// Example of how systems affect each other during gameplay
void ARPGGameMode::HandleQuestCompletion(APlayerCharacter* Player, FName QuestID)
{
    // Get quest data
    AQuestManager* QuestManager = Cast<AQuestManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AQuestManager::StaticClass()));
    if (!QuestManager)
    {
        return;
    }
    
    UQuestDataComponent* QuestDataComp = QuestManager->FindComponentByClass<UQuestDataComponent>();
    if (!QuestDataComp)
    {
        return;
    }
    
    // Update quest status
    QuestDataComp->SetQuestStatus(QuestID, EQuestStatus::Completed);
    
    // Give rewards
    QuestDataComp->GiveQuestRewards(QuestID, Player);
    
    // Update world state based on quest
    AWorldManager* WorldManager = Cast<AWorldManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AWorldManager::StaticClass()));
    if (WorldManager)
    {
        UWorldStateComponent* WorldStateComp = WorldManager->FindComponentByClass<UWorldStateComponent>();
        if (WorldStateComp)
        {
            // Affect world variables based on quest type
            if (QuestID == FName("Quest_DefendVillage"))
            {
                WorldStateComp->ChangeWorldVariable(FName("Village_Defense"), 20.0f);
                WorldStateComp->ChangeWorldVariable(FName("Faction_BanditThreat"), -15.0f);
            }
            else if (QuestID == FName("Quest_HarvestCrops"))
            {
                WorldStateComp->ChangeWorldVariable(FName("Village_Food"), 25.0f);
            }
            else if (QuestID == FName("Quest_DiplomacyMission"))
            {
                WorldStateComp->ChangeWorldVariable(FName("Faction_NobleInfluence"), 15.0f);
            }
        }
    }
    
    // Grant skill experience
    UPlayerSkillComponent* SkillComp = Player->FindComponentByClass<UPlayerSkillComponent>();
    if (SkillComp)
    {
        // Different quests improve different skills
        if (QuestID == FName("Quest_DefendVillage"))
        {
            SkillComp->AddExperience(FName("Combat"), 100.0f);
        }
        else if (QuestID == FName("Quest_HarvestCrops"))
        {
            SkillComp->AddExperience(FName("Survival"), 75.0f);
        }
        else if (QuestID == FName("Quest_DiplomacyMission"))
        {
            SkillComp->AddExperience(FName("Persuasion"), 120.0f);
        }
    }
    
    // Potentially unlock new quests
    QuestManager->CheckForNewAvailableQuests(Player);
}
```

These usage examples demonstrate how to implement and integrate all the RPG systems using Unreal Engine 5's StateTree plugin. The examples show:

1. How to set up and manage a quest system with dependent sub-quests and multiple completion paths
2. Creating dynamic dialogue trees that adapt to player skills, knowledge, and reputation
3. Implementing skill trees with prerequisites, effects, and ability unlocks
4. Building a crafting system with recipes, ingredients, and quality outcomes
5. Creating a world progression system that evolves based on player actions and time
6. Using the skill component for character progression and gameplay checks
7. Managing world state variables to create a dynamic environment
8. Integrating all systems together in a cohesive game framework

These examples provide a solid foundation for building complex, interconnected RPG systems that create rich, dynamic gameplay experiences.