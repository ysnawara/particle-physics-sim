#include "renderer.h"
#include "shader_utils.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

Renderer::Renderer() {}

Renderer::~Renderer() {
    cleanup();
}

bool Renderer::init(float screenWidth, float screenHeight) {
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;

    // ── Load and compile shaders ──
    std::string vertSrc = ShaderUtils::loadFile("shaders/particle.vert");
    std::string fragSrc = ShaderUtils::loadFile("shaders/particle.frag");

    if (vertSrc.empty() || fragSrc.empty()) {
        std::cerr << "Failed to load shader files" << std::endl;
        return false;
    }

    m_shaderProgram = ShaderUtils::createProgram(vertSrc.c_str(), fragSrc.c_str());
    if (!m_shaderProgram) return false;

    m_projLoc = glGetUniformLocation(m_shaderProgram, "uProjection");
    m_radiusLoc = glGetUniformLocation(m_shaderProgram, "uRadius");

    // ── Set up instanced quad ──
    // Unit quad vertices (will be scaled by radius in vertex shader)
    float quadVertices[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f,  1.0f,
    };

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_quadVBO);
    glGenBuffers(1, &m_instanceVBO);

    glBindVertexArray(m_vao);

    // Quad vertices (attribute 0)
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    // Instance data buffer (attributes 1 = position, 2 = speed)
    glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);
    // Allocate with GL_DYNAMIC_DRAW — updated every frame
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    // Attribute 1: instance position (vec2)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(InstanceData),
                          reinterpret_cast<void*>(offsetof(InstanceData, position)));
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);  // Advance once per instance

    // Attribute 2: instance speed (float)
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(InstanceData),
                          reinterpret_cast<void*>(offsetof(InstanceData, speed)));
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);

    glBindVertexArray(0);

    // Enable blending for soft particles
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);  // Additive blending for glow

    return true;
}

void Renderer::updateParticles(const std::vector<Particle>& particles) {
    m_instanceBuffer.resize(particles.size());

    for (size_t i = 0; i < particles.size(); ++i) {
        const auto& p = particles[i];
        glm::vec2 vel = p.position - p.prev_position;
        float speed = glm::length(vel);

        m_instanceBuffer[i].position = p.position;
        m_instanceBuffer[i].speed = speed;
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);
    GLsizeiptr dataSize = static_cast<GLsizeiptr>(m_instanceBuffer.size() * sizeof(InstanceData));

    // Orphan the buffer then upload — avoids GPU stalls
    glBufferData(GL_ARRAY_BUFFER, dataSize, nullptr, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, m_instanceBuffer.data());
}

void Renderer::render(int particleCount, float particleRadius) {
    glClearColor(0.02f, 0.02f, 0.05f, 1.0f);  // Near-black background
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(m_shaderProgram);

    // Orthographic projection matching screen pixels
    glm::mat4 proj = glm::ortho(0.0f, m_screenWidth, m_screenHeight, 0.0f, -1.0f, 1.0f);
    glUniformMatrix4fv(m_projLoc, 1, GL_FALSE, glm::value_ptr(proj));
    glUniform1f(m_radiusLoc, particleRadius);

    glBindVertexArray(m_vao);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, particleCount);
    glBindVertexArray(0);
}

void Renderer::resize(float width, float height) {
    m_screenWidth = width;
    m_screenHeight = height;
    glViewport(0, 0, static_cast<int>(width), static_cast<int>(height));
}

void Renderer::cleanup() {
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_quadVBO) glDeleteBuffers(1, &m_quadVBO);
    if (m_instanceVBO) glDeleteBuffers(1, &m_instanceVBO);
    if (m_shaderProgram) glDeleteProgram(m_shaderProgram);
    m_vao = m_quadVBO = m_instanceVBO = m_shaderProgram = 0;
}
