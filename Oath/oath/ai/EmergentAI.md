# Emergent AI System Design Document

## Overview

This document outlines the architecture and functionality of a Emergent AI system inspired by Oblivion's original `Radiant AI` design. The system creates believable NPC behaviors through a need-based utility system where characters make decisions based on their current needs, schedule, and environmental factors.

## Core Components

### 1. Need-Based Utility System

NPCs have various needs (hunger, thirst, sleep, social, etc.) that decay over time and motivate their behavior:

- Each need has a priority level (LOW, MEDIUM, HIGH, CRITICAL)
- Needs with lower satisfaction levels become higher priority
- Actions are evaluated based on how well they satisfy the most pressing needs

### 2. Action Selection Mechanism

NPCs select actions using a utility-based approach:
- Each action calculates a utility score based on NPC needs
- Higher priority needs have greater influence on the score
- NPCs select and perform the action with the highest utility
- Actions have duration and need effects (positive or negative)

### 3. Schedule System

NPCs follow daily schedules that override their need-based decision making:
- Each schedule entry specifies a location, action, and time range
- Priority-based scheduling for conflict resolution
- NPCs interrupt their need-based activities to follow their schedule
- Schedules can be modified based on events or world state

### 4. JSON Configuration

All AI data is stored in external JSON files for easy modification:
- NPC definitions (needs, stats, skills, inventory, schedule)
- Action definitions (effects, requirements, duration)
- World data (locations, factions, items)
- System parameters and constants

## Game System Integration

The Emergent AI connects with various game systems:

### Quest System
- NPCs can participate in quests as givers, targets, or participants
- Quest actions modify NPC behavior
- Quest state affects NPC schedules and priorities

### Dialogue System
- NPCs engage in contextual conversations
- Dialogue options affected by relationships and faction standing
- Conversations can trigger other systems (quests, trading, etc.)

### Character Progression System
- NPCs have statistics and skills that improve over time
- Skills affect action efficiency and availability
- NPCs can learn new abilities

### Crafting System
- NPCs craft items based on their skills and available resources
- Crafting affects economy and item availability
- Crafting stations limit where activities can occur

### Time System
- Day/night cycle affects NPC schedules
- Seasonal changes influence behaviors and events
- Weather affects travel and outdoor activities

### Weather System
- Multiple weather states (clear, cloudy, rainy, stormy, snowy)
- Weather influences NPC behaviors and preferences
- Region-specific weather patterns

### Crime System
- NPCs react to and report criminal activity
- Guard NPCs enforce laws with variable effectiveness
- Bounty and punishment mechanics

### Health System
- NPCs can become sick or injured
- Health affects performance and available actions
- Healing through rest, items, or other NPCs

### Economy System
- Dynamic pricing based on supply and demand
- NPCs participate in market activities (buying, selling, trading)
- Economic events affect the whole system

### Faction System
- NPCs belong to various factions
- Faction relationships affect individual NPC interactions
- Political shifts based on events and player actions

### NPC Relationship System
- NPCs form relationships (friendship, rivalry, romance)
- Relationship values affect interactions and dialogue
- Relationship networks create complex social dynamics

### Religion System
- NPCs worship deities and perform rituals
- Religious activities satisfy spiritual needs
- Temples and sacred locations for worship

### Spell Crafting System
- NPCs research and cast spells
- Magical effects interact with other systems
- Spell components and discovery mechanics

## NPC Implementation

### Base NPC Class
- Maintains needs, stats, and current action
- Updates needs over time
- Selects and performs actions

### Extended NPC Class
- Adds inventory management
- Skill system with improvement over time
- Faction standings and relationships
- Schedule management
- Quest tracking

## Action Types

The system implements various specialized actions:

- **QuestAction**: Actions related to quest completion
- **DialogueAction**: Conversation-based actions
- **CraftingAction**: Creating items at crafting stations
- **LocationAction**: Activities tied to specific locations
- **SocialAction**: Interactions between NPCs
- **ReligiousAction**: Worship and ritual activities
- **EconomicAction**: Trading, buying, and selling

## Implementation Architecture

The system uses a modular, component-based architecture:

### GameContext
- Central manager for all game systems
- Provides access to NPCs and actions
- Coordinates system updates
- Handles serialization and persistence

### GameSystem Base Class
- Common interface for all game systems
- Update, serialization, and configuration methods
- Integration with other systems

## Extensibility

The system is designed for easy extension:

- New need types can be added by extending the Need class
- Custom actions are created by deriving from the Action base class
- System behaviors can be modified through JSON configuration
- New game systems can be integrated by implementing the GameSystem interface

## Example Usage

A typical NPC in this system might:

1. Wake up in the morning based on their schedule
2. Eat breakfast to satisfy hunger and thirst needs
3. Travel to their workplace (blacksmith, bakery, etc.)
4. Perform work actions that generate wealth but decrease other needs
5. Take lunch when hunger becomes a high priority
6. Return to work in the afternoon
7. Visit the tavern in the evening for social needs
8. Return home to sleep when their sleep need becomes critical

All of these actions would be selected either by the schedule system or by evaluating the utility of available actions based on the NPC's current needs.

## Technical Requirements

- C++ implementation with object-oriented design
- JSON for external configuration using nlohmann/json library
- Save/load functionality for persisting state

## Conclusion

This Emergent AI system creates believable, dynamic NPC behaviors through a combination of need-based utility, scheduling, and integration with multiple game systems. The result is a living world where NPCs make contextual decisions based on their individual needs, abilities, and the state of the game world.