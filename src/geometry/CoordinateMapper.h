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
#include <QRect>
#include <QRectF>

// std
#include <optional>

// Coordinate conversions used by the effect. All operations are explicit
// about their source and destination spaces; task-button geometries supplied
// in Wayland surface-local logical coordinates must not be passed through the
// output scale or rotation adapters.
namespace CoordinateMapper {

// A task button as Plasma publishes it (relative to the panel surface) in
// global logical coordinates, given the panel's global logical top-left.
QRectF surfaceToGlobal(const QRect& surfaceLocal, const QPointF& panelPos);

// Manhattan distance from the centre of @p rect to @p point.
qreal centreDistance(const QRectF& rect, const QPointF& point);

// Maps a logical output-local rectangle to the native, untransformed buffer
// pixel space of the output.
std::optional<QRectF> outputLocalToNativePixels(const QRectF& logicalRect, const OutputInfo& output);

// Inverse of outputLocalToNativePixels().
std::optional<QRectF> nativePixelsToOutputLocal(const QRectF& nativeRect, const OutputInfo& output);

// Maps a global logical rectangle to native, untransformed buffer pixels.
std::optional<QRectF> globalToNativePixels(const QRectF& globalRect, const OutputInfo& output);

// Inverse of globalToNativePixels().
std::optional<QRectF> nativePixelsToGlobal(const QRectF& nativeRect, const OutputInfo& output);

} // namespace CoordinateMapper
