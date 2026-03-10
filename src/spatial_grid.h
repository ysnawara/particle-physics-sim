#pragma once

#include <vector>
#include <cstdint>
#include <unordered_map>
#include <glm/glm.hpp>

class SpatialGrid {
public:
    SpatialGrid(float cellSize, int expectedParticles = 50000);

    void clear();
    void insert(int index, glm::vec2 pos);

    // Calls callback(int neighborIndex) for each particle in neighboring cells
    template<typename Func>
    void queryNeighbors(glm::vec2 pos, Func&& callback) const {
        int cx = static_cast<int>(std::floor(pos.x / m_cellSize));
        int cy = static_cast<int>(std::floor(pos.y / m_cellSize));

        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                uint64_t key = hashCell(cx + dx, cy + dy);
                auto it = m_grid.find(key);
                if (it != m_grid.end()) {
                    for (int idx : it->second) {
                        callback(idx);
                    }
                }
            }
        }
    }

private:
    float m_cellSize;
    std::unordered_map<uint64_t, std::vector<int>> m_grid;

    static uint64_t hashCell(int x, int y) {
        // Szudzik's pairing function for signed integers
        uint32_t a = static_cast<uint32_t>(x) * 73856093u;
        uint32_t b = static_cast<uint32_t>(y) * 19349663u;
        return static_cast<uint64_t>(a ^ b);
    }
};
