// GuardResponseType.hpp
#pragma once

#include <string>

// Struct for accessing guard response types
struct GuardResponseType {
    static std::string get(const std::string& type);

    static std::string ARREST();
    static std::string ATTACK();
    static std::string FINE();
    static std::string WARN();
    static std::string IGNORE();
};