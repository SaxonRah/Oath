#include <JSONLoader.hpp>

// Functions to load game data from JSON
bool loadGameData(TAController& controller)
{
    try {
        // Make sure the data directory exists
        if (!std::filesystem::exists("data")) {
            std::filesystem::create_directory("data");
            std::cout << "Created data directory" << std::endl;
            createDefaultJSONFiles();
        }

        // Load quests
        std::ifstream questFile("data/quests.json");
        if (questFile.is_open()) {
            json questData = json::parse(questFile);
            loadQuestsFromJSON(controller, questData);
            std::cout << "Loaded quest data" << std::endl;
        } else {
            std::cerr << "Failed to open data/quests.json" << std::endl;
        }

        // Load NPCs and dialogue
        std::ifstream npcFile("data/npcs.json");
        if (npcFile.is_open()) {
            json npcData = json::parse(npcFile);
            loadNPCsFromJSON(controller, npcData);
            std::cout << "Loaded NPC data" << std::endl;
        } else {
            std::cerr << "Failed to open data/npcs.json" << std::endl;
        }

        // Load skills and progression
        std::ifstream skillsFile("data/skills.json");
        if (skillsFile.is_open()) {
            json skillsData = json::parse(skillsFile);
            loadSkillsFromJSON(controller, skillsData);
            std::cout << "Loaded skills data" << std::endl;
        } else {
            std::cerr << "Failed to open data/skills.json" << std::endl;
        }

        // Load crafting recipes
        std::ifstream craftingFile("data/crafting.json");
        if (craftingFile.is_open()) {
            json craftingData = json::parse(craftingFile);
            loadCraftingFromJSON(controller, craftingData);
            std::cout << "Loaded crafting data" << std::endl;
        } else {
            std::cerr << "Failed to open data/crafting.json" << std::endl;
        }

        // Load world data
        std::ifstream worldFile("data/world.json");
        if (worldFile.is_open()) {
            json worldData = json::parse(worldFile);
            loadWorldFromJSON(controller, worldData);
            std::cout << "Loaded world data" << std::endl;
        } else {
            std::cerr << "Failed to open data/world.json" << std::endl;
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading game data: " << e.what() << std::endl;
        return false;
    }
}

// Load quests from JSON
void loadQuestsFromJSON(TAController& controller, const json& questData)
{
    for (const auto& questEntry : questData["quests"]) {
        // Create main quest node
        QuestNode* quest = dynamic_cast<QuestNode*>(
            controller.createNode<QuestNode>(questEntry["id"]));

        quest->questTitle = questEntry["title"];
        quest->questDescription = questEntry["description"];
        quest->questState = questEntry["state"];
        quest->isAcceptingState = questEntry["isAcceptingState"];

        // Load rewards
        for (const auto& rewardData : questEntry["rewards"]) {
            QuestNode::QuestReward reward;
            reward.type = rewardData["type"];
            reward.amount = rewardData["amount"];
            reward.itemId = rewardData["itemId"];
            quest->rewards.push_back(reward);
        }

        // Load requirements
        for (const auto& reqData : questEntry["requirements"]) {
            QuestNode::QuestRequirement req;
            req.type = reqData["type"];
            req.target = reqData["target"];
            req.value = reqData["value"];
            quest->requirements.push_back(req);
        }

        // Create and link subquests
        std::map<std::string, QuestNode*> questNodes;
        questNodes[quest->nodeName] = quest;

        // First, create all subquest nodes
        for (const auto& subquestData : questEntry["subquests"]) {
            QuestNode* subquest = dynamic_cast<QuestNode*>(
                controller.createNode<QuestNode>(subquestData["id"]));

            subquest->questTitle = subquestData["title"];
            subquest->questDescription = subquestData["description"];
            subquest->questState = subquestData["state"];
            subquest->isAcceptingState = subquestData["isAcceptingState"];

            // Load rewards
            for (const auto& rewardData : subquestData["rewards"]) {
                QuestNode::QuestReward reward;
                reward.type = rewardData["type"];
                reward.amount = rewardData["amount"];
                reward.itemId = rewardData["itemId"];
                subquest->rewards.push_back(reward);
            }

            // Load requirements
            for (const auto& reqData : subquestData["requirements"]) {
                QuestNode::QuestRequirement req;
                req.type = reqData["type"];
                req.target = reqData["target"];
                req.value = reqData["value"];
                subquest->requirements.push_back(req);
            }

            questNodes[subquest->nodeName] = subquest;
            quest->addChild(subquest);
        }

        // Now set up transitions (after all nodes are created)
        for (const auto& subquestData : questEntry["subquests"]) {
            QuestNode* subquest = questNodes[subquestData["id"]];

            // Add transitions
            for (const auto& transData : subquestData["transitions"]) {
                std::string action = transData["action"];
                std::string targetId = transData["target"];
                std::string description = transData["description"];

                subquest->addTransition(
                    [action](const TAInput& input) {
                        return input.type == "action" && std::get<std::string>(input.parameters.at("name")) == action;
                    },
                    questNodes[targetId],
                    description);
            }
        }

        // Register the quest system
        if (questEntry["id"] == "MainQuest") {
            controller.setSystemRoot("QuestSystem", quest);
        }
    }
}

// Load NPCs and dialogue from JSON
void loadNPCsFromJSON(TAController& controller, const json& npcData)
{
    std::map<std::string, NPC*> npcs;
    std::map<std::string, DialogueNode*> dialogueNodes;

    for (const auto& npcEntry : npcData["npcs"]) {
        // Create NPC
        NPC* npc = new NPC(npcEntry["name"], npcEntry["description"]);
        npc->relationshipValue = npcEntry["relationshipValue"];

        // Create all dialogue nodes first
        for (const auto& dialogueData : npcEntry["dialogueNodes"]) {
            DialogueNode* dialogueNode = dynamic_cast<DialogueNode*>(
                controller.createNode<DialogueNode>(
                    dialogueData["id"],
                    dialogueData["speakerName"],
                    dialogueData["dialogueText"]));

            dialogueNodes[dialogueData["id"]] = dialogueNode;
            npc->dialogueNodes[dialogueData["id"]] = dialogueNode;
        }

        // Set root dialogue
        npc->rootDialogue = dialogueNodes[npcEntry["rootDialogue"]];

        // Now connect responses after all nodes are created
        for (const auto& dialogueData : npcEntry["dialogueNodes"]) {
            DialogueNode* currentNode = dialogueNodes[dialogueData["id"]];

            for (const auto& responseData : dialogueData["responses"]) {
                // Create response function for requirements and effects
                std::function<bool(const GameContext&)> reqFunc = [](const GameContext&) { return true; };

                if (responseData.contains("requirements") && !responseData["requirements"].empty()) {
                    reqFunc = [responseData](const GameContext& ctx) {
                        for (const auto& req : responseData["requirements"]) {
                            // Implement requirement checking based on req type
                            if (req["type"] == "skill") {
                                if (!ctx.playerStats.hasSkill(req["skill"], req["level"])) {
                                    return false;
                                }
                            } else if (req["type"] == "item") {
                                if (!ctx.playerInventory.hasItem(req["item"], req["amount"])) {
                                    return false;
                                }
                            }
                            // Add other requirement types as needed
                        }
                        return true;
                    };
                }

                std::function<void(GameContext*)> effectFunc = [](GameContext*) {};

                if (responseData.contains("effects") && !responseData["effects"].empty()) {
                    effectFunc = [responseData](GameContext* ctx) {
                        for (const auto& effect : responseData["effects"]) {
                            // Implement effects based on type
                            if (effect["type"] == "quest" && effect["action"] == "activate") {
                                ctx->questJournal[effect["target"]] = "Active";
                                std::cout << "Quest activated: " << effect["target"] << std::endl;
                            } else if (effect["type"] == "knowledge" && effect["action"] == "add") {
                                ctx->playerStats.learnFact(effect["target"]);
                            } else if (effect["type"] == "faction" && effect["action"] == "change") {
                                ctx->playerStats.changeFactionRep(effect["target"], effect["amount"]);
                            }
                            // Add other effect types as needed
                        }
                    };
                }

                // Add the response with its target, requirements, and effects
                currentNode->addResponse(
                    responseData["text"],
                    dialogueNodes[responseData["targetNode"]],
                    reqFunc,
                    effectFunc);
            }
        }

        npcs[npcEntry["id"]] = npc;
    }

    // Create a dialogue controller node
    TANode* dialogueControllerNode = controller.createNode("DialogueController");
    controller.setSystemRoot("DialogueSystem", dialogueControllerNode);

    // Store NPCs for later reference
    controller.gameData["npcs"] = npcs;
}

// Load skills and progression from JSON
void loadSkillsFromJSON(TAController& controller, const json& skillsData)
{
    // Create skill tree root
    TANode* skillTreeRoot = controller.createNode("SkillTreeRoot");

    std::map<std::string, SkillNode*> skillNodes;

    // First pass: create all skill nodes
    for (const auto& skillEntry : skillsData["skills"]) {
        SkillNode* skillNode = dynamic_cast<SkillNode*>(
            controller.createNode<SkillNode>(
                skillEntry["id"],
                skillEntry["skillName"],
                skillEntry.value("level", 0),
                skillEntry.value("maxLevel", 5)));

        skillNode->description = skillEntry["description"];

        // Load requirements
        for (const auto& reqData : skillEntry["requirements"]) {
            SkillNode::SkillRequirement req;
            req.type = reqData["type"];
            req.target = reqData["target"];
            req.level = reqData["level"];
            skillNode->requirements.push_back(req);
        }

        // Load effects
        for (const auto& effectData : skillEntry["effects"]) {
            SkillNode::SkillEffect effect;
            effect.type = effectData["type"];
            effect.target = effectData["target"];
            effect.value = effectData["value"];
            skillNode->effects.push_back(effect);
        }

        // Load costs if any
        if (skillEntry.contains("costs")) {
            for (const auto& costData : skillEntry["costs"]) {
                SkillNode::SkillCost cost;
                cost.type = costData["type"];
                if (costData.contains("itemId")) {
                    cost.itemId = costData["itemId"];
                }
                cost.amount = costData["amount"];
                skillNode->costs.push_back(cost);
            }
        }

        skillNodes[skillEntry["id"]] = skillNode;
        skillTreeRoot->addChild(skillNode);
    }

    // Second pass: connect child skills
    for (const auto& skillEntry : skillsData["skills"]) {
        if (skillEntry.contains("childSkills")) {
            for (const auto& childData : skillEntry["childSkills"]) {
                SkillNode* childNode = dynamic_cast<SkillNode*>(
                    controller.createNode<SkillNode>(
                        childData["id"],
                        childData["skillName"],
                        childData.value("level", 0),
                        childData.value("maxLevel", 5)));

                childNode->description = childData["description"];

                // Load requirements
                for (const auto& reqData : childData["requirements"]) {
                    SkillNode::SkillRequirement req;
                    req.type = reqData["type"];
                    req.target = reqData["target"];
                    req.level = reqData["level"];
                    childNode->requirements.push_back(req);
                }

                // Load effects
                for (const auto& effectData : childData["effects"]) {
                    SkillNode::SkillEffect effect;
                    effect.type = effectData["type"];
                    effect.target = effectData["target"];
                    effect.value = effectData["value"];
                    childNode->effects.push_back(effect);
                }

                skillNodes[childData["id"]] = childNode;
                skillNodes[skillEntry["id"]]->addChild(childNode);
            }
        }
    }

    // Create character classes
    TANode* classSelectionNode = controller.createNode("ClassSelection");

    for (const auto& classEntry : skillsData["classes"]) {
        ClassNode* classNode = dynamic_cast<ClassNode*>(
            controller.createNode<ClassNode>(
                classEntry["id"],
                classEntry["className"]));

        classNode->description = classEntry["description"];

        // Load stat bonuses
        for (const auto& [stat, bonus] : classEntry["statBonuses"].items()) {
            classNode->statBonuses[stat] = bonus;
        }

        // Load starting abilities
        for (const auto& ability : classEntry["startingAbilities"]) {
            classNode->startingAbilities.insert(ability);
        }

        // Link class skills
        for (const auto& skillId : classEntry["classSkills"]) {
            if (skillNodes.count(skillId)) {
                classNode->classSkills.push_back(skillNodes[skillId]);
            }
        }

        classSelectionNode->addChild(classNode);
    }

    // Register systems
    controller.setSystemRoot("ProgressionSystem", skillTreeRoot);
    controller.setSystemRoot("ClassSystem", classSelectionNode);
}

// Load crafting from JSON
void loadCraftingFromJSON(TAController& controller, const json& craftingData)
{
    TANode* craftingRoot = controller.createNode("CraftingRoot");

    for (const auto& stationData : craftingData["craftingStations"]) {
        CraftingNode* station = dynamic_cast<CraftingNode*>(
            controller.createNode<CraftingNode>(
                stationData["id"],
                stationData["stationType"]));

        station->description = stationData["description"];

        // Load recipes
        for (const auto& recipeData : stationData["recipes"]) {
            Recipe recipe(recipeData["recipeId"], recipeData["name"]);
            recipe.description = recipeData["description"];
            recipe.discovered = recipeData["discovered"];

            // Load ingredients
            for (const auto& ingredientData : recipeData["ingredients"]) {
                Recipe::Ingredient ingredient;
                ingredient.itemId = ingredientData["itemId"];
                ingredient.quantity = ingredientData["quantity"];
                recipe.ingredients.push_back(ingredient);
            }

            // Load skill requirements
            for (const auto& [skill, level] : recipeData["skillRequirements"].items()) {
                recipe.skillRequirements[skill] = level;
            }

            // Load result
            recipe.result.itemId = recipeData["result"]["itemId"];
            recipe.result.name = recipeData["result"]["name"];
            recipe.result.type = recipeData["result"]["type"];
            recipe.result.quantity = recipeData["result"]["quantity"];

            // Load result properties
            for (const auto& [key, value] : recipeData["result"]["properties"].items()) {
                if (value.is_number_integer()) {
                    recipe.result.properties[key] = value.get<int>();
                } else if (value.is_number_float()) {
                    recipe.result.properties[key] = value.get<float>();
                } else if (value.is_string()) {
                    recipe.result.properties[key] = value.get<std::string>();
                } else if (value.is_boolean()) {
                    recipe.result.properties[key] = value.get<bool>();
                }
            }

            station->addRecipe(recipe);
        }

        craftingRoot->addChild(station);
    }

    controller.setSystemRoot("CraftingSystem", craftingRoot);
}

// Load world from JSON
void loadWorldFromJSON(TAController& controller, const json& worldData)
{
    std::map<std::string, RegionNode*> regions;
    std::map<std::string, LocationNode*> locations;

    // First pass: create all region nodes
    for (const auto& regionData : worldData["regions"]) {
        RegionNode* region = dynamic_cast<RegionNode*>(
            controller.createNode<RegionNode>(
                regionData["id"],
                regionData["regionName"]));

        region->description = regionData["description"];
        region->controllingFaction = regionData["controllingFaction"];

        // Create locations in this region
        for (const auto& locationData : regionData["locations"]) {
            LocationNode* location = dynamic_cast<LocationNode*>(
                controller.createNode<LocationNode>(
                    locationData["id"],
                    locationData["locationName"],
                    locationData["currentState"]));

            location->description = locationData["description"];

            // Load state descriptions
            for (const auto& [state, desc] : locationData["stateDescriptions"].items()) {
                location->stateDescriptions[state] = desc;
            }

            // Load access conditions
            for (const auto& conditionData : locationData["accessConditions"]) {
                LocationNode::AccessCondition condition;
                condition.type = conditionData["type"];
                condition.target = conditionData["target"];
                condition.value = conditionData["value"];
                location->accessConditions.push_back(condition);
            }

            locations[locationData["id"]] = location;
            region->locations.push_back(location);
        }

        // Load possible events
        for (const auto& eventData : regionData["possibleEvents"]) {
            RegionNode::RegionEvent event;
            event.name = eventData["name"];
            event.description = eventData["description"];
            event.probability = eventData["probability"];

            // Set up condition function
            event.condition = [eventData](const GameContext& ctx) {
                if (eventData["condition"]["type"] == "worldflag") {
                    return ctx.worldState.hasFlag(eventData["condition"]["flag"]) == eventData["condition"]["value"].get<bool>();
                } else if (eventData["condition"]["type"] == "skill") {
                    return ctx.playerStats.hasSkill(
                        eventData["condition"]["skill"],
                        eventData["condition"]["value"]);
                }
                return true;
            };

            // Set up effect function
            event.effect = [eventData](GameContext* ctx) {
                if (!ctx)
                    return;

                if (eventData["effect"]["type"] == "location") {
                    ctx->worldState.setLocationState(
                        eventData["effect"]["target"],
                        eventData["effect"]["state"]);
                } else if (eventData["effect"]["type"] == "item") {
                    ctx->playerInventory.addItem(
                        Item(
                            eventData["effect"]["item"],
                            eventData["effect"]["item"],
                            "event_reward",
                            1,
                            eventData["effect"]["quantity"]));
                }
            };

            region->possibleEvents.push_back(event);
        }

        regions[regionData["id"]] = region;
    }

    // Second pass: connect regions
    for (const auto& regionData : worldData["regions"]) {
        RegionNode* region = regions[regionData["id"]];

        // Connect regions
        for (const auto& connectedId : regionData["connectedRegions"]) {
            if (regions.count(connectedId)) {
                region->connectedRegions.push_back(regions[connectedId]);
            }
        }
    }

    // Set up NPCs in locations and activities
    auto& npcMap = controller.gameData["npcs"];

    for (const auto& regionData : worldData["regions"]) {
        for (const auto& locationData : regionData["locations"]) {
            LocationNode* location = locations[locationData["id"]];

            // Add NPCs
            if (locationData.contains("npcs")) {
                for (const auto& npcId : locationData["npcs"]) {
                    if (npcMap.count(npcId)) {
                        location->npcs.push_back(npcMap[npcId]);
                    }
                }
            }

            // Add activities (quests, crafting, etc.)
            if (locationData.contains("activities")) {
                for (const auto& activityId : locationData["activities"]) {
                    // Try to find the activity in various systems
                    TANode* activity = nullptr;

                    if (controller.systemRoots.count("QuestSystem") && controller.systemRoots["QuestSystem"]->nodeName == activityId) {
                        activity = controller.systemRoots["QuestSystem"];
                    } else if (controller.systemRoots.count("CraftingSystem")) {
                        // Search in crafting children
                        for (TANode* child : controller.systemRoots["CraftingSystem"]->childNodes) {
                            if (child->nodeName == activityId) {
                                activity = child;
                                break;
                            }
                        }
                    }

                    if (activity) {
                        location->activities.push_back(activity);
                    }
                }
            }
        }
    }

    // Load time system
    TimeNode* timeSystem = dynamic_cast<TimeNode*>(
        controller.createNode<TimeNode>("TimeSystem"));

    timeSystem->day = worldData["timeSystem"]["day"];
    timeSystem->hour = worldData["timeSystem"]["hour"];
    timeSystem->season = worldData["timeSystem"]["season"];
    timeSystem->timeOfDay = worldData["timeSystem"]["timeOfDay"];

    // Register the world and time systems
    controller.setSystemRoot("WorldSystem", regions["VillageRegion"]); // Start in village
    controller.setSystemRoot("TimeSystem", timeSystem);
}

// Function to create default JSON files
void createDefaultJSONFiles()
{
    // Create quests.json
    std::ofstream questFile("data/quests.json");
    if (questFile.is_open()) {
        json questData;
        questData["quests"] = json::array({ { { "id", "MainQuest" },
            { "title", "Defend the Village" },
            { "description", "The village is under threat. Prepare its defenses!" },
            { "state", "Available" },
            { "isAcceptingState", false },
            { "rewards", json::array({ { { "type", "experience" }, { "amount", 500 }, { "itemId", "" } }, { { "type", "gold" }, { "amount", 200 }, { "itemId", "" } }, { { "type", "faction" }, { "amount", 25 }, { "itemId", "villagers" } }, { { "type", "item" }, { "amount", 1 }, { "itemId", "defenders_shield" } } }) },
            { "requirements", json::array() },
            { "subquests", json::array({ { { "id", "RepairWalls" }, { "title", "Repair the Walls" }, { "description", "The village walls are in disrepair. Fix them!" }, { "state", "Available" }, { "isAcceptingState", false }, { "rewards", json::array({ { { "type", "experience" }, { "amount", 100 }, { "itemId", "" } }, { { "type", "gold" }, { "amount", 50 }, { "itemId", "" } }, { { "type", "faction" }, { "amount", 10 }, { "itemId", "villagers" } } }) }, { "requirements", json::array({ { { "type", "skill" }, { "target", "crafting" }, { "value", 1 } } }) }, { "transitions", json::array({ { { "action", "repair_complete" }, { "target", "MainQuest" }, { "description", "Complete wall repairs" } } }) } }, { { "id", "TrainMilitia" }, { "title", "Train the Militia" }, { "description", "The villagers need combat training." }, { "state", "Available" }, { "isAcceptingState", false }, { "rewards", json::array({ { { "type", "experience" }, { "amount", 150 }, { "itemId", "" } }, { { "type", "skill" }, { "amount", 1 }, { "itemId", "combat" } } }) }, { "requirements", json::array({ { { "type", "skill" }, { "target", "combat" }, { "value", 2 } } }) }, { "transitions", json::array({ { { "action", "training_complete" }, { "target", "MainQuest" }, { "description", "Complete militia training" } } }) } }, { { "id", "GatherSupplies" }, { "title", "Gather Supplies" }, { "description", "The village needs food and resources." }, { "state", "Available" }, { "isAcceptingState", false }, { "rewards", json::array({ { { "type", "experience" }, { "amount", 100 }, { "itemId", "" } }, { { "type", "item" }, { "amount", 1 }, { "itemId", "rare_herb" } } }) }, { "requirements", json::array({ { { "type", "skill" }, { "target", "survival" }, { "value", 1 } } }) }, { "transitions", json::array({ { { "action", "supplies_gathered" }, { "target", "MainQuest" }, { "description", "Finish gathering supplies" } } }) } } }) } } });
        questFile << std::setw(4) << questData << std::endl;
        questFile.close();
    }

    // Create npcs.json
    std::ofstream npcFile("data/npcs.json");
    if (npcFile.is_open()) {
        json npcData;
        npcData["npcs"] = json::array({ { { "id", "elder_marius" },
            { "name", "Elder Marius" },
            { "description", "The wise leader of the village" },
            { "relationshipValue", 0 },
            { "rootDialogue", "ElderGreeting" },
            { "dialogueNodes", json::array({ { { "id", "ElderGreeting" }, { "speakerName", "Elder Marius" }, { "dialogueText", "Greetings, traveler. Our village faces difficult times." }, { "responses", json::array({ { { "text", "What threat does the village face?" }, { "targetNode", "AskThreat" }, { "requirements", json::array() }, { "effects", json::array() } }, { { "text", "Is there something I can help with?" }, { "targetNode", "AskHelp" }, { "requirements", json::array() }, { "effects", json::array() } }, { { "text", "I need to go. Farewell." }, { "targetNode", "Farewell" }, { "requirements", json::array() }, { "effects", json::array() } } }) } }, { { "id", "AskThreat" }, { "speakerName", "Elder Marius" }, { "dialogueText", "Bandits have been raiding nearby settlements. I fear we're next." }, { "responses", json::array({ { { "text", "How can I help against these bandits?" }, { "targetNode", "AskHelp" }, { "requirements", json::array() }, { "effects", json::array() } }, { { "text", "I'll be on my way." }, { "targetNode", "Farewell" }, { "requirements", json::array() }, { "effects", json::array() } } }) } }, { { "id", "AskHelp" }, { "speakerName", "Elder Marius" }, { "dialogueText", "We need someone skilled to help prepare our defenses." }, { "responses", json::array({ { { "text", "I'll help defend the village." }, { "targetNode", "AcceptQuest" }, { "requirements", json::array() }, { "effects", json::array({ { { "type", "quest" }, { "action", "activate" }, { "target", "MainQuest" } }, { { "type", "knowledge" }, { "action", "add" }, { "target", "village_under_threat" } }, { { "type", "faction" }, { "action", "change" }, { "target", "villagers" }, { "amount", 5 } } }) } }, { { "text", "I'm not interested in helping." }, { "targetNode", "RejectQuest" }, { "requirements", json::array() }, { "effects", json::array() } }, { { "text", "I need to think about it." }, { "targetNode", "Farewell" }, { "requirements", json::array() }, { "effects", json::array() } } }) } }, { { "id", "AcceptQuest" }, { "speakerName", "Elder Marius" }, { "dialogueText", "Thank you! This means a lot to our community. We need the walls repaired, the militia trained, and supplies gathered." }, { "responses", json::array({ { { "text", "I'll get started right away." }, { "targetNode", "Farewell" }, { "requirements", json::array() }, { "effects", json::array() } } }) } }, { { "id", "RejectQuest" }, { "speakerName", "Elder Marius" }, { "dialogueText", "I understand. Perhaps you'll reconsider when you have time." }, { "responses", json::array({ { { "text", "Goodbye." }, { "targetNode", "Farewell" }, { "requirements", json::array() }, { "effects", json::array() } } }) } }, { { "id", "Farewell" }, { "speakerName", "Elder Marius" }, { "dialogueText", "Safe travels, friend. Return if you need anything." }, { "responses", json::array() } } }) } } });
        npcFile << std::setw(4) << npcData << std::endl;
        npcFile.close();
    }

    // Create skills.json
    std::ofstream skillsFile("data/skills.json");
    if (skillsFile.is_open()) {
        json skillsData;
        // Add skills data
        skillsData["skills"] = json::array({ { { "id", "CombatBasics" },
                                                 { "skillName", "combat" },
                                                 { "description", "Basic combat techniques and weapon handling." },
                                                 { "level", 0 },
                                                 { "maxLevel", 5 },
                                                 { "requirements", json::array() },
                                                 { "effects", json::array({ { { "type", "stat" }, { "target", "strength" }, { "value", 1 } } }) },
                                                 { "costs", json::array() },
                                                 { "childSkills", json::array({ { { "id", "Swordsmanship" }, { "skillName", "swordsmanship" }, { "description", "Advanced sword techniques for greater damage and defense." }, { "level", 0 }, { "maxLevel", 5 }, { "requirements", json::array({ { { "type", "skill" }, { "target", "combat" }, { "level", 2 } } }) }, { "effects", json::array({ { { "type", "ability" }, { "target", "power_attack" }, { "value", 0 } } }) } }, { { "id", "Archery" }, { "skillName", "archery" }, { "description", "Precision with bows and other ranged weapons." }, { "level", 0 }, { "maxLevel", 5 }, { "requirements", json::array({ { { "type", "skill" }, { "target", "combat" }, { "level", 2 } } }) }, { "effects", json::array({ { { "type", "ability" }, { "target", "precise_shot" }, { "value", 0 } } }) } } }) } },
            { { "id", "SurvivalBasics" },
                { "skillName", "survival" },
                { "description", "Basic survival skills for harsh environments." },
                { "level", 0 },
                { "maxLevel", 5 },
                { "requirements", json::array() },
                { "effects", json::array({ { { "type", "stat" }, { "target", "constitution" }, { "value", 1 } } }) },
                { "costs", json::array() },
                { "childSkills", json::array({ { { "id", "Herbalism" }, { "skillName", "herbalism" }, { "description", "Knowledge of medicinal and poisonous plants." }, { "level", 0 }, { "maxLevel", 5 }, { "requirements", json::array({ { { "type", "skill" }, { "target", "survival" }, { "level", 2 } } }) }, { "effects", json::array({ { { "type", "ability" }, { "target", "herbal_remedy" }, { "value", 0 } } }) } }, { { "id", "Tracking" }, { "skillName", "tracking" }, { "description", "Follow trails and find creatures in the wilderness." }, { "level", 0 }, { "maxLevel", 5 }, { "requirements", json::array({ { { "type", "skill" }, { "target", "survival" }, { "level", 1 } } }) }, { "effects", json::array({ { { "type", "ability" }, { "target", "track_prey" }, { "value", 0 } } }) } } }) } },
            { { "id", "CraftingBasics" },
                { "skillName", "crafting" },
                { "description", "Basic crafting and repair techniques." },
                { "level", 0 },
                { "maxLevel", 5 },
                { "requirements", json::array() },
                { "effects", json::array({ { { "type", "stat" }, { "target", "dexterity" }, { "value", 1 } } }) },
                { "costs", json::array() },
                { "childSkills", json::array({ { { "id", "Blacksmithing" }, { "skillName", "blacksmithing" }, { "description", "Forge and improve metal weapons and armor." }, { "level", 0 }, { "maxLevel", 5 }, { "requirements", json::array({ { { "type", "skill" }, { "target", "crafting" }, { "level", 2 } } }) }, { "effects", json::array({ { { "type", "ability" }, { "target", "forge_weapon" }, { "value", 0 } } }) } }, { { "id", "Alchemy" }, { "skillName", "alchemy" }, { "description", "Create potions and elixirs with magical effects." }, { "level", 0 }, { "maxLevel", 5 }, { "requirements", json::array({ { { "type", "skill" }, { "target", "crafting" }, { "level", 1 } }, { { "type", "skill" }, { "target", "herbalism" }, { "level", 1 } } }) }, { "effects", json::array({ { { "type", "ability" }, { "target", "brew_potion" }, { "value", 0 } } }) } } }) } } });

        // Add classes data
        skillsData["classes"] = json::array({ { { "id", "Warrior" },
                                                  { "className", "Warrior" },
                                                  { "description", "Masters of combat, strong and resilient." },
                                                  { "statBonuses", { { "strength", 3 }, { "constitution", 2 } } },
                                                  { "startingAbilities", { "weapon_specialization" } },
                                                  { "classSkills", { "CombatBasics", "Swordsmanship" } } },
            { { "id", "Ranger" },
                { "className", "Ranger" },
                { "description", "Wilderness experts, skilled with bow and blade." },
                { "statBonuses", { { "dexterity", 2 }, { "wisdom", 2 } } },
                { "startingAbilities", { "animal_companion" } },
                { "classSkills", { "Archery", "Tracking" } } },
            { { "id", "Alchemist" },
                { "className", "Alchemist" },
                { "description", "Masters of potions and elixirs." },
                { "statBonuses", { { "intelligence", 3 }, { "dexterity", 1 } } },
                { "startingAbilities", { "potion_mastery" } },
                { "classSkills", { "Herbalism", "Alchemy" } } } });

        skillsFile << std::setw(4) << skillsData << std::endl;
        skillsFile.close();
    }
    // Create crafting.json
    std::ofstream craftingFile("data/crafting.json");
    if (craftingFile.is_open()) {
        json craftingData;
        craftingData["craftingStations"] = json::array({ { { "id", "BlacksmithStation" },
                                                             { "stationType", "Blacksmith" },
                                                             { "description", "A forge with anvil, hammers, and other metalworking tools." },
                                                             { "recipes", json::array({ { { "recipeId", "sword_recipe" }, { "name", "Iron Sword" }, { "description", "A standard iron sword, good for combat." }, { "discovered", true }, { "ingredients", json::array({ { { "itemId", "iron_ingot" }, { "quantity", 2 } }, { { "itemId", "leather_strips" }, { "quantity", 1 } } }) }, { "skillRequirements", { { "blacksmithing", 1 } } }, { "result", { { "itemId", "iron_sword" }, { "name", "Iron Sword" }, { "type", "weapon" }, { "quantity", 1 }, { "properties", { { "damage", 10 } } } } } }, { { "recipeId", "armor_recipe" }, { "name", "Leather Armor" }, { "description", "Basic protective gear made from leather." }, { "discovered", true }, { "ingredients", json::array({ { { "itemId", "leather" }, { "quantity", 5 } }, { { "itemId", "metal_studs" }, { "quantity", 10 } } }) }, { "skillRequirements", { { "crafting", 2 } } }, { "result", { { "itemId", "leather_armor" }, { "name", "Leather Armor" }, { "type", "armor" }, { "quantity", 1 }, { "properties", { { "defense", 5 } } } } } } }) } },
            { { "id", "AlchemyStation" },
                { "stationType", "Alchemy" },
                { "description", "A workbench with alembics, mortars, and various containers for brewing." },
                { "recipes", json::array({ { { "recipeId", "health_potion_recipe" }, { "name", "Minor Healing Potion" }, { "description", "A potion that restores a small amount of health." }, { "discovered", true }, { "ingredients", json::array({ { { "itemId", "red_herb" }, { "quantity", 2 } }, { { "itemId", "water_flask" }, { "quantity", 1 } } }) }, { "skillRequirements", { { "alchemy", 1 } } }, { "result", { { "itemId", "minor_healing_potion" }, { "name", "Minor Healing Potion" }, { "type", "potion" }, { "quantity", 1 }, { "properties", { { "heal_amount", 25 } } } } } } }) } },
            { { "id", "CookingStation" },
                { "stationType", "Cooking" },
                { "description", "A firepit with cooking pots and utensils." },
                { "recipes", json::array({ { { "recipeId", "stew_recipe" }, { "name", "Hearty Stew" }, { "description", "A filling meal that provides temporary stat bonuses." }, { "discovered", true }, { "ingredients", json::array({ { { "itemId", "meat" }, { "quantity", 2 } }, { { "itemId", "vegetables" }, { "quantity", 3 } } }) }, { "skillRequirements", { { "cooking", 1 } } }, { "result", { { "itemId", "hearty_stew" }, { "name", "Hearty Stew" }, { "type", "food" }, { "quantity", 2 }, { "properties", { { "effect_duration", 300 } } } } } } }) } } });
        craftingFile << std::setw(4) << craftingData << std::endl;
        craftingFile.close();
    }

    // Create world.json
    std::ofstream worldFile("data/world.json");
    if (worldFile.is_open()) {
        json worldData;
        worldData["regions"] = json::array({ { { "id", "VillageRegion" },
                                                 { "regionName", "Oakvale Village" },
                                                 { "description", "A peaceful farming village surrounded by wooden palisades." },
                                                 { "controllingFaction", "villagers" },
                                                 { "connectedRegions", { "ForestRegion", "MountainRegion" } },
                                                 { "locations", json::array({ { { "id", "VillageCenter" }, { "locationName", "Village Center" }, { "description", "The bustling center of the village with a market and well." }, { "currentState", "normal" }, { "stateDescriptions", { { "damaged", "The village center shows signs of damage from bandit raids." }, { "rebuilt", "The village center has been rebuilt stronger than before." } } }, { "accessConditions", json::array() }, { "npcs", { "elder_marius" } }, { "activities", { "MainQuest" } } }, { { "id", "VillageInn" }, { "locationName", "The Sleeping Dragon Inn" }, { "description", "A cozy inn where travelers find rest and information." }, { "currentState", "normal" }, { "stateDescriptions", {} }, { "accessConditions", json::array() }, { "npcs", json::array() }, { "activities", json::array() } }, { { "id", "VillageForge" }, { "locationName", "Blacksmith's Forge" }, { "description", "The local blacksmith's workshop with a roaring forge." }, { "currentState", "normal" }, { "stateDescriptions", {} }, { "accessConditions", json::array() }, { "npcs", json::array() }, { "activities", { "BlacksmithStation" } } } }) },
                                                 { "possibleEvents", json::array({ { { "name", "Bandit Raid" }, { "description", "A small group of bandits is attacking the village outskirts!" }, { "condition", { { "type", "worldflag" }, { "flag", "village_defended" }, { "value", false } } }, { "effect", { { "type", "location" }, { "target", "village" }, { "state", "under_attack" } } }, { "probability", 0.2 } } }) } },
            { { "id", "ForestRegion" },
                { "regionName", "Green Haven Forest" },
                { "description", "A dense forest with ancient trees and hidden paths." },
                { "controllingFaction", "forest guardians" },
                { "connectedRegions", { "VillageRegion", "MountainRegion" } },
                { "locations", json::array({ { { "id", "ForestClearing" }, { "locationName", "Forest Clearing" }, { "description", "A peaceful clearing in the heart of the forest." }, { "currentState", "normal" }, { "stateDescriptions", {} }, { "accessConditions", json::array() }, { "npcs", json::array() }, { "activities", json::array() } }, { { "id", "AncientGroves" }, { "locationName", "Ancient Groves" }, { "description", "An area with trees older than any human memory." }, { "currentState", "normal" }, { "stateDescriptions", {} }, { "accessConditions", json::array({ { { "type", "skill" }, { "target", "survival" }, { "value", 2 } } }) }, { "npcs", json::array() }, { "activities", json::array() } } }) },
                { "possibleEvents", json::array({ { { "name", "Rare Herb Sighting" }, { "description", "You spot a patch of rare medicinal herbs growing nearby." }, { "condition", { { "type", "skill" }, { "skill", "herbalism" }, { "value", 1 } } }, { "effect", { { "type", "item" }, { "item", "rare_herb" }, { "quantity", 1 } } }, { "probability", 0.3 } } }) } },
            { { "id", "MountainRegion" },
                { "regionName", "Stone Peak Mountains" },
                { "description", "Rugged mountains with treacherous paths and hidden caves." },
                { "controllingFaction", "mountainfolk" },
                { "connectedRegions", { "VillageRegion", "ForestRegion" } },
                { "locations", json::array({ { { "id", "MountainPass" }, { "locationName", "Mountain Pass" }, { "description", "A winding path through the mountains." }, { "currentState", "normal" }, { "stateDescriptions", {} }, { "accessConditions", json::array() }, { "npcs", json::array() }, { "activities", json::array() } }, { { "id", "AbandonedMine" }, { "locationName", "Abandoned Mine" }, { "description", "An old mine, no longer in use. Rumors say something lurks within." }, { "currentState", "normal" }, { "stateDescriptions", {} }, { "accessConditions", json::array({ { { "type", "item" }, { "target", "torch" }, { "value", 1 } } }) }, { "npcs", json::array() }, { "activities", json::array() } } }) },
                { "possibleEvents", json::array() } } });

        worldData["timeSystem"] = {
            { "day", 1 },
            { "hour", 6 },
            { "season", "spring" },
            { "timeOfDay", "morning" }
        };

        worldFile << std::setw(4) << worldData << std::endl;
        worldFile.close();
    }
}