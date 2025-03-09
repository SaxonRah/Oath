// ResourceNode.cpp
#include "ResourceNode.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"

AResourceNode::AResourceNode()
{
    PrimaryActorTick.bCanEverTick = false;
    
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
    
    // Default values
    HarvestAmount = 1;
    MaxHarvests = 3;
    RespawnTime = 300.0f; // 5 minutes
    ResourceType = EResourceType::None;
    CurrentHarvests = 0;
    bIsDepleted = false;
}

void AResourceNode::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize ResourceData based on ResourceType if not set
    if (ResourceData.Name.IsEmpty())
    {
        switch (ResourceType)
        {
            case EResourceType::Wood:
                ResourceData.Name = TEXT("Wood");
                ResourceData.Rarity = EResourceRarity::Common;
                ResourceData.BaseValue = 1;
                break;
                
            case EResourceType::Stone:
                ResourceData.Name = TEXT("Stone");
                ResourceData.Rarity = EResourceRarity::Common;
                ResourceData.BaseValue = 1;
                break;
                
            case EResourceType::Herb:
                ResourceData.Name = TEXT("Herb");
                ResourceData.Rarity = EResourceRarity::Common;
                ResourceData.BaseValue = 2;
                break;
                
            case EResourceType::Ore:
                ResourceData.Name = TEXT("Iron Ore");
                ResourceData.Rarity = EResourceRarity::Uncommon;
                ResourceData.BaseValue = 5;
                break;
                
            case EResourceType::Crystal:
                ResourceData.Name = TEXT("Mana Crystal");
                ResourceData.Rarity = EResourceRarity::Rare;
                ResourceData.BaseValue = 15;
                break;
                
            case EResourceType::Animal:
                ResourceData.Name = TEXT("Hide");
                ResourceData.Rarity = EResourceRarity::Common;
                ResourceData.BaseValue = 3;
                break;
                
            default:
                ResourceData.Name = TEXT("Unknown Resource");
                ResourceData.Rarity = EResourceRarity::Common;
                ResourceData.BaseValue = 1;
                break;
        }
    }
}

void AResourceNode::OnHarvested()
{
    CurrentHarvests++;
    
    // Play harvest effects
    if (HarvestEffect)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HarvestEffect, GetActorLocation(), GetActorRotation());
    }
    
    if (HarvestSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, HarvestSound, GetActorLocation());
    }
    
    // Check if resource is depleted
    if (CurrentHarvests >= MaxHarvests)
    {
        bIsDepleted = true;
        
        // Change appearance to show depletion
        MeshComponent->SetVisibility(false);
        
        // Call blueprint event
        OnResourceDepleted();
        
        // Start respawn timer
        GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &AResourceNode::Respawn, RespawnTime, false);
    }
}

void AResourceNode::Respawn()
{
    CurrentHarvests = 0;
    bIsDepleted = false;
    
    // Restore appearance
    MeshComponent->SetVisibility(true);
    
    // Maybe spawn a respawn effect here
    UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HarvestEffect, GetActorLocation(), GetActorRotation(), FVector(0.5f));
}