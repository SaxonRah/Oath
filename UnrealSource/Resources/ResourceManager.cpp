// ResourceManager.cpp
#include "ResourceManager.h"

UResourceManager::UResourceManager()
{
    InitializeDefaultResources();
}

void UResourceManager::InitializeDefaultResources()
{
    // Common building resources
    FResourceData wood;
    wood.Name = TEXT("Wood");
    wood.Rarity = EResourceRarity::Common;
    wood.Description = TEXT("Common lumber used for basic construction.");
    wood.BaseValue = 1;
    
    FResourceData stone;
    stone.Name = TEXT("Stone");
    stone.Rarity = EResourceRarity::Common;
    stone.Description = TEXT("Common stone used for basic construction.");
    stone.BaseValue = 1;
    
    FResourceData clay;
    clay.Name = TEXT("Clay");
    clay.Rarity = EResourceRarity::Common;
    clay.Description = TEXT("Moldable clay used for pottery and basic construction.");
    clay.BaseValue = 1;
    
    // Uncommon resources
    FResourceData iron;
    iron.Name = TEXT("Iron");
    iron.Rarity = EResourceRarity::Uncommon;
    iron.Description = TEXT("Durable metal used for tools, weapons, and advanced construction.");
    iron.BaseValue = 5;
    
    FResourceData silver;
    silver.Name = TEXT("Silver");
    silver.Rarity = EResourceRarity::Uncommon;
    silver.Description = TEXT("Precious metal used for decoration and certain magical items.");
    silver.BaseValue = 10;
    
    FResourceData herbs;
    herbs.Name = TEXT("Medicinal Herbs");
    herbs.Rarity = EResourceRarity::Uncommon;
    herbs.Description = TEXT("Various useful plants with medicinal properties.");
    herbs.BaseValue = 8;
    
    // Rare resources
    FResourceData gold;
    gold.Name = TEXT("Gold Ore");
    gold.Rarity = EResourceRarity::Rare;
    gold.Description = TEXT("Valuable metal prized for its luster and rarity.");
    gold.BaseValue = 25;
    
    FResourceData crystals;
    crystals.Name = TEXT("Magic Crystals");
    crystals.Rarity = EResourceRarity::Rare;
    crystals.Description = TEXT("Crystallized magical energy with various applications.");
    crystals.BaseValue = 30;
    
    // Legendary resources
    FResourceData dragonScale;
    dragonScale.Name = TEXT("Dragon Scale");
    dragonScale.Rarity = EResourceRarity::Legendary;
    dragonScale.Description = TEXT("Nearly indestructible scales from a dragon's hide.");
    dragonScale.BaseValue = 100;
    
    FResourceData heartwood;
    heartwood.Name = TEXT("Ancient Heartwood");
    heartwood.Rarity = EResourceRarity::Legendary;
    heartwood.Description = TEXT("Wood from the core of thousand-year-old trees, imbued with natural magic.");
    heartwood.BaseValue = 100;
    
    // Artifact resources
    FResourceData starfall;
    starfall.Name = TEXT("Starfall Metal");
    starfall.Rarity = EResourceRarity::Artifact;
    starfall.Description = TEXT("Metal from a fallen star, containing otherworldly properties.");
    starfall.BaseValue = 500;
    
    // Add resources to the array
    AvailableResources.Add(wood);
    AvailableResources.Add(stone);
    AvailableResources.Add(clay);
    AvailableResources.Add(iron);
    AvailableResources.Add(silver);
    AvailableResources.Add(herbs);
    AvailableResources.Add(gold);
    AvailableResources.Add(crystals);
    AvailableResources.Add(dragonScale);
    AvailableResources.Add(heartwood);
    AvailableResources.Add(starfall);
    
    // Build lookup map
    for (int32 i = 0; i < AvailableResources.Num(); ++i)
    {
        ResourceNameToIndex.Add(AvailableResources[i].Name, i);
    }
}

FResourceData UResourceManager::GetResourceByName(const FString& ResourceName, bool& bFound) const
{
    bFound = false;
    
    // Check if resource exists in our lookup
    if (ResourceNameToIndex.Contains(ResourceName))
    {
        int32 index = ResourceNameToIndex[ResourceName];
        bFound = true;
        return AvailableResources[index];
    }
    
    // Return empty resource if not found
    return FResourceData();
}

TArray<FResourceData> UResourceManager::GetResourcesByRarity(EResourceRarity Rarity) const
{
    TArray<FResourceData> result;
    
    for (const FResourceData& resource : AvailableResources)
    {
        if (resource.Rarity == Rarity)
        {
            result.Add(resource);
        }
    }
    
    return result;
}

int32 UResourceManager::GetResourceBaseValue(const FString& ResourceName) const
{
    bool bFound = false;
    FResourceData resource = GetResourceByName(ResourceName, bFound);
    
    if (bFound)
    {
        return resource.BaseValue;
    }
    
    return 0;
}

void UResourceManager::AddResource(const FResourceData& NewResource)
{
    // Check if resource already exists
    bool bFound = false;
    GetResourceByName(NewResource.Name, bFound);
    
    if (bFound)
    {
        UE_LOG(LogTemp, Warning, TEXT("Resource %s already exists, not adding duplicate"), *NewResource.Name);
        return;
    }
    
    // Add new resource
    int32 newIndex = AvailableResources.Add(NewResource);
    ResourceNameToIndex.Add(NewResource.Name, newIndex);
    
    UE_LOG(LogTemp, Log, TEXT("Added new resource: %s (Rarity: %s, Value: %d)"), 
           *NewResource.Name, 
           *GetEnumValueAsString<EResourceRarity>("EResourceRarity", NewResource.Rarity), 
           NewResource.BaseValue);
}

void UResourceManager::LoadResourcesFromDataTable(UDataTable* ResourceTable)
{
    if (!ResourceTable)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid Resource Data Table provided"));
        return;
    }
    
    // Clear existing resources
    AvailableResources.Empty();
    ResourceNameToIndex.Empty();
    
    // Load resources from data table
    TArray<FName> rowNames = ResourceTable->GetRowNames();
    for (const FName& rowName : rowNames)
    {
        FResourceTableRow* row = ResourceTable->FindRow<FResourceTableRow>(rowName, TEXT("ResourceManager::LoadResourcesFromDataTable"));
        if (row)
        {
            FResourceData resource;
            resource.Name = row->Name;
            resource.Rarity = row->Rarity;
            resource.Description = row->Description;
            resource.BaseValue = row->BaseValue;
            resource.Icon = row->Icon;
            
            // Add to available resources
            int32 newIndex = AvailableResources.Add(resource);
            ResourceNameToIndex.Add(resource.Name, newIndex);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Loaded %d resources from data table"), AvailableResources.Num());
}

// Helper function to convert enum values to strings
template<typename TEnum>
FString UResourceManager::GetEnumValueAsString(const FString& EnumName, TEnum Value)
{
    UEnum* Enum = FindObject<UEnum>(ANY_PACKAGE, *EnumName, true);
    if (!Enum)
    {
        return FString("Invalid");
    }
    return Enum->GetNameStringByValue(static_cast<int64>(Value));
}