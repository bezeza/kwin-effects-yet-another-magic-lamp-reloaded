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

#pragma once

// Own
#include "Model.h"
#include "common.h"

// kwineffects
#include <effect/offscreeneffect.h>

// Qt
#include <QElapsedTimer>
#include <QHash>
#include <QPointer>

struct AnimationData {
    Model model;
    KWin::EffectWindowVisibleRef visibleRef;
};

// A minimized window kept painted while it waits for its task-button geometry.
struct PendingMinimize {
    KWin::EffectWindowVisibleRef visibleRef;
    int attempts = 0;
    // Identifies the retry chain that owns this entry, so a timer left over
    // from an earlier minimize of the same window cannot drive a new one.
    quint64 generation = 0;
};

// When a window last gave Plasma a reason to publish a task button: it was
// added, or it moved to another output (a task manager that shows only the
// current screen gets a new delegate then).
struct WindowHistory {
    QElapsedTimer sinceAdded;
    QElapsedTimer sinceOutputChange;
    const KWin::LogicalOutput* output = nullptr;
};

class YetAnotherMagicLampEffect : public KWin::OffscreenEffect {
    Q_OBJECT

public:
    YetAnotherMagicLampEffect();
    ~YetAnotherMagicLampEffect() override;

    void reconfigure(ReconfigureFlags flags) override;

    // KWin 6.7 removed the presentation timestamp from the pre-paint hook
    // signatures. The CMake probe KWIN_PREPAINT_HAS_PRESENT_TIME picks the
    // signature that matches the installed KWin so `override` stays correct.
#ifdef KWIN_PREPAINT_HAS_PRESENT_TIME
    void prePaintScreen(KWin::ScreenPrePaintData& data, std::chrono::milliseconds presentTime) override;
    void prePaintWindow(KWin::RenderView* view, KWin::EffectWindow* w, KWin::WindowPrePaintData& data, std::chrono::milliseconds presentTime) override;
#else
    void prePaintScreen(KWin::ScreenPrePaintData& data) override;
    void prePaintWindow(KWin::RenderView* view, KWin::EffectWindow* w, KWin::WindowPrePaintData& data) override;
#endif
    void postPaintScreen() override;

    // KWin 6.8 made paintWindow() return bool. The CMake probe
    // KWIN_PAINTWINDOW_RETURNS_BOOL picks the matching return type; the body
    // returns the next effect's result either way (returning a void
    // expression from a void function is valid C++).
#ifdef KWIN_PAINTWINDOW_RETURNS_BOOL
    using PaintWindowResult = bool;
#else
    using PaintWindowResult = void;
#endif
    PaintWindowResult paintWindow(const KWin::RenderTarget& renderTarget, const KWin::RenderViewport& viewport,
        KWin::EffectWindow* w, int mask, const KWin::Region& region,
        KWin::WindowPaintData& data) override;

    bool isActive() const override;
    int requestedEffectChainPosition() const override;

    static bool supported();

protected:
    void apply(KWin::EffectWindow* window, int mask, KWin::WindowPaintData& data, KWin::WindowQuadList& quads) override;

private Q_SLOTS:
    void slotWindowAdded(KWin::EffectWindow* w);
    void slotMinimizedChanged(KWin::EffectWindow* w);
    void slotWindowClosed(KWin::EffectWindow* w);
    void slotWindowDeleted(KWin::EffectWindow* w);
    void slotActiveFullScreenEffectChanged();

private:
    void startMinimize(KWin::EffectWindow* w);
    void animateMinimize(KWin::EffectWindow* w, const QRectF& target);
    void startUnminimize(KWin::EffectWindow* w);
    void scheduleMinimizeRetry(KWin::EffectWindow* w, quint64 generation);
    bool taskButtonsSettled(KWin::EffectWindow* w) const;
    void logState(const char* event) const;

    Model::Parameters m_modelParameters;
    int m_gridResolution;

    QMap<KWin::EffectWindow*, AnimationData> m_animations;
    // Windows waiting for iconGeometry to become valid before animation starts
    QMap<KWin::EffectWindow*, PendingMinimize> m_pendingMinimize;
    quint64 m_pendingGeneration = 0;
    int m_retryTimersInFlight = 0;
    QHash<KWin::EffectWindow*, WindowHistory> m_history;
};

inline int YetAnotherMagicLampEffect::requestedEffectChainPosition() const
{
    return 50;
}
