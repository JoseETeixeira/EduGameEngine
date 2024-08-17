#pragma once
#include <glm.hpp>
#include "System.h"
#include <entt.hpp>
#include <iostream>

// Define components outside of the MovementSystem class
struct PositionComponent
{
    PositionComponent(const glm::vec3 &pos) : position(pos) {}
    glm::vec3 position;
};

struct VelocityComponent
{
    VelocityComponent(const glm::vec3 &vel) : velocity(vel) {}
    glm::vec3 velocity;
};

class MovementSystem : public System
{
public:
    void Tick(entt::registry &registry, float deltaTime) override
    {
        auto view = registry.view<PositionComponent, VelocityComponent>();

        for (auto entity : view)
        {
            auto &position = view.get<PositionComponent>(entity);
            const auto &velocity = view.get<VelocityComponent>(entity);

            // Update position based on velocity
            position.position += velocity.velocity * deltaTime;
            std::cout << position.position.x << ", " << position.position.y << ", " << position.position.z << std::endl;
        }
    }
};
