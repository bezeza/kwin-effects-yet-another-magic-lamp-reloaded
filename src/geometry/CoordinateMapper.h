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

// Coordinate conversions the effect relies on, as pure functions on Qt types
// so they can be tested without a running KWin. All rects are in KWin's
// logical units; "global" is the compositor space LogicalOutput::geometry()
// lives in, "surface-local" is relative to a panel surface's top-left corner.
namespace CoordinateMapper {

// A task button as Plasma publishes it (org_kde_plasma_window
// .set_minimized_geometry: integers relative to the panel surface) in global
// coordinates, given the panel's global top-left (KWin::Window::pos()).
QRectF surfaceToGlobal(const QRect& surfaceLocal, const QPointF& panelPos);

// Manhattan distance from the centre of @p rect to @p point: how KWin 6.7+
// ranks panels against a window's centre in Window::iconGeometry(), and how
// the effect breaks ties between panels on the same output.
qreal centreDistance(const QRectF& rect, const QPointF& point);

} // namespace CoordinateMapper
