/*
 * Copyright (C) 2018 Vlad Zagorodniy <vladzzag@gmail.com>
 * Copyright (C) 2026 Roy Bock <dev@si13n7.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

// Own
#include "Model.h"
#include "geometry/AnimationRecovery.h"
#include "geometry/ModelGeometry.h"
#include "kwincompat.h"

#include <algorithm>
#include <cmath>

static inline std::chrono::milliseconds durationFraction(std::chrono::milliseconds duration, qreal fraction)
{
    constexpr qint64 maximumDurationMs = 9999;
    if (!std::isfinite(fraction)) {
        return std::chrono::milliseconds(1);
    }

    const long double scaled = static_cast<long double>(duration.count())
        * static_cast<long double>(qMax(fraction, qreal(0)));
    if (!std::isfinite(scaled)) {
        return std::chrono::milliseconds(maximumDurationMs);
    }

    const long double bounded = std::clamp(scaled, 1.0L, static_cast<long double>(maximumDurationMs));
    return std::chrono::milliseconds(static_cast<qint64>(std::llround(bounded)));
}

Model::Model(KWin::EffectWindow* window)
    : m_window(window)
{
}

// Mirrors KDE's own Magic Lamp logic exactly:
// 1. Find panel via intersects() — Plasma sometimes publishes icon geometry
//    slightly larger than the panel, so contains() misses it.
// 2. Use clientArea(ScreenArea, window) — the WINDOW's screen center — to
//    determine direction. This correctly handles cross-screen icons: even if
//    the icon is on a different screen, comparing to the window's screen
//    center always gives the right direction.
static std::optional<Direction> realizeDirection(const KWin::EffectWindow* window, const QRectF& iconRect)
{
    if (!window || !KWin::effects) {
        return std::nullopt;
    }

    KWin::EffectWindow* panel = nullptr;
    QRectF panelGeometry;
    for (KWin::EffectWindow* candidate : KWin::effects->stackingOrder()) {
        if (!candidate || !candidate->isDock()) {
            continue;
        }
        const QRectF candidateGeometry = candidate->frameGeometry();
        if (!ModelGeometry::isFinitePositiveRect(candidateGeometry) || !candidateGeometry.intersects(iconRect)) {
            continue;
        }
        panel = candidate;
        panelGeometry = candidateGeometry;
        break;
    }

    if (panel) {
        QRectF windowScreen = KWin::effects->clientArea(KWin::ScreenArea, window);
        if (!ModelGeometry::isFinitePositiveRect(windowScreen)) {
            windowScreen = window->frameGeometry();
        }
        if (!ModelGeometry::isFinitePositiveRect(windowScreen)) {
            return std::nullopt;
        }

        if (panelGeometry.width() >= panelGeometry.height()) {
            return iconRect.center().y() <= windowScreen.center().y()
                ? Direction::Top
                : Direction::Bottom;
        }
        return iconRect.center().x() <= windowScreen.center().x()
            ? Direction::Left
            : Direction::Right;
    }

    // If the icon center is outside every output, use the window's output.
    KWin::LogicalOutput* screen = KWin::effects->screenAt(iconRect.center().toPoint());
    if (!screen) {
        screen = window->screen();
    }
    if (!screen) {
        return std::nullopt;
    }

    const QRectF screenRect = KWinCompat::fullScreenArea(KWin::effects, screen);
    if (!ModelGeometry::isFinitePositiveRect(screenRect)) {
        return std::nullopt;
    }

    const QPointF center = iconRect.center();
    const qreal left = qAbs(center.x() - screenRect.left());
    const qreal top = qAbs(center.y() - screenRect.top());
    const qreal right = qAbs(screenRect.right() - center.x());
    const qreal bottom = qAbs(screenRect.bottom() - center.y());
    const qreal nearest = qMin(qMin(left, top), qMin(right, bottom));
    if (!std::isfinite(nearest)) {
        return std::nullopt;
    }
    if (qFuzzyCompare(left, nearest)) {
        return Direction::Left;
    }
    if (qFuzzyCompare(top, nearest)) {
        return Direction::Top;
    }
    if (qFuzzyCompare(right, nearest)) {
        return Direction::Right;
    }
    return Direction::Bottom;
}

bool Model::start(AnimationKind kind)
{
    m_kind = kind;

    const auto startState = CoordinateMapper::prepareAnimationStart(
        {m_paused, m_done, false}, m_timeLine.running());
    m_paused = startState.paused;
    m_done = startState.done;

    if (startState.reverseTimeline) {
        m_timeLine.toggleDirection();
        return true;
    }

    // Invalid or transient window geometry must not enter the render path:
    // the transform contains divisions by the window extent and distance to
    // the task button. Skip the animation and let KWin perform the normal
    // minimize/restore operation instead.
    if (!m_window
        || !ModelGeometry::isFinitePositiveRect(m_window->frameGeometry())
        || !ModelGeometry::isFinitePositiveRect(m_iconGeometry)) {
        m_done = true;
        m_clip = false;
        return false;
    }

    const auto direction = realizeDirection(m_window, m_iconGeometry);
    if (!direction) {
        m_done = true;
        m_clip = false;
        return false;
    }
    m_direction = *direction;
    const auto geometry = ModelGeometry::geometryForTarget(
        m_direction, m_window->frameGeometry(), m_iconGeometry,
        m_parameters.bumpDistance, m_parameters.shapeFactor);
    if (!geometry) {
        m_done = true;
        m_clip = false;
        return false;
    }
    m_bumpDistance = geometry->bumpDistance;
    m_shapeFactor = geometry->shapeFactor;
    m_done = false;

    switch (m_kind) {
    case AnimationKind::Minimize:
        if (m_bumpDistance != 0) {
            m_stage = AnimationStage::Bump;
            m_timeLine.reset();
            m_timeLine.setDirection(KWin::TimeLine::Forward);
            m_timeLine.setDuration(m_parameters.bumpDuration);
            m_timeLine.setEasingCurve(QEasingCurve::Linear);
            m_clip = false;
        } else {
            m_stage = AnimationStage::Stretch1;
            m_timeLine.reset();
            m_timeLine.setDirection(KWin::TimeLine::Forward);
            m_timeLine.setDuration(durationFraction(m_parameters.stretchDuration, m_shapeFactor));
            m_timeLine.setEasingCurve(QEasingCurve::Linear);
            m_clip = true;
        }
        break;

    case AnimationKind::Unminimize:
        m_stage = AnimationStage::Squash;
        m_timeLine.reset();
        m_timeLine.setDirection(KWin::TimeLine::Backward);
        m_timeLine.setDuration(m_parameters.squashDuration);
        m_timeLine.setEasingCurve(QEasingCurve::Linear);
        m_clip = true;
        break;

    default:
        Q_UNREACHABLE();
    }

    return true;
}

void Model::advance(std::chrono::milliseconds presentTime)
{
    if (m_paused) {
        return;
    }

    m_timeLine.advance(presentTime);
    if (!m_timeLine.done()) {
        return;
    }

    switch (m_kind) {
    case AnimationKind::Minimize:
        updateMinimizeStage();
        break;

    case AnimationKind::Unminimize:
        updateUnminimizeStage();
        break;

    default:
        Q_UNREACHABLE();
    }
}

void Model::pause()
{
    if (m_paused) {
        return;
    }

    // TimeLine's animation clock measures from the last advance timestamp.
    // Re-setting the current elapsed value resets that clock while preserving
    // the stage progress, preventing a long recovery wait from being counted
    // as one enormous frame delta.
    const auto elapsed = m_timeLine.elapsed();
    m_timeLine.reset();
    m_timeLine.setElapsed(elapsed);
    m_paused = true;
}

void Model::resume()
{
    m_paused = false;
}

bool Model::retargetIconGeometry(const QRectF& rect)
{
    if (m_done || !m_window
        || !ModelGeometry::isFinitePositiveRect(m_window->frameGeometry())
        || !ModelGeometry::isFinitePositiveRect(rect)) {
        return false;
    }

    const auto direction = realizeDirection(m_window, rect);
    if (!direction) {
        return false;
    }

    const auto geometry = ModelGeometry::geometryForTarget(
        *direction, m_window->frameGeometry(), rect,
        m_parameters.bumpDistance, m_parameters.shapeFactor);
    if (!geometry) {
        return false;
    }

    // Do not reset or replace m_timeLine: the active stage, direction of travel,
    // and elapsed progress are intentionally retained across the output change.
    m_iconGeometry = rect;
    m_direction = *direction;
    m_bumpDistance = geometry->bumpDistance;
    m_shapeFactor = geometry->shapeFactor;
    return true;
}

void Model::updateMinimizeStage()
{
    switch (m_stage) {
    case AnimationStage::Bump:
        m_stage = AnimationStage::Stretch1;
        m_timeLine.reset();
        m_timeLine.setDirection(KWin::TimeLine::Forward);
        m_timeLine.setDuration(
            durationFraction(m_parameters.stretchDuration, m_shapeFactor));
        m_timeLine.setEasingCurve(QEasingCurve::Linear);
        m_clip = true;
        m_done = false;
        return;

    case AnimationStage::Stretch1:
    case AnimationStage::Stretch2:
        m_stage = AnimationStage::Squash;
        m_timeLine.reset();
        m_timeLine.setDirection(KWin::TimeLine::Forward);
        m_timeLine.setDuration(m_parameters.squashDuration);
        m_timeLine.setEasingCurve(QEasingCurve::Linear);
        m_clip = true;
        m_done = false;
        return;

    case AnimationStage::Squash:
        m_done = true;
        return;

    default:
        Q_UNREACHABLE();
    }
}

void Model::updateUnminimizeStage()
{
    switch (m_stage) {
    case AnimationStage::Bump:
        m_done = true;
        return;

    case AnimationStage::Stretch1:
        if (m_bumpDistance == 0) {
            m_done = true;
            return;
        }
        m_stage = AnimationStage::Bump;
        m_timeLine.reset();
        m_timeLine.setDirection(KWin::TimeLine::Backward);
        m_timeLine.setDuration(m_parameters.bumpDuration);
        m_timeLine.setEasingCurve(QEasingCurve::Linear);
        m_clip = false;
        m_done = false;
        return;

    case AnimationStage::Stretch2:
        m_done = true;
        return;

    case AnimationStage::Squash:
        m_stage = AnimationStage::Stretch2;
        m_timeLine.reset();
        m_timeLine.setDirection(KWin::TimeLine::Backward);
        m_timeLine.setDuration(
            durationFraction(m_parameters.unminimizeStretchDuration, m_shapeFactor));
        m_timeLine.setEasingCurve(QEasingCurve::Linear);
        m_clip = false;
        m_done = false;
        return;

    default:
        Q_UNREACHABLE();
    }
}

bool Model::done() const
{
    return m_done;
}

struct TransformParameters {
    QEasingCurve shapeCurve;
    Direction direction;
    QRectF iconRect;
    qreal stretchProgress;
    qreal squashProgress;
    qreal bumpProgress;
    qreal bumpDistance;
};

static inline qreal interpolate(qreal from, qreal to, qreal t)
{
    return from * (1.0 - t) + to * t;
}

static void transformQuadsLeft(
    const KWin::EffectWindow* window,
    const TransformParameters& params,
    KWin::WindowQuadList& quads)
{
    const QRectF iconRect = params.iconRect;
    const QRectF windowRect = window->frameGeometry();

    const qreal distance = windowRect.right() - iconRect.right() + params.bumpDistance;

    for (int i = 0; i < quads.count(); ++i) {
        KWin::WindowQuad& quad = quads[i];

        const qreal leftOffset = quad[0].x() - interpolate(0.0, distance, params.squashProgress);
        const qreal rightOffset = quad[2].x() - interpolate(0.0, distance, params.squashProgress);

        const qreal leftScale = params.stretchProgress * params.shapeCurve.valueForProgress((windowRect.width() - leftOffset) / distance);
        const qreal rightScale = params.stretchProgress * params.shapeCurve.valueForProgress((windowRect.width() - rightOffset) / distance);

        const qreal targetTopLeftY = iconRect.y() + iconRect.height() * quad[0].y() / windowRect.height();
        const qreal targetTopRightY = iconRect.y() + iconRect.height() * quad[1].y() / windowRect.height();
        const qreal targetBottomRightY = iconRect.y() + iconRect.height() * quad[2].y() / windowRect.height();
        const qreal targetBottomLeftY = iconRect.y() + iconRect.height() * quad[3].y() / windowRect.height();

        quad[0].setY(quad[0].y() + leftScale * (targetTopLeftY - (windowRect.y() + quad[0].y())));
        quad[3].setY(quad[3].y() + leftScale * (targetBottomLeftY - (windowRect.y() + quad[3].y())));
        quad[1].setY(quad[1].y() + rightScale * (targetTopRightY - (windowRect.y() + quad[1].y())));
        quad[2].setY(quad[2].y() + rightScale * (targetBottomRightY - (windowRect.y() + quad[2].y())));

        const qreal targetLeftOffset = leftOffset + params.bumpDistance * params.bumpProgress;
        const qreal targetRightOffset = rightOffset + params.bumpDistance * params.bumpProgress;

        quad[0].setX(targetLeftOffset);
        quad[3].setX(targetLeftOffset);
        quad[1].setX(targetRightOffset);
        quad[2].setX(targetRightOffset);
    }
}

static void transformQuadsTop(
    const KWin::EffectWindow* window,
    const TransformParameters& params,
    KWin::WindowQuadList& quads)
{
    const QRectF iconRect = params.iconRect;
    const QRectF windowRect = window->frameGeometry();

    const qreal distance = windowRect.bottom() - iconRect.bottom() + params.bumpDistance;

    for (int i = 0; i < quads.count(); ++i) {
        KWin::WindowQuad& quad = quads[i];

        const qreal topOffset = quad[0].y() - interpolate(0.0, distance, params.squashProgress);
        const qreal bottomOffset = quad[2].y() - interpolate(0.0, distance, params.squashProgress);

        const qreal topScale = params.stretchProgress * params.shapeCurve.valueForProgress((windowRect.height() - topOffset) / distance);
        const qreal bottomScale = params.stretchProgress * params.shapeCurve.valueForProgress((windowRect.height() - bottomOffset) / distance);

        const qreal targetTopLeftX = iconRect.x() + iconRect.width() * quad[0].x() / windowRect.width();
        const qreal targetTopRightX = iconRect.x() + iconRect.width() * quad[1].x() / windowRect.width();
        const qreal targetBottomRightX = iconRect.x() + iconRect.width() * quad[2].x() / windowRect.width();
        const qreal targetBottomLeftX = iconRect.x() + iconRect.width() * quad[3].x() / windowRect.width();

        quad[0].setX(quad[0].x() + topScale * (targetTopLeftX - (windowRect.x() + quad[0].x())));
        quad[1].setX(quad[1].x() + topScale * (targetTopRightX - (windowRect.x() + quad[1].x())));
        quad[2].setX(quad[2].x() + bottomScale * (targetBottomRightX - (windowRect.x() + quad[2].x())));
        quad[3].setX(quad[3].x() + bottomScale * (targetBottomLeftX - (windowRect.x() + quad[3].x())));

        const qreal targetTopOffset = topOffset + params.bumpDistance * params.bumpProgress;
        const qreal targetBottomOffset = bottomOffset + params.bumpDistance * params.bumpProgress;

        quad[0].setY(targetTopOffset);
        quad[1].setY(targetTopOffset);
        quad[2].setY(targetBottomOffset);
        quad[3].setY(targetBottomOffset);
    }
}

static void transformQuadsRight(
    const KWin::EffectWindow* window,
    const TransformParameters& params,
    KWin::WindowQuadList& quads)
{
    const QRectF iconRect = params.iconRect;
    const QRectF windowRect = window->frameGeometry();

    const qreal distance = iconRect.left() - windowRect.left() + params.bumpDistance;

    for (int i = 0; i < quads.count(); ++i) {
        KWin::WindowQuad& quad = quads[i];

        const qreal leftOffset = quad[0].x() + interpolate(0.0, distance, params.squashProgress);
        const qreal rightOffset = quad[2].x() + interpolate(0.0, distance, params.squashProgress);

        const qreal leftScale = params.stretchProgress * params.shapeCurve.valueForProgress(leftOffset / distance);
        const qreal rightScale = params.stretchProgress * params.shapeCurve.valueForProgress(rightOffset / distance);

        const qreal targetTopLeftY = iconRect.y() + iconRect.height() * quad[0].y() / windowRect.height();
        const qreal targetTopRightY = iconRect.y() + iconRect.height() * quad[1].y() / windowRect.height();
        const qreal targetBottomRightY = iconRect.y() + iconRect.height() * quad[2].y() / windowRect.height();
        const qreal targetBottomLeftY = iconRect.y() + iconRect.height() * quad[3].y() / windowRect.height();

        quad[0].setY(quad[0].y() + leftScale * (targetTopLeftY - (windowRect.y() + quad[0].y())));
        quad[3].setY(quad[3].y() + leftScale * (targetBottomLeftY - (windowRect.y() + quad[3].y())));
        quad[1].setY(quad[1].y() + rightScale * (targetTopRightY - (windowRect.y() + quad[1].y())));
        quad[2].setY(quad[2].y() + rightScale * (targetBottomRightY - (windowRect.y() + quad[2].y())));

        const qreal targetLeftOffset = leftOffset - params.bumpDistance * params.bumpProgress;
        const qreal targetRightOffset = rightOffset - params.bumpDistance * params.bumpProgress;

        quad[0].setX(targetLeftOffset);
        quad[3].setX(targetLeftOffset);
        quad[1].setX(targetRightOffset);
        quad[2].setX(targetRightOffset);
    }
}

static void transformQuadsBottom(
    const KWin::EffectWindow* window,
    const TransformParameters& params,
    KWin::WindowQuadList& quads)
{
    const QRectF iconRect = params.iconRect;
    const QRectF windowRect = window->frameGeometry();

    const qreal distance = iconRect.top() - windowRect.top() + params.bumpDistance;

    for (int i = 0; i < quads.count(); ++i) {
        KWin::WindowQuad& quad = quads[i];

        const qreal topOffset = quad[0].y() + interpolate(0.0, distance, params.squashProgress);
        const qreal bottomOffset = quad[2].y() + interpolate(0.0, distance, params.squashProgress);

        const qreal topScale = params.stretchProgress * params.shapeCurve.valueForProgress(topOffset / distance);
        const qreal bottomScale = params.stretchProgress * params.shapeCurve.valueForProgress(bottomOffset / distance);

        const qreal targetTopLeftX = iconRect.x() + iconRect.width() * quad[0].x() / windowRect.width();
        const qreal targetTopRightX = iconRect.x() + iconRect.width() * quad[1].x() / windowRect.width();
        const qreal targetBottomRightX = iconRect.x() + iconRect.width() * quad[2].x() / windowRect.width();
        const qreal targetBottomLeftX = iconRect.x() + iconRect.width() * quad[3].x() / windowRect.width();

        quad[0].setX(quad[0].x() + topScale * (targetTopLeftX - (windowRect.x() + quad[0].x())));
        quad[1].setX(quad[1].x() + topScale * (targetTopRightX - (windowRect.x() + quad[1].x())));
        quad[2].setX(quad[2].x() + bottomScale * (targetBottomRightX - (windowRect.x() + quad[2].x())));
        quad[3].setX(quad[3].x() + bottomScale * (targetBottomLeftX - (windowRect.x() + quad[3].x())));

        const qreal targetTopOffset = topOffset - params.bumpDistance * params.bumpProgress;
        const qreal targetBottomOffset = bottomOffset - params.bumpDistance * params.bumpProgress;

        quad[0].setY(targetTopOffset);
        quad[1].setY(targetTopOffset);
        quad[2].setY(targetBottomOffset);
        quad[3].setY(targetBottomOffset);
    }
}

static void transformQuads(
    const KWin::EffectWindow* window,
    const TransformParameters& params,
    KWin::WindowQuadList& quads)
{
    switch (params.direction) {
    case Direction::Left:
        transformQuadsLeft(window, params, quads);
        break;

    case Direction::Top:
        transformQuadsTop(window, params, quads);
        break;

    case Direction::Right:
        transformQuadsRight(window, params, quads);
        break;

    case Direction::Bottom:
        transformQuadsBottom(window, params, quads);
        break;

    default:
        Q_UNREACHABLE();
    }
}

void Model::apply(KWin::WindowQuadList& quads) const
{
    if (!hasValidGeometry()) {
        return;
    }

    switch (m_stage) {
    case AnimationStage::Bump:
        applyBump(quads);
        break;

    case AnimationStage::Stretch1:
        applyStretch1(quads);
        break;

    case AnimationStage::Stretch2:
        applyStretch2(quads);
        break;

    case AnimationStage::Squash:
        applySquash(quads);
        break;
    }
}

void Model::applyBump(KWin::WindowQuadList& quads) const
{
    TransformParameters params;
    params.shapeCurve = m_parameters.shapeCurve;
    params.direction = m_direction;
    params.iconRect = m_iconGeometry;
    params.squashProgress = 0.0;
    params.stretchProgress = 0.0;
    params.bumpProgress = m_timeLine.value();
    params.bumpDistance = m_bumpDistance;
    transformQuads(m_window, params, quads);
}

void Model::applyStretch1(KWin::WindowQuadList& quads) const
{
    TransformParameters params;
    params.shapeCurve = m_parameters.shapeCurve;
    params.direction = m_direction;
    params.iconRect = m_iconGeometry;
    params.squashProgress = 0.0;
    params.stretchProgress = m_shapeFactor * m_timeLine.value();
    params.bumpProgress = 1.0;
    params.bumpDistance = m_bumpDistance;
    transformQuads(m_window, params, quads);
}

void Model::applyStretch2(KWin::WindowQuadList& quads) const
{
    TransformParameters params;
    params.shapeCurve = m_parameters.shapeCurve;
    params.direction = m_direction;
    params.iconRect = m_iconGeometry;
    params.squashProgress = 0.0;
    params.stretchProgress = m_shapeFactor * m_timeLine.value();
    params.bumpProgress = params.stretchProgress;
    params.bumpDistance = m_bumpDistance;
    transformQuads(m_window, params, quads);
}

void Model::applySquash(KWin::WindowQuadList& quads) const
{
    TransformParameters params;
    params.shapeCurve = m_parameters.shapeCurve;
    params.direction = m_direction;
    params.iconRect = m_iconGeometry;
    params.squashProgress = m_timeLine.value();
    params.stretchProgress = qMin(m_shapeFactor + params.squashProgress, 1.0);
    params.bumpProgress = 1.0;
    params.bumpDistance = m_bumpDistance;
    transformQuads(m_window, params, quads);
}

Model::Parameters Model::parameters() const
{
    return m_parameters;
}

void Model::setParameters(const Parameters& parameters)
{
    m_parameters = parameters;
}

KWin::EffectWindow* Model::window() const
{
    return m_window;
}

void Model::setWindow(KWin::EffectWindow* window)
{
    m_window = window;
    m_iconGeometry = QRectF(); // reset so start() re-reads from window or override
}

void Model::setIconGeometry(const QRectF& rect)
{
    m_iconGeometry = rect;
}

bool Model::needsClip() const
{
    return m_clip && hasValidGeometry()
        && ModelGeometry::isFinitePositiveRect(m_window->expandedGeometry());
}

QRegion Model::clipRegion() const
{
    if (!hasValidGeometry()) {
        return {};
    }

    const QRectF iconRect = m_iconGeometry;
    QRectF clipRect = m_window->expandedGeometry();
    if (!ModelGeometry::isFinitePositiveRect(clipRect)) {
        return {};
    }

    switch (m_direction) {
    case Direction::Top:
        clipRect.translate(0, m_bumpDistance);
        clipRect.setTop(iconRect.top());
        clipRect.setLeft(qMin(iconRect.left(), clipRect.left()));
        clipRect.setRight(qMax(iconRect.right(), clipRect.right()));
        break;

    case Direction::Right:
        clipRect.translate(-m_bumpDistance, 0);
        clipRect.setRight(iconRect.right());
        clipRect.setTop(qMin(iconRect.top(), clipRect.top()));
        clipRect.setBottom(qMax(iconRect.bottom(), clipRect.bottom()));
        break;

    case Direction::Bottom:
        clipRect.translate(0, -m_bumpDistance);
        clipRect.setBottom(iconRect.bottom());
        clipRect.setLeft(qMin(iconRect.left(), clipRect.left()));
        clipRect.setRight(qMax(iconRect.right(), clipRect.right()));
        break;

    case Direction::Left:
        clipRect.translate(m_bumpDistance, 0);
        clipRect.setLeft(iconRect.left());
        clipRect.setTop(qMin(iconRect.top(), clipRect.top()));
        clipRect.setBottom(qMax(iconRect.bottom(), clipRect.bottom()));
        break;

    default:
        Q_UNREACHABLE();
    }

    return clipRect.toAlignedRect();
}

bool Model::hasValidGeometry() const
{
    if (!m_window) {
        return false;
    }

    return ModelGeometry::distanceToIcon(
        m_direction,
        m_window->frameGeometry(),
        m_iconGeometry,
        m_bumpDistance).has_value();
}
