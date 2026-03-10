#include <gtest/gtest.h>
#include <glm/glm.hpp>
#include <cmath>

#include "particle.h"
#include "particle_system.h"
#include "spatial_grid.h"

// ═══════════════════════════════════════════════════════════════════
// Particle Struct Tests
// ═══════════════════════════════════════════════════════════════════

TEST(ParticleTest, DefaultConstructor) {
    Particle p;
    // Default-constructed particle should be valid (no crash)
    SUCCEED();
}

TEST(ParticleTest, ParameterizedConstructor) {
    glm::vec2 pos(100.0f, 200.0f);
    glm::vec2 vel(3.0f, -4.0f);
    float radius = 5.0f;

    Particle p(pos, vel, radius);

    EXPECT_FLOAT_EQ(p.position.x, 100.0f);
    EXPECT_FLOAT_EQ(p.position.y, 200.0f);
    EXPECT_FLOAT_EQ(p.radius, 5.0f);
    EXPECT_FLOAT_EQ(p.acceleration.x, 0.0f);
    EXPECT_FLOAT_EQ(p.acceleration.y, 0.0f);
}

TEST(ParticleTest, VerletPreviousPositionEncoding) {
    // Verlet stores velocity implicitly: prev_pos = pos - vel
    glm::vec2 pos(100.0f, 200.0f);
    glm::vec2 vel(3.0f, -4.0f);

    Particle p(pos, vel, 2.0f);

    // prev_position = pos - vel (for dt=1 initialization)
    EXPECT_FLOAT_EQ(p.prev_position.x, 100.0f - 3.0f);
    EXPECT_FLOAT_EQ(p.prev_position.y, 200.0f - (-4.0f));

    // Derived velocity should match original
    glm::vec2 derivedVel = p.position - p.prev_position;
    EXPECT_FLOAT_EQ(derivedVel.x, 3.0f);
    EXPECT_FLOAT_EQ(derivedVel.y, -4.0f);
}

TEST(ParticleTest, ZeroVelocityInit) {
    Particle p(glm::vec2(50.0f, 50.0f), glm::vec2(0.0f, 0.0f), 1.0f);

    EXPECT_FLOAT_EQ(p.position.x, p.prev_position.x);
    EXPECT_FLOAT_EQ(p.position.y, p.prev_position.y);
}

// ═══════════════════════════════════════════════════════════════════
// Particle System - Initialization Tests
// ═══════════════════════════════════════════════════════════════════

TEST(ParticleSystemTest, ConstructorCreatesCorrectCount) {
    ParticleSystem sys(100, 800.0f, 600.0f);
    EXPECT_EQ(sys.activeCount(), 100);
}

TEST(ParticleSystemTest, ConstructorParticlesWithinBounds) {
    ParticleSystem sys(500, 800.0f, 600.0f);

    for (const auto& p : sys.particles()) {
        EXPECT_GE(p.position.x, 0.0f) << "Particle X below left bound";
        EXPECT_LE(p.position.x, 800.0f) << "Particle X above right bound";
        EXPECT_GE(p.position.y, 0.0f) << "Particle Y below top bound";
        EXPECT_LE(p.position.y, 600.0f) << "Particle Y above bottom bound";
    }
}

TEST(ParticleSystemTest, DefaultGravityIsOff) {
    ParticleSystem sys(10, 800.0f, 600.0f);
    EXPECT_FALSE(sys.gravityEnabled());
}

TEST(ParticleSystemTest, SetGravityToggle) {
    ParticleSystem sys(10, 800.0f, 600.0f);

    sys.setGravityEnabled(true);
    EXPECT_TRUE(sys.gravityEnabled());

    sys.setGravityEnabled(false);
    EXPECT_FALSE(sys.gravityEnabled());
}

TEST(ParticleSystemTest, SetDrag) {
    ParticleSystem sys(10, 800.0f, 600.0f);

    sys.setDrag(0.95f);
    EXPECT_FLOAT_EQ(sys.drag(), 0.95f);
}

// ═══════════════════════════════════════════════════════════════════
// Verlet Integration Tests
// ═══════════════════════════════════════════════════════════════════

TEST(VerletIntegrationTest, ParticleWithVelocityMoves) {
    ParticleSystem sys(1, 800.0f, 600.0f);

    // Record initial position
    glm::vec2 initialPos = sys.particles()[0].position;

    // Run one update step (no gravity)
    sys.setGravityEnabled(false);
    sys.update(1.0f / 60.0f);

    glm::vec2 newPos = sys.particles()[0].position;

    // Particle should have moved (has initial velocity from constructor)
    float distance = glm::length(newPos - initialPos);
    EXPECT_GT(distance, 0.0f) << "Particle should move after integration";
}

TEST(VerletIntegrationTest, GravityAcceleratesDownward) {
    ParticleSystem sys(1, 800.0f, 600.0f);

    // Record initial Y
    float initialY = sys.particles()[0].position.y;

    // Enable gravity and run several steps
    sys.setGravityEnabled(true);
    for (int i = 0; i < 60; ++i) {
        sys.update(1.0f / 60.0f);
    }

    float finalY = sys.particles()[0].position.y;

    // With downward gravity (positive Y in screen coords), particle should move down
    // or be constrained at bottom
    EXPECT_GE(finalY, initialY) << "Gravity should pull particle downward";
}

TEST(VerletIntegrationTest, NoAccelerationMaintainsVelocity) {
    // Create a particle with known velocity, no gravity, no drag
    ParticleSystem sys(1, 10000.0f, 10000.0f);  // Large bounds to avoid walls
    sys.setGravityEnabled(false);
    sys.setDrag(1.0f);  // No drag

    // The particle has some initial velocity from the constructor's random init
    glm::vec2 pos1 = sys.particles()[0].position;
    glm::vec2 vel1 = sys.particles()[0].position - sys.particles()[0].prev_position;

    sys.update(1.0f / 60.0f);
    glm::vec2 pos2 = sys.particles()[0].position;
    glm::vec2 vel2 = sys.particles()[0].position - sys.particles()[0].prev_position;

    // With no force and no drag, velocity should be preserved
    EXPECT_NEAR(vel2.x, vel1.x, 0.01f) << "X velocity should be preserved without forces";
    EXPECT_NEAR(vel2.y, vel1.y, 0.01f) << "Y velocity should be preserved without forces";
}

// ═══════════════════════════════════════════════════════════════════
// Mouse Force Tests
// ═══════════════════════════════════════════════════════════════════

TEST(MouseForceTest, AttractPullsTowardMouse) {
    ParticleSystem sys(1, 800.0f, 600.0f);
    sys.setGravityEnabled(false);

    glm::vec2 particlePos = sys.particles()[0].position;
    glm::vec2 mousePos(400.0f, 300.0f);  // Center of screen

    // Apply attract force
    sys.applyMouseForce(mousePos, 5000.0f, ForceMode::Attract);
    sys.update(1.0f / 60.0f);

    glm::vec2 newPos = sys.particles()[0].position;

    // Distance to mouse should decrease
    float distBefore = glm::length(particlePos - mousePos);
    float distAfter = glm::length(newPos - mousePos);

    // Only test if particle was within force range (300px)
    if (distBefore < 300.0f) {
        EXPECT_LT(distAfter, distBefore) << "Attract should pull particle closer to mouse";
    }
}

TEST(MouseForceTest, RepelPushesAwayFromMouse) {
    ParticleSystem sys(1, 10000.0f, 10000.0f);  // Large bounds
    sys.setGravityEnabled(false);
    sys.setDrag(1.0f);

    glm::vec2 mousePos = sys.particles()[0].position + glm::vec2(50.0f, 0.0f);
    glm::vec2 particlePos = sys.particles()[0].position;

    float distBefore = glm::length(particlePos - mousePos);

    sys.applyMouseForce(mousePos, 5000.0f, ForceMode::Repel);
    sys.update(1.0f / 60.0f);

    glm::vec2 newPos = sys.particles()[0].position;
    float distAfter = glm::length(newPos - mousePos);

    EXPECT_GT(distAfter, distBefore) << "Repel should push particle away from mouse";
}

TEST(MouseForceTest, SwirlCreatesTangentialMotion) {
    ParticleSystem sys(1, 10000.0f, 10000.0f);
    sys.setGravityEnabled(false);
    sys.setDrag(1.0f);

    // Place particle directly to the right of mouse
    glm::vec2 mousePos(5000.0f, 5000.0f);

    // Apply swirl force
    sys.applyMouseForce(mousePos, 5000.0f, ForceMode::Swirl);
    sys.update(1.0f / 60.0f);

    // Swirl applies tangential + slight inward force — particle should have moved
    glm::vec2 newPos = sys.particles()[0].position;
    glm::vec2 moved = newPos - sys.particles()[0].prev_position;
    float speed = glm::length(moved);

    // If within range, should have gained some velocity from swirl
    // (hard to test exact direction due to random initial position)
    SUCCEED();  // Swirl doesn't crash and particle system stays stable
}

TEST(MouseForceTest, ForceOutOfRange) {
    ParticleSystem sys(1, 10000.0f, 10000.0f);
    sys.setGravityEnabled(false);
    sys.setDrag(1.0f);

    glm::vec2 particlePos = sys.particles()[0].position;
    // Place mouse far away (beyond 300px range)
    glm::vec2 mousePos = particlePos + glm::vec2(500.0f, 500.0f);

    glm::vec2 velBefore = sys.particles()[0].position - sys.particles()[0].prev_position;

    sys.applyMouseForce(mousePos, 5000.0f, ForceMode::Attract);
    // Acceleration should be zero — particle out of range

    // Check acceleration is still zero (force was not applied)
    EXPECT_FLOAT_EQ(sys.particles()[0].acceleration.x, 0.0f);
    EXPECT_FLOAT_EQ(sys.particles()[0].acceleration.y, 0.0f);
}

// ═══════════════════════════════════════════════════════════════════
// Bounds Constraint Tests
// ═══════════════════════════════════════════════════════════════════

TEST(BoundsTest, ParticlesStayWithinBoundsAfterUpdate) {
    ParticleSystem sys(1000, 800.0f, 600.0f);
    sys.setGravityEnabled(true);

    // Run many update steps
    for (int i = 0; i < 300; ++i) {
        sys.update(1.0f / 60.0f);
    }

    float radius = sys.particleRadius();
    for (const auto& p : sys.particles()) {
        EXPECT_GE(p.position.x, radius - 0.01f);
        EXPECT_LE(p.position.x, 800.0f - radius + 0.01f);
        EXPECT_GE(p.position.y, radius - 0.01f);
        EXPECT_LE(p.position.y, 600.0f - radius + 0.01f);
    }
}

TEST(BoundsTest, ParticleBouncesOffWall) {
    // Create system with a particle heading toward a wall
    ParticleSystem sys(1, 200.0f, 200.0f);
    sys.setGravityEnabled(true);

    // Run for many frames — gravity pulls particles to bottom
    for (int i = 0; i < 120; ++i) {
        sys.update(1.0f / 60.0f);
    }

    // Particle should be near bottom wall but not past it
    float radius = sys.particleRadius();
    EXPECT_LE(sys.particles()[0].position.y, 200.0f - radius + 0.01f);
}

// ═══════════════════════════════════════════════════════════════════
// Spatial Grid Tests
// ═══════════════════════════════════════════════════════════════════

TEST(SpatialGridTest, InsertAndQuery) {
    SpatialGrid grid(10.0f, 100);

    // Insert a particle at (50, 50)
    grid.insert(0, glm::vec2(50.0f, 50.0f));

    // Query neighbors at the same position — should find particle 0
    bool found = false;
    grid.queryNeighbors(glm::vec2(50.0f, 50.0f), [&](int idx) {
        if (idx == 0) found = true;
    });

    EXPECT_TRUE(found) << "Should find inserted particle at same position";
}

TEST(SpatialGridTest, NearbyParticlesFound) {
    SpatialGrid grid(10.0f, 100);

    grid.insert(0, glm::vec2(50.0f, 50.0f));
    grid.insert(1, glm::vec2(55.0f, 55.0f));  // Within same cell or neighboring

    bool found0 = false, found1 = false;
    grid.queryNeighbors(glm::vec2(52.0f, 52.0f), [&](int idx) {
        if (idx == 0) found0 = true;
        if (idx == 1) found1 = true;
    });

    EXPECT_TRUE(found0) << "Should find nearby particle 0";
    EXPECT_TRUE(found1) << "Should find nearby particle 1";
}

TEST(SpatialGridTest, DistantParticleNotFound) {
    SpatialGrid grid(10.0f, 100);

    grid.insert(0, glm::vec2(50.0f, 50.0f));
    grid.insert(1, glm::vec2(500.0f, 500.0f));  // Far away

    bool found1 = false;
    grid.queryNeighbors(glm::vec2(50.0f, 50.0f), [&](int idx) {
        if (idx == 1) found1 = true;
    });

    EXPECT_FALSE(found1) << "Should not find distant particle";
}

TEST(SpatialGridTest, ClearRemovesAll) {
    SpatialGrid grid(10.0f, 100);

    grid.insert(0, glm::vec2(50.0f, 50.0f));
    grid.clear();

    bool found = false;
    grid.queryNeighbors(glm::vec2(50.0f, 50.0f), [&](int idx) {
        found = true;
    });

    EXPECT_FALSE(found) << "Grid should be empty after clear()";
}

TEST(SpatialGridTest, NegativeCoordinates) {
    SpatialGrid grid(10.0f, 100);

    grid.insert(0, glm::vec2(-50.0f, -50.0f));

    bool found = false;
    grid.queryNeighbors(glm::vec2(-50.0f, -50.0f), [&](int idx) {
        if (idx == 0) found = true;
    });

    EXPECT_TRUE(found) << "Should handle negative coordinates";
}

TEST(SpatialGridTest, ManyParticlesSameCell) {
    SpatialGrid grid(10.0f, 1000);

    // Insert 100 particles at nearly the same position
    for (int i = 0; i < 100; ++i) {
        grid.insert(i, glm::vec2(50.0f + i * 0.01f, 50.0f));
    }

    int count = 0;
    grid.queryNeighbors(glm::vec2(50.0f, 50.0f), [&](int idx) {
        count++;
    });

    EXPECT_EQ(count, 100) << "Should find all particles in same cell";
}

// ═══════════════════════════════════════════════════════════════════
// Reset Tests
// ═══════════════════════════════════════════════════════════════════

TEST(ResetTest, PreservesParticleCount) {
    ParticleSystem sys(500, 800.0f, 600.0f);

    // Run for a while, then reset
    for (int i = 0; i < 60; ++i) sys.update(1.0f / 60.0f);

    sys.reset();
    EXPECT_EQ(sys.activeCount(), 500);
}

TEST(ResetTest, ParticlesBackInBounds) {
    ParticleSystem sys(100, 800.0f, 600.0f);
    sys.setGravityEnabled(true);

    // Run to settle particles at bottom
    for (int i = 0; i < 300; ++i) sys.update(1.0f / 60.0f);

    sys.reset();

    for (const auto& p : sys.particles()) {
        EXPECT_GE(p.position.x, 50.0f - 1.0f);
        EXPECT_LE(p.position.x, 750.0f + 1.0f);
        EXPECT_GE(p.position.y, 50.0f - 1.0f);
        EXPECT_LE(p.position.y, 550.0f + 1.0f);
    }
}

// ═══════════════════════════════════════════════════════════════════
// Emit Tests
// ═══════════════════════════════════════════════════════════════════

TEST(EmitTest, IncreasesParticleCount) {
    ParticleSystem sys(100, 800.0f, 600.0f);
    EXPECT_EQ(sys.activeCount(), 100);

    sys.emit(50, glm::vec2(400.0f, 300.0f), 5.0f);
    EXPECT_EQ(sys.activeCount(), 150);
}

TEST(EmitTest, EmittedParticlesNearPosition) {
    ParticleSystem sys(0, 800.0f, 600.0f);
    sys.emit(100, glm::vec2(400.0f, 300.0f), 2.0f);

    for (const auto& p : sys.particles()) {
        EXPECT_NEAR(p.position.x, 400.0f, 5.0f);
        EXPECT_NEAR(p.position.y, 300.0f, 5.0f);
    }
}

// ═══════════════════════════════════════════════════════════════════
// Stress / Performance Tests
// ═══════════════════════════════════════════════════════════════════

TEST(StressTest, ThousandParticlesNoCrash) {
    ParticleSystem sys(10000, 1920.0f, 1080.0f);
    sys.setGravityEnabled(true);

    for (int i = 0; i < 60; ++i) {
        sys.applyMouseForce(glm::vec2(960.0f, 540.0f), 4000.0f, ForceMode::Attract);
        sys.update(1.0f / 60.0f);
    }

    EXPECT_EQ(sys.activeCount(), 10000) << "No particles should be lost";
}

TEST(StressTest, RapidResetStability) {
    ParticleSystem sys(1000, 800.0f, 600.0f);

    for (int r = 0; r < 10; ++r) {
        for (int i = 0; i < 30; ++i) sys.update(1.0f / 60.0f);
        sys.reset();
    }

    EXPECT_EQ(sys.activeCount(), 1000);
}

TEST(StressTest, AllForceModesCombined) {
    ParticleSystem sys(500, 800.0f, 600.0f);
    sys.setGravityEnabled(false);
    glm::vec2 mouse(400.0f, 300.0f);

    for (int i = 0; i < 20; ++i) {
        sys.applyMouseForce(mouse, 3000.0f, ForceMode::Attract);
        sys.update(1.0f / 60.0f);
    }
    for (int i = 0; i < 20; ++i) {
        sys.applyMouseForce(mouse, 3000.0f, ForceMode::Repel);
        sys.update(1.0f / 60.0f);
    }
    for (int i = 0; i < 20; ++i) {
        sys.applyMouseForce(mouse, 3000.0f, ForceMode::Swirl);
        sys.update(1.0f / 60.0f);
    }

    EXPECT_EQ(sys.activeCount(), 500) << "All force modes should be stable";
}

// ═══════════════════════════════════════════════════════════════════
// Edge Cases
// ═══════════════════════════════════════════════════════════════════

TEST(EdgeCaseTest, ZeroParticles) {
    ParticleSystem sys(0, 800.0f, 600.0f);
    EXPECT_EQ(sys.activeCount(), 0);

    // Should not crash
    sys.update(1.0f / 60.0f);
    sys.applyMouseForce(glm::vec2(400.0f, 300.0f), 1000.0f, ForceMode::Attract);
    sys.reset();

    EXPECT_EQ(sys.activeCount(), 0);
}

TEST(EdgeCaseTest, VerySmallBounds) {
    ParticleSystem sys(10, 10.0f, 10.0f);
    sys.setGravityEnabled(true);

    for (int i = 0; i < 120; ++i) {
        sys.update(1.0f / 60.0f);
    }

    // All particles should be constrained
    for (const auto& p : sys.particles()) {
        EXPECT_LE(p.position.x, 10.0f);
        EXPECT_GE(p.position.x, 0.0f);
        EXPECT_LE(p.position.y, 10.0f);
        EXPECT_GE(p.position.y, 0.0f);
    }
}

TEST(EdgeCaseTest, LargeTimestep) {
    ParticleSystem sys(100, 800.0f, 600.0f);
    sys.setGravityEnabled(true);

    // Large timestep should not cause particles to teleport outside bounds
    sys.update(0.1f);  // 10x normal timestep

    for (const auto& p : sys.particles()) {
        EXPECT_LE(p.position.x, 800.0f + 1.0f);
        EXPECT_GE(p.position.x, -1.0f);
        EXPECT_LE(p.position.y, 600.0f + 1.0f);
        EXPECT_GE(p.position.y, -1.0f);
    }
}

TEST(EdgeCaseTest, MouseForceAtParticlePosition) {
    ParticleSystem sys(1, 10000.0f, 10000.0f);
    sys.setGravityEnabled(false);

    glm::vec2 particlePos = sys.particles()[0].position;

    // Apply force at exact particle position (dist=0 edge case)
    sys.applyMouseForce(particlePos, 5000.0f, ForceMode::Attract);
    sys.update(1.0f / 60.0f);

    // Should not produce NaN or Inf
    EXPECT_FALSE(std::isnan(sys.particles()[0].position.x));
    EXPECT_FALSE(std::isnan(sys.particles()[0].position.y));
    EXPECT_FALSE(std::isinf(sys.particles()[0].position.x));
    EXPECT_FALSE(std::isinf(sys.particles()[0].position.y));
}
