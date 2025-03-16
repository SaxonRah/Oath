// CrimeType.cpp
#include "CrimeType.hpp"

#include "CrimeLawConfig.hpp"

std::string CrimeType::get(const std::string& type)
{
    return crimeLawConfig["crimeTypes"][type];
}

std::string CrimeType::THEFT() { return get("THEFT"); }
std::string CrimeType::ASSAULT() { return get("ASSAULT"); }
std::string CrimeType::MURDER() { return get("MURDER"); }
std::string CrimeType::TRESPASSING() { return get("TRESPASSING"); }
std::string CrimeType::VANDALISM() { return get("VANDALISM"); }
std::string CrimeType::PICKPOCKETING() { return get("PICKPOCKETING"); }
std::string CrimeType::PRISON_BREAK() { return get("PRISON_BREAK"); }