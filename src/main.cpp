#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "particle_system.h"
#include "renderer.h"
#include "input_handler.h"

#include <iostream>
#include <string>
#include <sstream>

// ── Configuration ─────────────────────────────────────────────────
constexpr int    INITIAL_PARTICLE_COUNT = 30000;
constexpr float  INITIAL_WIDTH  = 1280.0f;
constexpr float  INITIAL_HEIGHT = 720.0f;
constexpr float  FIXED_DT       = 1.0f / 60.0f;  // Physics timestep (60 Hz)
constexpr float  MOUSE_FORCE    = 4000.0f;
constexpr float  PARTICLE_RADIUS = 2.5f;

static void printControls() {
    std::cout << "\n"
              << "  +================================================+\n"
              << "  |       PARTICLE PHYSICS SIMULATOR                |\n"
              << "  +================================================+\n"
              << "  |  Mouse Controls:                                |\n"
              << "  |    Left Click + Hold  Apply force field         |\n"
              << "  |                                                 |\n"
              << "  |  Force Modes:                                   |\n"
              << "  |    Z   Attract mode (pull particles in)         |\n"
              << "  |    X   Repel mode (push particles away)         |\n"
              << "  |    C   Swirl mode (spin particles around)       |\n"
              << "  |                                                 |\n"
              << "  |  Simulation:                                    |\n"
              << "  |    G       Toggle gravity on/off                |\n"
              << "  |    Space   Pause / Resume                       |\n"
              << "  |    R       Reset all particles                  |\n"
              << "  |    Esc     Quit                                 |\n"
              << "  +================================================+\n"
              << std::endl;
}

int main() {
    printControls();
    // ── GLFW init ──
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow* window = glfwCreateWindow(
        static_cast<int>(INITIAL_WIDTH),
        static_cast<int>(INITIAL_HEIGHT),
        "Particle Physics Simulator",
        nullptr, nullptr
    );

    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // VSync on

    // ── GLAD init ──
    if (!gladLoadGL(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_MULTISAMPLE);

    // ── Initialize systems ──
    ParticleSystem particleSys(INITIAL_PARTICLE_COUNT, INITIAL_WIDTH, INITIAL_HEIGHT);
    particleSys.setParticleRadius(PARTICLE_RADIUS);

    Renderer renderer;
    if (!renderer.init(INITIAL_WIDTH, INITIAL_HEIGHT)) {
        std::cerr << "Failed to initialize renderer" << std::endl;
        return -1;
    }

    InputHandler input;
    input.init(window);

    // ── Fixed timestep accumulator ──
    double lastTime = glfwGetTime();
    float accumulator = 0.0f;
    int frameCount = 0;
    double fpsTimer = lastTime;

    // ── Resize callback data ──
    struct WindowData {
        Renderer* renderer;
        ParticleSystem* particleSys;
    };
    WindowData wdata{&renderer, &particleSys};
    glfwSetWindowUserPointer(window, &wdata);

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* w, int width, int height) {
        auto* data = static_cast<WindowData*>(glfwGetWindowUserPointer(w));
        float fw = static_cast<float>(width);
        float fh = static_cast<float>(height);
        data->renderer->resize(fw, fh);
        data->particleSys->setBoundsSize(fw, fh);
    });

    // ── Main loop ──
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // ESC to close
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        input.update();

        // Handle toggles
        if (input.shouldReset()) {
            particleSys.reset();
            input.clearReset();
        }
        if (input.gravityToggled()) {
            particleSys.setGravityEnabled(!particleSys.gravityEnabled());
            input.clearGravityToggle();
        }

        // ── Fixed timestep physics ──
        double currentTime = glfwGetTime();
        float frameTime = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;

        // Clamp frame time to avoid spiral of death
        if (frameTime > 0.05f) frameTime = 0.05f;

        if (!input.isPaused()) {
            accumulator += frameTime;

            while (accumulator >= FIXED_DT) {
                // Apply mouse force if holding click
                if (input.mouseDown()) {
                    particleSys.applyMouseForce(
                        input.mousePos(), MOUSE_FORCE, input.forceMode()
                    );
                }

                particleSys.update(FIXED_DT);
                accumulator -= FIXED_DT;
            }
        }

        // ── Render ──
        renderer.updateParticles(particleSys.particles());
        renderer.render(particleSys.activeCount(), PARTICLE_RADIUS);

        glfwSwapBuffers(window);

        // ── FPS counter in title bar ──
        frameCount++;
        if (currentTime - fpsTimer >= 0.5) {
            double fps = frameCount / (currentTime - fpsTimer);
            std::ostringstream title;
            title << "Particle Physics Simulator | "
                  << particleSys.activeCount() << " particles | "
                  << static_cast<int>(fps) << " FPS | Mode: ";

            switch (input.forceMode()) {
                case ForceMode::Attract: title << "Attract (Z)"; break;
                case ForceMode::Repel:   title << "Repel (X)";   break;
                case ForceMode::Swirl:   title << "Swirl (C)";   break;
            }

            if (input.isPaused()) title << " | PAUSED";
            if (!particleSys.gravityEnabled()) title << " | No Gravity";

            glfwSetWindowTitle(window, title.str().c_str());
            frameCount = 0;
            fpsTimer = currentTime;
        }
    }

    renderer.cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
