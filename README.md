# Particle Physics Simulator

A real-time 2D particle physics simulator built in C++17 with OpenGL 3.3 instanced rendering. Features Verlet integration physics, spatial hash grid collision detection, and interactive mouse-driven force fields.

## Features

- 🔵 **30,000+ particles** simulated at 60fps with instanced GPU rendering
- ⚡ **Verlet integration** for stable, energy-conserving physics
- 🔲 **Spatial hash grid** for O(n) broad-phase collision detection
- 🖱️ **Interactive force fields** — attract, repel, and swirl particles with the mouse
- 🎨 **Velocity-based color gradient** — particles shift from blue (slow) to red (fast)
- 💡 **Soft glow rendering** with additive blending and per-pixel circular falloff
- ⏸️ **Real-time controls** — pause, reset, toggle gravity, switch force modes

## Controls

| Key | Action |
|-----|--------|
| **Left Click + Hold** | Apply force field at cursor |
| **Z** | Switch to Attract mode |
| **X** | Switch to Repel mode |
| **C** | Switch to Swirl mode |
| **Space** | Pause / Resume simulation |
| **G** | Toggle gravity on/off |
| **R** | Reset particles |
| **Esc** | Quit |

## Build

Requires CMake 3.16+ and a C++17 compiler. Dependencies (GLFW, GLM) are fetched automatically.

```bash
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

Run the executable:
```bash
./build/Release/ParticleSim.exe
```

## Architecture

```
src/
├── main.cpp              # Application loop, fixed-timestep accumulator
├── particle.h            # Particle struct (Verlet: position + prev_position)
├── particle_system.h/cpp # Physics: integration, gravity, mouse forces, collisions
├── spatial_grid.h/cpp    # Uniform grid spatial hashing for O(n) collision
├── renderer.h/cpp        # OpenGL instanced rendering pipeline
├── input_handler.h/cpp   # GLFW input with edge-detected key toggles
└── shader_utils.h/cpp    # Shader compilation utilities

shaders/
├── particle.vert         # Instanced vertex shader (quad billboard)
└── particle.frag         # Velocity color gradient + circular soft glow

external/glad/            # Bundled OpenGL 3.3 Core loader
```

## Tech Stack

- **C++17** — modern language features, structured bindings, constexpr
- **OpenGL 3.3 Core** — instanced rendering with `glDrawArraysInstanced`
- **GLFW** — window management and input handling
- **GLM** — mathematics library (vectors, matrices, projections)
- **GLAD** — OpenGL function loader
- **CMake** — cross-platform build system with FetchContent
