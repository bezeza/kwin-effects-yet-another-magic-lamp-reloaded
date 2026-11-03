/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 */

// Own
#include "RotationAdapter.h"

// std
#include <algorithm>
#include <cmath>

namespace CoordinateMapper {

namespace {

bool isValidSize(const QSizeF& size)
{
    return std::isfinite(size.width()) && std::isfinite(size.height())
        && size.width() > 0.0 && size.height() > 0.0;
}

bool isFinite(const QPointF& point)
{
    return std::isfinite(point.x()) && std::isfinite(point.y());
}

TransformKind inverseTransform(TransformKind transform)
{
    switch (transform) {
    case TransformKind::Normal:
        return TransformKind::Normal;
    case TransformKind::Rotate90:
        return TransformKind::Rotate270;
    case TransformKind::Rotate180:
        return TransformKind::Rotate180;
    case TransformKind::Rotate270:
        return TransformKind::Rotate90;
    case TransformKind::FlipX:
    case TransformKind::FlipX90:
    case TransformKind::FlipX180:
    case TransformKind::FlipX270:
        return transform;
    case TransformKind::Unknown:
        return TransformKind::Unknown;
    }
    return TransformKind::Unknown;
}

std::optional<QPointF> mapPoint(
    const QPointF& point, const QSizeF& bounds, TransformKind transform)
{
    if (!isFinite(point) || !isValidSize(bounds)) {
        return std::nullopt;
    }

    QPointF result;
    switch (transform) {
    case TransformKind::Normal:
        result = point;
        break;
    case TransformKind::Rotate90:
        result = QPointF(point.y(), bounds.width() - point.x());
        break;
    case TransformKind::Rotate180:
        result = QPointF(bounds.width() - point.x(), bounds.height() - point.y());
        break;
    case TransformKind::Rotate270:
        result = QPointF(bounds.height() - point.y(), point.x());
        break;
    case TransformKind::FlipX:
        result = QPointF(bounds.width() - point.x(), point.y());
        break;
    case TransformKind::FlipX90:
        result = QPointF(point.y(), point.x());
        break;
    case TransformKind::FlipX180:
        result = QPointF(point.x(), bounds.height() - point.y());
        break;
    case TransformKind::FlipX270:
        result = QPointF(bounds.height() - point.y(), bounds.width() - point.x());
        break;
    case TransformKind::Unknown:
        return std::nullopt;
    }
    return isFinite(result) ? std::optional<QPointF>(result) : std::nullopt;
}

std::optional<QRectF> mapRect(
    const QRectF& rect, const QSizeF& bounds, TransformKind transform)
{
    if (!rect.isValid() || !isValidSize(bounds)) {
        return std::nullopt;
    }

    const auto topLeft = mapPoint(rect.topLeft(), bounds, transform);
    const auto topRight = mapPoint(rect.topRight(), bounds, transform);
    const auto bottomLeft = mapPoint(rect.bottomLeft(), bounds, transform);
    const auto bottomRight = mapPoint(rect.bottomRight(), bounds, transform);
    if (!topLeft || !topRight || !bottomLeft || !bottomRight) {
        return std::nullopt;
    }

    const qreal left = std::min({topLeft->x(), topRight->x(), bottomLeft->x(), bottomRight->x()});
    const qreal top = std::min({topLeft->y(), topRight->y(), bottomLeft->y(), bottomRight->y()});
    const qreal right = std::max({topLeft->x(), topRight->x(), bottomLeft->x(), bottomRight->x()});
    const qreal bottom = std::max({topLeft->y(), topRight->y(), bottomLeft->y(), bottomRight->y()});

    const QRectF result(QPointF(left, top), QPointF(right, bottom));
    if (!std::isfinite(left) || !std::isfinite(top)
        || !std::isfinite(right) || !std::isfinite(bottom) || !result.isValid()) {
        return std::nullopt;
    }
    return result;
}

} // namespace

QSizeF RotationAdapter::transformedSize(const QSizeF& nativeSize, TransformKind transform)
{
    if (!isValidSize(nativeSize) || transform == TransformKind::Unknown) {
        return QSizeF();
    }

    switch (transform) {
    case TransformKind::Rotate90:
    case TransformKind::Rotate270:
    case TransformKind::FlipX90:
    case TransformKind::FlipX270:
        return nativeSize.transposed();
    default:
        return nativeSize;
    }
}

std::optional<QPointF> RotationAdapter::toTransformed(
    const QPointF& nativePoint, const QSizeF& nativeBounds, TransformKind transform)
{
    return mapPoint(nativePoint, nativeBounds, transform);
}

std::optional<QRectF> RotationAdapter::toTransformed(
    const QRectF& nativeRect, const QSizeF& nativeBounds, TransformKind transform)
{
    return mapRect(nativeRect, nativeBounds, transform);
}

std::optional<QPointF> RotationAdapter::toNative(
    const QPointF& transformedPoint, const QSizeF& nativeBounds, TransformKind transform)
{
    const QSizeF transformedBounds = transformedSize(nativeBounds, transform);
    return mapPoint(transformedPoint, transformedBounds, inverseTransform(transform));
}

std::optional<QRectF> RotationAdapter::toNative(
    const QRectF& transformedRect, const QSizeF& nativeBounds, TransformKind transform)
{
    const QSizeF transformedBounds = transformedSize(nativeBounds, transform);
    return mapRect(transformedRect, transformedBounds, inverseTransform(transform));
}

} // namespace CoordinateMapper
