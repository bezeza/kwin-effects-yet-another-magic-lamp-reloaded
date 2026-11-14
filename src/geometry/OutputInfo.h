/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 */

#pragma once

// Qt
#include <QMetaType>
#include <QRectF>
#include <QSize>
#include <QString>
#include <QtGlobal>

// std
#include <cmath>

namespace CoordinateMapper {

// Mirrors KWin::OutputTransform::Kind without exposing KWin types to the
// pure geometry adapters and their unit tests.
enum class TransformKind : quint8 {
    Normal,
    Rotate90,
    Rotate180,
    Rotate270,
    FlipX,
    FlipX90,
    FlipX180,
    FlipX270,
    Unknown,
};

// A value snapshot of one LogicalOutput.
//
// geometry is in KWin global logical coordinates. KWin has already applied
// the output transform to this geometry; do not rotate or scale it again.
// nativePixelSize is the untransformed mode size, while transformedPixelSize
// is the physical size after the output transform.
struct OutputInfo {
    QString id;
    QString name;
    QRectF geometry;
    qreal scale = 1.0;
    QSize nativePixelSize;
    QSize transformedPixelSize;
    TransformKind transform = TransformKind::Unknown;

    bool isValid() const
    {
        return !id.isEmpty()
            && geometry.isValid()
            && std::isfinite(geometry.x())
            && std::isfinite(geometry.y())
            && std::isfinite(geometry.width())
            && std::isfinite(geometry.height())
            && std::isfinite(geometry.right())
            && std::isfinite(geometry.bottom())
            && std::isfinite(scale)
            && scale > 0.0
            && nativePixelSize.width() > 0
            && nativePixelSize.height() > 0
            && transformedPixelSize.width() > 0
            && transformedPixelSize.height() > 0
            && transform != TransformKind::Unknown;
    }

    bool operator==(const OutputInfo& other) const
    {
        return id == other.id
            && name == other.name
            && geometry == other.geometry
            && qFuzzyCompare(scale, other.scale)
            && nativePixelSize == other.nativePixelSize
            && transformedPixelSize == other.transformedPixelSize
            && transform == other.transform;
    }
};

} // namespace CoordinateMapper

Q_DECLARE_METATYPE(CoordinateMapper::TransformKind)
