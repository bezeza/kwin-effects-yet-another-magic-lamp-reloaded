/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 */

// Own
#include "ScaleAdapter.h"

// std
#include <cmath>
#include <limits>

namespace CoordinateMapper {

namespace {

bool isFinite(const QPointF& point)
{
    return std::isfinite(point.x()) && std::isfinite(point.y());
}

bool isFinite(const QRectF& rect)
{
    return std::isfinite(rect.x()) && std::isfinite(rect.y())
        && std::isfinite(rect.width()) && std::isfinite(rect.height());
}

bool fitsInt(qreal value)
{
    return std::isfinite(value)
        && value >= std::numeric_limits<int>::min()
        && value <= std::numeric_limits<int>::max();
}

} // namespace

bool ScaleAdapter::isValidScale(qreal scale)
{
    return std::isfinite(scale) && scale > 0.0;
}

std::optional<QPointF> ScaleAdapter::toPixels(const QPointF& logical, qreal scale)
{
    if (!isValidScale(scale) || !isFinite(logical)) {
        return std::nullopt;
    }

    const QPointF result(logical.x() * scale, logical.y() * scale);
    return isFinite(result) ? std::optional<QPointF>(result) : std::nullopt;
}

std::optional<QRectF> ScaleAdapter::toPixels(const QRectF& logical, qreal scale)
{
    if (!isValidScale(scale) || !logical.isValid() || !isFinite(logical)) {
        return std::nullopt;
    }

    const QRectF result(logical.x() * scale, logical.y() * scale,
        logical.width() * scale, logical.height() * scale);
    return isFinite(result) && result.isValid() ? std::optional<QRectF>(result) : std::nullopt;
}

std::optional<QPointF> ScaleAdapter::toLogical(const QPointF& pixels, qreal scale)
{
    if (!isValidScale(scale) || !isFinite(pixels)) {
        return std::nullopt;
    }

    const QPointF result(pixels.x() / scale, pixels.y() / scale);
    return isFinite(result) ? std::optional<QPointF>(result) : std::nullopt;
}

std::optional<QRectF> ScaleAdapter::toLogical(const QRectF& pixels, qreal scale)
{
    if (!isValidScale(scale) || !pixels.isValid() || !isFinite(pixels)) {
        return std::nullopt;
    }

    const QRectF result(pixels.x() / scale, pixels.y() / scale,
        pixels.width() / scale, pixels.height() / scale);
    return isFinite(result) && result.isValid() ? std::optional<QRectF>(result) : std::nullopt;
}

std::optional<QRect> ScaleAdapter::toAlignedPixels(const QRectF& logical, qreal scale)
{
    const auto scaled = toPixels(logical, scale);
    if (!scaled) {
        return std::nullopt;
    }

    const qreal left = std::floor(scaled->left());
    const qreal top = std::floor(scaled->top());
    const qreal right = std::ceil(scaled->right());
    const qreal bottom = std::ceil(scaled->bottom());
    const qreal width = right - left;
    const qreal height = bottom - top;

    if (!fitsInt(left) || !fitsInt(top) || !fitsInt(right) || !fitsInt(bottom)
        || !fitsInt(width) || !fitsInt(height)
        || width <= 0.0 || height <= 0.0) {
        return std::nullopt;
    }

    return QRect(static_cast<int>(left), static_cast<int>(top),
        static_cast<int>(width), static_cast<int>(height));
}

} // namespace CoordinateMapper
