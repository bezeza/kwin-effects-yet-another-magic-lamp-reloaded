/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 */

// Own
#include "WindowTracker.h"

// KWin
#include <core/output.h>

namespace CoordinateMapper {

WindowTracker::WindowTracker(QObject* parent)
    : QObject(parent)
{
}

bool WindowTracker::trackWindow(KWin::EffectWindow* window)
{
    if (!window || m_entries.contains(window)) {
        return false;
    }

    Entry entry;
    entry.sinceAdded.start();
    entry.sinceOutputChange.start();
    entry.output = window->screen();

    m_entries.insert(window, entry);
    auto it = m_entries.find(window);
    it->geometryConnection = connect(window,
        &KWin::EffectWindow::windowFrameGeometryChanged,
        this,
        [this](KWin::EffectWindow* changedWindow) {
            updateWindowOutput(changedWindow);
        });
    it->destroyedConnection = connect(window, &QObject::destroyed, this, [this, window]() {
        untrackWindow(window);
    });

    return true;
}

void WindowTracker::untrackWindow(KWin::EffectWindow* window)
{
    auto it = m_entries.find(window);
    if (it == m_entries.end()) {
        return;
    }

    QObject::disconnect(it->geometryConnection);
    QObject::disconnect(it->destroyedConnection);
    m_entries.erase(it);
}

bool WindowTracker::isTracked(KWin::EffectWindow* window) const
{
    return window && m_entries.contains(window);
}

bool WindowTracker::taskButtonsSettled(KWin::EffectWindow* window, qint64 settleMs) const
{
    const auto it = m_entries.constFind(window);
    if (it == m_entries.constEnd()) {
        // If this effect did not observe the window being added, there is no
        // reliable event timestamp from which to infer that Plasma is pending.
        return true;
    }

    return hasSettled(it->sinceAdded.elapsed(), it->sinceOutputChange.elapsed(), settleMs);
}

bool WindowTracker::hasSettled(qint64 sinceAddedMs, qint64 sinceOutputChangeMs, qint64 settleMs)
{
    return settleMs >= 0
        && sinceAddedMs >= settleMs
        && sinceOutputChangeMs >= settleMs;
}

void WindowTracker::updateWindowOutput(KWin::EffectWindow* window)
{
    if (!window) {
        return;
    }

    auto it = m_entries.find(window);
    if (it == m_entries.end()) {
        return;
    }

    const KWin::LogicalOutput* output = window->screen();
    if (it->output != output) {
        it->output = output;
        it->sinceOutputChange.start();
    }
}

} // namespace CoordinateMapper
