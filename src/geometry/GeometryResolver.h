/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 */

#pragma once

// Own
#include "CoordinateMapper.h"

// Qt
#include <QList>
#include <QPointF>
#include <QRectF>

// std
#include <cmath>
#include <limits>
#include <optional>

namespace KWin
{
class EffectWindow;
}

namespace CoordinateMapper {

class OutputManager;

// A candidate button geometry published by one Plasma task manager.
// belongsToTargetOutput is determined by comparing the panel's LogicalOutput
// with the minimized window's LogicalOutput, not by geometric proximity.
struct PanelButtonCandidate {
    bool belongsToTargetOutput = false;
    QRectF panelGeometry;
    QRectF buttonGeometry;
};

// Pure selection logic, kept independent from KWin so its edge cases can be
// covered by unit tests.
inline std::optional<QRectF> nearestButtonOnOutput(
    const QList<PanelButtonCandidate>& candidates, const QPointF& windowCenter)
{
    if (!std::isfinite(windowCenter.x()) || !std::isfinite(windowCenter.y())) {
        return std::nullopt;
    }

    std::optional<QRectF> best;
    qreal bestDistance = std::numeric_limits<qreal>::max();

    for (const PanelButtonCandidate& candidate : candidates) {
        const QRectF& panel = candidate.panelGeometry;
        const QRectF& button = candidate.buttonGeometry;

        if (!candidate.belongsToTargetOutput || !panel.isValid() || !button.isValid()) {
            continue;
        }
        if (!std::isfinite(panel.x()) || !std::isfinite(panel.y())
            || !std::isfinite(panel.width()) || !std::isfinite(panel.height())
            || !std::isfinite(button.x()) || !std::isfinite(button.y())
            || !std::isfinite(button.width()) || !std::isfinite(button.height())) {
            continue;
        }

        const qreal distance = centreDistance(panel, windowCenter);
        if (!std::isfinite(distance)) {
            continue;
        }

        // Strict comparison intentionally preserves the first candidate on a
        // distance tie, matching the previous iteration behavior.
        if (distance < bestDistance) {
            bestDistance = distance;
            best = button;
        }
    }

    return best;
}

class GeometryResolver final
{
public:
    explicit GeometryResolver(OutputManager* outputs);

    // Returns the task-button rectangle in global logical coordinates.
    // No value means the effect must leave KWin's normal minimize/restore path
    // untouched.
    std::optional<QRectF> resolve(KWin::EffectWindow* effectWindow) const;

private:
    OutputManager* m_outputs = nullptr; // owned by YetAnotherMagicLampEffect
};

} // namespace CoordinateMapper
