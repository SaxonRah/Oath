// TATypes.h
#pragma once

#include "CoreMinimal.h"
#include "TATypes.generated.h"

/**
 * Type enum for variant values
 */
UENUM(BlueprintType)
enum class EVariantType : uint8
{
    Null,
    Integer,
    Float,
    Boolean,
    String,
    Vector,
    Rotator,
    Pointer
};

/**
 * Variant data type for flexible data storage
 */
USTRUCT(BlueprintType)
struct TREEAUTOMATA_API FTAVariant
{
    GENERATED_BODY()

    // Constructors for different types
    FTAVariant() : Type(EVariantType::Null), IntValue(0) {}
    FTAVariant(int32 InValue) : Type(EVariantType::Integer), IntValue(InValue) {}
    FTAVariant(float InValue) : Type(EVariantType::Float), FloatValue(InValue) {}
    FTAVariant(bool InValue) : Type(EVariantType::Boolean), BoolValue(InValue) {}
    FTAVariant(const FString& InValue) : Type(EVariantType::String), StringValue(InValue) {}
    FTAVariant(const FVector& InValue) : Type(EVariantType::Vector), VectorValue(InValue) {}
    FTAVariant(const FRotator& InValue) : Type(EVariantType::Rotator), RotatorValue(InValue) {}
    FTAVariant(void* InValue) : Type(EVariantType::Pointer), PtrValue(InValue) {}

    // Type of this variant
    UPROPERTY(BlueprintReadOnly, Category = "Variant")
    EVariantType Type;

    // Value storage (use union to save memory)
    union {
        int32 IntValue;
        float FloatValue;
        bool BoolValue;
        void* PtrValue;
    };

    // Non-POD types need to be outside the union
    UPROPERTY()
    FString StringValue;

    UPROPERTY()
    FVector VectorValue;

    UPROPERTY()
    FRotator RotatorValue;

    // Value accessors
    int32 AsInt() const { return Type == EVariantType::Integer ? IntValue : 0; }
    float AsFloat() const { return Type == EVariantType::Float ? FloatValue : 0.0f; }
    float AsNumber() const { return Type == EVariantType::Integer ? (float)IntValue : (Type == EVariantType::Float ? FloatValue : 0.0f); }
    bool AsBool() const { return Type == EVariantType::Boolean ? BoolValue : false; }
    FString AsString() const { return Type == EVariantType::String ? StringValue : FString(); }
    FVector AsVector() const { return Type == EVariantType::Vector ? VectorValue : FVector::ZeroVector; }
    FRotator AsRotator() const { return Type == EVariantType::Rotator ? RotatorValue : FRotator::ZeroRotator; }
    void* AsPtr() const { return Type == EVariantType::Pointer ? PtrValue : nullptr; }

    // Type checking
    template<typename T>
    bool IsType() const { return false; } // Base case defaults to false

    // String conversion
    FString ToString() const;

    // Comparison operators
    bool operator==(const FTAVariant& Other) const;
    bool operator!=(const FTAVariant& Other) const { return !(*this == Other); }

    // Serialization
    friend FArchive& operator<<(FArchive& Ar, FTAVariant& Variant);
};

// Template specializations for IsType
template<> inline bool FTAVariant::IsType<int32>() const { return Type == EVariantType::Integer; }
template<> inline bool FTAVariant::IsType<float>() const { return Type == EVariantType::Float; }
template<> inline bool FTAVariant::IsType<bool>() const { return Type == EVariantType::Boolean; }
template<> inline bool FTAVariant::IsType<FString>() const { return Type == EVariantType::String; }
template<> inline bool FTAVariant::IsType<FVector>() const { return Type == EVariantType::Vector; }
template<> inline bool FTAVariant::IsType<FRotator>() const { return Type == EVariantType::Rotator; }
template<> inline bool FTAVariant::IsType<void*>() const { return Type == EVariantType::Pointer; }

/**
 * Enum for comparison operators
 */
UENUM(BlueprintType)
enum class EComparisonOperator : uint8
{
    Equal,
    NotEqual,
    Greater,
    GreaterEqual,
    Less,
    LessEqual
};

/**
 * Enum for actor spawn location
 */
UENUM(BlueprintType)
enum class EActorSpawnLocation : uint8
{
    PlayerLocation,
    PlayerViewpoint,
    CustomLocation
};

/**
 * Status of an action
 */
UENUM(BlueprintType)
enum class ETAActionStatus : uint8
{
    Available,
    Unavailable,
    Locked,
    Completed
};

/**
 * Record of a tree automata event
 */
USTRUCT(BlueprintType)
struct TREEAUTOMATA_API FTAEventRecord
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Tree Automata|Events")
    FString EventType;

    UPROPERTY(BlueprintReadOnly, Category = "Tree Automata|Events")
    TMap<FString, FTAVariant> EventData;

    UPROPERTY(BlueprintReadOnly, Category = "Tree Automata|Events")
    FDateTime Timestamp;
};

/**
 * Tree automata event listener
 */
USTRUCT()
struct TREEAUTOMATA_API FTAEventListener
{
    GENERATED_BODY()

    // Weak pointer to listener object
    TWeakObjectPtr<UObject> Listener;

    // Function to call
    FName FunctionName;
};

/**
 * Parameters for event callbacks
 */
USTRUCT(BlueprintType)
struct TREEAUTOMATA_API FTAEventParameters
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Tree Automata|Events")
    FString EventType;

    UPROPERTY(BlueprintReadOnly, Category = "Tree Automata|Events")
    TMap<FString, FTAVariant> EventData;
};

/**
 * Information about available actions
 */
USTRUCT(BlueprintType)
struct TREEAUTOMATA_API FTAActionInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Tree Automata")
    FString ActionID;

    UPROPERTY(BlueprintReadOnly, Category = "Tree Automata")
    FText DisplayName;

    UPROPERTY(BlueprintReadOnly, Category = "Tree Automata")
    FGuid TargetNodeID;

    UPROPERTY(BlueprintReadOnly, Category = "Tree Automata")
    FString TargetNodeName;

    UPROPERTY(BlueprintReadOnly, Category = "Tree Automata")
    FText Description;

    UPROPERTY(BlueprintReadOnly, Category = "Tree Automata")
    ETAActionStatus AvailabilityStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Tree Automata")
    TMap<FString, FTAVariant> AdditionalData;
};

/**
 * Node in a path through the automaton
 */
USTRUCT(BlueprintType)
struct TREEAUTOMATA_API FTAPathNode
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Tree Automata")
    FGuid NodeID;

    UPROPERTY(BlueprintReadOnly, Category = "Tree Automata")
    FString NodeName;

    UPROPERTY(BlueprintReadOnly, Category = "Tree Automata")
    FGuid TransitionID;

    UPROPERTY(BlueprintReadOnly, Category = "Tree Automata")
    FString TransitionName;

    UPROPERTY(BlueprintReadOnly, Category = "Tree Automata")
    TMap<FString, FTAVariant> NodeData;
};

/**
 * Path through an automaton
 */
USTRUCT(BlueprintType)
struct TREEAUTOMATA_API FTAPath
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Tree Automata")
    TArray<FTAPathNode> Nodes;

    UPROPERTY(BlueprintReadOnly, Category = "Tree Automata")
    float Cost;

    UPROPERTY(BlueprintReadOnly, Category = "Tree Automata")
    float Probability;
};

// Forward declarations of all major classes for header includes
class FTANode;
struct FTATransition;
struct FTACondition;
class FTAAction;
class UTAController;
class UTAInstance;
class UTAEventManager;