
#include <Generator_World.hpp>

#include <ctime>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

// Forward declarations
class Location;
class Map;

// Random number generator
class Random {
private:
    std::mt19937 rng;

public:
    Random()
        : rng(std::time(nullptr))
    {
    }

    int getInt(int min, int max)
    {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(rng);
    }

    double getDouble(double min, double max)
    {
        std::uniform_real_distribution<double> dist(min, max);
        return dist(rng);
    }

    bool getBool(double probability = 0.5)
    {
        std::bernoulli_distribution dist(probability);
        return dist(rng);
    }

    template <typename T>
    T getRandomElement(const std::vector<T>& vec)
    {
        if (vec.empty())
            throw std::runtime_error("Cannot select from empty vector");
        return vec[getInt(0, vec.size() - 1)];
    }
};

// Global random instance
Random random;

// Represents a point on the map
struct Point {
    int x;
    int y;

    Point(int x = 0, int y = 0)
        : x(x)
        , y(y)
    {
    }

    double distanceTo(const Point& other) const
    {
        int dx = x - other.x;
        int dy = y - other.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    json toJson() const
    {
        return json { { "x", x }, { "y", y } };
    }

    static Point fromJson(const json& j)
    {
        return Point(j["x"], j["y"]);
    }
};

// Base class for all location types
class Location {
protected:
    std::string name;
    std::string type;
    Point position;
    int size;
    std::vector<std::string> features;

public:
    Location(const std::string& name, const std::string& type, const Point& position, int size)
        : name(name)
        , type(type)
        , position(position)
        , size(size)
    {
    }

    virtual ~Location() = default;

    const std::string& getName() const { return name; }
    const std::string& getType() const { return type; }
    const Point& getPosition() const { return position; }
    int getSize() const { return size; }

    void addFeature(const std::string& feature)
    {
        features.push_back(feature);
    }

    virtual json toJson() const
    {
        return json {
            { "name", name },
            { "type", type },
            { "position", position.toJson() },
            { "size", size },
            { "features", features }
        };
    }

    static std::unique_ptr<Location> fromJson(const json& j);
};

// City/Town location type
class Settlement : public Location {
private:
    int population;
    std::vector<std::string> buildings;

public:
    Settlement(const std::string& name, const Point& position, int size, int population)
        : Location(name, "settlement", position, size)
        , population(population)
    {
    }

    void addBuilding(const std::string& building)
    {
        buildings.push_back(building);
    }

    json toJson() const override
    {
        json j = Location::toJson();
        j["population"] = population;
        j["buildings"] = buildings;
        return j;
    }

    static std::unique_ptr<Settlement> fromJson(const json& j)
    {
        auto settlement = std::make_unique<Settlement>(
            j["name"],
            Point::fromJson(j["position"]),
            j["size"],
            j["population"]);

        for (const auto& feature : j["features"]) {
            settlement->addFeature(feature);
        }

        for (const auto& building : j["buildings"]) {
            settlement->addBuilding(building);
        }

        return settlement;
    }
};

// Dungeon location type
class Dungeon : public Location {
private:
    int depth;
    std::string difficulty;
    std::vector<std::string> monsters;
    std::vector<std::string> treasures;

public:
    Dungeon(const std::string& name, const Point& position, int size, int depth, const std::string& difficulty)
        : Location(name, "dungeon", position, size)
        , depth(depth)
        , difficulty(difficulty)
    {
    }

    void addMonster(const std::string& monster)
    {
        monsters.push_back(monster);
    }

    void addTreasure(const std::string& treasure)
    {
        treasures.push_back(treasure);
    }

    json toJson() const override
    {
        json j = Location::toJson();
        j["depth"] = depth;
        j["difficulty"] = difficulty;
        j["monsters"] = monsters;
        j["treasures"] = treasures;
        return j;
    }

    static std::unique_ptr<Dungeon> fromJson(const json& j)
    {
        auto dungeon = std::make_unique<Dungeon>(
            j["name"],
            Point::fromJson(j["position"]),
            j["size"],
            j["depth"],
            j["difficulty"]);

        for (const auto& feature : j["features"]) {
            dungeon->addFeature(feature);
        }

        for (const auto& monster : j["monsters"]) {
            dungeon->addMonster(monster);
        }

        for (const auto& treasure : j["treasures"]) {
            dungeon->addTreasure(treasure);
        }

        return dungeon;
    }
};

// Wilderness location type
class Wilderness : public Location {
private:
    std::string terrain;
    std::vector<std::string> resources;
    std::vector<std::string> dangers;

public:
    Wilderness(const std::string& name, const Point& position, int size, const std::string& terrain)
        : Location(name, "wilderness", position, size)
        , terrain(terrain)
    {
    }

    void addResource(const std::string& resource)
    {
        resources.push_back(resource);
    }

    void addDanger(const std::string& danger)
    {
        dangers.push_back(danger);
    }

    json toJson() const override
    {
        json j = Location::toJson();
        j["terrain"] = terrain;
        j["resources"] = resources;
        j["dangers"] = dangers;
        return j;
    }

    static std::unique_ptr<Wilderness> fromJson(const json& j)
    {
        auto wilderness = std::make_unique<Wilderness>(
            j["name"],
            Point::fromJson(j["position"]),
            j["size"],
            j["terrain"]);

        for (const auto& feature : j["features"]) {
            wilderness->addFeature(feature);
        }

        for (const auto& resource : j["resources"]) {
            wilderness->addResource(resource);
        }

        for (const auto& danger : j["dangers"]) {
            wilderness->addDanger(danger);
        }

        return wilderness;
    }
};

// Implementation of static Location::fromJson
std::unique_ptr<Location> Location::fromJson(const json& j)
{
    std::string type = j["type"];

    if (type == "settlement") {
        return Settlement::fromJson(j);
    } else if (type == "dungeon") {
        return Dungeon::fromJson(j);
    } else if (type == "wilderness") {
        return Wilderness::fromJson(j);
    } else {
        throw std::runtime_error("Unknown location type: " + type);
    }
}

// Represents connection between locations
struct Connection {
    int from;
    int to;
    std::string type;
    int difficulty;

    Connection(int from, int to, const std::string& type, int difficulty)
        : from(from)
        , to(to)
        , type(type)
        , difficulty(difficulty)
    {
    }

    json toJson() const
    {
        return json {
            { "from", from },
            { "to", to },
            { "type", type },
            { "difficulty", difficulty }
        };
    }

    static Connection fromJson(const json& j)
    {
        return Connection(
            j["from"],
            j["to"],
            j["type"],
            j["difficulty"]);
    }
};

// The complete map
class Map {
private:
    std::string name;
    int width;
    int height;
    std::string description;
    std::vector<std::unique_ptr<Location>> locations;
    std::vector<Connection> connections;

public:
    Map(const std::string& name, int width, int height, const std::string& description)
        : name(name)
        , width(width)
        , height(height)
        , description(description)
    {
    }

    void addLocation(std::unique_ptr<Location> location)
    {
        locations.push_back(std::move(location));
    }

    void addConnection(const Connection& connection)
    {
        connections.push_back(connection);
    }

    // Generate connections between locations based on proximity
    void generateConnections(double maxDistance, double connectionProbability = 0.7)
    {
        for (size_t i = 0; i < locations.size(); ++i) {
            for (size_t j = i + 1; j < locations.size(); ++j) {
                double distance = locations[i]->getPosition().distanceTo(locations[j]->getPosition());
                if (distance <= maxDistance && random.getBool(connectionProbability)) {
                    std::vector<std::string> roadTypes = { "path", "road", "river", "bridge" };
                    std::string roadType = random.getRandomElement(roadTypes);
                    int difficulty = random.getInt(1, 5);
                    connections.push_back(Connection(i, j, roadType, difficulty));
                }
            }
        }
    }

    json toJson() const
    {
        json j;
        j["name"] = name;
        j["width"] = width;
        j["height"] = height;
        j["description"] = description;

        json locs = json::array();
        for (const auto& loc : locations) {
            locs.push_back(loc->toJson());
        }
        j["locations"] = locs;

        json conns = json::array();
        for (const auto& conn : connections) {
            conns.push_back(conn.toJson());
        }
        j["connections"] = conns;

        return j;
    }

    static Map fromJson(const json& j)
    {
        Map map(j["name"], j["width"], j["height"], j["description"]);

        for (const auto& locJson : j["locations"]) {
            map.addLocation(Location::fromJson(locJson));
        }

        for (const auto& connJson : j["connections"]) {
            map.addConnection(Connection::fromJson(connJson));
        }

        return map;
    }

    void saveToFile(const std::string& filename) const
    {
        std::ofstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file for writing: " + filename);
        }

        file << toJson().dump(4);
        file.close();
    }

    static Map loadFromFile(const std::string& filename)
    {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file for reading: " + filename);
        }

        json j;
        file >> j;
        file.close();

        return fromJson(j);
    }
};

// Map generator class
class MapGenerator {
private:
    json templates;

    // Load templates from file
    void loadTemplates(const std::string& filename)
    {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open templates file: " + filename);
        }

        file >> templates;
        file.close();
    }

    // Generate a random name based on template patterns
    std::string generateName(const std::string& type) const
    {
        const auto& nameTemplates = templates["names"][type];
        if (nameTemplates.is_array() && !nameTemplates.empty()) {
            std::vector<std::string> options;
            for (const auto& nameTemplate : nameTemplates) {
                options.push_back(nameTemplate);
            }
            return random.getRandomElement(options);
        }

        // Fallback names if templates are missing
        std::vector<std::string> fallbacks;
        if (type == "settlement") {
            fallbacks = { "Oakville", "Riverside", "Mountainview", "Hillcrest", "Valleyforge" };
        } else if (type == "dungeon") {
            fallbacks = { "Dark Cave", "Ancient Ruins", "Forgotten Crypt", "Cursed Mine", "Shadow Labyrinth" };
        } else if (type == "wilderness") {
            fallbacks = { "Dark Forest", "Misty Swamp", "Rocky Highlands", "Endless Plains", "Scorched Desert" };
        } else {
            fallbacks = { "Unknown Location", "Mysterious Place", "Unnamed Region" };
        }

        return random.getRandomElement(fallbacks);
    }

    // Generate features for a location
    std::vector<std::string> generateFeatures(const std::string& type, int count) const
    {
        std::vector<std::string> result;
        const auto& featureTemplates = templates["features"][type];

        if (featureTemplates.is_array() && !featureTemplates.empty()) {
            std::vector<std::string> options;
            for (const auto& feature : featureTemplates) {
                options.push_back(feature);
            }

            for (int i = 0; i < count; ++i) {
                if (!options.empty()) {
                    std::string feature = random.getRandomElement(options);
                    result.push_back(feature);

                    // Remove to avoid duplicates
                    auto it = std::find(options.begin(), options.end(), feature);
                    if (it != options.end()) {
                        options.erase(it);
                    }
                }
            }
        }

        return result;
    }

    // Generate a settlement (city/town)
    std::unique_ptr<Settlement> generateSettlement(int mapWidth, int mapHeight)
    {
        Point position(random.getInt(0, mapWidth), random.getInt(0, mapHeight));
        int size = random.getInt(1, 10);
        int population = size * random.getInt(100, 1000);

        auto settlement = std::make_unique<Settlement>(
            generateName("settlement"),
            position,
            size,
            population);

        // Add features
        for (const auto& feature : generateFeatures("settlement", random.getInt(2, 5))) {
            settlement->addFeature(feature);
        }

        // Add buildings
        const auto& buildingTemplates = templates["buildings"];
        if (buildingTemplates.is_array() && !buildingTemplates.empty()) {
            int buildingCount = random.getInt(3, 7);
            for (int i = 0; i < buildingCount; ++i) {
                std::vector<std::string> options;
                for (const auto& building : buildingTemplates) {
                    options.push_back(building);
                }
                settlement->addBuilding(random.getRandomElement(options));
            }
        }

        return settlement;
    }

    // Generate a dungeon
    std::unique_ptr<Dungeon> generateDungeon(int mapWidth, int mapHeight)
    {
        Point position(random.getInt(0, mapWidth), random.getInt(0, mapHeight));
        int size = random.getInt(1, 8);
        int depth = random.getInt(1, 10);

        std::vector<std::string> difficulties = { "easy", "medium", "hard", "deadly" };
        std::string difficulty = random.getRandomElement(difficulties);

        auto dungeon = std::make_unique<Dungeon>(
            generateName("dungeon"),
            position,
            size,
            depth,
            difficulty);

        // Add features
        for (const auto& feature : generateFeatures("dungeon", random.getInt(2, 4))) {
            dungeon->addFeature(feature);
        }

        // Add monsters
        const auto& monsterTemplates = templates["monsters"];
        if (monsterTemplates.is_array() && !monsterTemplates.empty()) {
            int monsterCount = random.getInt(3, 8);
            for (int i = 0; i < monsterCount; ++i) {
                std::vector<std::string> options;
                for (const auto& monster : monsterTemplates) {
                    options.push_back(monster);
                }
                dungeon->addMonster(random.getRandomElement(options));
            }
        }

        // Add treasures
        const auto& treasureTemplates = templates["treasures"];
        if (treasureTemplates.is_array() && !treasureTemplates.empty()) {
            int treasureCount = random.getInt(2, 5);
            for (int i = 0; i < treasureCount; ++i) {
                std::vector<std::string> options;
                for (const auto& treasure : treasureTemplates) {
                    options.push_back(treasure);
                }
                dungeon->addTreasure(random.getRandomElement(options));
            }
        }

        return dungeon;
    }

    // Generate a wilderness area
    std::unique_ptr<Wilderness> generateWilderness(int mapWidth, int mapHeight)
    {
        Point position(random.getInt(0, mapWidth), random.getInt(0, mapHeight));
        int size = random.getInt(3, 15);

        std::vector<std::string> terrains = { "forest", "mountains", "swamp", "plains", "desert", "tundra", "jungle" };
        std::string terrain = random.getRandomElement(terrains);

        auto wilderness = std::make_unique<Wilderness>(
            generateName("wilderness"),
            position,
            size,
            terrain);

        // Add features
        for (const auto& feature : generateFeatures("wilderness", random.getInt(2, 6))) {
            wilderness->addFeature(feature);
        }

        // Add resources
        const auto& resourceTemplates = templates["resources"];
        if (resourceTemplates.is_array() && !resourceTemplates.empty()) {
            int resourceCount = random.getInt(2, 6);
            for (int i = 0; i < resourceCount; ++i) {
                std::vector<std::string> options;
                for (const auto& resource : resourceTemplates) {
                    options.push_back(resource);
                }
                wilderness->addResource(random.getRandomElement(options));
            }
        }

        // Add dangers
        const auto& dangerTemplates = templates["dangers"];
        if (dangerTemplates.is_array() && !dangerTemplates.empty()) {
            int dangerCount = random.getInt(1, 4);
            for (int i = 0; i < dangerCount; ++i) {
                std::vector<std::string> options;
                for (const auto& danger : dangerTemplates) {
                    options.push_back(danger);
                }
                wilderness->addDanger(random.getRandomElement(options));
            }
        }

        return wilderness;
    }

public:
    MapGenerator(const std::string& templateFile)
    {
        loadTemplates(templateFile);
    }

    Map generateMap(const std::string& name, int width, int height, const std::string& description,
        int numSettlements, int numDungeons, int numWilderness)
    {
        Map map(name, width, height, description);

        // Generate settlements
        for (int i = 0; i < numSettlements; ++i) {
            map.addLocation(generateSettlement(width, height));
        }

        // Generate dungeons
        for (int i = 0; i < numDungeons; ++i) {
            map.addLocation(generateDungeon(width, height));
        }

        // Generate wilderness areas
        for (int i = 0; i < numWilderness; ++i) {
            map.addLocation(generateWilderness(width, height));
        }

        // Generate connections
        map.generateConnections(width * 0.3);

        return map;
    }
};

// Sample template JSON creation
void createSampleTemplates(const std::string& filename)
{
    json templates;

    // Name templates
    templates["names"]["settlement"] = {
        "Oakville", "Riverdale", "Highcastle", "Windhelm", "Stormpoint",
        "Dawnguard", "Whiterun", "Falkreath", "Winterhold", "Solitude"
    };

    templates["names"]["dungeon"] = {
        "Forgotten Depths", "Shadow Maze", "Haunted Catacombs", "Ancient Tomb",
        "Dragon's Lair", "Forsaken Mine", "Bandit Hideout", "Cursed Temple",
        "Witch's Cavern", "Lost Sanctuary"
    };

    templates["names"]["wilderness"] = {
        "Misty Woods", "Thunderpeak Mountains", "Sunken Marsh", "Endless Plains",
        "Desolate Wastes", "Frozen Tundra", "Verdant Valley", "Scorched Desert",
        "Enchanted Forest", "Twilight Glades"
    };

    // Features
    templates["features"]["settlement"] = {
        "City walls", "Central market", "Temple district", "Noble quarter",
        "Harbor", "Trade route", "Mining industry", "Farming community",
        "Military garrison", "Famous tavern", "Skilled craftsmen"
    };

    templates["features"]["dungeon"] = {
        "Hidden entrance", "Collapsing passages", "Ancient traps", "Forgotten library",
        "Underground river", "Crystal cavern", "Magical anomalies", "Sacrificial chamber",
        "Treasure vault", "Bottomless pit", "Strange statues"
    };

    templates["features"]["wilderness"] = {
        "Ancient ruins", "Natural spring", "Magical nexus", "Mystical stones",
        "Hidden valley", "Towering cliffs", "Dense fog", "Unique flora",
        "Migratory creatures", "Strange weather", "Breathtaking vistas"
    };

    // Buildings
    templates["buildings"] = {
        "Blacksmith", "Tavern", "Temple", "Town Hall", "Market", "Bakery",
        "Alchemist", "Stable", "Inn", "Guard Tower", "Library", "Warehouse",
        "Mill", "Fishery", "Tannery", "Jewelry Shop", "Tailor"
    };

    // Monsters
    templates["monsters"] = {
        "Goblin", "Orc", "Skeleton", "Zombie", "Ghost", "Vampire", "Werewolf",
        "Troll", "Ogre", "Dragon", "Giant Spider", "Slime", "Bandit", "Cultist",
        "Demon", "Minotaur", "Harpy", "Kobold", "Elemental", "Golem"
    };

    // Treasures
    templates["treasures"] = {
        "Gold coins", "Ancient artifact", "Magic scroll", "Enchanted weapon",
        "Precious gems", "Royal crown", "Ornate armor", "Mystical orb",
        "Healing potion", "Spell book", "Legendary sword", "Cursed ring"
    };

    // Resources
    templates["resources"] = {
        "Timber", "Medicinal herbs", "Wild game", "Berries", "Iron ore",
        "Gold vein", "Silver deposit", "Crystal formation", "Rare plants",
        "Fresh water", "Fertile soil", "Stone quarry", "Clay deposit"
    };

    // Dangers
    templates["dangers"] = {
        "Quicksand", "Poisonous plants", "Flash floods", "Avalanches",
        "Predatory beasts", "Bandits", "Territorial monsters", "Disease",
        "Extreme weather", "Rockslides", "Sinkholes", "Magical corruption"
    };

    // Save to file
    std::ofstream file(filename);
    file << templates.dump(4);
    file.close();
}

int main()
{
    // Create sample templates if needed
    createSampleTemplates("map_templates.json");

    try {
        // Initialize generator with templates
        MapGenerator generator("map_templates.json");

        // Generate a new map
        Map map = generator.generateMap(
            "Realm of Adventure",
            100, // width
            100, // height
            "A vast land filled with settlements, dangerous dungeons, and wild territories.",
            5, // settlements
            7, // dungeons
            8 // wilderness areas
        );

        // Save generated map to file
        map.saveToFile("generated_map.json");
        std::cout << "Map generated and saved to 'generated_map.json'" << std::endl;

        // Example of loading the map back
        Map loadedMap = Map::loadFromFile("generated_map.json");
        std::cout << "Map successfully loaded back from file" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    // Add this pause before exiting
    std::cout << "\nPress Enter to exit...";
    std::cin.get();

    return 0;
}