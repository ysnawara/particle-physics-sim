#version 330 core

layout(location = 0) in vec2 aQuadPos;      // Unit quad vertex
layout(location = 1) in vec2 aInstancePos;   // Per-instance particle position
layout(location = 2) in float aSpeed;        // Per-instance speed for coloring

uniform mat4 uProjection;
uniform float uRadius;

out vec2 vLocalPos;   // Quad-local position for circular falloff
out float vSpeed;     // Pass speed to fragment shader

void main() {
    // Scale quad by particle radius and translate to instance position
    vec2 worldPos = aInstancePos + aQuadPos * uRadius;
    gl_Position = uProjection * vec4(worldPos, 0.0, 1.0);

    vLocalPos = aQuadPos;
    vSpeed = aSpeed;
}
