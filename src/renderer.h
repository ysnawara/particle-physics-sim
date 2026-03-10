#pragma once

#include "particle.h"
#include <glad/glad.h>
#include <vector>

class Renderer {
public:
    Renderer();
    ~Renderer();

    bool init(float screenWidth, float screenHeight);
    void updateParticles(const std::vector<Particle>& particles);
    void render(int particleCount, float particleRadius);
    void resize(float width, float height);
    void cleanup();

private:
    GLuint m_shaderProgram = 0;

    // Instanced rendering buffers
    GLuint m_vao = 0;
    GLuint m_quadVBO = 0;      // Static quad vertex data
    GLuint m_instanceVBO = 0;  // Dynamic per-particle data

    // Uniform locations
    GLint m_projLoc = -1;
    GLint m_radiusLoc = -1;

    float m_screenWidth = 0.0f;
    float m_screenHeight = 0.0f;

    // Per-instance data streamed to GPU
    struct InstanceData {
        glm::vec2 position;
        float     speed;  // For velocity-based color mapping
    };
    std::vector<InstanceData> m_instanceBuffer;
};
