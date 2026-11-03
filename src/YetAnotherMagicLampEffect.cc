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
#include "YetAnotherMagicLampEffect.h"
#include "Model.h"
#include "geometry/GeometryResolver.h"
#include "geometry/OutputManager.h"
#include "geometry/WindowTracker.h"
#include "kwincompat.h"

// Auto-generated
#include "YetAnotherMagicLampConfig.h"

// kwineffects
#include <core/renderviewport.h>
#include <core/output.h>

// Qt
#include <QDebug>
#include <QLoggingCategory>
#include <QPointer>
#include <QTimer>

// std
#include <chrono>
#include <cmath>

// Diagnostics are off by default. Enable them with
// QT_LOGGING_RULES="kwin_effect_yetanothermagiclamp.debug=true".
Q_LOGGING_CATEGORY(lcMagicLamp, "kwin_effect_yetanothermagiclamp", QtWarningMsg)

static quintptr windowId(const void* window)
{
    return reinterpret_cast<quintptr>(window);
}

enum ShapeCurve {
    Linear = 0,
    Quad = 1,
    Cubic = 2,
    Quart = 3,
    Quint = 4,
    Sine = 5,
    Circ = 6,
    Bounce = 7,
    Bezier = 8
};

YetAnotherMagicLampEffect::YetAnotherMagicLampEffect()
{
    m_outputManager = std::make_unique<CoordinateMapper::OutputManager>(KWin::effects);
    m_geometryResolver = std::make_unique<CoordinateMapper::GeometryResolver>(m_outputManager.get());
    m_windowTracker = std::make_unique<CoordinateMapper::WindowTracker>();

    reconfigure(ReconfigureAll);

    for (KWin::EffectWindow* w : KWin::effects->stackingOrder()) {
        slotWindowAdded(w);
    }

    connect(KWin::effects, &KWin::EffectsHandler::windowAdded,
        this, &YetAnotherMagicLampEffect::slotWindowAdded);
    connect(KWin::effects, &KWin::EffectsHandler::windowClosed,
        this, &YetAnotherMagicLampEffect::slotWindowClosed);
    connect(KWin::effects, &KWin::EffectsHandler::windowDeleted,
        this, &YetAnotherMagicLampEffect::slotWindowDeleted);
    connect(KWin::effects, &KWin::EffectsHandler::activeFullScreenEffectChanged,
        this, &YetAnotherMagicLampEffect::slotActiveFullScreenEffectChanged);
}

YetAnotherMagicLampEffect::~YetAnotherMagicLampEffect()
{
}

void YetAnotherMagicLampEffect::reconfigure(ReconfigureFlags flags)
{
    Q_UNUSED(flags)

    YetAnotherMagicLampConfig::self()->read();

    QEasingCurve curve;
    const auto shapeCurve = static_cast<ShapeCurve>(YetAnotherMagicLampConfig::shapeCurve());
    switch (shapeCurve) {
    case ShapeCurve::Linear:
        curve.setType(QEasingCurve::Linear);
        break;

    case ShapeCurve::Quad:
        curve.setType(QEasingCurve::InOutQuad);
        break;

    case ShapeCurve::Cubic:
        curve.setType(QEasingCurve::InOutCubic);
        break;

    case ShapeCurve::Quart:
        curve.setType(QEasingCurve::InOutQuart);
        break;

    case ShapeCurve::Quint:
        curve.setType(QEasingCurve::InOutQuint);
        break;

    case ShapeCurve::Sine:
        curve.setType(QEasingCurve::InOutSine);
        break;

    case ShapeCurve::Circ:
        curve.setType(QEasingCurve::InOutCirc);
        break;

    case ShapeCurve::Bounce:
        curve.setType(QEasingCurve::InOutBounce);
        break;

    case ShapeCurve::Bezier:
        // With the cubic bezier curve, "0" corresponds to the furtherst edge
        // of a window, "1" corresponds to the closest edge.
        curve.setType(QEasingCurve::BezierSpline);
        curve.addCubicBezierSegment(
            QPointF(0.3, 0.0),
            QPointF(0.7, 1.0),
            QPointF(1.0, 1.0));
        break;
    default:
        // Fallback to the sine curve.
        curve.setType(QEasingCurve::InOutSine);
        break;
    }
    m_modelParameters.shapeCurve = curve;

    const auto baseDuration = KWinCompat::toMilliseconds(
        animationTime<YetAnotherMagicLampConfig>(std::chrono::milliseconds(300)));
    const auto configuredSquashDuration = YetAnotherMagicLampConfig::squashDuration();
    m_modelParameters.squashDuration = configuredSquashDuration > 0
        ? std::chrono::milliseconds(configuredSquashDuration)
        : baseDuration;
    const auto configuredStretchDuration = YetAnotherMagicLampConfig::stretchDuration();
    m_modelParameters.stretchDuration = configuredStretchDuration > 0
        ? std::chrono::milliseconds(configuredStretchDuration)
        : std::chrono::milliseconds(qMax(qRound(baseDuration.count() * 0.7), 1));
    const auto configuredUnminimizeStretchDuration = YetAnotherMagicLampConfig::unminimizeStretchDuration();
    m_modelParameters.unminimizeStretchDuration = configuredUnminimizeStretchDuration > 0
        ? std::chrono::milliseconds(configuredUnminimizeStretchDuration)
        : m_modelParameters.stretchDuration;
    const auto configuredBumpDuration = YetAnotherMagicLampConfig::bumpDuration();
    m_modelParameters.bumpDuration = configuredBumpDuration > 0
        ? std::chrono::milliseconds(configuredBumpDuration)
        : baseDuration;
    m_modelParameters.shapeFactor = YetAnotherMagicLampConfig::initialShapeFactor();
    m_modelParameters.bumpDistance = YetAnotherMagicLampConfig::maxBumpDistance();

    m_gridResolution = YetAnotherMagicLampConfig::gridResolution();
}

#ifdef KWIN_PREPAINT_HAS_PRESENT_TIME
void YetAnotherMagicLampEffect::prePaintScreen(KWin::ScreenPrePaintData& data, std::chrono::milliseconds presentTime)
{
    for (AnimationData& animData : m_animations) {
        animData.model.advance(presentTime);
    }

    data.mask |= PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS;

    KWinCompat::prePaintScreen(KWin::effects, data, presentTime);
}

void YetAnotherMagicLampEffect::prePaintWindow(KWin::RenderView* view, KWin::EffectWindow* w, KWin::WindowPrePaintData& data, std::chrono::milliseconds presentTime)
{
    if (m_animations.contains(w)) {
        data.mask |= PAINT_WINDOW_TRANSFORMED;
    }
    KWinCompat::prePaintWindow(KWin::effects, view, w, data, presentTime);
}
#else
void YetAnotherMagicLampEffect::prePaintScreen(KWin::ScreenPrePaintData& data)
{
    const auto presentTime = KWinCompat::monotonicTimestamp();
    for (AnimationData& animData : m_animations) {
        animData.model.advance(presentTime);
    }

    data.mask |= PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS;

    KWinCompat::prePaintScreen(KWin::effects, data, presentTime);
}

void YetAnotherMagicLampEffect::prePaintWindow(KWin::RenderView* view, KWin::EffectWindow* w, KWin::WindowPrePaintData& data)
{
    if (m_animations.contains(w)) {
        data.mask |= PAINT_WINDOW_TRANSFORMED;
    }
    KWinCompat::prePaintWindow(KWin::effects, view, w, data, KWinCompat::monotonicTimestamp());
}
#endif

void YetAnotherMagicLampEffect::postPaintScreen()
{
    for (auto it = m_animations.begin(); it != m_animations.end();) {
        if (it->model.done()) {
            unredirect(it.key());
            it = m_animations.erase(it);
        } else {
            ++it;
        }
    }

    KWin::effects->addRepaintFull();
    KWin::effects->postPaintScreen();
}

YetAnotherMagicLampEffect::PaintWindowResult YetAnotherMagicLampEffect::paintWindow(const KWin::RenderTarget& renderTarget,
    const KWin::RenderViewport& viewport,
    KWin::EffectWindow* w, int mask,
    const KWin::Region& region,
    KWin::WindowPaintData& data)
{
    auto it = m_animations.constFind(w);
    if (it != m_animations.constEnd() && it->model.needsClip()) {
        const KWin::Region clipLogical(it->model.clipRegion());
        const KWin::Region clip = viewport.mapToDeviceCoordinatesAligned(clipLogical);
        return KWin::effects->paintWindow(renderTarget, viewport, w, mask, clip, data);
    }

    return KWin::effects->paintWindow(renderTarget, viewport, w, mask, region, data);
}

void YetAnotherMagicLampEffect::apply(KWin::EffectWindow* window, int mask, KWin::WindowPaintData& data, KWin::WindowQuadList& quads)
{
    Q_UNUSED(mask)
    Q_UNUSED(data)

    auto it = m_animations.constFind(window);
    if (it == m_animations.constEnd()) {
        return;
    }

    quads = quads.makeGrid(m_gridResolution);
    (*it).model.apply(quads);
}

bool YetAnotherMagicLampEffect::isActive() const
{
    return !m_animations.isEmpty();
}

bool YetAnotherMagicLampEffect::supported()
{
    if (!KWin::effects->animationsSupported()) {
        return false;
    }
    if (KWin::effects->isOpenGLCompositing()) {
        return true;
    }
    return false;
}

// Bookkeeping snapshot for debugging leaks, logged on lifecycle events only
// (never from the paint hooks): every pending entry and every animation holds
// one visible reference.
void YetAnotherMagicLampEffect::logState(const char* event) const
{
    qCDebug(lcMagicLamp).noquote() << "state"
                                   << "event=" << event
                                   << "pending=" << m_pendingMinimize.size()
                                   << "animations=" << m_animations.size()
                                   << "retryTimers=" << m_retryTimersInFlight;
}

void YetAnotherMagicLampEffect::slotWindowAdded(KWin::EffectWindow* w)
{
    if (!m_windowTracker->trackWindow(w)) {
        return;
    }

    connect(w, &KWin::EffectWindow::minimizedChanged,
        this, &YetAnotherMagicLampEffect::slotMinimizedChanged);
    logState("window-added");
}

void YetAnotherMagicLampEffect::slotMinimizedChanged(KWin::EffectWindow* w)
{
    if (w->isMinimized()) {
        startMinimize(w);
    } else {
        startUnminimize(w);
    }
}

void YetAnotherMagicLampEffect::startMinimize(KWin::EffectWindow* w)
{
    if (KWin::effects->activeFullScreenEffect()) {
        return;
    }

    const auto target = m_geometryResolver->resolve(w);
    if (!target) {
        // On Wayland the Plasma taskbar publishes the button geometry
        // asynchronously (after QML layout). Keep the window visible and retry
        // until it arrives; scheduleMinimizeRetry() bounds the wait.
        if (!m_pendingMinimize.contains(w)) {
            PendingMinimize& pending = m_pendingMinimize[w];
            pending.visibleRef = KWin::EffectWindowVisibleRef(w, KWin::EffectWindow::PAINT_DISABLED_BY_MINIMIZE);
            pending.generation = ++m_pendingGeneration;
            scheduleMinimizeRetry(w, pending.generation);
        }
        return;
    }

    animateMinimize(w, *target);
}

// Starts the minimize animation towards @p target, which the caller resolved;
// it is not resolved again here, so the target that was checked is the one
// that is used.
void YetAnotherMagicLampEffect::animateMinimize(KWin::EffectWindow* w, const QRectF& target)
{
    AnimationData& animData = m_animations[w];
    animData.model.setWindow(w);
    animData.model.setParameters(m_modelParameters);
    qCDebug(lcMagicLamp).noquote() << "minimize.target"
                                   << "window=" << windowId(w)
                                   << "target=" << target;
    animData.model.setIconGeometry(target);
    animData.model.start(Model::AnimationKind::Minimize);
    animData.visibleRef = KWin::EffectWindowVisibleRef(w, KWin::EffectWindow::PAINT_DISABLED_BY_MINIMIZE);
    // Released only now, so the window is never left unreferenced for a frame.
    m_pendingMinimize.remove(w);

    redirect(w);

    KWin::effects->addRepaintFull();
    logState("minimize-started");
}

// Plasma publishes a button geometry from a 200 ms task-init timer and lays the
// task manager out again 500 ms after a change, so a real answer comes within
// well under a second. Some windows never get one: with two or more task
// managers loaded, a window that joins an existing group is not published at
// all, and an output without a task manager has nothing to publish. Retrying
// forever would keep such a window painted while minimized, so after the last
// attempt the pending entry is dropped, which releases the visible reference
// and lets KWin hide the window without an animation.
//
// How long to wait adapts to whether Plasma can still be expected to publish:
// only a window that was added or changed output within the last second can
// still get a button; for any other window one more check is enough.
static constexpr int s_iconGeometryRetryIntervalMs = 150;
static constexpr int s_iconGeometryMaxRetries = 6;
static constexpr int s_settledRetries = 1;
static constexpr qint64 s_taskButtonsSettleMs = 1000;

void YetAnotherMagicLampEffect::scheduleMinimizeRetry(KWin::EffectWindow* w, quint64 generation)
{
    QPointer<KWin::EffectWindow> weakRef(w);
    ++m_retryTimersInFlight;
    QTimer::singleShot(s_iconGeometryRetryIntervalMs, this, [this, weakRef, generation]() {
        --m_retryTimersInFlight;
        KWin::EffectWindow* win = weakRef.data();
        if (!win) {
            return; // deleted; slotWindowDeleted() already dropped the entry
        }
        auto it = m_pendingMinimize.find(win);
        if (it == m_pendingMinimize.end() || it->generation != generation) {
            return; // unminimized, cancelled by a full-screen effect, or superseded
        }
        if (!win->isMinimized()) {
            m_pendingMinimize.erase(it);
            return;
        }
        if (KWin::effects->activeFullScreenEffect()) {
            // slotActiveFullScreenEffectChanged() normally got here first.
            m_pendingMinimize.erase(it);
            return;
        }
        ++it->attempts;
        // Resolved once and handed on as is: resolving again in
        // startMinimize() could come back empty and leave this entry, and the
        // visible reference it holds, behind with no timer to release it.
        const auto target = m_geometryResolver->resolve(win);
        if (target) {
            qCDebug(lcMagicLamp).noquote() << "minimize.icon-geometry-arrived"
                                           << "window=" << windowId(win)
                                           << "attempts=" << it->attempts;
            animateMinimize(win, *target);
            return;
        }
        const bool settled = m_windowTracker->taskButtonsSettled(win, s_taskButtonsSettleMs);
        if (it->attempts >= s_iconGeometryMaxRetries
            || (settled && it->attempts >= s_settledRetries)) {
            qCDebug(lcMagicLamp).noquote() << "minimize.no-icon-geometry"
                                           << "window=" << windowId(win)
                                           << "attempts=" << it->attempts
                                           << "settled=" << settled
                                           << "action=hide-without-animation";
            m_pendingMinimize.erase(it);
            logState("wait-ended");
            return;
        }
        qCDebug(lcMagicLamp).noquote() << "minimize.waiting-for-icon-geometry"
                                       << "window=" << windowId(win)
                                       << "attempts=" << it->attempts
                                       << "settled=" << settled;
        scheduleMinimizeRetry(win, generation);
    });
}

void YetAnotherMagicLampEffect::startUnminimize(KWin::EffectWindow* w)
{
    // Restored before the minimize could start: nothing left to wait for.
    m_pendingMinimize.remove(w);
    logState("restore");

    if (KWin::effects->activeFullScreenEffect()) {
        return;
    }

    // Restored while the minimize is still playing: run it backwards towards
    // the target it already has (Model::start() reverses a running timeline).
    auto running = m_animations.find(w);
    if (running != m_animations.end() && !running->model.done()) {
        running->model.start(Model::AnimationKind::Unminimize);
        KWin::effects->addRepaintFull();
        return;
    }

    // No button on the window's output: restore the normal way, unanimated.
    const auto target = m_geometryResolver->resolve(w);
    if (!target) {
        return;
    }

    AnimationData& animData = m_animations[w];
    animData.model.setWindow(w);
    animData.model.setParameters(m_modelParameters);
    animData.model.setIconGeometry(*target);
    animData.model.start(Model::AnimationKind::Unminimize);

    redirect(w);

    KWin::effects->addRepaintFull();
}

void YetAnotherMagicLampEffect::slotWindowClosed(KWin::EffectWindow* w)
{
    // A window closed while waiting for its button must not stay painted.
    m_pendingMinimize.remove(w);
    logState("window-closed");
}

void YetAnotherMagicLampEffect::slotWindowDeleted(KWin::EffectWindow* w)
{
    m_windowTracker->untrackWindow(w);
    m_pendingMinimize.remove(w);
    m_animations.remove(w);
    logState("window-deleted");
}

void YetAnotherMagicLampEffect::slotActiveFullScreenEffectChanged()
{
    if (KWin::effects->activeFullScreenEffect() != nullptr) {
        // Dropping the entries releases their visible references; their
        // timers find no entry and end.
        qCDebug(lcMagicLamp).noquote() << "minimize.cancelled"
                                       << "reason=full-screen-effect"
                                       << "pending=" << m_pendingMinimize.size();
        m_pendingMinimize.clear();
        for (auto it = m_animations.constBegin(); it != m_animations.constEnd(); ++it) {
            unredirect(it.key());
        }
        m_animations.clear();
        logState("full-screen-effect");
    }
}
