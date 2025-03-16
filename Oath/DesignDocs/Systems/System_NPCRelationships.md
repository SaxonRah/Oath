# Comprehensive NPC Relationship System for Oath

NPC Relationship System:
- Friendship, rivalry, romance states
- Gift preferences and conversation options
- Companion loyalty and mood tracking
- NPC schedules and routines
- Background relationship networks between NPCs

---

# Comprehensive NPC Relationship System for Oath

1. **Detailed NPC Profiles**
   - Personality traits that affect relationship dynamics
   - Gift preferences and conversation topics
   - Daily schedules and location tracking
   - Backstory elements and relationship histories

2. **Complex Relationship States**
   - Friendship, rivalry, romance, family bonds
   - Relationship values from -100 to 100
   - Multiple relationship types (Acquaintance through Spouse)
   - Temporary mood states (Happy, Angry, Grateful, etc.)

3. **NPC Schedules and Routines**
   - Time-based location tracking
   - Different weekday and weekend schedules
   - Activity-specific interactions

4. **Gift System**
   - Item category preferences
   - Favorite and disliked specific items
   - Gift reaction multipliers based on preferences
   - Time restrictions to prevent gift spamming

5. **Conversation System**
   - Topic preferences and taboo subjects
   - Personality-influenced dialogue options
   - Reputation-gated conversation options

6. **NPC Network Relationships**
   - NPCs have relationships with each other
   - Relationship compatibility based on personality traits
   - Supporting and opposing trait pairs

7. **Relationship Progression**
   - Natural relationship decay over time
   - Special relationship milestones (friend to romance to marriage)
   - Relationship requirements for different actions

The system features three main node types:
- `RelationshipBrowserNode` for viewing and filtering relationships
- `NPCInteractionNode` for direct NPC interaction 
- `NPCInfoNode` for viewing detailed NPC information

All of these work with the existing tree automata architecture, making it a plug-and-play addition to the game. The implementation is robust with proper save/load functionality and time-based updates.