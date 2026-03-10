#version 330 core

in vec2 vLocalPos;
in float vSpeed;

out vec4 fragColor;

// Maps speed to a vibrant color gradient:
// Slow (blue) → Medium (cyan) → Fast (white/yellow) → Very fast (orange/red)
vec3 speedToColor(float speed) {
    float t = clamp(speed / 8.0, 0.0, 1.0);  // Normalize speed

    // Multi-stop gradient
    vec3 c0 = vec3(0.1, 0.2, 0.8);   // Deep blue (still)
    vec3 c1 = vec3(0.0, 0.8, 1.0);   // Cyan
    vec3 c2 = vec3(0.2, 1.0, 0.6);   // Teal-green
    vec3 c3 = vec3(1.0, 0.9, 0.3);   // Warm yellow
    vec3 c4 = vec3(1.0, 0.3, 0.1);   // Hot orange-red

    if (t < 0.25) return mix(c0, c1, t / 0.25);
    if (t < 0.5)  return mix(c1, c2, (t - 0.25) / 0.25);
    if (t < 0.75) return mix(c2, c3, (t - 0.5) / 0.25);
    return mix(c3, c4, (t - 0.75) / 0.25);
}

void main() {
    // Distance from center of the quad (circular particle)
    float dist = length(vLocalPos);

    // Discard pixels outside the circle
    if (dist > 1.0) discard;

    // Soft circular falloff — bright center, fading edges
    float alpha = 1.0 - smoothstep(0.0, 1.0, dist);
    alpha *= alpha;  // Extra falloff for softer glow

    // Speed-based color
    vec3 color = speedToColor(vSpeed);

    // Boost brightness at center for glow effect
    float glow = exp(-dist * 2.0) * 0.5;
    color += vec3(glow);

    fragColor = vec4(color, alpha * 0.6);
}
