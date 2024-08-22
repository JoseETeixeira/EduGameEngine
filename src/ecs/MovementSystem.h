#pragma once
#include <glm.hpp>
#include "System.h"
#include <entt.hpp>
#include <iostream>

// Define the TransformComponent to include position, rotation, and scale
// Ensure TransformComponent is defined
struct TransformComponent
{
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    entt::entity parent;

    TransformComponent(const glm::vec3 &pos = glm::vec3(0.0f),
                       const glm::vec3 &rot = glm::vec3(0.0f),
                       const glm::vec3 &scl = glm::vec3(1.0f),
                       entt::entity par = entt::null)
        : position(pos), rotation(rot), scale(scl), parent(par) {}
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
        // Get a view of entities with both TransformComponent and VelocityComponent
        auto view = registry.view<TransformComponent, VelocityComponent>();

        for (auto entity : view)
        {
            auto &transform = view.get<TransformComponent>(entity);
            const auto &velocity = view.get<VelocityComponent>(entity);

            // Update position based on velocity
            transform.position += velocity.velocity * deltaTime;

            // Debugging output to check positions
            std::cout << "Entity " << static_cast<uint32_t>(entity) << " Position: "
                      << transform.position.x << ", " << transform.position.y << ", " << transform.position.z << std::endl;
        }
    }
};
