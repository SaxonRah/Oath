// CrimeType.hpp
#pragma once

#include <string>

// Struct for accessing crime types
struct CrimeType {

    static std::string get(const std::string& type);

    static std::string THEFT();
    static std::string ASSAULT();
    static std::string MURDER();
    static std::string TRESPASSING();
    static std::string VANDALISM();
    static std::string PICKPOCKETING();
    static std::string PRISON_BREAK();
};