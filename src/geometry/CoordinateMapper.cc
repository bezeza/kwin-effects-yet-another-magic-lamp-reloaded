/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 */

// Own
#include "CoordinateMapper.h"

namespace CoordinateMapper {

QRectF surfaceToGlobal(const QRect& surfaceLocal, const QPointF& panelPos)
{
    return QRectF(surfaceLocal).translated(panelPos);
}

qreal centreDistance(const QRectF& rect, const QPointF& point)
{
    return (rect.center() - point).manhattanLength();
}

} // namespace CoordinateMapper
