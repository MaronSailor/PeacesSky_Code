#pragma once

#include <glm.hpp>
#include <gtc/quaternion.hpp>

struct Transform
{
    glm::vec3 position{ 0.0f };
    glm::quat rotation{ 1, 0, 0, 0 };
    glm::vec3 scale{ 1.0f };

    glm::vec3 forward() const
    {
        return rotation * glm::vec3(0, 0, -1);
    }

    glm::vec3 right() const
    {
        return rotation * glm::vec3(1, 0, 0);
    }

    glm::vec3 up() const
    {
        return rotation * glm::vec3(0, 1, 0);
    }
};