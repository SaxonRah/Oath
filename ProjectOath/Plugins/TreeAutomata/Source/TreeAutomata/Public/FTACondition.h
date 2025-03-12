// FTACondition.h
#pragma once

#include "CoreMinimal.h"
#include "TAContext.h"
#include "FTACondition.generated.h"

/**
 * Abstract base class for conditions
 */
class TREEAUTOMATA_API FTACondition
{
public:
    // Constructor
    FTACondition();
    
    // Destructor
    virtual ~FTACondition();
    
    // Evaluate if condition is met given the context
    virtual bool Evaluate(const FTAContext& Context) const = 0;
    
    // Description for editing and debugging
    virtual FString GetDescription() const = 0;
    
    // Clone this condition
    virtual TSharedPtr<FTACondition> Clone() const = 0;
    
    // Unique identifier
    FGuid ConditionID;
    
    // Condition name
    FString ConditionName;
    
    // Invert result
    bool bInverted;
    
    // Serialize the condition
    virtual void Serialize(FArchive& Ar);
};

/**
 * Boolean condition that always returns true or false
 */
class TREEAUTOMATA_API FBooleanCondition : public FTACondition
{
public:
    // Constructor
    FBooleanCondition(bool bInValue = true);
    
    // Value to return
    bool bValue;
    
    // Implementation
    virtual bool Evaluate(const FTAContext& Context) const override;
    virtual FString GetDescription() const override;
    virtual TSharedPtr<FTACondition> Clone() const override;
    virtual void Serialize(FArchive& Ar) override;
};

/**
 * Variable comparison condition
 */
class TREEAUTOMATA_API FVariableCondition : public FTACondition
{
public:
    // Constructor
    FVariableCondition();
    
    // Variable names
    FString VariableName;
    
    // Check global state instead of node state
    bool bUseGlobalState;
    
    // Value to compare against
    FTAVariant CompareValue;
    
    // Comparison operator
    EComparisonOperator ComparisonType;
    
    // Implementation
    virtual bool Evaluate(const FTAContext& Context) const override;
    virtual FString GetDescription() const override;
    virtual TSharedPtr<FTACondition> Clone() const override;
    virtual void Serialize(FArchive& Ar) override;
    
    // Compare two variants based on the operator
    bool CompareVariants(const FTAVariant& A, const FTAVariant& B) const;
};

/**
 * Input ID condition - checks if the current input matches
 */
class TREEAUTOMATA_API FInputCondition : public FTACondition
{
public:
    // Constructor
    FInputCondition(const FString& InInputID = TEXT(""));
    
    // Input ID to match
    FString InputID;
    
    // Implementation
    virtual bool Evaluate(const FTAContext& Context) const override;
    virtual FString GetDescription() const override;
    virtual TSharedPtr<FTACondition> Clone() const override;
    virtual void Serialize(FArchive& Ar) override;
};