/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 */

#pragma once

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
