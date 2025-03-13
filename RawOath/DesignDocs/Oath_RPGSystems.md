# Oath_RPGSystems: Comprehensive RPG Game Systems

Oath_RPGSystems is a C++ implementation of interconnected RPG game systems using a tree automata architecture.

## Core Architecture

The program is built around a tree automata structure where:

- **TANode** - Base node class that represents game states
- **NodeID** - Unique identifier system with persistent IDs
- **TAController** - Central manager that handles transitions between states and system coordination
- **GameContext** - Global state containing player and world information

## Game Systems

### 1. Quest System
- Hierarchical quest structure with main quests and sub-quests
- Quest requirements based on skills, items, or reputation
- Quest rewards (experience, items, faction reputation)
- Quest states (Available, Active, Completed, Failed)
- Quest tracking via journal

Example implemented quest: "Defend the Village" with three sub-quests:
- Repair the village walls
- Train the militia
- Gather supplies

### 2. Dialogue System
- Branching dialogue trees
- Character responses based on player stats and knowledge
- Dialogue history tracking
- NPC relationship system
- Dialogue-triggered effects (quest activation, reputation changes)

### 3. Character Progression System
- Skill tree with hierarchical abilities
- Class specializations (Warrior, Ranger, Alchemist) 
- Skill requirements and prerequisites
- Stat improvements from skills
- Special ability unlocks

Skill branches implemented:
- Combat (basics, swordsmanship, archery)
- Survival (basics, herbalism, tracking)
- Crafting (basics, blacksmithing, alchemy)

### 4. Crafting System
- Crafting stations (blacksmith, alchemy, cooking)
- Recipe discovery
- Ingredient requirements
- Skill requirements for crafting
- Item properties for crafted items

Examples of craftable items:
- Iron Sword (blacksmithing)
- Leather Armor (crafting)
- Minor Healing Potion (alchemy)
- Hearty Stew (cooking)

### 5. World Progression System
- Geographic regions with connections
- Locations within regions
- Location states that change with story progression
- NPCs tied to locations
- Location-specific activities
- Access requirements for certain areas

World structure:
- Oakvale Village Region (Village Center, Inn, Blacksmith's Forge)
- Green Haven Forest (Forest Clearing, Ancient Groves)
- Stone Peak Mountains (Mountain Pass, Abandoned Mine)

### 6. Time System
- Day/night cycle
- Seasonal changes
- Time-based events
- Waiting/resting mechanics

## Save/Load System

A robust save/load system that:
- Uses persistent IDs for reliable object tracking
- Serializes and deserializes all game state
- Includes comprehensive error handling
- Validates save files with integrity checks
- Handles version compatibility

The system saves:
- Current node positions in all systems
- Character stats and skills
- World states and flags
- Inventory contents
- Quest journal
- Dialogue history

## Program Flow

The demonstration in `main()` shows:
1. System initialization and setup
2. World exploration (village region → village center)
3. NPC dialogue with Elder Marius to receive the main quest
4. Crafting an iron sword at the blacksmith
5. Skill progression (combat basics → swordsmanship)
6. Quest progression (completing the wall repair sub-quest)
7. Time passage simulation
8. State saving and loading demonstration

## Technical Features
- Extensive error handling throughout load/save operations
- Type-safe variant-based property system
- Hierarchical node relationships
- Clean separation of systems through interfaces
- File I/O validation with integrity checks
- Memory management for dynamically created nodes
