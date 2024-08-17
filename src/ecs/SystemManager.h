#pragma once
#include <vector>
#include <memory>
#include <entt.hpp>
#include "System.h" // Include the System header file

class SystemManager
{
public:
    void AddSystem(std::shared_ptr<System> system)
    {
        systems.push_back(system);
    }

    void TickAllSystems(entt::registry &registry, float deltaTime)
    {
        for (auto &system : systems)
        {
            system->Tick(registry, deltaTime);
        }
    }

private:
    std::vector<std::shared_ptr<System>> systems;
};
