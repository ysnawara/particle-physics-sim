#include "particle_system.h"
#include <cmath>
#include <algorithm>

ParticleSystem::ParticleSystem(int count, float width, float height)
    : m_grid(m_particleRadius * 4.0f, count)
    , m_rng(42)
    , m_width(width)
    , m_height(height)
{
    m_particles.reserve(count);

    std::uniform_real_distribution<float> distX(50.0f, width - 50.0f);
    std::uniform_real_distribution<float> distY(50.0f, height - 50.0f);
    std::uniform_real_distribution<float> distV(-2.0f, 2.0f);

    for (int i = 0; i < count; ++i) {
        glm::vec2 pos(distX(m_rng), distY(m_rng));
        glm::vec2 vel(distV(m_rng), distV(m_rng));
        m_particles.emplace_back(pos, vel, m_particleRadius);
    }
}

void ParticleSystem::update(float dt) {
    // Apply gravity
    if (m_gravityEnabled) {
        applyGravity(glm::vec2(0.0f, 300.0f));  // Downward gravity (screen coords)
    }

    // Integrate positions using Verlet
    integrateVerlet(dt);

    // Keep particles within bounds
    constrainBounds();
}

void ParticleSystem::integrateVerlet(float dt) {
    float dt2 = dt * dt;

    for (auto& p : m_particles) {
        glm::vec2 velocity = p.position - p.prev_position;

        // Apply drag to velocity
        velocity *= m_drag;

        // Verlet: new_pos = pos + velocity + acc * dt^2
        glm::vec2 newPos = p.position + velocity + p.acceleration * dt2;

        p.prev_position = p.position;
        p.position = newPos;
        p.acceleration = glm::vec2(0.0f);  // Reset acceleration each frame
    }
}

void ParticleSystem::applyGravity(glm::vec2 gravity) {
    for (auto& p : m_particles) {
        p.acceleration += gravity;
    }
}

void ParticleSystem::applyMouseForce(glm::vec2 mousePos, float strength, ForceMode mode) {
    for (auto& p : m_particles) {
        glm::vec2 diff = mousePos - p.position;
        float dist = glm::length(diff);

        if (dist < 1.0f) dist = 1.0f;

        // Force falls off with distance, capped at a radius
        float maxRadius = 300.0f;
        if (dist > maxRadius) continue;

        float falloff = 1.0f - (dist / maxRadius);
        falloff *= falloff;  // Quadratic falloff for smoother feel

        glm::vec2 dir = diff / dist;  // Normalized direction

        switch (mode) {
            case ForceMode::Attract:
                p.acceleration += dir * strength * falloff;
                break;
            case ForceMode::Repel:
                p.acceleration -= dir * strength * falloff;
                break;
            case ForceMode::Swirl: {
                // Tangential force (perpendicular to radial direction)
                glm::vec2 tangent(-dir.y, dir.x);
                p.acceleration += tangent * strength * falloff * 0.8f;
                // Slight inward pull to keep the swirl together
                p.acceleration += dir * strength * falloff * 0.2f;
                break;
            }
        }
    }
}

void ParticleSystem::resolveCollisions() {
    float diameter = m_particleRadius * 2.0f;

    // Rebuild spatial grid
    m_grid.clear();
    for (int i = 0; i < static_cast<int>(m_particles.size()); ++i) {
        m_grid.insert(i, m_particles[i].position);
    }

    // Check neighbors
    for (int i = 0; i < static_cast<int>(m_particles.size()); ++i) {
        Particle& a = m_particles[i];

        m_grid.queryNeighbors(a.position, [&](int j) {
            if (j <= i) return;  // Skip self and already-checked pairs

            Particle& b = m_particles[j];
            glm::vec2 diff = b.position - a.position;
            float dist = glm::length(diff);

            if (dist < diameter && dist > 0.001f) {
                // Particles overlap — push them apart
                glm::vec2 normal = diff / dist;
                float overlap = diameter - dist;

                // Move each particle half the overlap distance
                a.position -= normal * (overlap * 0.5f);
                b.position += normal * (overlap * 0.5f);
            }
        });
    }
}

void ParticleSystem::constrainBounds() {
    float r = m_particleRadius;

    for (auto& p : m_particles) {
        // Derive velocity for bounce response
        glm::vec2 vel = p.position - p.prev_position;

        // Left wall
        if (p.position.x < r) {
            p.position.x = r;
            p.prev_position.x = p.position.x + vel.x * m_restitution;
        }
        // Right wall
        if (p.position.x > m_width - r) {
            p.position.x = m_width - r;
            p.prev_position.x = p.position.x + vel.x * m_restitution;
        }
        // Top wall
        if (p.position.y < r) {
            p.position.y = r;
            p.prev_position.y = p.position.y + vel.y * m_restitution;
        }
        // Bottom wall
        if (p.position.y > m_height - r) {
            p.position.y = m_height - r;
            p.prev_position.y = p.position.y + vel.y * m_restitution;
        }
    }
}

void ParticleSystem::emit(int count, glm::vec2 position, float spread) {
    std::uniform_real_distribution<float> dist(-spread, spread);
    for (int i = 0; i < count; ++i) {
        glm::vec2 vel(dist(m_rng), dist(m_rng));
        m_particles.emplace_back(position, vel, m_particleRadius);
    }
}

void ParticleSystem::reset() {
    int count = static_cast<int>(m_particles.size());
    m_particles.clear();

    std::uniform_real_distribution<float> distX(50.0f, m_width - 50.0f);
    std::uniform_real_distribution<float> distY(50.0f, m_height - 50.0f);
    std::uniform_real_distribution<float> distV(-2.0f, 2.0f);

    for (int i = 0; i < count; ++i) {
        glm::vec2 pos(distX(m_rng), distY(m_rng));
        glm::vec2 vel(distV(m_rng), distV(m_rng));
        m_particles.emplace_back(pos, vel, m_particleRadius);
    }
}
