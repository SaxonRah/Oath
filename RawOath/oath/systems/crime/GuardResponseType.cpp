// GuardResponseType.cpp
#include "GuardResponseType.hpp"

#include "CrimeLawConfig.hpp"

std::string GuardResponseType::get(const std::string& type)
{
    return crimeLawConfig["guardResponseTypes"][type];
}

std::string GuardResponseType::ARREST() { return get("ARREST"); }
std::string GuardResponseType::ATTACK() { return get("ATTACK"); }
std::string GuardResponseType::FINE() { return get("FINE"); }
std::string GuardResponseType::WARN() { return get("WARN"); }
std::string GuardResponseType::IGNORE() { return get("IGNORE"); }