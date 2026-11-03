/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 */

// Own
#include "CoordinateMapper.h"
#include "RotationAdapter.h"
#include "ScaleAdapter.h"

namespace CoordinateMapper {

QRectF surfaceToGlobal(const QRect& surfaceLocal, const QPointF& panelPos)
{
    return QRectF(surfaceLocal).translated(panelPos);
}

qreal centreDistance(const QRectF& rect, const QPointF& point)
{
    return (rect.center() - point).manhattanLength();
}

std::optional<QRectF> outputLocalToNativePixels(const QRectF& logicalRect, const OutputInfo& output)
{
    if (!output.isValid()) {
        return std::nullopt;
    }

    const auto transformedPixels = ScaleAdapter::toPixels(logicalRect, output.scale);
    if (!transformedPixels) {
        return std::nullopt;
    }

    return RotationAdapter::toNative(
        *transformedPixels, QSizeF(output.nativePixelSize), output.transform);
}

std::optional<QRectF> nativePixelsToOutputLocal(const QRectF& nativeRect, const OutputInfo& output)
{
    if (!output.isValid()) {
        return std::nullopt;
    }

    const auto transformedPixels = RotationAdapter::toTransformed(
        nativeRect, QSizeF(output.nativePixelSize), output.transform);
    if (!transformedPixels) {
        return std::nullopt;
    }

    return ScaleAdapter::toLogical(*transformedPixels, output.scale);
}

std::optional<QRectF> globalToNativePixels(const QRectF& globalRect, const OutputInfo& output)
{
    if (!output.isValid() || !globalRect.isValid()) {
        return std::nullopt;
    }

    const QRectF logicalOutputLocal = globalRect.translated(-output.geometry.topLeft());
    return outputLocalToNativePixels(logicalOutputLocal, output);
}

std::optional<QRectF> nativePixelsToGlobal(const QRectF& nativeRect, const OutputInfo& output)
{
    if (!output.isValid()) {
        return std::nullopt;
    }

    const auto logicalOutputLocal = nativePixelsToOutputLocal(nativeRect, output);
    if (!logicalOutputLocal) {
        return std::nullopt;
    }

    return logicalOutputLocal->translated(output.geometry.topLeft());
}

} // namespace CoordinateMapper
