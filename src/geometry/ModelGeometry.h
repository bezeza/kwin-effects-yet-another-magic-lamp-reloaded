/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 */

#pragma once

#include "common.h"

#include <QRectF>

#include <algorithm>
#include <cmath>
#include <optional>

namespace ModelGeometry {

inline constexpr qreal MinimumAnimationDistance = 1.0;

inline bool isFinitePositiveRect(const QRectF& rect) noexcept
{
    return std::isfinite(rect.x())
        && std::isfinite(rect.y())
        && std::isfinite(rect.width())
        && std::isfinite(rect.height())
        && std::isfinite(rect.right())
        && std::isfinite(rect.bottom())
        && rect.width() > 0
        && rect.height() > 0;
}

struct AnimationGeometry {
    qreal bumpDistance = 0;
    qreal shapeFactor = 0;
};

inline std::optional<qreal> distanceToIcon(
    Direction direction,
    const QRectF& windowRect,
    const QRectF& iconRect,
    qreal bumpDistance) noexcept
{
    if (!isFinitePositiveRect(windowRect)
        || !isFinitePositiveRect(iconRect)
        || !std::isfinite(bumpDistance)
        || bumpDistance < 0) {
        return std::nullopt;
    }

    qreal distance = 0;
    switch (direction) {
    case Direction::Top:
        distance = windowRect.bottom() - iconRect.bottom() + bumpDistance;
        break;
    case Direction::Right:
        distance = iconRect.left() - windowRect.left() + bumpDistance;
        break;
    case Direction::Bottom:
        distance = iconRect.top() - windowRect.top() + bumpDistance;
        break;
    case Direction::Left:
        distance = windowRect.right() - iconRect.right() + bumpDistance;
        break;
    }

    if (!std::isfinite(distance) || distance < MinimumAnimationDistance) {
        return std::nullopt;
    }
    return distance;
}

inline std::optional<qreal> shapeFactor(
    Direction direction,
    const QRectF& windowRect,
    const QRectF& iconRect,
    qreal bumpDistance,
    qreal configuredFactor) noexcept
{
    const auto distance = distanceToIcon(direction, windowRect, iconRect, bumpDistance);
    if (!distance || !std::isfinite(configuredFactor)) {
        return std::nullopt;
    }

    const qreal movingExtent = (direction == Direction::Left || direction == Direction::Right)
        ? windowRect.width()
        : windowRect.height();
    if (!std::isfinite(movingExtent) || movingExtent <= 0) {
        return std::nullopt;
    }

    const qreal minimumFactor = movingExtent / *distance;
    if (!std::isfinite(minimumFactor)) {
        return std::nullopt;
    }

    return std::max(std::clamp(configuredFactor, qreal(0), qreal(1)), minimumFactor);
}

inline std::optional<AnimationGeometry> geometryForTarget(
    Direction direction,
    const QRectF& windowRect,
    const QRectF& iconRect,
    qreal configuredBumpDistance,
    qreal configuredShapeFactor) noexcept
{
    if (!isFinitePositiveRect(windowRect) || !isFinitePositiveRect(iconRect)
        || !std::isfinite(configuredShapeFactor)) {
        return std::nullopt;
    }

    qreal baseBumpDistance = 0;
    switch (direction) {
    case Direction::Top:
        baseBumpDistance = std::max(qreal(0), iconRect.bottom() - windowRect.top());
        break;
    case Direction::Right:
        baseBumpDistance = std::max(qreal(0), windowRect.right() - iconRect.left());
        break;
    case Direction::Bottom:
        baseBumpDistance = std::max(qreal(0), windowRect.bottom() - iconRect.top());
        break;
    case Direction::Left:
        baseBumpDistance = std::max(qreal(0), iconRect.right() - windowRect.left());
        break;
    default:
        return std::nullopt;
    }

    const qreal extraBumpDistance = std::isfinite(configuredBumpDistance)
        ? std::max(qreal(0), configuredBumpDistance)
        : qreal(0);
    const qreal bumpDistance = baseBumpDistance + std::min(baseBumpDistance, extraBumpDistance);
    if (!std::isfinite(bumpDistance)) {
        return std::nullopt;
    }

    const auto factor = shapeFactor(direction, windowRect, iconRect, bumpDistance, configuredShapeFactor);
    if (!factor) {
        return std::nullopt;
    }

    return AnimationGeometry{bumpDistance, *factor};
}

} // namespace ModelGeometry
