#pragma once

#include "particle.h"
#include "spatial_grid.h"
#include <vector>
#include <random>
#include <glm/glm.hpp>

enum class ForceMode {
    Attract,
    Repel,
    Swirl
};

class ParticleSystem {
public:
    ParticleSystem(int count, float width, float height);

    void update(float dt);
    void applyGravity(glm::vec2 gravity);
    void applyMouseForce(glm::vec2 mousePos, float strength, ForceMode mode);
    void resolveCollisions();
    void constrainBounds();
    void emit(int count, glm::vec2 position, float spread);
    void reset();

    const std::vector<Particle>& particles() const { return m_particles; }
    int activeCount() const { return static_cast<int>(m_particles.size()); }

    void setGravityEnabled(bool enabled) { m_gravityEnabled = enabled; }
    bool gravityEnabled() const { return m_gravityEnabled; }

    void setDrag(float drag) { m_drag = drag; }
    float drag() const { return m_drag; }

    void setParticleRadius(float r) { m_particleRadius = r; }
    float particleRadius() const { return m_particleRadius; }

    void setBoundsSize(float w, float h) { m_width = w; m_height = h; }

private:
    std::vector<Particle> m_particles;
    SpatialGrid m_grid;
    std::mt19937 m_rng;

    float m_width;
    float m_height;
    float m_particleRadius = 2.0f;
    float m_drag = 0.998f;
    bool  m_gravityEnabled = false;
    float m_restitution = 0.9f;

    void integrateVerlet(float dt);
};
