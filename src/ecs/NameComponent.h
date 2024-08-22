#pragma once
#include <string>

// Define the NameComponent to store the name of an entity
struct NameComponent
{
    std::string name;

    // Constructor to initialize the name
    NameComponent(const std::string &entityName = "")
        : name(entityName) {}
};
