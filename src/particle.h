#pragma once

#include <glm/glm.hpp>

struct Particle {
    glm::vec2 position;
    glm::vec2 prev_position;
    glm::vec2 acceleration;
    float     radius;

    Particle() = default;

    Particle(glm::vec2 pos, glm::vec2 vel, float r)
        : position(pos)
        , prev_position(pos - vel)   // Verlet: prev = pos - vel*dt (dt=1 for init)
        , acceleration(0.0f)
        , radius(r)
    {}
};
