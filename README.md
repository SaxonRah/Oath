# Flax Engine 
Uses Flax.
- https://flaxengine.com/
- https://github.com/FlaxEngine/FlaxEngine
  
# TODO List
- _TODO_Main.md
- _TODO_AI.md
- _TODO_CrimeLaw.md
- _TODO_Economy.md
- _TODO_Faction.md
- _TODO_Mount.md
- _TODO_Relationship.md
- _TODO_Religion.md
- _TODO_SpellCrafting.md

```
oath/
├─ ai/
│  ├─ _TODO_AI.md
│  ├─ EmergentAI.cpp
│  ├─ EmergentAI.hpp
│  ├─ EmergentAI.json
│  ├─ EmergentAI.md
│  ├─ EmergentNPC.cpp
│  └─ EmergentNPC.hpp
├─ core/
│  ├─ NodeID.cpp
│  ├─ NodeID.hpp
│  ├─ TAAction.hpp
│  ├─ TAController.cpp
│  ├─ TAController.hpp
│  ├─ TAInput.hpp
│  ├─ TANode.cpp
│  ├─ TANode.hpp
│  └─ TATransitionRule.hpp
├─ data/
│  ├─ CharacterStats.cpp
│  ├─ CharacterStats.hpp
│  ├─ GameContext.hpp
│  ├─ Inventory.cpp
│  ├─ Inventory.hpp
│  ├─ Item.cpp
│  ├─ Item.hpp
│  ├─ Recipe.cpp
│  ├─ Recipe.hpp
│  ├─ WorldState.cpp
│  └─ WorldState.hpp
├─ include/
│  └─ nlohmann/
│     └─ json.hpp
├─ resources/
│  ├─ config/
│  │  └─ Settings.json
│  ├─ json/
│  │  ├─ Crafting.json
│  │  ├─ CrimeLaw.json
│  │  ├─ Diseases.json
│  │  ├─ Economy.json
│  │  ├─ FactionReputation.json
│  │  ├─ Mount.json
│  │  ├─ NPCRelationships.json
│  │  ├─ NPCs.json
│  │  ├─ Quests.json
│  │  ├─ ReligionDeity.json
│  │  ├─ Skills.json
│  │  ├─ Weather.json
│  │  └─ World.json
│  └─ saves/
│     └─ save_001.json
├─ systems/
│  ├─ crafting/
│  │  ├─ CraftingNode.cpp
│  │  └─ CraftingNode.hpp
│  ├─ crime/
│  │  ├─ _TODO_CrimeLaw.md
│  │  ├─ BountyPaymentNode.cpp
│  │  ├─ BountyPaymentNode.hpp
│  │  ├─ CrimeLawConfig.cpp
│  │  ├─ CrimeLawConfig.hpp
│  │  ├─ CrimeLawContext.cpp
│  │  ├─ CrimeLawContext.hpp
│  │  ├─ CrimeLawSystem.cpp
│  │  ├─ CrimeLawSystem.hpp
│  │  ├─ CrimeRecord.cpp
│  │  ├─ CrimeRecord.hpp
│  │  ├─ CrimeSystemNode.cpp
│  │  ├─ CrimeSystemNode.hpp
│  │  ├─ CrimeType.cpp
│  │  ├─ CrimeType.hpp
│  │  ├─ CriminalRecord.cpp
│  │  ├─ CriminalRecord.hpp
│  │  ├─ GuardEncounterNode.cpp
│  │  ├─ GuardEncounterNode.hpp
│  │  ├─ GuardResponseType.cpp
│  │  ├─ GuardResponseType.hpp
│  │  ├─ JailNode.cpp
│  │  ├─ JailNode.hpp
│  │  ├─ PickpocketNode.cpp
│  │  ├─ PickpocketNode.hpp
│  │  ├─ TheftExecutionNode.cpp
│  │  ├─ TheftExecutionNode.hpp
│  │  ├─ TheftNode.cpp
│  │  └─ TheftNode.hpp
│  ├─ dialogue/
│  │  ├─ DialogueNode.cpp
│  │  ├─ DialogueNode.hpp
│  │  ├─ NPC.cpp
│  │  └─ NPC.hpp
│  ├─ economy/
│  │  ├─ _TODO_Economy.md
│  │  ├─ BusinessInvestment.cpp
│  │  ├─ BusinessInvestment.hpp
│  │  ├─ EconomicEvent.cpp
│  │  ├─ EconomicEvent.hpp
│  │  ├─ EconomicSystemNode.cpp
│  │  ├─ EconomicSystemNode.hpp
│  │  ├─ InvestmentNode.cpp
│  │  ├─ InvestmentNode.hpp
│  │  ├─ Market.cpp
│  │  ├─ Market.hpp
│  │  ├─ MarketNode.cpp
│  │  ├─ MarketNode.hpp
│  │  ├─ MarketTypes.cpp
│  │  ├─ MarketTypes.hpp
│  │  ├─ Property.cpp
│  │  ├─ Property.hpp
│  │  ├─ PropertyNode.cpp
│  │  ├─ PropertyNode.hpp
│  │  ├─ PropertyTypes.cpp
│  │  ├─ PropertyTypes.hpp
│  │  ├─ TradeCommodity.cpp
│  │  ├─ TradeCommodity.hpp
│  │  ├─ TradeRoute.cpp
│  │  └─ TradeRoute.hpp
│  ├─ faction/
│  │  ├─ _TODO_Faction.md
│  │  ├─ Faction.cpp
│  │  ├─ Faction.hpp
│  │  ├─ FactionDialogueNode.cpp
│  │  ├─ FactionDialogueNode.hpp
│  │  ├─ FactionLocationNode.cpp
│  │  ├─ FactionLocationNode.hpp
│  │  ├─ FactionQuestNode.cpp
│  │  ├─ FactionQuestNode.hpp
│  │  ├─ FactionRelationship.cpp
│  │  ├─ FactionRelationship.hpp
│  │  ├─ FactionSystemNode.cpp
│  │  ├─ FactionSystemNode.hpp
│  │  ├─ PoliticalEvent.cpp
│  │  └─ PoliticalEvent.hpp
│  ├─ health/
│  │  ├─ Disease.cpp
│  │  ├─ Disease.hpp
│  │  ├─ DiseaseManager.cpp
│  │  ├─ DiseaseManager.hpp
│  │  ├─ HealingMethod.cpp
│  │  ├─ HealingMethod.hpp
│  │  ├─ HealthContext.hpp
│  │  ├─ HealthNodes.cpp
│  │  ├─ HealthNodes.hpp
│  │  ├─ HealthSetup.cpp
│  │  ├─ HealthState.cpp
│  │  ├─ HealthState.hpp
│  │  ├─ Immunity.cpp
│  │  ├─ Immunity.hpp
│  │  ├─ NutritionNode.cpp
│  │  ├─ NutritionNode.hpp
│  │  ├─ NutritionState.cpp
│  │  ├─ NutritionState.hpp
│  │  ├─ Symptom.cpp
│  │  ├─ Symptom.hpp
│  │  └─ TODO_DiseaseHealth.md
│  ├─ mount/
│  │  ├─ _TODO_Mount.md
│  │  ├─ Mount.cpp
│  │  ├─ Mount.hpp
│  │  ├─ MountBreed.cpp
│  │  ├─ MountBreed.hpp
│  │  ├─ MountBreedingNode.cpp
│  │  ├─ MountBreedingNode.hpp
│  │  ├─ MountEquipment.cpp
│  │  ├─ MountEquipment.hpp
│  │  ├─ MountEquipmentShopNode.cpp
│  │  ├─ MountEquipmentShopNode.hpp
│  │  ├─ MountInteractionNode.cpp
│  │  ├─ MountInteractionNode.hpp
│  │  ├─ MountRacingNode.cpp
│  │  ├─ MountRacingNode.hpp
│  │  ├─ MountStable.cpp
│  │  ├─ MountStable.hpp
│  │  ├─ MountStableNode.cpp
│  │  ├─ MountStableNode.hpp
│  │  ├─ MountStats.cpp
│  │  ├─ MountStats.hpp
│  │  ├─ MountSystem.cpp
│  │  ├─ MountSystem.hpp
│  │  ├─ MountSystemConfig.cpp
│  │  ├─ MountSystemConfig.hpp
│  │  ├─ MountSystemController.cpp
│  │  ├─ MountSystemController.hpp
│  │  ├─ MountTrainingNode.cpp
│  │  ├─ MountTrainingNode.hpp
│  │  ├─ MountTrainingSession.cpp
│  │  └─ MountTrainingSession.hpp
│  ├─ progression/
│  │  ├─ ClassNode.cpp
│  │  ├─ ClassNode.hpp
│  │  ├─ SkillNode.cpp
│  │  └─ SkillNode.hpp
│  ├─ quest/
│  │  ├─ QuestNode.cpp
│  │  └─ QuestNode.hpp
│  ├─ relationship/
│  │  ├─ _TODO_Relationship.md
│  │  ├─ NPCInfoNode.cpp
│  │  ├─ NPCInfoNode.hpp
│  │  ├─ NPCInteractionNode.cpp
│  │  ├─ NPCInteractionNode.hpp
│  │  ├─ NPCRelationshipManager.cpp
│  │  ├─ NPCRelationshipManager.hpp
│  │  ├─ RelationshipBrowserNode.cpp
│  │  ├─ RelationshipBrowserNode.hpp
│  │  ├─ RelationshipConfig.cpp
│  │  ├─ RelationshipConfig.hpp
│  │  ├─ RelationshipNPC.cpp
│  │  ├─ RelationshipNPC.hpp
│  │  ├─ RelationshipSystemController.cpp
│  │  ├─ RelationshipSystemController.hpp
│  │  └─ RelationshipTypes.hpp
│  ├─ religion/
│  │  ├─ _TODO_Religion.md
│  │  ├─ BlessingNode.cpp
│  │  ├─ BlessingNode.hpp
│  │  ├─ DeityNode.cpp
│  │  ├─ DeityNode.hpp
│  │  ├─ PrayerNode.cpp
│  │  ├─ PrayerNode.hpp
│  │  ├─ ReligionController.cpp
│  │  ├─ ReligionController.hpp
│  │  ├─ ReligiousGameContext.cpp
│  │  ├─ ReligiousGameContext.hpp
│  │  ├─ ReligiousQuestNode.cpp
│  │  ├─ ReligiousQuestNode.hpp
│  │  ├─ ReligiousStats.cpp
│  │  ├─ ReligiousStats.hpp
│  │  ├─ RitualNode.cpp
│  │  ├─ RitualNode.hpp
│  │  ├─ TempleNode.cpp
│  │  └─ TempleNode.hpp
│  ├─ spellcrafting/
│  │  ├─ _TODO_SpellCrafting.md
│  │  ├─ MagicTrainingNode.cpp
│  │  ├─ MagicTrainingNode.hpp
│  │  ├─ SpellbookNode.cpp
│  │  ├─ SpellbookNode.hpp
│  │  ├─ SpellComponent.cpp
│  │  ├─ SpellComponent.hpp
│  │  ├─ SpellCraftingNode.cpp
│  │  ├─ SpellCraftingNode.hpp
│  │  ├─ SpellCraftingSystem.cpp
│  │  ├─ SpellCraftingSystem.hpp
│  │  ├─ SpellDelivery.cpp
│  │  ├─ SpellDelivery.hpp
│  │  ├─ SpellDesign.cpp
│  │  ├─ SpellDesign.hpp
│  │  ├─ SpellEnums.hpp
│  │  ├─ SpellExaminationNode.cpp
│  │  ├─ SpellExaminationNode.hpp
│  │  ├─ SpellModifier.cpp
│  │  ├─ SpellModifier.hpp
│  │  ├─ SpellResearch.hpp
│  │  ├─ SpellUtils.cpp
│  │  └─ SpellUtils.hpp
│  ├─ weather/
│  │  ├─ WeatherCondition.cpp
│  │  ├─ WeatherCondition.hpp
│  │  ├─ WeatherSystemNode.cpp
│  │  ├─ WeatherSystemNode.hpp
│  │  ├─ WeatherTypes.cpp
│  │  └─ WeatherTypes.hpp
│  └─ world/
│     ├─ LocationNode.cpp
│     ├─ LocationNode.hpp
│     ├─ RegionNode.cpp
│     ├─ RegionNode.hpp
│     ├─ TimeNode.cpp
│     └─ TimeNode.hpp
├─ utils/
│  ├─ JSONLoader.cpp
│  ├─ JSONLoader.hpp
│  ├─ JSONSerializer.cpp
│  └─ JSONSerializer.hpp
├─ _TODO_Main.md
├─ main.cpp
└─ new_main.cpp
```
