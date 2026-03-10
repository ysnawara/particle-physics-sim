#include "input_handler.h"
#include "particle_system.h"  // For ForceMode enum

void InputHandler::init(GLFWwindow* window) {
    m_window = window;
}

void InputHandler::update() {
    // Mouse position
    double mx, my;
    glfwGetCursorPos(m_window, &mx, &my);
    m_mousePos = glm::vec2(static_cast<float>(mx), static_cast<float>(my));

    // Mouse button
    m_mouseDown = glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    // --- Edge-detected key presses ---

    // Space: toggle pause
    bool spaceNow = glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS;
    if (spaceNow && !m_spaceWasPressed) m_paused = !m_paused;
    m_spaceWasPressed = spaceNow;

    // R: reset
    bool rNow = glfwGetKey(m_window, GLFW_KEY_R) == GLFW_PRESS;
    if (rNow && !m_rWasPressed) m_shouldReset = true;
    m_rWasPressed = rNow;

    // G: toggle gravity
    bool gNow = glfwGetKey(m_window, GLFW_KEY_G) == GLFW_PRESS;
    if (gNow && !m_gWasPressed) m_gravityToggled = true;
    m_gWasPressed = gNow;

    // Z: attract mode
    bool zNow = glfwGetKey(m_window, GLFW_KEY_Z) == GLFW_PRESS;
    if (zNow && !m_zWasPressed) m_forceModeIndex = 0;
    m_zWasPressed = zNow;

    // X: repel mode
    bool xNow = glfwGetKey(m_window, GLFW_KEY_X) == GLFW_PRESS;
    if (xNow && !m_xWasPressed) m_forceModeIndex = 1;
    m_xWasPressed = xNow;

    // C: swirl mode
    bool cNow = glfwGetKey(m_window, GLFW_KEY_C) == GLFW_PRESS;
    if (cNow && !m_cWasPressed) m_forceModeIndex = 2;
    m_cWasPressed = cNow;
}

ForceMode InputHandler::forceMode() const {
    switch (m_forceModeIndex) {
        case 0:  return ForceMode::Attract;
        case 1:  return ForceMode::Repel;
        case 2:  return ForceMode::Swirl;
        default: return ForceMode::Attract;
    }
}
