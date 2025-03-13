// TreeAutomataManager.cpp
#include "TreeAutomataManager.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "Json.h"
#include "JsonObjectConverter.h"
#include "Serialization/JsonSerializer.h"

ATreeAutomataManager::ATreeAutomataManager()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // Create the component instances
    QuestAutomaton = CreateDefaultSubobject<UQuestAutomaton>(TEXT("QuestAutomaton"));
    DialogueAutomaton = CreateDefaultSubobject<UDialogueAutomaton>(TEXT("DialogueAutomaton"));
    SkillTreeAutomaton = CreateDefaultSubobject<USkillTreeAutomaton>(TEXT("SkillTreeAutomaton"));
    CraftingAutomaton = CreateDefaultSubobject<UCraftingAutomaton>(TEXT("CraftingAutomaton"));
}

void ATreeAutomataManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize automata if needed
    if (!QuestAutomaton->IsValidLowLevel() || QuestAutomaton->GetNodes().Num() == 0)
    {
        QuestAutomaton->InitializeRoot("QuestSystem");
    }
    
    if (!DialogueAutomaton->IsValidLowLevel() || DialogueAutomaton->GetNodes().Num() == 0)
    {
        DialogueAutomaton->InitializeRoot("DialogueSystem");
    }
    
    if (!SkillTreeAutomaton->IsValidLowLevel() || SkillTreeAutomaton->GetNodes().Num() == 0)
    {
        SkillTreeAutomaton->InitializeRoot("SkillSystem");
    }
    
    if (!CraftingAutomaton->IsValidLowLevel() || CraftingAutomaton->GetNodes().Num() == 0)
    {
        CraftingAutomaton->InitializeRoot("CraftingSystem");
    }
}

UQuestAutomaton* ATreeAutomataManager::GetQuestAutomaton() const
{
    return QuestAutomaton;
}

UDialogueAutomaton* ATreeAutomataManager::GetDialogueAutomaton() const
{
    return DialogueAutomaton;
}

USkillTreeAutomaton* ATreeAutomataManager::GetSkillTreeAutomaton() const
{
    return SkillTreeAutomaton;
}

UCraftingAutomaton* ATreeAutomataManager::GetCraftingAutomaton() const
{
    return CraftingAutomaton;
}

bool ATreeAutomataManager::SaveToFile(const FString& FilePath)
{
    // Create a JSON object to hold all automata
    TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject);
    
    // Add each automaton's serialized data
    RootObject->SetStringField(TEXT("QuestAutomaton"), QuestAutomaton->SerializeToJson());
    RootObject->SetStringField(TEXT("DialogueAutomaton"), DialogueAutomaton->SerializeToJson());
    RootObject->SetStringField(TEXT("SkillTreeAutomaton"), SkillTreeAutomaton->SerializeToJson());
    RootObject->SetStringField(TEXT("CraftingAutomaton"), CraftingAutomaton->SerializeToJson());
    
    // Serialize to string
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);
    
    // Write to file
    return FFileHelper::SaveStringToFile(OutputString, *FilePath);
}

bool ATreeAutomataManager::LoadFromFile(const FString& FilePath)
{
    FString JsonString;
    
    // Read from file
    if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load automata from file: %s"), *FilePath);
        return false;
    }
    
    TSharedPtr<FJsonObject> RootObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
    
    if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to parse automata JSON from file"));
        return false;
    }
    
    // Load each automaton's data
    FString QuestJson = RootObject->GetStringField(TEXT("QuestAutomaton"));
    FString DialogueJson = RootObject->GetStringField(TEXT("DialogueAutomaton"));
    FString SkillTreeJson = RootObject->GetStringField(TEXT("SkillTreeAutomaton"));
    FString CraftingJson = RootObject->GetStringField(TEXT("CraftingAutomaton"));
    
    bool Success = QuestAutomaton->DeserializeFromJson(QuestJson) &&
                   DialogueAutomaton->DeserializeFromJson(DialogueJson) &&
                   SkillTreeAutomaton->DeserializeFromJson(SkillTreeJson) &&
                   CraftingAutomaton->DeserializeFromJson(CraftingJson);
    
    return Success;
}