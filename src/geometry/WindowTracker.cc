/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 */

// Own
#include "WindowTracker.h"
#include "OutputManager.h"

// KWin
#include <core/output.h>
#include <effect/effectwindow.h>
#include <window.h>

namespace CoordinateMapper {

WindowTracker::WindowTracker(OutputManager* outputs, OutputChangeCallback outputChangeCallback, QObject* parent)
    : QObject(parent)
    , m_outputChangeCallback(outputChangeCallback)
{
    if (!outputs) {
        return;
    }

    connect(outputs, &OutputManager::outputAboutToBeRemoved,
        this, &WindowTracker::handleOutputAboutToBeRemoved);
    connect(outputs, &OutputManager::outputsChanged,
        this, &WindowTracker::resetAllWindows);
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

    if (KWin::Window* internalWindow = window->window()) {
        it->outputConnection = connect(internalWindow, &KWin::Window::outputChanged,
            this, [this, window](KWin::LogicalOutput*) {
                updateWindowOutput(window);
            });
    }

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
    QObject::disconnect(it->outputConnection);
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

void WindowTracker::updateWindowOutput(KWin::EffectWindow* window)
{
    if (!window) {
        return;
    }

    auto it = m_entries.find(window);
    if (it == m_entries.end()) {
        return;
    }

    KWin::LogicalOutput* output = window->screen();
    if (outputIdentityChanged(it->output.data(), output)) {
        it->output = output;
        it->sinceOutputChange.start();
        if (m_outputChangeCallback) {
            m_outputChangeCallback(window);
        }
    }
}

void WindowTracker::handleOutputAboutToBeRemoved(KWin::LogicalOutput* output)
{
    if (!output) {
        return;
    }

    for (auto it = m_entries.begin(); it != m_entries.end(); ++it) {
        if (it->output.data() == output) {
            // Do not retain an output identity after KWin has announced its removal.
            // The subsequent Window::outputChanged signal will record the replacement.
            it->output = nullptr;
            it->sinceOutputChange.start();
        }
    }
}

void WindowTracker::resetAllWindows()
{
    // Any output addition, removal, mode, scale, transform, or layout change
    // can make Plasma relayout task managers on more than the changed output.
    for (auto it = m_entries.begin(); it != m_entries.end(); ++it) {
        it->sinceOutputChange.start();
    }
}

} // namespace CoordinateMapper
