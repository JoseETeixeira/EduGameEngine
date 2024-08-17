#pragma once
#include <entt.hpp>

class System
{
public:
    virtual ~System() = default;
    virtual void Tick(entt::registry &registry, float deltaTime) = 0; // Pass registry reference to Tick
};
