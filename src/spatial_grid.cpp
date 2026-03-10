#include "spatial_grid.h"

SpatialGrid::SpatialGrid(float cellSize, int expectedParticles)
    : m_cellSize(cellSize)
{
    m_grid.reserve(static_cast<size_t>(expectedParticles / 4));
}

void SpatialGrid::clear() {
    for (auto& [key, bucket] : m_grid) {
        bucket.clear();
    }
}

void SpatialGrid::insert(int index, glm::vec2 pos) {
    int cx = static_cast<int>(std::floor(pos.x / m_cellSize));
    int cy = static_cast<int>(std::floor(pos.y / m_cellSize));
    uint64_t key = hashCell(cx, cy);
    m_grid[key].push_back(index);
}
