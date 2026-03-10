#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

enum class ForceMode;  // Forward declaration

class InputHandler {
public:
    void init(GLFWwindow* window);

    // Call each frame
    void update();

    // Mouse state
    glm::vec2 mousePos() const { return m_mousePos; }
    bool mouseDown() const { return m_mouseDown; }

    // Force mode
    ForceMode forceMode() const;

    // Toggle states
    bool isPaused() const { return m_paused; }
    bool shouldReset() const { return m_shouldReset; }
    bool gravityToggled() const { return m_gravityToggled; }

    void clearReset() { m_shouldReset = false; }
    void clearGravityToggle() { m_gravityToggled = false; }

private:
    GLFWwindow* m_window = nullptr;
    glm::vec2   m_mousePos{0.0f};
    bool        m_mouseDown = false;
    bool        m_paused = false;
    bool        m_shouldReset = false;
    bool        m_gravityToggled = false;

    int m_forceModeIndex = 0;  // 0=attract, 1=repel, 2=swirl

    // Key state tracking for edge detection
    bool m_spaceWasPressed = false;
    bool m_rWasPressed = false;
    bool m_gWasPressed = false;
    bool m_zWasPressed = false;
    bool m_xWasPressed = false;
    bool m_cWasPressed = false;
};
