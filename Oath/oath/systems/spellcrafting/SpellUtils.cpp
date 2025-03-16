#include "SpellUtils.hpp"

SpellEffectType stringToEffectType(const std::string& typeStr)
{
    if (typeStr == "damage")
        return SpellEffectType::Damage;
    if (typeStr == "healing")
        return SpellEffectType::Healing;
    if (typeStr == "protection")
        return SpellEffectType::Protection;
    if (typeStr == "control")
        return SpellEffectType::Control;
    if (typeStr == "alteration")
        return SpellEffectType::Alteration;
    if (typeStr == "conjuration")
        return SpellEffectType::Conjuration;
    if (typeStr == "illusion")
        return SpellEffectType::Illusion;
    if (typeStr == "divination")
        return SpellEffectType::Divination;

    // Default
    return SpellEffectType::Damage;
}

SpellDeliveryMethod stringToDeliveryMethod(const std::string& methodStr)
{
    if (methodStr == "touch")
        return SpellDeliveryMethod::Touch;
    if (methodStr == "projectile")
        return SpellDeliveryMethod::Projectile;
    if (methodStr == "area_of_effect")
        return SpellDeliveryMethod::AreaOfEffect;
    if (methodStr == "self")
        return SpellDeliveryMethod::Self;
    if (methodStr == "ray")
        return SpellDeliveryMethod::Ray;
    if (methodStr == "rune")
        return SpellDeliveryMethod::Rune;

    // Default
    return SpellDeliveryMethod::Touch;
}

SpellTargetType stringToTargetType(const std::string& targetStr)
{
    if (targetStr == "single_target")
        return SpellTargetType::SingleTarget;
    if (targetStr == "multi_target")
        return SpellTargetType::MultiTarget;
    if (targetStr == "self")
        return SpellTargetType::Self;
    if (targetStr == "allies_only")
        return SpellTargetType::AlliesOnly;
    if (targetStr == "enemies_only")
        return SpellTargetType::EnemiesOnly;
    if (targetStr == "area_effect")
        return SpellTargetType::AreaEffect;

    // Default
    return SpellTargetType::SingleTarget;
}

std::string effectTypeToString(SpellEffectType type)
{
    switch (type) {
    case SpellEffectType::Damage:
        return "damage";
    case SpellEffectType::Healing:
        return "healing";
    case SpellEffectType::Protection:
        return "protection";
    case SpellEffectType::Control:
        return "control";
    case SpellEffectType::Alteration:
        return "alteration";
    case SpellEffectType::Conjuration:
        return "conjuration";
    case SpellEffectType::Illusion:
        return "illusion";
    case SpellEffectType::Divination:
        return "divination";
    default:
        return "unknown";
    }
}

std::string deliveryMethodToString(SpellDeliveryMethod method)
{
    switch (method) {
    case SpellDeliveryMethod::Touch:
        return "touch";
    case SpellDeliveryMethod::Projectile:
        return "projectile";
    case SpellDeliveryMethod::AreaOfEffect:
        return "area_of_effect";
    case SpellDeliveryMethod::Self:
        return "self";
    case SpellDeliveryMethod::Ray:
        return "ray";
    case SpellDeliveryMethod::Rune:
        return "rune";
    default:
        return "unknown";
    }
}

std::string targetTypeToString(SpellTargetType type)
{
    switch (type) {
    case SpellTargetType::SingleTarget:
        return "single_target";
    case SpellTargetType::MultiTarget:
        return "multi_target";
    case SpellTargetType::Self:
        return "self";
    case SpellTargetType::AlliesOnly:
        return "allies_only";
    case SpellTargetType::EnemiesOnly:
        return "enemies_only";
    case SpellTargetType::AreaEffect:
        return "area_effect";
    default:
        return "unknown";
    }
}