// TATypes.cpp
#include "TATypes.h"

FString FTAVariant::ToString() const
{
    switch (Type)
    {
        case EVariantType::Integer:
            return FString::FromInt(IntValue);
        case EVariantType::Float:
            return FString::Printf(TEXT("%f"), FloatValue);
        case EVariantType::Boolean:
            return BoolValue ? TEXT("true") : TEXT("false");
        case EVariantType::String:
            return StringValue;
        case EVariantType::Vector:
            return VectorValue.ToString();
        case EVariantType::Rotator:
            return RotatorValue.ToString();
        case EVariantType::Pointer:
            return FString::Printf(TEXT("0x%p"), PtrValue);
        case EVariantType::Null:
        default:
            return TEXT("null");
    }
}

bool FTAVariant::operator==(const FTAVariant& Other) const
{
    // Different types are never equal
    if (Type != Other.Type)
        return false;
    
    // Compare based on type
    switch (Type)
    {
        case EVariantType::Integer:
            return IntValue == Other.IntValue;
        case EVariantType::Float:
            return FMath::IsNearlyEqual(FloatValue, Other.FloatValue);
        case EVariantType::Boolean:
            return BoolValue == Other.BoolValue;
        case EVariantType::String:
            return StringValue == Other.StringValue;
        case EVariantType::Vector:
            return VectorValue.Equals(Other.VectorValue);
        case EVariantType::Rotator:
            return RotatorValue.Equals(Other.RotatorValue);
        case EVariantType::Pointer:
            return PtrValue == Other.PtrValue;
        case EVariantType::Null:
            return true; // Null equals null
        default:
            return false;
    }
}

FArchive& operator<<(FArchive& Ar, FTAVariant& Variant)
{
    // Serialize type
    uint8 TypeValue = (uint8)Variant.Type;
    Ar << TypeValue;
    Variant.Type = (EVariantType)TypeValue;
    
    // Serialize value based on type
    switch (Variant.Type)
    {
        case EVariantType::Integer:
            Ar << Variant.IntValue;
            break;
        case EVariantType::Float:
            Ar << Variant.FloatValue;
            break;
        case EVariantType::Boolean:
            Ar << Variant.BoolValue;
            break;
        case EVariantType::String:
            Ar << Variant.StringValue;
            break;
        case EVariantType::Vector:
            Ar << Variant.VectorValue;
            break;
        case EVariantType::Rotator:
            Ar << Variant.RotatorValue;
            break;
        case EVariantType::Pointer:
            // We can't actually serialize pointers, so we store a null
            if (Ar.IsLoading())
            {
                Variant.PtrValue = nullptr;
            }
            break;
        case EVariantType::Null:
        default:
            // Nothing to serialize for null
            break;
    }
    
    return Ar;
}