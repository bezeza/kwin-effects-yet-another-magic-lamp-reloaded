/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 */

#pragma once

// Qt
#include <QPointF>
#include <QRect>
#include <QRectF>

// std
#include <optional>

namespace CoordinateMapper {

// Converts coordinates between logical output-local units and physical pixels.
// This adapter does not apply output rotation and must not be used for global
// coordinates or Wayland surface-local task-button geometries.
class ScaleAdapter final
{
public:
    static bool isValidScale(qreal scale);

    static std::optional<QPointF> toPixels(const QPointF& logical, qreal scale);
    static std::optional<QRectF> toPixels(const QRectF& logical, qreal scale);

    static std::optional<QPointF> toLogical(const QPointF& pixels, qreal scale);
    static std::optional<QRectF> toLogical(const QRectF& pixels, qreal scale);

    // Expands fractional pixel edges outwards so the integer rect fully covers
    // the logical rect after scaling.
    static std::optional<QRect> toAlignedPixels(const QRectF& logical, qreal scale);
};

} // namespace CoordinateMapper
