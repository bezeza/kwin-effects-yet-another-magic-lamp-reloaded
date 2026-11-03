/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 */

#pragma once

// KWin
#include <effect/effectwindow.h>

// Qt
#include <QElapsedTimer>
#include <QHash>
#include <QMetaObject>
#include <QObject>

namespace CoordinateMapper {

// Tracks the time windows have spent on their current output. Plasma task
// managers may publish a new task button after a window is added or moves to
// another output; consumers can use this bounded settling window when retrying
// geometry resolution.
class WindowTracker final : public QObject
{
public:
    explicit WindowTracker(QObject* parent = nullptr);
    ~WindowTracker() override = default;

    // Returns true only when a new window was registered.
    bool trackWindow(KWin::EffectWindow* window);
    void untrackWindow(KWin::EffectWindow* window);

    bool isTracked(KWin::EffectWindow* window) const;
    bool taskButtonsSettled(KWin::EffectWindow* window, qint64 settleMs) const;

    // Pure threshold logic, exposed for deterministic unit tests.
    static bool hasSettled(qint64 sinceAddedMs, qint64 sinceOutputChangeMs, qint64 settleMs);

private:
    struct Entry {
        QElapsedTimer sinceAdded;
        QElapsedTimer sinceOutputChange;
        const KWin::LogicalOutput* output = nullptr;
        QMetaObject::Connection geometryConnection;
        QMetaObject::Connection destroyedConnection;
    };

    void updateWindowOutput(KWin::EffectWindow* window);

    QHash<KWin::EffectWindow*, Entry> m_entries;
};

} // namespace CoordinateMapper
