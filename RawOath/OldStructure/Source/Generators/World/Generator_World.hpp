#ifndef GENERATOR_WORLD_HPP
#define GENERATOR_WORLD_HPP

#include <cmath>
#include <memory>
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
    Random();
    int getInt(int min, int max);
    double getDouble(double min, double max);
    bool getBool(double probability = 0.5);

    template <typename T>
    T getRandomElement(const std::vector<T>& vec);
};

// Global random instance declaration
extern Random random;

// Represents a point on the map
struct Point {
    int x;
    int y;

    Point(int x = 0, int y = 0);
    double distanceTo(const Point& other) const;
    json toJson() const;
    static Point fromJson(const json& j);
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
    Location(const std::string& name, const std::string& type, const Point& position, int size);
    virtual ~Location() = default;

    const std::string& getName() const;
    const std::string& getType() const;
    const Point& getPosition() const;
    int getSize() const;

    void addFeature(const std::string& feature);
    virtual json toJson() const;
    static std::unique_ptr<Location> fromJson(const json& j);
};

// City/Town location type
class Settlement : public Location {
private:
    int population;
    std::vector<std::string> buildings;

public:
    Settlement(const std::string& name, const Point& position, int size, int population);
    void addBuilding(const std::string& building);
    json toJson() const override;
    static std::unique_ptr<Settlement> fromJson(const json& j);
};

// Dungeon location type
class Dungeon : public Location {
private:
    int depth;
    std::string difficulty;
    std::vector<std::string> monsters;
    std::vector<std::string> treasures;

public:
    Dungeon(const std::string& name, const Point& position, int size, int depth, const std::string& difficulty);
    void addMonster(const std::string& monster);
    void addTreasure(const std::string& treasure);
    json toJson() const override;
    static std::unique_ptr<Dungeon> fromJson(const json& j);
};

// Wilderness location type
class Wilderness : public Location {
private:
    std::string terrain;
    std::vector<std::string> resources;
    std::vector<std::string> dangers;

public:
    Wilderness(const std::string& name, const Point& position, int size, const std::string& terrain);
    void addResource(const std::string& resource);
    void addDanger(const std::string& danger);
    json toJson() const override;
    static std::unique_ptr<Wilderness> fromJson(const json& j);
};

// Represents connection between locations
struct Connection {
    int from;
    int to;
    std::string type;
    int difficulty;

    Connection(int from, int to, const std::string& type, int difficulty);
    json toJson() const;
    static Connection fromJson(const json& j);
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
    Map(const std::string& name, int width, int height, const std::string& description);
    void addLocation(std::unique_ptr<Location> location);
    void addConnection(const Connection& connection);
    void generateConnections(double maxDistance, double connectionProbability = 0.7);
    json toJson() const;
    static Map fromJson(const json& j);
    void saveToFile(const std::string& filename) const;
    static Map loadFromFile(const std::string& filename);
};

// Map generator class
class MapGenerator {
private:
    json templates;

    void loadTemplates(const std::string& filename);
    std::string generateName(const std::string& type) const;
    std::vector<std::string> generateFeatures(const std::string& type, int count) const;
    std::unique_ptr<Settlement> generateSettlement(int mapWidth, int mapHeight);
    std::unique_ptr<Dungeon> generateDungeon(int mapWidth, int mapHeight);
    std::unique_ptr<Wilderness> generateWilderness(int mapWidth, int mapHeight);

public:
    MapGenerator(const std::string& templateFile);
    Map generateMap(const std::string& name, int width, int height, const std::string& description,
        int numSettlements, int numDungeons, int numWilderness);
};

// Function to create sample templates
void createSampleTemplates(const std::string& filename);

// Template function implementation must be in the header
template <typename T>
T Random::getRandomElement(const std::vector<T>& vec)
{
    if (vec.empty())
        throw std::runtime_error("Cannot select from empty vector");
    return vec[getInt(0, vec.size() - 1)];
}

#endif // GENERATOR_WORLD_HPP