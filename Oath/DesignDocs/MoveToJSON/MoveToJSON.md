# Using JSON over hardcoded values

### **Benefits of JSON Over `.cpp` Hardcoding**
- **Easier Modding:** Non-programmers can tweak game values and save games in JSON files.
- **More Organized Code:** Keeps `.cpp` files clean.
- **Cross-Platform Compatibility:** JSON files are easy to parse and edit.
- **Debugging Simplicity:** Human-readable format compared to binary `.dat` files.

### 1. **Using JSON for Game Data**
Instead of defining enums, maps, and structs in `.cpp` files, we store them in JSON files, making them easier to modify.

#### **Example: Weather System Data (JSON)**
Instead of hardcoding weather types and probabilities in `AdvancedWeatherSystem.cpp`, create a JSON file (`weather_data.json`):

```json
{
    "seasonal_weather": {
        "spring": {
            "Clear": 0.35,
            "Cloudy": 0.30,
            "Foggy": 0.15,
            "Rainy": 0.15,
            "Stormy": 0.05
        },
        "summer": {
            "Clear": 0.50,
            "Cloudy": 0.20,
            "Foggy": 0.05,
            "Rainy": 0.15,
            "Stormy": 0.10
        }
    },
    "weather_effects": {
        "Foggy": {
            "ReducedVisibility": true
        },
        "Rainy": {
            "SlowMovement": true,
            "PenaltyToSkill": true
        }
    }
}
```

#### **C++ Code to Load JSON Data**
Use a library like [`nlohmann/json`](https://github.com/nlohmann/json) to parse JSON.

```cpp
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class WeatherSystem {
public:
    json weatherData;

    void loadWeatherData(const std::string& filename) {
        std::ifstream file(filename);
        if (!file) {
            std::cerr << "Failed to open " << filename << std::endl;
            return;
        }
        file >> weatherData;
    }

    void printWeatherInfo() {
        for (auto& [season, conditions] : weatherData["seasonal_weather"].items()) {
            std::cout << "Season: " << season << std::endl;
            for (auto& [weather, probability] : conditions.items()) {
                std::cout << "  " << weather << ": " << probability * 100 << "% chance" << std::endl;
            }
        }
    }
};

int main() {
    WeatherSystem ws;
    ws.loadWeatherData("weather_data.json");
    ws.printWeatherInfo();
}
```

---

### 2. **Saving and Loading Game State Using JSON**
Traditionally, game states are saved in binary `.dat` files. We can switch to JSON for easier debugging and modding.

#### **Example: Saving Game State to JSON**
We create a function to save the current game state.

```cpp
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct GameState {
    std::string playerName;
    int health;
    int gold;
    std::string currentRegion;
};

void saveGameState(const GameState& state, const std::string& filename) {
    json saveData;
    saveData["playerName"] = state.playerName;
    saveData["health"] = state.health;
    saveData["gold"] = state.gold;
    saveData["currentRegion"] = state.currentRegion;

    std::ofstream file(filename);
    if (!file) {
        std::cerr << "Failed to open " << filename << " for writing!" << std::endl;
        return;
    }
    file << saveData.dump(4); // Pretty-print JSON with 4 spaces
    std::cout << "Game saved successfully!" << std::endl;
}
```

#### **Example: Loading Game State from JSON**
```cpp
GameState loadGameState(const std::string& filename) {
    GameState state;
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Failed to open " << filename << " for reading!" << std::endl;
        return state;
    }

    json saveData;
    file >> saveData;

    state.playerName = saveData["playerName"];
    state.health = saveData["health"];
    state.gold = saveData["gold"];
    state.currentRegion = saveData["currentRegion"];

    std::cout << "Game loaded successfully!" << std::endl;
    return state;
}
```

#### **Example Usage in Main Function**
```cpp
int main() {
    GameState myGame = {"Hero", 100, 500, "MountainPass"};
    
    saveGameState(myGame, "savegame.json");

    GameState loadedGame = loadGameState("savegame.json");
    std::cout << "Loaded Player: " << loadedGame.playerName << ", Gold: " << loadedGame.gold << std::endl;
}
```

---

### 3. **Converting Other Systems to JSON**
Each system (e.g., Economy, Crime, Factions) can have its own JSON files.

#### **Example: Crime & Law System JSON**
Instead of hardcoded crime types in `CrimeLawSystem.cpp`, we use a `crimes.json` file:

```json
{
    "crimes": {
        "theft": {"bounty": 50, "severity": 2},
        "assault": {"bounty": 100, "severity": 5},
        "murder": {"bounty": 1000, "severity": 10}
    },
    "responses": {
        "fine": {"min_bounty": 0, "max_bounty": 200},
        "arrest": {"min_bounty": 200, "max_bounty": 1000},
        "attack": {"min_bounty": 1000, "max_bounty": 10000}
    }
}
```

#### **C++ Code to Load Crime System JSON**
```cpp
class CrimeSystem {
public:
    json crimeData;

    void loadCrimeData(const std::string& filename) {
        std::ifstream file(filename);
        if (!file) {
            std::cerr << "Failed to open " << filename << std::endl;
            return;
        }
        file >> crimeData;
    }

    void displayCrimes() {
        for (auto& [crime, details] : crimeData["crimes"].items()) {
            std::cout << "Crime: " << crime
                      << " | Bounty: " << details["bounty"]
                      << " | Severity: " << details["severity"] << std::endl;
        }
    }
};

int main() {
    CrimeSystem cs;
    cs.loadCrimeData("crimes.json");
    cs.displayCrimes();
}
```