/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 */

#pragma once

// Own
#include "OutputInfo.h"

// Qt
#include <QPointF>
#include <QRectF>
#include <QSizeF>

// std
#include <optional>

namespace CoordinateMapper {

// Maps between the native, untransformed output buffer and the transformed
// output-pixel coordinate space. The formulas mirror KWin::OutputTransform.
class RotationAdapter final
{
public:
    static QSizeF transformedSize(const QSizeF& nativeSize, TransformKind transform);

    static std::optional<QPointF> toTransformed(
        const QPointF& nativePoint, const QSizeF& nativeBounds, TransformKind transform);
    static std::optional<QRectF> toTransformed(
        const QRectF& nativeRect, const QSizeF& nativeBounds, TransformKind transform);

    static std::optional<QPointF> toNative(
        const QPointF& transformedPoint, const QSizeF& nativeBounds, TransformKind transform);
    static std::optional<QRectF> toNative(
        const QRectF& transformedRect, const QSizeF& nativeBounds, TransformKind transform);
};

} // namespace CoordinateMapper
