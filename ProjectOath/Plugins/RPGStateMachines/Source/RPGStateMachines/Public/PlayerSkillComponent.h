#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerSkillComponent.generated.h"

USTRUCT(BlueprintType)
struct FSkillData
{
    GENERATED_BODY()
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 CurrentLevel = 0;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float CurrentExperience = 0.0f;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float ExperienceToNextLevel = 100.0f;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bIsUnlocked = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSkillLevelChanged, FName, SkillID, int32, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillUnlocked, FName, SkillID);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RPGSTATEMACHINES_API UPlayerSkillComponent : public UActorComponent
{
    GENERATED_BODY()

public:    
    UPlayerSkillComponent();

    virtual void BeginPlay() override;
    
    UFUNCTION(BlueprintCallable, Category = "Skills")
    int32 GetSkillLevel(FName SkillID) const;
    
    UFUNCTION(BlueprintCallable, Category = "Skills")
    bool IsSkillUnlocked(FName SkillID) const;
    
    UFUNCTION(BlueprintCallable, Category = "Skills")
    void AddExperience(FName SkillID, float ExperienceAmount);
    
    UFUNCTION(BlueprintCallable, Category = "Skills")
    void UnlockSkill(FName SkillID);
    
    UFUNCTION(BlueprintCallable, Category = "Skills")
    int32 GetAvailableSkillPoints() const;
    
    UFUNCTION(BlueprintCallable, Category = "Skills")
    void AddSkillPoints(int32 Points);
    
    UFUNCTION(BlueprintCallable, Category = "Skills")
    bool ImproveSkill(FName SkillID, int32 PointCost);
    
    UFUNCTION(BlueprintCallable, Category = "Skills")
    void SetAttemptingToImprove(FName SkillID, bool bIsAttempting);
    
    UFUNCTION(BlueprintCallable, Category = "Skills")
    bool IsAttemptingToImprove(FName SkillID) const;
    
    UFUNCTION(BlueprintCallable, Category = "Skills")
    void NotifySkillAvailable(FName SkillID);
    
    UPROPERTY(BlueprintAssignable, Category = "Skills")
    FOnSkillLevelChanged OnSkillLevelChanged;
    
    UPROPERTY(BlueprintAssignable, Category = "Skills")
    FOnSkillUnlocked OnSkillUnlocked;
    
private:
    UPROPERTY(VisibleAnywhere, Category = "Skills")
    TMap<FName, FSkillData> Skills;
    
    UPROPERTY(VisibleAnywhere, Category = "Skills")
    int32 AvailableSkillPoints = 0;
    
    UPROPERTY(VisibleAnywhere, Category = "Skills")
    FName SkillBeingImproved;
    
    float GetExperienceRequiredForLevel(FName SkillID, int32 Level) const;
};