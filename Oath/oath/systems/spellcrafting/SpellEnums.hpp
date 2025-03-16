#pragma once

// Define spell effect types
enum class SpellEffectType {
    Damage,
    Healing,
    Protection,
    Control,
    Alteration,
    Conjuration,
    Illusion,
    Divination
};

// Define delivery methods
enum class SpellDeliveryMethod {
    Touch,
    Projectile,
    AreaOfEffect,
    Self,
    Ray,
    Rune
};

// Define targeting types
enum class SpellTargetType {
    SingleTarget,
    MultiTarget,
    Self,
    AlliesOnly,
    EnemiesOnly,
    AreaEffect
};