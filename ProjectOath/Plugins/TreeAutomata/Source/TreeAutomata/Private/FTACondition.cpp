// FTACondition.cpp
#include "FTACondition.h"

//------------------------------------------------------------------------------
// FTACondition
//------------------------------------------------------------------------------

FTACondition::FTACondition()
    : ConditionID(FGuid::NewGuid())
    , ConditionName(TEXT("Base Condition"))
    , bInverted(false)
{
}

FTACondition::~FTACondition()
{
}

void FTACondition::Serialize(FArchive& Ar)
{
    Ar << ConditionID;
    Ar << ConditionName;
    Ar << bInverted;
}

//------------------------------------------------------------------------------
// FBooleanCondition
//------------------------------------------------------------------------------

FBooleanCondition::FBooleanCondition(bool bInValue)
    : FTACondition()
    , bValue(bInValue)
{
    ConditionName = TEXT("Boolean Condition");
}

bool FBooleanCondition::Evaluate(const FTAContext& Context) const
{
    return bInverted ? !bValue : bValue;
}

FString FBooleanCondition::GetDescription() const
{
    return FString::Printf(TEXT("Always returns %s"), bValue ? TEXT("true") : TEXT("false"));
}

TSharedPtr<FTACondition> FBooleanCondition::Clone() const
{
    TSharedPtr<FBooleanCondition> Clone = MakeShared<FBooleanCondition>(bValue);
    Clone->bInverted = bInverted;
    Clone->ConditionName = ConditionName;
    return Clone;
}

void FBooleanCondition::Serialize(FArchive& Ar)
{
    Super::Serialize(Ar);
    Ar << bValue;
}

//------------------------------------------------------------------------------
// FVariableCondition
//------------------------------------------------------------------------------

FVariableCondition::FVariableCondition()
    : FTACondition()
    , VariableName(TEXT(""))
    , bUseGlobalState(false)
    , ComparisonType(EComparisonOperator::Equal)
{
    ConditionName = TEXT("Variable Condition");
}

bool FVariableCondition::Evaluate(const FTAContext& Context) const
{
    // Get the variable value
    FTAVariant Value;
    
    if (bUseGlobalState)
    {
        const FTAVariant* FoundValue = Context.GlobalState.Find(VariableName);
        if (FoundValue)
        {
            Value = *FoundValue;
        }
        else
        {
            // Variable not found
            return bInverted;
        }
    }
    else
    {
        // This would use node state in a real implementation
        // For this example, fallback to input parameters
        const FTAVariant* FoundValue = Context.InputParams.Find(VariableName);
        if (FoundValue)
        {
            Value = *FoundValue;
        }
        else
        {
            // Variable not found
            return bInverted;
        }
    }
    
    // Compare the values
    bool Result = CompareVariants(Value, CompareValue);
    return bInverted ? !Result : Result;
}

FString FVariableCondition::GetDescription() const
{
    FString Source = bUseGlobalState ? TEXT("Global") : TEXT("Node");
    FString OperatorStr;
    
    switch (ComparisonType)
    {
        case EComparisonOperator::Equal:
            OperatorStr = TEXT("==");
            break;
        case EComparisonOperator::NotEqual:
            OperatorStr = TEXT("!=");
            break;
        case EComparisonOperator::Greater:
            OperatorStr = TEXT(">");
            break;
        case EComparisonOperator::GreaterEqual:
            OperatorStr = TEXT(">=");
            break;
        case EComparisonOperator::Less:
            OperatorStr = TEXT("<");
            break;
        case EComparisonOperator::LessEqual:
            OperatorStr = TEXT("<=");
            break;
        default:
            OperatorStr = TEXT("?");
            break;
    }
    
    return FString::Printf(TEXT("%s variable '%s' %s %s"), 
        *Source, 
        *VariableName, 
        *OperatorStr, 
        *CompareValue.ToString());
}

TSharedPtr<FTACondition> FVariableCondition::Clone() const
{
    TSharedPtr<FVariableCondition> Clone = MakeShared<FVariableCondition>();
    Clone->bInverted = bInverted;
    Clone->ConditionName = ConditionName;
    Clone->VariableName = VariableName;
    Clone->bUseGlobalState = bUseGlobalState;
    Clone->CompareValue = CompareValue;
    Clone->ComparisonType = ComparisonType;
    return Clone;
}

void FVariableCondition::Serialize(FArchive& Ar)
{
    Super::Serialize(Ar);
    Ar << VariableName;
    Ar << bUseGlobalState;
    
    // CompareValue serialization would depend on FTAVariant implementation
    
    int32 OpInt = (int32)ComparisonType;
    Ar << OpInt;
    ComparisonType = (EComparisonOperator)OpInt;
}

bool FVariableCondition::CompareVariants(const FTAVariant& A, const FTAVariant& B) const
{
    // Type checking and conversion would be needed in a real implementation
    // Simplified for this example
    
    switch (ComparisonType)
    {
        case EComparisonOperator::Equal:
            return A == B;
        case EComparisonOperator::NotEqual:
            return A != B;
        case EComparisonOperator::Greater:
            return A.AsNumber() > B.AsNumber();
        case EComparisonOperator::GreaterEqual:
            return A.AsNumber() >= B.AsNumber();
        case EComparisonOperator::Less:
            return A.AsNumber() < B.AsNumber();
        case EComparisonOperator::LessEqual:
            return A.AsNumber() <= B.AsNumber();
        default:
            return false;
    }
}

//------------------------------------------------------------------------------
// FInputCondition
//------------------------------------------------------------------------------

FInputCondition::FInputCondition(const FString& InInputID)
    : FTACondition()
    , InputID(InInputID)
{
    ConditionName = TEXT("Input Condition");
}

bool FInputCondition::Evaluate(const FTAContext& Context) const
{
    bool Result = (Context.InputID == InputID);
    return bInverted ? !Result : Result;
}

FString FInputCondition::GetDescription() const
{
    return FString::Printf(TEXT("Input ID == '%s'"), *InputID);
}

TSharedPtr<FTACondition> FInputCondition::Clone() const
{
    TSharedPtr<FInputCondition> Clone = MakeShared<FInputCondition>(InputID);
    Clone->bInverted = bInverted;
    Clone->ConditionName = ConditionName;
    return Clone;
}

void FInputCondition::Serialize(FArchive& Ar)
{
    Super::Serialize(Ar);
    Ar << InputID;
}