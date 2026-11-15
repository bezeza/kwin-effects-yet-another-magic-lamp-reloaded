/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 */

#pragma once

#include <cstdint>
#include <limits>

namespace CoordinateMapper {

// KWin's WindowQuadList::makeGrid() expects a strictly positive maximum
// quad size. A value of zero makes its subdivision loops non-terminating.
// Very small values can also create an excessive number of quads per frame.
// Keep the persisted GridResolution key for compatibility, but sanitize it
// before it reaches the KWin rendering API.
inline constexpr unsigned int MinGridCellSize = 16;
inline constexpr unsigned int MaxGridCellSize = 99;
inline constexpr unsigned int MaxAnimationDurationMs = 9999;
inline constexpr unsigned int MaxBumpDistance = 99;
 
// Estimate the number of rectangular cells produced for one full-window quad.
// A real KWin WindowQuadList may contain multiple source quads, so this is a
// diagnostic estimate rather than a promise about the final rendered mesh.
constexpr std::uint64_t estimatedGridQuadCount(std::uint64_t width,
                                               std::uint64_t height,
                                               unsigned int maxQuadSize) noexcept
{
    if (width == 0 || height == 0 || maxQuadSize == 0) {
        return 0;
    }

    const std::uint64_t cellSize = maxQuadSize;
    const std::uint64_t columns = width / cellSize + (width % cellSize == 0 ? 0 : 1);
    const std::uint64_t rows = height / cellSize + (height % cellSize == 0 ? 0 : 1);
    const std::uint64_t maximum = std::numeric_limits<std::uint64_t>::max();
    if (rows != 0 && columns > maximum / rows) {
        return maximum;
    }
    return columns * rows;
}


constexpr int boundedAnimationDuration(long long configuredValue) noexcept
{
    if (configuredValue < 1) {
        return 1;
    }
    if (configuredValue > MaxAnimationDurationMs) {
        return static_cast<int>(MaxAnimationDurationMs);
    }
    return static_cast<int>(configuredValue);
}

constexpr int boundedBumpDistance(unsigned int configuredValue) noexcept
{
    return static_cast<int>(configuredValue > MaxBumpDistance
        ? MaxBumpDistance
        : configuredValue);
}

constexpr int boundedGridCellSize(unsigned int configuredValue) noexcept
{
    if (configuredValue < MinGridCellSize) {
        return static_cast<int>(MinGridCellSize);
    }
    if (configuredValue > MaxGridCellSize) {
        return static_cast<int>(MaxGridCellSize);
    }
    return static_cast<int>(configuredValue);
}

} // namespace CoordinateMapper
